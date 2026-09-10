# Cheap "modern BIOS" look (not Click BIOS)

Flash stays MrChromebox kip 8MiB ROM + HP splash.
This only styles the *boot menu* after firmware.

## Install rEFInd on the EFI partition

Linux:
  sudo apt install refind
  sudo refind-install
  sudo cp refind.conf /boot/efi/EFI/refind/refind.conf

Windows (admin):
  mountvol B: /S
  mkdir B:\EFI\refind
  Copy rEFInd files from https://www.rodsbooks.com/refind/ into B:\EFI\refind\
  copy refind.conf there
  In edk2 Boot Manager, Boot from file: EFI\refind\refind_x64.efi
  If that fails on Bay Trail, try refind_ia32.efi

## Optional theme
https://github.com/bobafetthotmail/refind-theme-regular
or rEFInd-minimal. Copy theme dir next to refind.conf and add:
  include theme/theme.conf

## Firmware
ESC still opens real setup (Secure Boot, TPM, password).
USB mouse for rEFInd. Chromebook pad after Windows + Coolstar only.

Do not flash rEFInd into SPI as the only payload.
