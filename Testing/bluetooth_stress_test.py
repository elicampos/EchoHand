#!/usr/bin/env python3
import serial, serial.tools.list_ports, time, random, string

BAUD, PACKETS, TARGET_DIST, MAX_LOSS = 38400, 100, 5.0, 5.0

def test_loss(ser):
    sent, recv = 0, 0
    for _ in range(PACKETS):
        pkt = ''.join(random.choices(string.ascii_letters, k=32))
        try:
            ser.write((pkt + '\n').encode())
            ser.flush()
            sent += 1
            if ser.readline(): recv += 1
        except: pass
    return sent, recv, ((sent-recv)/sent*100) if sent else 100

def main():
    print("=== EchoHand Bluetooth Stress Test ===\n")
    
    print("Available ports:")
    for p in serial.tools.list_ports.comports():
        print(f"  {p.device}: {p.description}")
    
    port = input("\nEnter COM port: ").strip()
    if not port.upper().startswith("COM"): port = "COM" + port
    
    try:
        ser = serial.Serial(port, BAUD, timeout=2)
        time.sleep(1)
        ser.reset_input_buffer()
        print("Connected!\n")
    except Exception as e:
        print(f"Failed: {e}")
        return
    
    print("Enter distance (m) after each move. Type 'done' to finish.\n")
    
    results = []
    passed = False
    
    while True:
        inp = input("Distance (m): ").strip().lower()
        if inp == 'done': break
        try: dist = float(inp)
        except: continue
        
        sent, recv, loss = test_loss(ser)
        print(f"[{dist}m] Sent: {sent}  Recv: {recv}  Loss: {loss:.1f}%")
        results.append((dist, loss))
        
        if loss > MAX_LOSS:
            print(f"\n!! Loss exceeded {MAX_LOSS}% at {dist}m\n")
            break
        if dist >= TARGET_DIST:
            passed = True
            print(f"\n{TARGET_DIST}m reached with {loss:.1f}% loss\n")
            break
    
    ser.close()
    
    print("--- RESULTS ---")
    if results:
        print(f"Max distance: {max(r[0] for r in results)}m")
        print(f"Final loss: {results[-1][1]:.1f}%")
    print(f"Status: {'PASS' if passed else 'FAIL'}")

if __name__ == "__main__":
    try: main()
    except KeyboardInterrupt: print("\nCancelled")
