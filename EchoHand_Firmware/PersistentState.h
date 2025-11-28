#pragma once
#include <stdint.h>

// This will be used as the basic data structure for the state of the EchoHand for when other tasks need to access the data
struct EchoStateSnapshot {
  int fingerAngles[5]; 
  float servoTargetAngles[5];
  uint16_t vibrationRPMs[5];
  float joystickXY[2];
  uint32_t buttonsBitmask;
  uint8_t batteryPercent;
  uint32_t revision;
};

class PersistentState {
 public:

  // We will use a singleton instance of this class for global access. This method returns a reference to the singleton instance.
  // Use the get and set functions to access an individual field. Take a snapshot if you need to ensure a consistent view of the data, such as for the bluetooth service.

  static PersistentState& instance() {
    static PersistentState singleton;
    return singleton;
  }

  /* The get and set functions will handle the index checking and increment the revision number.The revision number 
     is used to detect if the data has changed since the last snapshot
     if the revision number has changed, the caller may call again
     The incrementRevision function is called by the set functions, get functions, and takeSnapshot function.
     */ 

  // Write APIs
  void setFingerAngle(uint8_t index, float angle) {
    if (index < 5) { fingerAngles_[index] = angle; incrementRevision(); }
  }
  void setServoTargetAngle(uint8_t index, float angle) {
    if (index < 5) { servoTargetAngles_[index] = angle; incrementRevision(); }
  }
  void setVibrationRPM(uint8_t index, uint16_t rpm) {
    if (index < 5) { vibrationRPMs_[index] = rpm; incrementRevision(); }
  }
  void setJoystick(float x, float y) {
    joystickXY_[0] = x; joystickXY_[1] = y; incrementRevision();
  }
  void setButtonsBitmask(uint32_t mask) { buttonsBitmask_ = mask; incrementRevision(); }
  void setBatteryPercent(uint8_t percent) { batteryPercent_ = percent; incrementRevision(); }

  // Read APIs ( for single values)
  int getFingerAngle(uint8_t index) const { return index < 5 ? fingerAngles_[index] : 0.0f; }
  float getServoTargetAngle(uint8_t index) const { return index < 5 ? servoTargetAngles_[index] : 0.0f; }
  uint16_t getVibrationRPM(uint8_t index) const { return index < 5 ? vibrationRPMs_[index] : 0; }
  void getJoystick(float& x, float& y) const { x = joystickXY_[0]; y = joystickXY_[1]; }
  uint32_t getButtonsBitmask() const { return buttonsBitmask_; }
  uint8_t getBatteryPercent() const { return batteryPercent_; }

  void takeSnapshot(EchoStateSnapshot& out) const {
    uint32_t before = revision_;
    out.fingerAngles[0] = fingerAngles_[0];
    out.fingerAngles[1] = fingerAngles_[1];
    out.fingerAngles[2] = fingerAngles_[2];
    out.fingerAngles[3] = fingerAngles_[3];
    out.fingerAngles[4] = fingerAngles_[4];
    out.servoTargetAngles[0] = servoTargetAngles_[0];
    out.servoTargetAngles[1] = servoTargetAngles_[1];
    out.servoTargetAngles[2] = servoTargetAngles_[2];
    out.servoTargetAngles[3] = servoTargetAngles_[3];
    out.servoTargetAngles[4] = servoTargetAngles_[4];
    out.vibrationRPMs[0] = vibrationRPMs_[0];
    out.vibrationRPMs[1] = vibrationRPMs_[1];
    out.vibrationRPMs[2] = vibrationRPMs_[2];
    out.vibrationRPMs[3] = vibrationRPMs_[3];
    out.vibrationRPMs[4] = vibrationRPMs_[4];
    out.joystickXY[0] = joystickXY_[0];
    out.joystickXY[1] = joystickXY_[1];
    out.buttonsBitmask = buttonsBitmask_;
    out.batteryPercent = batteryPercent_;
    out.revision = revision_;

    // If changed during read, caller may call again.
    (void)before;
  }

  uint32_t revision() const { return revision_; }

 private:
  PersistentState()
      : revision_(0),
        fingerAngles_{0, 0, 0, 0, 0},
        servoTargetAngles_{0, 0, 0, 0, 0},
        vibrationRPMs_{0, 0, 0, 0, 0},
        joystickXY_{0, 0},
        buttonsBitmask_(0),
        batteryPercent_(100) {}

  void incrementRevision() { revision_++; }

  volatile uint32_t revision_;
  volatile float fingerAngles_[5];
  volatile float servoTargetAngles_[5];
  volatile uint16_t vibrationRPMs_[5];
  volatile float joystickXY_[2];
  volatile uint32_t buttonsBitmask_;
  volatile uint8_t batteryPercent_;
};


