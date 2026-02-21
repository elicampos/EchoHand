#!/usr/bin/env python3
import serial, serial.tools.list_ports, time, re, sys, subprocess

BAUD_BT, BAUD_USB = 38400, 115200
FINGER_NAMES = ["Thumb", "Index", "Middle", "Ring", "Pinky"]
OPEN_THRESH, CLOSED_THRESH = 1000, 3000

def parse(line):
    m = re.search(r'A(-?\d+)B(-?\d+)C(-?\d+)D(-?\d+)E(-?\d+)F(-?\d+)G(-?\d+)(L?)(H?)(J?)(K?)', line)
    if not m: return None
    return {
        'fingers': [int(m.group(i)) for i in range(1,6)],
        'joy': (int(m.group(6)), int(m.group(7))),
        'trigger': m.group(8)=='L', 'joy_btn': m.group(9)=='H',
        'a_btn': m.group(10)=='J', 'b_btn': m.group(11)=='K'
    }

def read_state(ser, timeout=2.0):
    buf = ""
    start = time.time()
    while time.time() - start < timeout:
        if ser.in_waiting:
            buf += ser.read(ser.in_waiting).decode('utf-8', errors='ignore')
            while '\n' in buf:
                line, buf = buf.split('\n', 1)
                state = parse(line)
                if state: return state
        time.sleep(0.01)
    return None

def check_steamvr():
    try:
        r = subprocess.run(['tasklist', '/FI', 'IMAGENAME eq vrserver.exe'], capture_output=True, text=True, timeout=5)
        return 'vrserver.exe' in r.stdout
    except: return False

def test_gesture(ser, check_fn, duration=2.0):
    valid, total = 0, 0
    start = time.time()
    while time.time() - start < duration:
        state = read_state(ser, 0.2)
        if state:
            total += 1
            if check_fn(state): valid += 1
            f = state['fingers']
            sys.stdout.write(f"\r  Fingers: {f}  ")
            sys.stdout.flush()
    print()
    return total > 0 and (valid/total) >= 0.7

def main():
    print("\n" + "="*50)
    print("  ECHOHAND PAYLOAD INTEGRITY TEST")
    print("="*50 + "\n")
    
    if check_steamvr():
        print("WARNING: SteamVR is running! Close it first.")
        if input("Continue? (y/N): ").lower() != 'y': return
    else:
        print("OK: SteamVR not running")
    
    print("\nAvailable ports:")
    for p in serial.tools.list_ports.comports():
        print(f"  {p.device}: {p.description}")
    
    port = input("\nEnter COM port: ").strip()
    if not port.upper().startswith("COM"): port = "COM" + port
    is_bt = input("Bluetooth? (y/N): ").lower() == 'y'
    
    try:
        ser = serial.Serial(port, BAUD_BT if is_bt else BAUD_USB, timeout=1)
        time.sleep(0.5)
        ser.reset_input_buffer()
    except Exception as e:
        print(f"FAIL: {e}")
        return
    
    results = []
    
    print("\n--- Connection Test ---")
    state = read_state(ser)
    if state:
        print(f"OK: Receiving data")
        results.append(("Connection", True))
    else:
        print("FAIL: No data")
        results.append(("Connection", False))
        ser.close()
        return
    
    print("\n--- Data Range Test ---")
    samples = [read_state(ser, 0.5) for _ in range(30)]
    samples = [s for s in samples if s]
    if len(samples) >= 10:
        all_valid = all(0 <= v <= 4095 for s in samples for v in s['fingers'])
        for i, name in enumerate(FINGER_NAMES):
            vals = [s['fingers'][i] for s in samples]
            print(f"  {name}: {min(vals)} - {max(vals)}")
        print("OK: Ranges valid" if all_valid else "FAIL: Out of range")
        results.append(("Data Ranges", all_valid))
    
    print("\n--- Gesture Tests ---")
    gestures = [
        ("Open Hand", "extend all fingers", lambda s: all(f < OPEN_THRESH for f in s['fingers'])),
        ("Closed Fist", "make a fist", lambda s: all(f > CLOSED_THRESH for f in s['fingers'])),
        ("Thumbs Up", "thumb up only", lambda s: s['fingers'][0] < OPEN_THRESH and all(f > CLOSED_THRESH for f in s['fingers'][1:])),
        ("Point", "point with index", lambda s: s['fingers'][1] < OPEN_THRESH and s['fingers'][0] > CLOSED_THRESH and all(f > CLOSED_THRESH for f in s['fingers'][2:])),
    ]
    for name, instr, check in gestures:
        print(f"\n{name}: {instr}")
        input("Press ENTER when ready...")
        passed = test_gesture(ser, check)
        print(f"{'OK' if passed else 'FAIL'}: {name}")
        results.append((f"Gesture: {name}", passed))
    
    print("\n--- Button Tests ---")
    buttons = [("Trigger", "trigger", "pinch thumb+index"), ("A Button", "a_btn", "press A"), ("B Button", "b_btn", "press B")]
    for name, key, instr in buttons:
        print(f"\n{name}: {instr}")
        input("Press ENTER, then press button...")
        detected = False
        for _ in range(30):
            state = read_state(ser, 0.1)
            if state and state.get(key):
                detected = True
                break
        print(f"{'OK' if detected else 'FAIL'}: {name}")
        results.append((f"Button: {name}", detected))
    
    ser.close()
    
    print("\n" + "="*50)
    print("  RESULTS")
    print("="*50)
    passed = sum(1 for _, p in results if p)
    for name, p in results:
        print(f"  [{'PASS' if p else 'FAIL'}] {name}")
    print(f"\nTotal: {passed}/{len(results)}")
    print(f"\n{'PASSED' if passed == len(results) else 'FAILED'}")

if __name__ == "__main__":
    try: main()
    except KeyboardInterrupt: print("\nCancelled")
