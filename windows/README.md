# Windows security helper (not firmware)

`kip-windows-security.ps1` enables **Microsoft Defender** realtime pieces, firewall, and prints TPM/BitLocker status.

It is **not** HP Wolf, not Bitdefender, and it is **not** inside the `.rom`.
Copy it to the Chromebook after Windows is installed and run PowerShell as Administrator:

```
Set-ExecutionPolicy -Scope Process Bypass
.\kip-windows-security.ps1
```

Set the firmware setup password and Secure Boot in edk2 on the device. Do not put passwords in this repo.
