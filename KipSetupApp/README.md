# KipSetupApp

UEFI application skeleton for ESC setup. Build **inside** MrChromebox edk2
(`UefiPayloadPkg`), not as a standalone Linux binary.

1. Copy this folder into the edk2 tree next to UefiPayloadPkg.
2. Add `KipSetupApp/KipSetupApp.inf` to the payload DSC [Components].
3. Add a FILE APPLICATION section to the FDF (GUID from the INF).
4. Point BDS ESC path at that FILE GUID (later). Until then, launch from
   Boot from file for testing.
5. Keep daily kip ROM unchanged until this is proven.

Does not include Click/Visual BIOS graphics. Pointer = USB or future
CrosEcPointerDxe.
