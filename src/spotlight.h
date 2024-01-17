/*
    SPDX-FileCopyrightText: 2023 Vlad Zahorodnii <vlad.zahorodnii@kde.org>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#pragma once

#include "shakedetector.h"

#include "effect/effect.h"
#include "effect/offscreenquickview.h"
#include "input_event_spy.h"

#include <QTimer>
#include <QVariantAnimation>

namespace KWin
{

class Cursor;
class GLTexture;

class SpotlightEffect : public Effect, public InputEventSpy
{
    Q_OBJECT

public:
    SpotlightEffect();
    ~SpotlightEffect() override;

    static bool supported();

    void reconfigure(ReconfigureFlags flags) override;
    void pointerEvent(MouseEvent *event) override;
    bool isActive() const override;
    void paintScreen(const RenderTarget &renderTarget, const RenderViewport &viewport, int mask, const QRegion &region, Output *screen) override;

    int circleX() const;
    int circleY() const;
    int circleRadius() const;

private:
    bool m_isActive = false;

    QTimer m_effectStopTimer;
    int m_effectTimeout;

    QVariantAnimation m_inAnimation;
    QVariantAnimation m_outAnimation;
    int m_animationTime;
    int m_spotlightRadius;
    int m_screenRadius; // distance from the pointer to the furthest corner of the screen
    qreal m_animationValue; // factor of the animation from screenRadius to the target (1.0 to 0.0)

    ShakeDetector m_shakeDetector;

    std::unique_ptr<GLTexture> m_spotlightTexture;
};

} // namespace KWin
