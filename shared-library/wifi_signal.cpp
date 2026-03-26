// wifi-dbm — shared library implementation
//
// Build:
//   g++ -std=c++17 -shared -fPIC -o libwifidbm.so wifi_signal.cpp
//
// Install:
//   sudo cp libwifidbm.so /usr/local/lib/
//   sudo cp wifi_signal.h /usr/local/include/
//   sudo ldconfig
//
// License: MIT — https://github.com/YOUR_USERNAME/wifi-dbm

#include "wifi_signal.h"
#include <fstream>
#include <sstream>

int getWifiSignalDbm(const std::string& iface) {

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
