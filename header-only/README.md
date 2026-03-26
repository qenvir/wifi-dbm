# wifi-dbm — header-only

Single `.h` file. Drop it into your project and `#include` it — nothing to compile or install separately.

## Usage

Copy `wifi_signal.h` anywhere in your project include path, then:

```cpp
#include "wifi_signal.h"

int dbm = getWifiSignalDbm("wlp2s0");
```

## Compile your project normally

```bash
g++ -std=c++17 -o myapp myapp.cpp
```

No `-l` flags, no `.so`, no install step.

## Best for

- ROS / catkin packages — drop into `include/` folder
- Embedded Linux projects
- Any project where you want zero external dependencies
