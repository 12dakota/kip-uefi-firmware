# BanonElanPointerDxe (BANON first)

Acer Chromebook 15 CB3-532 / board BANON / Braswell.

On entry the DXE:
- probes Chrome EC LPC 0x800
- finds LPSS I2C 8086:22Cx and enables MEM
- issues a timed Elan read at 0x15 (ETP desc)
- always installs EFI_SIMPLE_POINTER_PROTOCOL

GetState returns motion only if that read yields non-zero bytes.
Otherwise NOT_READY (USB mouse / keys still used).

This is firmware setup only. Windows/Linux use Coolstar / elan_i2c.
SMBIOS "Acer" / "CB3-532" is a coreboot string, not this driver.
SNAPPY and KIP are later flavors. Do not flash BANON ROM on those.
