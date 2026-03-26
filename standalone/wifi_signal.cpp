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
#include <iostream>
#include <sstream>
#include <string>

constexpr int ERR_EXCEPTION      = 251;
constexpr int ERR_IMPLAUSIBLE    = 252;
constexpr int ERR_PARSE_FAIL     = 253;
constexpr int ERR_FILE_UNREADABLE = 254;
constexpr int ERR_NOT_FOUND      = 255;
constexpr int MAX_DBM            = 200;

int getWifiSignalDbm(const std::string& iface = "wlp2s0") {

    std::ifstream proc_file("/proc/net/wireless");
    if (!proc_file.is_open()) {
        return ERR_FILE_UNREADABLE;
    }

    std::string line{};
    std::getline(proc_file, line); // skip header line 1
    std::getline(proc_file, line); // skip header line 2

    while (std::getline(proc_file, line)) {
        if (line.find(iface) == std::string::npos) {
            continue;
        }

        // Format: "wlp2s0: 0000  58.  -52.  -256. ..."
        std::istringstream stream(line);
        std::string name{};
        int status{0};
        float link{0.0F};
        float signal{0.0F};
        float noise{0.0F};

        if (!(stream >> name >> status >> link >> signal >> noise)) { // NOLINT(hicpp-signed-bitwise)
            return ERR_PARSE_FAIL;
        }

        int dbm = static_cast<int>(-signal);

        if (dbm <= 0 || dbm > MAX_DBM) {
            return ERR_IMPLAUSIBLE;
        }

        return dbm;
    }

    return ERR_NOT_FOUND;
}

int main(int argc, const char* const argv[]) {

    std::string iface = "wlp2s0";

    if (argc > 1) {
        iface = argv[1];
    }

    try {
        std::cout << getWifiSignalDbm(iface) << '\n';
    } catch (...) {
        std::cout << ERR_EXCEPTION << '\n';
        return 1;
    }

    return 0;
}
