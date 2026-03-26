// wifi-dbm — unit tests
//
// Tests the core parsing logic by injecting mock /proc/net/wireless content
// via temp files. No external test framework required.
//
// Build:
//   g++ -std=c++17 -o test_wifi_signal test_wifi_signal.cpp && ./test_wifi_signal
//
// All tests print PASS/FAIL and exit with 0 (all pass) or 1 (any fail).

#include <cassert>
#include <cstdio>
#include <fstream>
#include <functional>
#include <iostream>
#include <sstream>
#include <string>
#include <vector>

// ---------------------------------------------------------------------------
// Testable variant of getWifiSignalDbm — identical logic, but reads from
// a caller-supplied path instead of the hardcoded /proc/net/wireless.
// ---------------------------------------------------------------------------
static int parseWifiSignalDbm(const std::string& iface, const std::string& procPath) {
    std::ifstream f(procPath);
    if (!f.is_open())
        return 254;

    std::string line;
    std::getline(f, line); // skip header line 1
    std::getline(f, line); // skip header line 2

    while (std::getline(f, line)) {
        if (line.find(iface) == std::string::npos) continue;

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

// ---------------------------------------------------------------------------
// Minimal test harness
// ---------------------------------------------------------------------------
struct TestResult { std::string name; bool passed; std::string message; };

static std::vector<TestResult> g_results;

static void check(const std::string& name, bool condition, const std::string& msg = "") {
    g_results.push_back({name, condition, msg});
    std::cout << (condition ? "[PASS] " : "[FAIL] ") << name;
    if (!condition && !msg.empty()) std::cout << "  →  " << msg;
    std::cout << "\n";
}

// Write content to a temp file, return its path.
static std::string makeTempFile(const std::string& content) {
    char path[] = "/tmp/wifi_test_XXXXXX";
    int fd = mkstemp(path);
    if (fd < 0) { std::perror("mkstemp"); std::exit(1); }
    std::ofstream f(path);
    f << content;
    return std::string(path);
}

// Standard /proc/net/wireless header lines
static const std::string HEADER =
    "Inter-| sta-|   Quality        |   Discarded packets               | Missed | WE\n"
    " face | tus | link level noise |  nwid  crypt   frag  retry   misc | beacon | 22\n";

// ---------------------------------------------------------------------------
// Tests
// ---------------------------------------------------------------------------

// 1. Typical good signal
static void test_valid_signal() {
    // -52 dBm → should return 52
    std::string content = HEADER +
        " wlp2s0: 0000   58.  -52.  -95.  0  0  0  0  0  0\n";
    auto path = makeTempFile(content);
    int result = parseWifiSignalDbm("wlp2s0", path);
    std::remove(path.c_str());
    check("valid signal -52 dBm returns 52", result == 52,
          "got " + std::to_string(result));
}

// 2. Weak but still valid signal (-99 dBm)
static void test_weak_signal() {
    std::string content = HEADER +
        " wlp2s0: 0000    2.  -99.  -95.  0  0  0  0  0  0\n";
    auto path = makeTempFile(content);
    int result = parseWifiSignalDbm("wlp2s0", path);
    std::remove(path.c_str());
    check("weak signal -99 dBm returns 99", result == 99,
          "got " + std::to_string(result));
}

// 3. Strong signal (-20 dBm — close range)
static void test_strong_signal() {
    std::string content = HEADER +
        " wlp2s0: 0000   70.  -20.  -95.  0  0  0  0  0  0\n";
    auto path = makeTempFile(content);
    int result = parseWifiSignalDbm("wlp2s0", path);
    std::remove(path.c_str());
    check("strong signal -20 dBm returns 20", result == 20,
          "got " + std::to_string(result));
}

// 4. Boundary: exactly -1 dBm (minimum valid absolute value)
static void test_boundary_min_dbm() {
    std::string content = HEADER +
        " wlp2s0: 0000   70.   -1.  -95.  0  0  0  0  0  0\n";
    auto path = makeTempFile(content);
    int result = parseWifiSignalDbm("wlp2s0", path);
    std::remove(path.c_str());
    check("boundary -1 dBm returns 1", result == 1,
          "got " + std::to_string(result));
}

// 5. Boundary: -110 dBm (maximum sensible absolute value)
static void test_boundary_max_dbm() {
    std::string content = HEADER +
        " wlp2s0: 0000    0. -110.  -95.  0  0  0  0  0  0\n";
    auto path = makeTempFile(content);
    int result = parseWifiSignalDbm("wlp2s0", path);
    std::remove(path.c_str());
    check("boundary -110 dBm returns 110", result == 110,
          "got " + std::to_string(result));
}

// 6. Implausible: signal is 0 dBm → dbm == 0 → rejected
static void test_implausible_zero() {
    std::string content = HEADER +
        " wlp2s0: 0000   70.    0.  -95.  0  0  0  0  0  0\n";
    auto path = makeTempFile(content);
    int result = parseWifiSignalDbm("wlp2s0", path);
    std::remove(path.c_str());
    check("implausible 0 dBm returns 252", result == 252,
          "got " + std::to_string(result));
}

// 7. Implausible: absurdly large value (> 200)
static void test_implausible_large() {
    std::string content = HEADER +
        " wlp2s0: 0000   70. -201.  -95.  0  0  0  0  0  0\n";
    auto path = makeTempFile(content);
    int result = parseWifiSignalDbm("wlp2s0", path);
    std::remove(path.c_str());
    check("implausible -201 dBm returns 252", result == 252,
          "got " + std::to_string(result));
}

// 8. Error 255: interface name not present in file
static void test_interface_not_found() {
    std::string content = HEADER +
        " wlp2s0: 0000   58.  -52.  -95.  0  0  0  0  0  0\n";
    auto path = makeTempFile(content);
    int result = parseWifiSignalDbm("wlan0", path);  // wrong interface
    std::remove(path.c_str());
    check("wrong interface name returns 255", result == 255,
          "got " + std::to_string(result));
}

// 9. Error 254: file does not exist
static void test_file_not_found() {
    int result = parseWifiSignalDbm("wlp2s0", "/tmp/wifi_nonexistent_file_xyzzy");
    check("missing file returns 254", result == 254,
          "got " + std::to_string(result));
}

// 10. Error 253: file exists but data line is malformed (missing fields)
static void test_parse_failure() {
    std::string content = HEADER +
        " wlp2s0: GARBAGE DATA NO NUMBERS HERE\n";
    auto path = makeTempFile(content);
    int result = parseWifiSignalDbm("wlp2s0", path);
    std::remove(path.c_str());
    check("malformed data line returns 253", result == 253,
          "got " + std::to_string(result));
}

// 11. Multiple interfaces — correct one is picked
static void test_multiple_interfaces() {
    std::string content = HEADER +
        " eth0:   0000   70.    0.  -95.  0  0  0  0  0  0\n"
        " wlan0:  0000   40.  -70.  -95.  0  0  0  0  0  0\n"
        " wlp2s0: 0000   58.  -52.  -95.  0  0  0  0  0  0\n";
    auto path = makeTempFile(content);
    int result = parseWifiSignalDbm("wlp2s0", path);
    std::remove(path.c_str());
    check("correct interface selected from multi-interface file", result == 52,
          "got " + std::to_string(result));
}

// 12. Result is always in valid range (1–110) or an error code (251–255)
static void test_return_value_contract() {
    std::string content = HEADER +
        " wlp2s0: 0000   58.  -67.  -95.  0  0  0  0  0  0\n";
    auto path = makeTempFile(content);
    int result = parseWifiSignalDbm("wlp2s0", path);
    std::remove(path.c_str());
    bool valid = (result >= 1 && result <= 110) || (result >= 251 && result <= 255);
    check("return value is within defined contract range", valid,
          "got " + std::to_string(result));
}

// 13. Fractional dBm truncates correctly (-52.9 → 52, not 53)
static void test_float_truncation() {
    std::string content = HEADER +
        " wlp2s0: 0000   58.  -52.9  -95.  0  0  0  0  0  0\n";
    auto path = makeTempFile(content);
    int result = parseWifiSignalDbm("wlp2s0", path);
    std::remove(path.c_str());
    check("fractional -52.9 dBm truncates to 52", result == 52,
          "got " + std::to_string(result));
}

// 14. Empty file (only headers, no interface lines)
static void test_empty_interfaces() {
    auto path = makeTempFile(HEADER);
    int result = parseWifiSignalDbm("wlp2s0", path);
    std::remove(path.c_str());
    check("file with headers only returns 255", result == 255,
          "got " + std::to_string(result));
}

// 15. Completely empty file
static void test_completely_empty_file() {
    auto path = makeTempFile("");
    int result = parseWifiSignalDbm("wlp2s0", path);
    std::remove(path.c_str());
    // No header lines → getline returns empty → no data → 255
    check("completely empty file returns 255", result == 255,
          "got " + std::to_string(result));
}

// ---------------------------------------------------------------------------
// Integration test: reads real /proc/net/wireless (if available)
// ---------------------------------------------------------------------------
static void test_integration_real_proc() {
    std::ifstream f("/proc/net/wireless");
    if (!f.is_open()) {
        std::cout << "[SKIP] integration test — /proc/net/wireless not available\n";
        return;
    }
    f.close();

    // We don't know the interface name, so just verify the file is parseable
    // by checking that a bogus interface returns 255 (not a crash or 253/254).
    int result = parseWifiSignalDbm("__no_such_iface__", "/proc/net/wireless");
    check("integration: real /proc/net/wireless readable (bogus iface → 255)",
          result == 255, "got " + std::to_string(result));
}

// ---------------------------------------------------------------------------
// main
// ---------------------------------------------------------------------------
int main() {
    std::cout << "=== wifi-dbm tests ===\n\n";

    test_valid_signal();
    test_weak_signal();
    test_strong_signal();
    test_boundary_min_dbm();
    test_boundary_max_dbm();
    test_implausible_zero();
    test_implausible_large();
    test_interface_not_found();
    test_file_not_found();
    test_parse_failure();
    test_multiple_interfaces();
    test_return_value_contract();
    test_float_truncation();
    test_empty_interfaces();
    test_completely_empty_file();
    test_integration_real_proc();

    int passed = 0, failed = 0;
    for (auto& r : g_results) { r.passed ? ++passed : ++failed; }

    std::cout << "\n--- " << passed << " passed, " << failed << " failed ---\n";
    return failed == 0 ? 0 : 1;
}
