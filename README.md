# Tobelsoft Macro ⚡

Tobelsoft Macro is a modern, high-performance Gaming Macro Tool built with Python and PyQt6. It features a sleek Cyberpunk/Dark UI and powerful hotkey management capabilities.

## ✨ Features

- **Advanced Hotkey Management**: 
  - Support for Keyboard and Mouse triggers.
  - Multi-key combos (e.g., `Ctrl+Shift+X`).
  - "Block Original Input" capability to suppress keys from other apps.
- **Dynamic Actions**:
  - Map triggers to key sequences or specific actions.
  - "Repeat While Held" functionality with adjustable delay.
- **Global Control**:
  - **Master Toggle**: Set global hotkeys (e.g., `F1`) to instantly Enable/Disable all macros.
  - Works even when the app is minimized or inactive.
- **Modern UI**:
  - Custom "Gaming" theme with glassmorphism effects.
  - Interactive setting pages and visualizations.
- **Persistence**:
  - Auto-saving configuration.
  - Import/Export functionality for sharing macro profiles.

## 🛠️ Installation

1.  **Clone the repository**:
    ```bash
    git clone https://github.com/fiko942/auto-macro.git
    cd tobelsoft-macro
    ```

2.  **Install dependencies**:
    ```bash
    pip install -r requirements.txt
    ```

3.  **Run the application**:
    ```bash
    python main.py
    ```

## 🎮 Usage

1.  **Adding a Macro**:
    - Click `+ Add Hotkey` on the dashboard.
    - Set your **Trigger** (press any key/mouse button).
    - Add **Actions** (keys to simulate when triggered).
    - Configure options like *Repeat* or *Block Input*.
    - Click `Save`.

2.  **Global Toggle**:
    - Go to `Settings` (Gear icon).
    - Under **Keymap Configuration**, add a key (e.g., `F1`).
    - Now you can press `F1` anywhere to toggle the macro system ON/OFF.

3.  **Management**:
    - Use the Toggle Switch on each item to enable/disable specific macros.
    - Use `Stop` / `Start` button in the top right for manual control.

## 📦 Requirements

- Python 3.8+
- Windows OS (Required for `win32` input blocking features)

## 📝 License

Proprietary / Custom License.
Created by **Tobelsoft**.
