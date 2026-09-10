# FrontPage in the COREBOOT payload

UefiPayloadPkg does not include OemPkg/FrontPage. To get the Mu sidebar:

1. PACKAGES_PATH must include mu_oem_sample and mu_plus (MsGraphicsPkg).
2. The payload FDF must list FrontPage + DisplayEngine + SimpleWindowManager.
3. BOOT_TO_FRONT_PAGE must stay FALSE so power-on still goes to Windows.
4. Drop DFCI, OSK, network, TTF. Bitmap font only or CBFS overflows.

Until those INFs are in the FV, the FD is still Tianocore setup.
