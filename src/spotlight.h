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

private:
    // configuration
    int m_animationTime;
    int m_effectTimeout;
    int m_spotlightRadius;

    bool m_isActive = false;

    QTimer m_effectStopTimer;
    QVariantAnimation m_inAnimation;
    QVariantAnimation m_outAnimation;
    qreal m_animationValue; // factor of the animation from screenRadius to the target (1.0 to 0.0)

    qreal m_furthestDistanceToScreenCorner; // distance from the pointer to the furthest corner of the screen

    std::unique_ptr<GLTexture> m_spotlightTexture;

    ShakeDetector m_shakeDetector;
};

} // namespace KWin
