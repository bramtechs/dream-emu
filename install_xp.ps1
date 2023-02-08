$OriginalLocation = Get-Location
Set-Location "C:\Program Files (x86)\Microsoft Visual Studio\Installer\"
$InstallPath = "C:\Program Files\Microsoft Visual Studio\2022\Preview"
$WorkLoads = '--add Microsoft.VisualStudio.Component.WinXP'
$Arguments = ('/c', "vs_installer.exe", 'modify', '--installPath', "`"$InstallPath`"", $WorkLoads, '--quiet', '--norestart', '--nocache')
$process = Start-Process -FilePath cmd.exe -ArgumentList $Arguments -Wait -PassThru -WindowStyle Hidden
if ($process.ExitCode -eq 0)
{
    Write-Host "components have been successfully added"
}
else
{
    Write-Host "components were not installed"
}
Set-Location $OriginalLocation
