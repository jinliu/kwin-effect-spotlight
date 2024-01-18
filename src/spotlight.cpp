/*
    SPDX-FileCopyrightText: 2024 Jin Liu <m.liu.jin@gmail.com>

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

const int TEXTURE_PADDING = 10;

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
        updateMaxScale();
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

    const int imageSize = (m_spotlightRadius + TEXTURE_PADDING) * 2;
    QImage image(imageSize, imageSize, QImage::Format_ARGB32);
    QPainter painter(&image);

    painter.setRenderHint(QPainter::RenderHint::Antialiasing);
    painter.fillRect(image.rect(), QColor(0, 0, 0));

    QBrush brush;
    brush.setStyle(Qt::SolidPattern);
    brush.setColor(QColor(255, 255, 255));
    painter.setBrush(brush);
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
            updateMaxScale();
            qreal start = 1.0;
            if (m_outAnimation.state() == QVariantAnimation::Running) {
                start = m_outAnimation.currentValue().toReal();
                m_outAnimation.stop();
            }
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

    QPointF center = cursorPos();

    if (screen != effects->screenAt(center.toPoint()))
        return;

    QRectF screenGeometry = screen->geometry();
    center -= screenGeometry.topLeft();

    qreal scale = 1 / (m_animationValue * m_maxScale + 1 - m_animationValue);
    QRectF source = QRectF(-center.x() * scale + TEXTURE_PADDING + m_spotlightRadius,
                           -center.y() * scale + TEXTURE_PADDING + m_spotlightRadius,
                           screenGeometry.width() * scale,
                           screenGeometry.height() * scale);
    QRectF fullscreen = viewport.renderRect();
    fullscreen.setSize(fullscreen.size() * viewport.scale());

    auto shader = ShaderManager::instance()->pushShader(ShaderTrait::MapTexture | ShaderTrait::TransformColorspace);
    shader->setColorspaceUniformsFromSRGB(renderTarget.colorDescription());
    QMatrix4x4 mvp = viewport.projectionMatrix();
    shader->setUniform(GLShader::ModelViewProjectionMatrix, mvp);

    const bool clipping = region != infiniteRegion();
    const QRegion clipRegion = clipping ? viewport.mapToRenderTarget(region) : infiniteRegion();

    qDebug() << "SpotlightEffect::paintScreen() screenGeometry" << screenGeometry << "center" << center << "scale" << 1/scale << "source" << source << "fullscreen" << fullscreen << "clipping" << clipping << "clipRegion" << clipRegion;

    if (clipping) {
        glEnable(GL_SCISSOR_TEST);
    }
    glEnable(GL_BLEND);
    glBlendColor(0.0f, 0.0f, 0.0f, 0.5f * (1.0f - m_animationValue));
    glBlendFunc(GL_CONSTANT_ALPHA, GL_ONE_MINUS_CONSTANT_ALPHA);
    m_spotlightTexture->render(source, clipRegion, fullscreen.size(), clipping);
    glDisable(GL_BLEND);
    if (clipping) {
        glDisable(GL_SCISSOR_TEST);
    }

    ShaderManager::instance()->popShader();
}

void SpotlightEffect::updateMaxScale()
{
    QPointF center = cursorPos();
    QRectF screenGeometry = effects->screenAt(center.toPoint())->geometry();
    center -= screenGeometry.topLeft();
    qreal x = qMax(center.x(), screenGeometry.width() - center.x());
    qreal y = qMax(center.y(), screenGeometry.height() - center.y());
    qreal furthestDistanceToScreenCorner = qSqrt(x * x + y * y);
    m_maxScale = furthestDistanceToScreenCorner / m_spotlightRadius;
}

} // namespace KWin

#include "moc_spotlight.cpp"
