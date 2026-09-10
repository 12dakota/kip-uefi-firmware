# Run in Windows PowerShell as Administrator AFTER kip Full ROM + Windows install.
# This is not firmware AV and not HP Wolf. It turns on inbox Windows security.

#Requires -RunAsAdministrator
$ErrorActionPreference = "Stop"

Write-Host "kip Windows security helper (Defender + BitLocker checks)"

# Defender
Set-MpPreference -DisableRealtimeMonitoring $false -ErrorAction SilentlyContinue
Set-MpPreference -DisableBehaviorMonitoring $false -ErrorAction SilentlyContinue
Set-MpPreference -DisableIOAVProtection $false -ErrorAction SilentlyContinue
Set-MpPreference -DisableScriptScanning $false -ErrorAction SilentlyContinue
Start-Service WinDefend -ErrorAction SilentlyContinue
Update-MpSignature -ErrorAction SilentlyContinue
Write-Host "Defender realtime/signature update requested."

# Firewall
Set-NetFirewallProfile -Profile Domain,Public,Private -Enabled True
Write-Host "Firewall profiles enabled."

# TPM presence
$tpm = Get-Tpm -ErrorAction SilentlyContinue
if ($tpm) {
  Write-Host ("TPM Present={0} Ready={1} Enabled={2}" -f $tpm.TpmPresent, $tpm.TpmReady, $tpm.TpmEnabled)
} else {
  Write-Host "TPM cmdlet missing; check TPM in firmware menu first."
}

# BitLocker status only — does not force-encrypt
Get-BitLockerVolume -ErrorAction SilentlyContinue | Format-Table MountPoint, VolumeStatus, EncryptionPercentage, ProtectionStatus
Write-Host "To encrypt C: after TPM is ready:"
Write-Host "  Enable-BitLocker -MountPoint 'C:' -EncryptionMethod XtsAes128 -UsedSpaceOnly -TpmProtector"

Write-Host "Done. Install Chromebook/Coolstar drivers separately. This script is not in the .rom."
