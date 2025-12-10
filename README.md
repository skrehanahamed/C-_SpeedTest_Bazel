# ⚡ Speed Test

A beautiful, modern internet speed test tool built with C++ and Bazel. Features both a CLI and a web-based GUI inspired by Ookla Speedtest.

![Speed Test GUI](https://img.shields.io/badge/C++-17-blue.svg)
![Bazel](https://img.shields.io/badge/Bazel-8.0+-green.svg)
![License](https://img.shields.io/badge/License-MIT-yellow.svg)

## ✨ Features

- 🖥️ **CLI Mode** - Terminal-based speed test with animated progress
- 🌐 **Web GUI** - Beautiful browser-based interface with a "GO" button
- 📊 **Detailed Results** - Download, Upload, Ping, and Jitter measurements
- 📍 **Server Info** - Shows IP address, hostname, and location
- 🎨 **Modern UI** - Dark theme with gradient colors and animations

## 📸 Screenshots

### Web GUI
```
![image](https://github.com/user-attachments/assets/dcdb644f-f5c4-4e34-be4b-c2ef650bd543)

```

### CLI Mode
```
  ![image](https://github.com/user-attachments/assets/f561cc98-3454-4875-8f49-bd26d207d669)

```

## 🚀 Quick Start

### Prerequisites

- C++17 compatible compiler (GCC 7+ or Clang 5+)
- [Bazel](https://bazel.build/install) build system

### Build & Run

**Web GUI (Recommended):**
```bash
# Build and run the web server
bazel run //speed_test:speed_test_gui

# Open http://localhost:8080 in your browser
# Click the GO button to start the test!
```

**CLI Mode:**
```bash
# Build and run the CLI version
bazel run //speed_test:speed_test
```

## 📁 Project Structure

```
speed_test/
├── BUILD.bazel      # Bazel build configuration
├── README.md        # This file
├── benchmark.h      # Speed test core library header
├── benchmark.cc     # Speed test core implementation
├── main.cc          # CLI entry point
└── server.cc        # Web GUI server
```

## 🛠️ Build Targets

| Target | Description |
|--------|-------------|
| `//speed_test:speed_test` | CLI speed test tool |
| `//speed_test:speed_test_gui` | Web-based GUI server |
| `//speed_test:benchmark_lib` | Core benchmark library |

## 🔧 Configuration

The web GUI runs on port **8080** by default. To change it, modify the port in `server.cc`:

```cpp
SpeedTestServer server(8080);  // Change port here
```

## 📖 API Endpoints (Web GUI)

| Endpoint | Description |
|----------|-------------|
| `GET /` | Main HTML page |
| `GET /api/info` | Server information (IP, hostname) |
| `GET /api/ping` | Ping and jitter test |
| `GET /api/download` | Download speed test |
| `GET /api/upload` | Upload speed test |

## 🤝 Contributing

1. Fork the repository
2. Create a feature branch (`git checkout -b feature/amazing-feature`)
3. Commit your changes (`git commit -m 'Add amazing feature'`)
4. Push to the branch (`git push origin feature/amazing-feature`)
5. Open a Pull Request

## 📄 License

This project is licensed under the MIT License - see the [LICENSE](../LICENSE) file for details.

## 🙏 Acknowledgments

- Inspired by [Ookla Speedtest](https://www.speedtest.net/)
- Built with [Bazel](https://bazel.build/)

---

Made with ❤️ and C++
