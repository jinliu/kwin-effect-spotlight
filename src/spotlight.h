/*
    SPDX-FileCopyrightText: 2024 Jin Liu <m.liu.jin@gmail.com>

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
    void updateMaxScale();

    // configuration
    int m_animationTime;
    int m_effectTimeout;
    int m_spotlightRadius;

    bool m_isActive = false;

    QTimer m_effectStopTimer;
    QVariantAnimation m_inAnimation;
    QVariantAnimation m_outAnimation;
    qreal m_animationValue; // value of the animation, from 1.0 (largest circle) to 0.0 (smallest circle)
    qreal m_maxScale; // max scale of the spotlight texture, corrsponding to m_animationValue=1.0

    std::unique_ptr<GLTexture> m_spotlightTexture;

    ShakeDetector m_shakeDetector;
};

} // namespace KWin
