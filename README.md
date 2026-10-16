# Switchy

Switchy is a small Windows tray app that uses **Caps Lock** to switch the
active keyboard layout. It is intended for Windows 10 and 11.

## What it does

| Shortcut | Action |
|---|---|
| **Caps Lock** | Switch keyboard layout |
| **Shift + Caps Lock** | Toggle the Caps Lock state |
| **Alt + Caps Lock** | Enable or disable Switchy |

When enabled, Switchy intercepts Caps Lock with a low-level keyboard hook.
The tray icon shows whether it is enabled: **On** means layout switching is
active; **Off** means Caps Lock behaves normally. Right-click the tray icon
and choose **Exit** to close the app.

By default, layout switching uses **Win + Space**. To suppress the Windows
language switcher pop-up, run Switchy with the `nopopup` argument; in that
mode it sends **Alt + Shift** instead. This uses the keyboard layout shortcut
configured in Windows.

## Install and start with Windows

1. Place `Switchy.exe` in a folder that will remain on your PC.
2. Press **Win + R**, enter `shell:startup`, and press **Enter**.
3. Create a shortcut to `Switchy.exe` in the Startup folder.

To use the no-pop-up mode, open the shortcut's **Properties** and add
`nopopup` after the executable path in **Target**, for example:

```text
"C:\Apps\Switchy\Switchy.exe" nopopup
```

Switchy uses a process-wide named mutex, so only one instance can run at a
time. If you need layout switching in applications running as administrator,
run Switchy at the same privilege level. You can configure that through Task
Scheduler.

## Build

Build with Visual Studio 2022 and the **Desktop development with C++**
workload, or from a Developer Command Prompt:

```bat
msbuild /m /p:Configuration=Release /p:Platform=x64 Switchy.sln
```

The expected executable path is `x64\Release\Switchy.exe`.
