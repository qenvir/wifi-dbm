// wifi-dbm — shared library header
// License: MIT — https://github.com/YOUR_USERNAME/wifi-dbm

#pragma once
#include <string>

// Returns absolute dBm value (e.g. 41 means -41 dBm)
//
// Error codes:
//   251 = unexpected C++ exception
//   252 = implausible signal value (hardware/driver issue)
//   253 = /proc/net/wireless parse failure
//   254 = /proc/net/wireless unreadable (no wifi hardware, permissions)
//   255 = interface not found (wrong name, driver unloaded)
int getWifiSignalDbm(const std::string& iface = "wlp2s0");
