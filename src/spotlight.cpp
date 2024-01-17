/*
    SPDX-FileCopyrightText: 2023 Vlad Zahorodnii <vlad.zahorodnii@kde.org>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "spotlight.h"
#include "spotlightconfig.h"

#include "core/rendertarget.h"
#include "core/renderviewport.h"
#include "cursor.h"
#include "effect/effecthandler.h"
#include "input_event.h"
#include "opengl/gltexture.h"
#include "opengl/glutils.h"

#include <QPainter>

namespace KWin
{

SpotlightEffect::SpotlightEffect()
{
    input()->installInputEventSpy(this);

    m_inAnimation.setEasingCurve(QEasingCurve::InOutCubic);
    connect(&m_inAnimation, &QVariantAnimation::valueChanged, this, [this]() {
        m_animationValue = m_inAnimation.currentValue().toReal();
        effects->addRepaintFull();
    });
    connect(&m_inAnimation, &QVariantAnimation::finished, this, [this]() {
        m_effectStopTimer.start(m_effectTimeout);
    });

    m_effectStopTimer.setSingleShot(true);
    connect(&m_effectStopTimer, &QTimer::timeout, this, [this]() {
        m_isActive = false;
        m_screenRadius = -1;
        m_outAnimation.setStartValue(0.0);
        m_outAnimation.setEndValue(1.0);
        m_outAnimation.setDuration(m_animationTime);
        m_outAnimation.start();
        qDebug() << "SpotlightEffect::deactivated";
    });

    m_outAnimation.setEasingCurve(QEasingCurve::InOutCubic);
    connect(&m_outAnimation, &QVariantAnimation::valueChanged, this, [this]() {
        m_animationValue = m_outAnimation.currentValue().toReal();
        effects->addRepaintFull();
    });

    SpotlightConfig::instance(effects->config());
    reconfigure(ReconfigureAll);
}

SpotlightEffect::~SpotlightEffect()
{
}

bool SpotlightEffect::supported()
{
    qDebug() << "SpotlightEffect::supported()" << effects->isOpenGLCompositing();
    return effects->isOpenGLCompositing();
}

void SpotlightEffect::reconfigure(ReconfigureFlags flags)
{
    qDebug() << "SpotlightEffect::reconfigure()";
    SpotlightConfig::self()->read();

    m_animationTime = animationTime(SpotlightConfig::animationTime());
    m_effectTimeout = SpotlightConfig::effectTimeout();
    m_spotlightRadius = SpotlightConfig::spotlightRadius();

    m_shakeDetector.setInterval(SpotlightConfig::timeInterval());
    m_shakeDetector.setSensitivity(SpotlightConfig::sensitivity());

    const int imageSize = m_spotlightRadius * 4;
    QImage image(imageSize, imageSize, QImage::Format_RGB32);
    QPainter painter(&image);
    painter.setRenderHint(QPainter::RenderHint::Antialiasing);

    painter.fillRect(image.rect(), QColor(0, 0, 0));

    QBrush brush;
    brush.setStyle(Qt::SolidPattern);
    brush.setColor(QColor(255, 255, 255));
    painter.setBrush(brush);
    painter.setCompositionMode(QPainter::CompositionMode_Source);
    painter.drawEllipse(image.rect().center(), m_spotlightRadius, m_spotlightRadius);

    m_spotlightTexture.reset();
    m_spotlightTexture = GLTexture::upload(image);
    m_spotlightTexture->setWrapMode(GL_CLAMP_TO_EDGE);
    m_spotlightTexture->setFilter(GL_LINEAR);
    qDebug() << "SpotlightEffect::reconfigure() success";
}

bool SpotlightEffect::isActive() const
{
    return m_isActive || m_outAnimation.state() == QVariantAnimation::Running;
}

void SpotlightEffect::pointerEvent(MouseEvent *event)
{
    if (event->type() != QEvent::MouseMove) {
        return;
    }

    if (const auto shakeFactor = m_shakeDetector.update(event)) {
        if (!m_isActive) {
            qDebug() << "SpotlightEffect::activated";
            m_isActive = true;
            qreal start = 1.0;
            if (m_outAnimation.state() == QVariantAnimation::Running) {
                start = m_outAnimation.currentValue().toReal();
                m_outAnimation.stop();
            }
            m_screenRadius = -1;
            m_inAnimation.setStartValue(start);
            m_inAnimation.setEndValue(0.0);
            m_inAnimation.setDuration(m_animationTime * start);
            m_inAnimation.start();
        } else if (m_effectStopTimer.isActive()) {
            m_effectStopTimer.start(m_effectTimeout);
        }
    }

    if (m_isActive)
        effects->addRepaintFull();
}

void SpotlightEffect::paintScreen(const RenderTarget &renderTarget, const RenderViewport &viewport, int mask, const QRegion &region, Output *screen)
{
    effects->paintScreen(renderTarget, viewport, mask, region, screen);

    QPoint center = cursorPos().toPoint();

    if (screen != effects->screenAt(center))
        return;

    if (m_screenRadius < 0) {
        int x = qMax(center.x(), screen->geometry().width() - center.x());
        int y = qMax(center.y(), screen->geometry().height() - center.y());
        m_screenRadius = qSqrt(x * x + y * y);
    }

    const auto rect = viewport.renderRect().toRect();
    const bool clipping = region != infiniteRegion();
    const QRegion clipRegion = clipping ? viewport.mapToRenderTarget(region) : infiniteRegion();
    if (clipping) {
        glEnable(GL_SCISSOR_TEST);
    }

    int r = circleRadius();
    float alpha = 0.5f * (1.0f - m_animationValue);
    glEnable(GL_BLEND);
    glBlendColor(0.0f, 0.0f, 0.0f, alpha);
    glBlendFunc(GL_CONSTANT_ALPHA, GL_ONE_MINUS_CONSTANT_ALPHA);
    auto shader = ShaderManager::instance()->pushShader(ShaderTrait::MapTexture | ShaderTrait::TransformColorspace);
    shader->setColorspaceUniformsFromSRGB(renderTarget.colorDescription());
    QMatrix4x4 mvp = viewport.projectionMatrix();
    // mvp.scale(double(r) / m_spotlightRadius);
    mvp.translate((circleX() - r * 2) * viewport.scale(), (circleY() - r * 2) * viewport.scale());
    shader->setUniform(GLShader::ModelViewProjectionMatrix, mvp);
    QSizeF size(r * 4, r * 4);
    size *= viewport.scale();
    m_spotlightTexture->render(clipRegion, size, clipping);
    ShaderManager::instance()->popShader();
    glDisable(GL_BLEND);

    if (clipping) {
        glDisable(GL_SCISSOR_TEST);
    }
}

int SpotlightEffect::circleX() const
{
    return cursorPos().x();
}

int SpotlightEffect::circleY() const
{
    return cursorPos().y();
}

int SpotlightEffect::circleRadius() const
{
    return m_spotlightRadius + (m_screenRadius - m_spotlightRadius) * m_animationValue;
}

} // namespace KWin

#include "moc_spotlight.cpp"
