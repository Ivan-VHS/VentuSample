param(
  [Parameter(Mandatory=$true)][string]$FilePath,
  [string[]]$Arguments = @(),
  [Parameter(Mandatory=$true)][string]$OutputPath,
  [int]$StartupSeconds = 15
)
$ErrorActionPreference = "Stop"
Add-Type -AssemblyName System.Windows.Forms
Add-Type -AssemblyName System.Drawing
Add-Type @"
using System;
using System.Runtime.InteropServices;
public static class VentuCaptureNative {
  [DllImport("user32.dll")] public static extern bool ShowWindowAsync(IntPtr hWnd, int nCmdShow);
  [DllImport("user32.dll")] public static extern bool SetForegroundWindow(IntPtr hWnd);
}
"@
try { Set-DisplayResolution -Width 1920 -Height 1080 -Force -ErrorAction Stop } catch { Write-Warning "Desktop resolution unchanged; PNG will be normalized." }
$target=Join-Path (Get-Location) $OutputPath
New-Item -ItemType Directory -Force -Path (Split-Path -Parent $target) | Out-Null
$p=Start-Process -FilePath $FilePath -ArgumentList $Arguments -PassThru -WindowStyle Normal
$h=[IntPtr]::Zero
for($i=0;$i -lt ($StartupSeconds*4);$i++){Start-Sleep -Milliseconds 250;$p.Refresh();if($p.HasExited){throw "Application exited before screenshot (code $($p.ExitCode))."};if($p.MainWindowHandle -ne [IntPtr]::Zero){$h=$p.MainWindowHandle;break}}
if($h -eq [IntPtr]::Zero){throw "No visible Qt main window was created."}
[void][VentuCaptureNative]::ShowWindowAsync($h,3);[void][VentuCaptureNative]::SetForegroundWindow($h);Start-Sleep -Seconds 2
$b=[System.Windows.Forms.Screen]::PrimaryScreen.Bounds
$s=New-Object System.Drawing.Bitmap($b.Width,$b.Height);$g=[System.Drawing.Graphics]::FromImage($s);$g.CopyFromScreen($b.Location,[System.Drawing.Point]::Empty,$b.Size)
$o=New-Object System.Drawing.Bitmap(1920,1080);$og=[System.Drawing.Graphics]::FromImage($o);$og.InterpolationMode=[System.Drawing.Drawing2D.InterpolationMode]::HighQualityBicubic;$og.DrawImage($s,0,0,1920,1080);$o.Save($target,[System.Drawing.Imaging.ImageFormat]::Png)
$og.Dispose();$o.Dispose();$g.Dispose();$s.Dispose();Write-Host "Captured live VentuSample window to $OutputPath.";try{Stop-Process -Id $p.Id -Force -ErrorAction SilentlyContinue}catch{}
