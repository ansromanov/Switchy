param(
	[switch]$NoPopup
)

$ErrorActionPreference = 'Stop'

$packageExe = Join-Path $PSScriptRoot 'Switchy.exe'
if (-not (Test-Path -LiteralPath $packageExe -PathType Leaf)) {
	throw "Switchy.exe was not found next to this installer: $packageExe"
}

$installDirectory = Join-Path $env:LOCALAPPDATA 'Programs\Switchy'
$startupDirectory = Join-Path $env:APPDATA 'Microsoft\Windows\Start Menu\Programs\Startup'
$installedExe = Join-Path $installDirectory 'Switchy.exe'
$shortcutPath = Join-Path $startupDirectory 'Switchy.lnk'

New-Item -ItemType Directory -Path $installDirectory -Force | Out-Null
New-Item -ItemType Directory -Path $startupDirectory -Force | Out-Null
Copy-Item -LiteralPath $packageExe -Destination $installedExe -Force

$shell = New-Object -ComObject WScript.Shell
$shortcut = $shell.CreateShortcut($shortcutPath)
$shortcut.TargetPath = $installedExe
$shortcut.WorkingDirectory = $installDirectory
$shortcut.Arguments = if ($NoPopup) { 'nopopup' } else { '' }
$shortcut.Save()

Write-Host "Installed Switchy to $installDirectory"
Write-Host "It will start when you sign in. Shortcut: $shortcutPath"
if ($NoPopup) {
	Write-Host 'No-popup mode is enabled for the startup shortcut.'
}
