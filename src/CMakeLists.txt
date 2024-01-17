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
