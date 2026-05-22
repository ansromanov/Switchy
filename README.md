# Switchy

Switches keyboard layout with the Caps Lock key on Windows 10/11.

## Installation

Put `Switchy.exe` in the startup folder (press **Win+R**, type `shell:startup`).

To hide the language switcher pop-up, place a shortcut in that folder with `nopopup` as the argument instead of the exe itself.

> **Note:** For layout switching to work in programs running as administrator, Switchy must also run as administrator. Automate this via Task Scheduler.

## Usage

| Shortcut | Action |
|---|---|
| **CapsLock** | Switch keyboard layout |
| **Shift+CapsLock** | Toggle CapsLock state |
| **Alt+CapsLock** | Enable / disable Switchy |

## Tray icon

Switchy runs in the system tray (bottom-right corner).

- **Green** — enabled, CapsLock switches layout
- **Gray** — disabled, CapsLock behaves normally

Hover for status tooltip. Right-click → **Exit** to quit.

## Build

Open `Switchy.sln` in Visual Studio 2022, or:

```
msbuild /m /p:Configuration=Release Switchy.sln
```

Output: `x64/Release/Switchy.exe`
