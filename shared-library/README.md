# wifi-dbm — shared library (.so)

Compile once as a shared library, link from any C++ project without recompiling.

## Build & install

```bash
# Build the shared library
g++ -std=c++17 -shared -fPIC -o libwifidbm.so wifi_signal.cpp

# Install system-wide
sudo cp libwifidbm.so /usr/local/lib/
sudo cp wifi_signal.h /usr/local/include/
sudo ldconfig
```

## Link from your project

```bash
g++ -std=c++17 -o myapp myapp.cpp -lwifidbm
```

## Example caller

```cpp
#include <wifi_signal.h>
#include <iostream>

int main() {
    int dbm = getWifiSignalDbm("wlp2s0");
    std::cout << "Signal: " << dbm << std::endl;
}
```

## Uninstall

```bash
sudo rm /usr/local/lib/libwifidbm.so
sudo rm /usr/local/include/wifi_signal.h
sudo ldconfig
```
