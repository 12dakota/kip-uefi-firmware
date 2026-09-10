# Project Mu port for kip (plan)

Goal: Mu UefiPayload.fd on top of MrChromebox coreboot kip.
Not a Mu-from-reset firmware. Not Visual BIOS.

## Stages (do not skip)

### M0 — already required
- Daily flash: MrChromebox kip Full ROM
- Experimental: splice MrChromebox edk2 payload (SMMSTORE + CbParse)

### M1 — Mu workspace
Submodules pinned to one Mu release:
- microsoft/mu_basecore
- microsoft/mu_plus
- microsoft/mu_tiano_plus
- microsoft/mu_oem_sample
- Keep a copy of MrChromebox UefiPayloadPkg (do not delete)

### M2 — boot FD (no FrontPage)
Platform DSC based on UefiPayloadPkg:
- BOOTLOADER=COREBOOT
- IA32 + X64
- VARIABLE_SUPPORT=SMMSTORE (from MrChromebox tree)
- BlParseLib = CbParseLib
- BdsDxe from Tiano/MrChromebox, not PcBdsPkg yet
- GUI_FRONT_PAGE=FALSE
Prove: splice, Windows boots twice (NVRAM persists).

### M3 — Mu packages beside that BDS
Add only:
- MsUiTheme (tiny bitmap font)
- DisplayEngine / SWM if size allows
Omit DFCI, OSK, Shell, network.

### M4 — FrontPage
BOOT_TO_FRONT_PAGE=FALSE
ESC or a boot option opens FrontPage.
If FV > CBFS budget, stop at M2.

## Success
Cold boot, no key → Windows.
ESC → setup or FrontPage.
Size 8388608.
USB mouse in setup. Pad still not in firmware.
