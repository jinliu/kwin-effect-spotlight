kcoreaddons_add_plugin(spotlight INSTALL_NAMESPACE "kwin/effects/plugins/")
target_sources(spotlight PRIVATE
    spotlight.cpp
    shakedetector.cpp
    main.cpp
)
kconfig_add_kcfg_files(spotlight spotlightconfig.kcfgc)

target_link_libraries(spotlight
    KF6::ConfigCore
    KF6::ConfigGui
    KF6::CoreAddons
    KF6::I18n
    
    KWin::kwin
)

set(kwin_spotlight_config_SOURCES spotlight_config.cpp)
ki18n_wrap_ui(kwin_spotlight_config_SOURCES spotlight_config.ui)
qt_add_dbus_interface(kwin_spotlight_config_SOURCES ${KWIN_EFFECTS_INTERFACE} kwineffects_interface)

kcoreaddons_add_plugin(kwin_spotlight_config INSTALL_NAMESPACE "kwin/effects/configs" SOURCES ${kwin_spotlight_config_SOURCES})
kconfig_add_kcfg_files(kwin_spotlight_config spotlightconfig.kcfgc)
target_link_libraries(kwin_spotlight_config
    KF6::ConfigCore
    KF6::ConfigGui
    KF6::ConfigWidgets
    KF6::CoreAddons
    KF6::I18n
    KF6::KCMUtils
    KF6::XmlGui

    KWin::kwin
)