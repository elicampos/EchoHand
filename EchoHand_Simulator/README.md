# EchoHand Simulation Guide

## 1. Environment Setup

Make sure you are running this on a windows machine as this is a windows only application.

### ESP-IDF Extension (VSCode)
1.  **Install the Extension**: Search for "ESP-IDF" in the VSCode Extensions marketplace and install it.
2.  **Setup / Configuration**:
    *   Open the setup wizard (Command Palette: `ESP-IDF: Configure ESP-IDF extension`).
    *   **CRITICAL**: Select version **v5.3.4**. This version is strictly required for compatibility with the Arduino library used in this project.
    *   Follow the prompts to install tools.
3.  **Target Selection**:
    *   Set the target device to **esp32s3**.
    *   (Command Palette: `ESP-IDF: Set Espressif Device Target` -> `esp32s3`).

### Wokwi Simulator Extension (VSCode)
1.  **Install the Extension**: Search for "Wokwi for VSCode".
2.  **License**: You will need a valid license. Follow the extension's prompts to get a license from the Wokwi website and activate it.

---

## 2. Running the Firmware Simulation

1.  **Build Firmware**:
    *   Use the ESP-IDF extension icons (usually at the bottom bar) or Command Palette `ESP-IDF: Build your project` to compile the firmware.
2.  **Start Simulation**:
    *   Open the `diagram.json` file (or open the Wokwi GUI).
    *   Start the simulation.
    *   **Note**: The Wokwi simulation acts as the TCP Server. It must be running *before* you start the desktop visualizer.

> [!IMPORTANT]
> **Focus Issue**: If the data stream from Wokwi stops or lags, make sure you **click back into the VSCode window** showing the simulation. Browsers/Electron apps sometimes throttle background tabs/windows.

---

## 3. Running the Desktop Simulator

The desktop application visualizes the hand data received from the Wokwi simulation.

1.  **Navigate to Directory**:
    Open a terminal and navigate to the simulator folder:
    ```powershell
    cd EchoHand_Simulator
    ```

2.  **Build Application**:
    Run `make` to compile the C++ application (ensure you have MinGW/Make installed):
    ```powershell
    make
    ```

3.  **Run Application**:
    Execute the binary from the `bin` folder:
    ```powershell
    .\bin\opengloves_sim.exe
    ```

---

## 4. Controls & Interaction

Once both the Simulation (Server) and Desktop App (Client) are running:

*   **Modify Payload**:
    *   **Potentiometers**: Adjust these in the Wokwi view to simulate **Flex Sensors** (controlling finger curl).
    *   **Buttons**: Click to simulate Controller Buttons (A, B, Trigger, etc.).
    *   **Joystick**: Drag to simulate joystick movement.

*   **Desktop App Controls**:
    *   `W`, `A`, `S`, `D`: Move Camera
    *   `Right Click + Drag`: Look Around
    *   `P`: Toggle Protocol Reference Overlay

---

## Important Notes

*   **Launch Order**: ALWAYS run the **Wokwi Simulation first**, then the **OpenGloves Simulator**. The simulator tries to connect to localhost on launch; if the server isn't there, it will fail.
*   **Missing Payloads**: If you don't see the hand moving, check that Wokwi is actually sending data (monitor the serial output in VSCode) and that the VSCode window is focused.
*   **Future Features**: 
    *   Support for servo feedback (computer -> esp32).
    *   Reading commands from the computer.
