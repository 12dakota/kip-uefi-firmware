# KipMu — our payload platform

Own tree for a COREBOOT UEFI payload aimed at kip.
Does not replace MrChromebox Full ROM until Windows boots twice.

Bring-up order:
1. Vendor MrChromebox UefiPayloadPkg (SMMSTORE, CbParse).
2. Pin one Mu release (mu_basecore / mu_plus / mu_oem_sample).
3. First FD: no FrontPage, no DFCI.
4. Splice into official kip 8MiB ROM as EXPERIMENTAL.

Build host: Linux, GCC5, IA32+X64, -D BOOTLOADER=COREBOOT.
