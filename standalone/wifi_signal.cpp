// wifi-dbm — standalone binary version
// Reads WiFi signal strength directly from /proc/net/wireless
//
// Build:
//   g++ -std=c++17 -o wifi-dbm wifi_signal.cpp
//
// Install:
//   sudo cp wifi-dbm /usr/bin/wifi-dbm
//
// Usage:
//   wifi-dbm              (default: wlp2s0)
//   wifi-dbm wlan0
//
// Output: single integer, e.g. 41 means -41 dBm
//
// Error codes:
//   251 = unexpected C++ exception
//   252 = implausible signal value (hardware/driver issue)
//   253 = /proc/net/wireless parse failure
//   254 = /proc/net/wireless unreadable (no wifi hardware, permissions)
//   255 = interface not found (wrong name, driver unloaded)
//
// License: MIT — https://github.com/YOUR_USERNAME/wifi-dbm

#include <fstream>
#include <sstream>
#include <string>
#include <iostream>

int getWifiSignalDbm(const std::string& iface = "wlp2s0") {

    std::ifstream f("/proc/net/wireless");
    if (!f.is_open())
        return 254;

    std::string line;
    std::getline(f, line); // skip header line 1
    std::getline(f, line); // skip header line 2

    while (std::getline(f, line)) {
        if (line.find(iface) == std::string::npos) continue;

        // Format: "wlp2s0: 0000  58.  -52.  -256. ..."
        std::istringstream ss(line);
        std::string name;
        int status;
        float link, signal, noise;

        if (!(ss >> name >> status >> link >> signal >> noise))
            return 253;

        int dbm = static_cast<int>(-signal);

        if (dbm <= 0 || dbm > 200)
            return 252;

        return dbm;
    }

    return 255;
}

int main(int argc, char* argv[]) {

    std::string iface = "wlp2s0"; // default interface

    if (argc > 1)
        iface = argv[1];

    try {
        std::cout << getWifiSignalDbm(iface) << std::endl;
    } catch (...) {
        std::cout << 251 << std::endl;
        return 1;
    }

    return 0;
}
