/*
    KWin - the KDE window manager
    This file is part of the KDE project.

    SPDX-FileCopyrightText: 2007 Rivo Laks <rivolaks@hot.ee>
    SPDX-FileCopyrightText: 2010 Jorge Mata <matamax123@gmail.com>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#pragma once

#include <kcmodule.h>

#include "ui_spotlight_config.h"

namespace KWin
{
class SpotlightEffectConfig : public KCModule
{
    Q_OBJECT
public:
    explicit SpotlightEffectConfig(QObject *parent, const KPluginMetaData &data);
    ~SpotlightEffectConfig() override;

public Q_SLOTS:
    void save() override;
    void load() override;
    void defaults() override;

private:
    Ui::SpotlightEffectConfigForm m_ui;
};

} // namespace
