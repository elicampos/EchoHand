// InternalSystem.cpp
// Echo Hand - Internal Systems (Pre-Alpha Build)
//
// This file is part of the PC-side internal systems component for the Echo Hand project.
// It connects to the ESP32 via a serial port, requests snapshots of the glove state,
// and sends dummy servo target angles and vibration RPMs to the device.
//
// Communication Protocol (newline-delimited JSON):
//   PC → ESP32:
//       {"type":"get_snapshot"}
//       {"type":"set_targets","angles":[a0,a1,a2,a3,a4]}
//       {"type":"set_rpms","rpms":[r0,r1,r2,r3,r4]}
//   ESP32 → PC:
//       {"type":"snapshot", ...}
//
// Build example (Linux/macOS):
//   g++ -std=c++17 InternalSystem.cpp -o InternalSystem -lboost_system -lpthread
//
// Run example:
//   ./InternalSystem --port COM5 --baud 115200 --seconds 30

#include <boost/asio.hpp>
#include <chrono>
#include <csignal>
#include <iomanip>
#include <iostream>
#include <optional>
#include <sstream>
#include <string>
#include <thread>
#include <vector>
#include <atomic>
#include <cmath>

using namespace std;
using boost::asio::serial_port;
using boost::asio::io_context;
namespace chrono = std::chrono;

// Global variable used to stop the loop when Ctrl+C is pressed
static atomic<bool> g_run{true};

// Signal handler that flips the global flag when Ctrl+C is pressed
static void on_sigint(int) { g_run = false; }

// Configuration structure for the serial connection and runtime settings
struct Config {
  string port = "COM5";   // Serial port name (change to match your system)
  unsigned baud = 115200; // Baud rate
  double seconds = 20.0;  // How long to run the program
};

// ---------------- JSON Message Builders ----------------

// Builds a JSON command that sets the servo target angles
string json_set_targets(const vector<double>& a) {
  ostringstream os;
  os << "{\"type\":\"set_targets\",\"angles\":["
     << fixed << setprecision(6);
  for (size_t i = 0; i < a.size(); ++i) {
    if (i) os << ',';
    os << a[i];
  }
  os << "]}";
  return os.str();
}

// Builds a JSON command that sets vibration motor RPMs
string json_set_rpms(const vector<int>& r) {
  ostringstream os;
  os << "{\"type\":\"set_rpms\",\"rpms\":[";
  for (size_t i = 0; i < r.size(); ++i) {
    if (i) os << ',';
    os << r[i];
  }
  os << "]}";
  return os.str();
}

// Builds a JSON command that requests a snapshot from the ESP32
string json_get_snapshot() {
  return "{\"type\":\"get_snapshot\"}";
}

// ---------------- Serial Communication Functions ----------------

// Writes a JSON line to the serial port and logs it
void write_line(serial_port& sp, const string& line) {
  string out = line + "\n";
  boost::asio::write(sp, boost::asio::buffer(out.data(), out.size()));
  cout << "[tx] " << line << endl;
}

// Tries to read one complete line (terminated by '\n') from the serial port
optional<string> try_read_line(serial_port& sp, string& buf) {
  char temp[256];
  boost::system::error_code ec;
  size_t n = sp.read_some(boost::asio::buffer(temp, sizeof(temp)), ec);
  if (ec && ec != boost::asio::error::would_block) {
    // Ignore transient read errors
    return optional<string>();
  }

  if (n > 0) {
    buf.append(temp, temp + n);
    auto pos = buf.find('\n');
    if (pos != string::npos) {
      string line = buf.substr(0, pos);
      if (!line.empty() && line.back() == '\r') line.pop_back(); // remove carriage return if needed
      buf.erase(0, pos + 1);
      return line;
    }
  }
  return nullopt;
}

// ---------------- Main Program ----------------

int main(int argc, char** argv) {
  // Parse command-line arguments: --port, --baud, --seconds
  Config cfg;
  for (int i = 1; i < argc; ++i) {
    string a(argv[i]);
    if (a == "--port" && i + 1 < argc) cfg.port = argv[++i];
    else if (a == "--baud" && i + 1 < argc) cfg.baud = static_cast<unsigned>(stoul(argv[++i]));
    else if (a == "--seconds" && i + 1 < argc) cfg.seconds = stod(argv[++i]);
  }

  // Register signal handler for Ctrl+C
  signal(SIGINT, on_sigint);

  io_context io;
  serial_port sp(io);
  bool dry = false; // dry = true if serial connection fails

  // Try to open the serial port
  boost::system::error_code ec;
  sp.open(cfg.port, ec);
  if (ec) {
    cerr << "[probe] Could not open " << cfg.port << ": " << ec.message()
         << "\n[probe] Running in dry mode (no serial output)" << endl;
    dry = true;
  } else {
    // Configure serial connection
    sp.set_option(serial_port::baud_rate(cfg.baud));
    sp.set_option(serial_port::character_size(8));
    sp.set_option(serial_port::parity(serial_port::parity::none));
    sp.set_option(serial_port::stop_bits(serial_port::stop_bits::one));
    sp.set_option(serial_port::flow_control(serial_port::flow_control::none));
#if defined(_WIN32)
    sp.non_blocking(true);
#endif
    cout << "[probe] Connected to " << cfg.port << " @ " << cfg.baud << endl;
  }

  // Timing setup for periodic communication
  auto t0 = chrono::steady_clock::now();
  auto next_snap = t0;
  auto next_cmd = t0;
  string rxbuf;
  size_t tick = 0;

  // Main loop
  while (g_run) {
    auto now = chrono::steady_clock::now();
    double elapsed = chrono::duration<double>(now - t0).count();
    if (elapsed >= cfg.seconds) break;

    // Request a snapshot every 250 ms
    if (now >= next_snap) {
      if (!dry) write_line(sp, json_get_snapshot());
      else cout << "[tx] " << json_get_snapshot() << " (dry)" << endl;
      next_snap = now + chrono::milliseconds(250);
    }

    // Send dummy targets and RPMs every 1 s
    if (now >= next_cmd) {
      double base = 0.3 + 0.2 * sin(2.0 * M_PI * 0.5 * elapsed);
      vector<double> angles = {base, base + 0.05, base + 0.10, base + 0.05, base};
      vector<int> rpms = (tick % 2 == 0)
                             ? vector<int>{0, 120, 0, 120, 0}
                             : vector<int>{120, 0, 120, 0, 120};

      if (!dry) {
        write_line(sp, json_set_targets(angles));
        write_line(sp, json_set_rpms(rpms));
      } else {
        cout << "[tx] " << json_set_targets(angles) << " (dry)\n";
        cout << "[tx] " << json_set_rpms(rpms) << " (dry)\n";
      }

      tick++;
      next_cmd = now + chrono::seconds(1);
    }

    // Print any snapshot messages received from ESP32
    if (!dry) {
      auto line = try_read_line(sp, rxbuf);
      if (line.has_value()) {
        cout << "[rx] " << *line << endl;
      }
    }

    // Sleep a bit to prevent CPU overload
    this_thread::sleep_for(chrono::milliseconds(10));
  }

  // Close serial port when done
  if (!dry) {
    boost::system::error_code ec2;
    sp.close(ec2);
  }

  cout << "[probe] Done." << endl;
  return 0;
}
