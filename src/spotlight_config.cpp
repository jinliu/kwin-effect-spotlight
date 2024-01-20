/*
    KWin - the KDE window manager
    This file is part of the KDE project.

    SPDX-FileCopyrightText: 2007 Rivo Laks <rivolaks@hot.ee>
    SPDX-FileCopyrightText: 2010 Jorge Mata <matamax123@gmail.com>

    SPDX-License-Identifier: GPL-2.0-or-later
*/
#include "spotlight_config.h"

#include <config-kwin.h>

// KConfigSkeleton
#include "spotlightconfig.h"
#include <kwineffects_interface.h>

#include <KLocalizedString>
#include <KPluginFactory>

K_PLUGIN_CLASS(KWin::SpotlightEffectConfig)

namespace KWin
{

SpotlightEffectConfig::SpotlightEffectConfig(QObject *parent, const KPluginMetaData &data)
    : KCModule(parent, data)
{
    SpotlightConfig::instance(KWIN_CONFIG);
    m_ui.setupUi(widget());

    addConfig(SpotlightConfig::self(), widget());
}

SpotlightEffectConfig::~SpotlightEffectConfig()
{
}

void SpotlightEffectConfig::load()
{
    KCModule::load();
}

void SpotlightEffectConfig::save()
{
    KCModule::save();
    OrgKdeKwinEffectsInterface interface(QStringLiteral("org.kde.KWin"),
                                         QStringLiteral("/Effects"),
                                         QDBusConnection::sessionBus());
    interface.reconfigureEffect(QStringLiteral("spotlight"));
}

void SpotlightEffectConfig::defaults()
{
    KCModule::defaults();
}

} // namespace

#include "spotlight_config.moc"

#include "moc_spotlight_config.cpp"
