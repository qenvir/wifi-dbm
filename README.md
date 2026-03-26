# wifi-dbm

Minimal C++ tool to read WiFi signal strength in dBm on Linux.  
Reads directly from `/proc/net/wireless` — no external dependencies, no subprocess, no `iwconfig`.

## Output

A single integer on stdout:

```
41
```

The value is the absolute dBm value (sign is implied negative).  
So `41` means `-41 dBm`.

## Error codes

Designed for embedded/robotics use where the result fits in one unsigned byte:

| Value | Meaning |
|-------|---------|
| 1–110 | Valid signal strength (`-1` to `-110` dBm) |
| 251   | Unexpected C++ exception |
| 252   | Implausible signal value (hardware or driver issue) |
| 253   | `/proc/net/wireless` parse failure (kernel format change) |
| 254   | `/proc/net/wireless` unreadable (no WiFi hardware, permissions) |
| 255   | Interface not found (wrong name, driver unloaded) |

Any value `>= 251` is an error. Any value `<= 110` is a valid signal.  
The range `111–250` is reserved for future use.

## Three versions

| Folder | Style | Best for |
|--------|-------|----------|
| [`standalone/`](standalone/) | Single binary, install to `/usr/bin/` | CLI use, scripting, shell |
| [`shared-library/`](shared-library/) | `.so` shared library + header | Large projects, avoid recompile |
| [`header-only/`](header-only/) | Single `.h` file, `inline` | ROS packages, embedded, simplest reuse |

## Signal quality reference

| dBm   | Quality     |
|-------|-------------|
| -30   | Exceptional |
| -50   | Good        |
| -70   | Fair        |
| -80   | Weak        |
| -100+ | Dead zone   |

## Tested on

- Ubuntu 18.04 / 20.04
- Raspberry Pi CM4 (interface: `wlan0`)
- MinisForum Core i9 Ubuntu (interface: `wlp2s0`)

## License

MIT
