# kwin-effect-spotlight
"Find My Mouse" effect "Spotlight"
[Demo](https://jinliu.github.io/kwin-effect-spotlight/Screencast.webm)

## Requirements

KDE Plasma 6.0 (Wayland session)

## Installation

Extract the following package into `/usr/lib/qt6/plugins/`:
[spotlight-0.1.tar.gz](https://github.com/jinliu/kwin-effect-spotlight/releases/download/v0.1/spotlight-0.1.tar.gz)

## Build

```bash
cmake -B build -S . -DCMAKE_BUILD_TYPE=Release
cmake --build build
sudo cmake --install build
```

## Thanks

`ShakeDetector.{cpp,h}`: from KDE KWin project (https://invent.kde.org/plasma/kwin/)
