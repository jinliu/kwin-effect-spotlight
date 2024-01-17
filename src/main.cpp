/*
    SPDX-FileCopyrightText: 2024 Jin Liu <m.liu.jin@gmail.com>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "spotlight.h"

namespace KWin
{

KWIN_EFFECT_FACTORY_SUPPORTED(SpotlightEffect,
                              "metadata.json",
                              return SpotlightEffect::supported();)

} // namespace KWin

#include "main.moc"
