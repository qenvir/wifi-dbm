# wifi-dbm — standalone binary

Single executable. Build once, install to `/usr/bin/`, call from anywhere.

## Build & install

```bash
g++ -std=c++17 -o wifi-dbm wifi_signal.cpp
sudo cp wifi-dbm /usr/bin/wifi-dbm
sudo chmod +x /usr/bin/wifi-dbm
```

## Usage

```bash
wifi-dbm              # default interface: wlp2s0
wifi-dbm wlan0        # specify interface
wifi-dbm wlp2s0
```

## Call from a shell script

```bash
SIGNAL=$(wifi-dbm wlp2s0)
echo "Signal: $SIGNAL dBm (negated)"
```

## Call from another C++ program via popen

```cpp
FILE* pipe = popen("wifi-dbm wlp2s0", "r");
int dbm = 251; // default to error
if (pipe) {
    fscanf(pipe, "%d", &dbm);
    pclose(pipe);
}
```

> Note: if you want to avoid popen entirely, use the header-only or shared-library version instead.
