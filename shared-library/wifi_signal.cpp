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

constexpr int ERR_IMPLAUSIBLE    = 252;
constexpr int ERR_PARSE_FAIL     = 253;
constexpr int ERR_FILE_UNREADABLE = 254;
constexpr int ERR_NOT_FOUND      = 255;
constexpr int MAX_DBM            = 200;

int getWifiSignalDbm(const std::string& iface) {

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
