// InternalSystem.cpp
// Echo Hand - PC internal system simulator
//
// Purpose
// - Use PersistentState.h on the PC
// - Generate dummy finger angles, servo targets, RPMs, joystick, buttons, battery
// - Take and print snapshots on a fixed interval
//
// Build (Linux or macOS)
//   g++ -std=c++17 InternalSystem.cpp -o InternalSystem
//
// Build (Windows, MSVC)
//   cl /std:c++17 InternalSystem.cpp /Fe:InternalSystem.exe
//
// Run
//   ./InternalSystem

#include <iostream>
#include <thread>
#include <chrono>
#include <cmath>
#include "PersistentState.h"

using namespace std;

static void step_simulated_inputs(double t_sec) {
  // Simulate flex angles as smooth sines
  for (int i = 0; i < 5; ++i) {
    float base = 0.4f + 0.1f * sinf(static_cast<float>(t_sec) + i * 0.6f);
    PersistentState::instance().setFingerAngle((uint8_t)i, base);
  }

  // Simulate joystick and buttons
  float jx = 0.5f * sinf(2.0f * 3.1415926f * 0.5f * static_cast<float>(t_sec));
  float jy = 0.5f * cosf(2.0f * 3.1415926f * 0.5f * static_cast<float>(t_sec));
  PersistentState::instance().setJoystick(jx, jy);

  uint32_t buttons = (static_cast<int>(t_sec) % 2 == 0) ? 0b00101u : 0b10010u;
  PersistentState::instance().setButtonsBitmask(buttons);

  // Simulate battery drain
  uint8_t bat = static_cast<uint8_t>(max(0, 95 - static_cast<int>(t_sec)));
  PersistentState::instance().setBatteryPercent(bat);
}

static void step_host_commands(double t_sec) {
  // Simulate target angles and vib RPMs as if a host app sent them
  for (int i = 0; i < 5; ++i) {
    float target = 0.6f + 0.08f * cosf(static_cast<float>(t_sec) + i * 0.4f);
    PersistentState::instance().setServoTargetAngle((uint8_t)i, target);
  }
  bool on = (static_cast<int>(t_sec) % 2 == 0);
  for (int i = 0; i < 5; ++i) {
    PersistentState::instance().setVibrationRPM((uint8_t)i, on ? 120 : 0);
  }
}

static void print_snapshot(uint32_t idx) {
  EchoStateSnapshot s{};
  PersistentState::instance().takeSnapshot(s);

  cout << "\n[snapshot " << idx << "] rev=" << s.revision << "\n";
  cout << "angles: ";
  for (int i = 0; i < 5; ++i) {
    cout << s.fingerAngles[i] << (i < 4 ? ", " : "\n");
  }
  cout << "targets: ";
  for (int i = 0; i < 5; ++i) {
    cout << s.servoTargetAngles[i] << (i < 4 ? ", " : "\n");
  }
  cout << "rpms: ";
  for (int i = 0; i < 5; ++i) {
    cout << s.vibrationRPMs[i] << (i < 4 ? ", " : "\n");
  }
  cout << "joystick: " << s.joystickXY[0] << ", " << s.joystickXY[1] << "\n";
  cout << "buttons: " << s.buttonsBitmask << "  battery: " << (int)s.batteryPercent << "%\n";
}

int main() {
  cout << "[pc] InternalSystem simulator starting\n";

  const double run_seconds = 12.0;
  const int tick_ms = 100;         // 10 Hz update
  const int print_every_ticks = 5; // print every 0.5 s

  auto t0 = chrono::steady_clock::now();
  uint32_t snap_idx = 0;
  int tick = 0;

  while (true) {
    auto now = chrono::steady_clock::now();
    double t = chrono::duration<double>(now - t0).count();
    if (t >= run_seconds) break;

    step_simulated_inputs(t);
    step_host_commands(t);

    if (tick % print_every_ticks == 0) {
      print_snapshot(snap_idx++);
    }

    this_thread::sleep_for(chrono::milliseconds(tick_ms));
    tick++;
  }

  cout << "\n[pc] InternalSystem simulator complete\n";
  return 0;
}
