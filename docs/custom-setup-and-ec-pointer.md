# Custom setup app + EC pointer (plan)

Daily flash image stays MrChromebox kip Full ROM.
This document is the chosen *next firmware project*, not a finished feature.

## Boot policy (unchanged)
- No key: Windows Boot Manager
- ESC: setup
- Custom UI and pad driver apply only after ESC

## Package A — KipSetupApp (UEFI application)
Replace or wrap Tianocore Boot Manager visually after ESC.

- Type: EFI_APPLICATION in UefiPayloadPkg FDF
- Protocols: GOP, SimpleText, SimplePointer / AbsolutePointer
- Screens: Boot order, Secure Boot status, TPM status, Exit/reset
- Input v1: USB mouse + keyboard
- Input v2: Chrome EC pad when Package B exists
- Not in v1: XMP, NPU, Click BIOS tiles, Visual BIOS clone

BDS change: ESC path launches KipSetupApp instead of (or before) the stock menu.

## Package B — CrosEcPointerDxe
- Bind to Chrome EC the same way coreboot already inits it
- Protocol: EFI_SIMPLE_POINTER_PROTOCOL (buttons + relative) first
- Then absolute if the pad reports abs frames
- Source of truth for commands: Linux cros_ec, Coolstar crosecbus
- Recover: if probe fails, stay silent so USB mouse still works

## Build
- Fork MrChromebox edk2 used by build-uefi.sh
- Do not switch payload to Mu/SBL
- Experimental ROM name: EXPERIMENTAL-kip-setup.rom
- Size must remain 8388608
- Test with WP off, USB keyboard always attached

## Out of scope
Baking setup passwords, HP Wolf, MSI/AMI binaries, rEFInd-as-only-payload.
