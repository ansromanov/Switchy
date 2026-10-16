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
and choose **Pause** or **Resume** to control switching, or **Exit** to close
the app. The pause state lasts until Switchy exits; it starts enabled next time.

By default, layout switching uses **Win + Space**. To suppress the Windows
language switcher pop-up, run Switchy with the `nopopup` argument; in that
mode it sends **Alt + Shift** instead. This uses the keyboard layout shortcut
configured in Windows.

## Install and start with Windows

Download and extract `Switchy-windows-x64.zip` from the project's
[GitHub Releases page](https://github.com/ansromanov/Switchy/releases). In
PowerShell, run the included installer from the extracted folder:

```powershell
powershell -NoProfile -ExecutionPolicy Bypass -File .\Install-Switchy.ps1
```

The installer copies Switchy to your user programs folder and creates a
shortcut in your Windows Startup folder, so it starts when you sign in. To
suppress the language switcher pop-up, install with:

```powershell
powershell -NoProfile -ExecutionPolicy Bypass -File .\Install-Switchy.ps1 -NoPopup
```

To uninstall, run `Uninstall-Switchy.ps1` from the extracted folder.

To configure no-pop-up mode manually, add `nopopup` after the executable path
in the startup shortcut's **Target**, for example:

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

Pushing a version tag such as `v1.0.0` builds the x64 app, packages the
installer scripts, and publishes `Switchy-windows-x64.zip` as a GitHub Release
asset. The executable includes the Visual C++ runtime, so a separate runtime
install is not required.
