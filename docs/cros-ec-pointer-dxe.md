# CrosEcPointerDxe first (setup app later)

Goal: EFI_SIMPLE_POINTER_PROTOCOL from the Chrome EC touchpad
so stock MrChromebox setup (ESC) can see a cursor.
KipSetupApp waits until this probes on hardware.

## Why pad first
edk2 Boot Manager already consumes SimplePointer if a DXE installs it.
USB mice already work that way. The gap is EC, not a new GUI.

## Non-goals
- Coolstar .sys in CBFS
- AMI/HP pad modules
- Flashing an empty stub as a Release ROM

## Probe plan on kip
1. Confirm pad works in Linux (cros_ec, evtest) — protocol live.
2. Same host command path coreboot used (LPC MEC / I2C).
3. DXE: locate EC, query version, enable pad stream or poll buttons+delta.
4. Install SimplePointer (RelX/RelY/buttons). Absolute later.
5. If EC probe fails: return EFI_UNSUPPORTED, do not block USB HID.

## edk2 hook
New module in the MrChromebox edk2 tree used by build-uefi.sh:
  UefiPayloadPkg/CrosEcPointerDxe/CrosEcPointerDxe.inf
  .c implements:
    - DriverBinding or entry that runs at DXE
    - WaitForInput event + GetState
Add INF to the payload DSC/FDF DXE_DEPEX on GOP optional, EC required.
Build kip, size 8388608, name EXPERIMENTAL-ec-pointer.rom

## Protocol references (not copy-paste into SPI)
- Linux drivers/platform/chrome/cros_ec*
- Coolstar crosecbus / crostouchpad*
- coreboot src/ec/google/chromeec

## Test
WP off, USB keyboard attached.
ESC setup: USB mouse still works if DXE fails.
If cursor moves with finger, v1 is done. Then KipSetupApp.
