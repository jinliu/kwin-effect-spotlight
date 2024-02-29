# kwin-effect-spotlight
"Find My Mouse" effect "Spotlight"

## Requirements

KDE Plasma 6.0

## Installation

Extract the following package into `/usr/lib/qt6/plugins/`:

## Build

```bash
cmake -B build -S . -DCMAKE_BUILD_TYPE=Release
cmake --build build
sudo cmake --install build
```

## Thanks

`ShakeDetector.{cpp,h}`: from KDE KWin project (https://invent.kde.org/plasma/kwin/)
