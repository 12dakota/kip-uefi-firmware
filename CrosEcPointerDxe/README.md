# CrosEcPointerDxe

Skeleton DXE for the kip Chrome EC touchpad.

`ProbeEc()` returns FALSE so this module **does nothing** until you
implement EC host commands. That is intentional.

Fill ProbeEc/EcPoll from:
- coreboot `src/ec/google/chromeec`
- Linux `drivers/platform/chrome`
- Coolstar crosecbus / crostouchpad

Add the INF to MrChromebox edk2 UefiPayloadPkg DSC/FDF, rebuild kip.
Test with USB keyboard. Daily ROM stays stock until ProbeEc is real.
