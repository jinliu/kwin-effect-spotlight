/*
    SPDX-FileCopyrightText: 2024 Jin Liu <m.liu.jin@gmail.com>

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
