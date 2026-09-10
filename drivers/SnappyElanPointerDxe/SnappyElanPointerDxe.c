/** @file
  SNAPPY (Google reef / Intel Apollo Lake) Elan touchpad DXE.

  Board facts:
    - ChromeOS board name: snappy
    - Pad: Elan on I2C 7-bit address 0x15 (same as Linux elan_i2c)
    - ACPI HID typically ELAN0000 once coreboot emits the I2C node
    - Host controller: Intel LPSS DesignWare I2C (PCI VID 8086, DID 5AAx)

  This module installs EFI_SIMPLE_POINTER_PROTOCOL so BDS/UiApp can
  attach a pointer device. GetState() does not yet parse Elan reports.
  PROBE_BUS defaults to 0: PCI scan only, no DW I2C MMIO, no hang.
**/

#include <Uefi.h>
#include <Library/UefiLib.h>
#include <Library/UefiBootServicesTableLib.h>
#include <Library/BaseMemoryLib.h>
#include <Library/DebugLib.h>
#include <Library/PciLib.h>
#include <Protocol/SimplePointer.h>
#include <IndustryStandard/Pci.h>

#ifndef PROBE_BUS
#define PROBE_BUS 0
#endif

#define SNAPPY_ELAN_ADDR  0x15
#define INTEL_VID         0x8086

/* Apollo Lake LPSS I2C functions used on reef/snappy */
STATIC CONST UINT16 mSnappyI2cDid[] = {
  0x5AAC, 0x5AAE, 0x5AB0, 0x5AB2, 0x5AB4, 0x5AB6,
  0
};

STATIC EFI_HANDLE                  mHandle;
STATIC EFI_SIMPLE_POINTER_PROTOCOL mPointer;
STATIC EFI_SIMPLE_POINTER_MODE     mMode;
STATIC EFI_EVENT                   mWaitEvent;
STATIC BOOLEAN                     mFoundI2c;

STATIC
VOID
EFIAPI
WaitCallback (
  IN EFI_EVENT Event,
  IN VOID      *Context
  )
{
}

STATIC
EFI_STATUS
EFIAPI
PointerReset (
  IN EFI_SIMPLE_POINTER_PROTOCOL *This,
  IN BOOLEAN                     ExtendedVerification
  )
{
  return EFI_SUCCESS;
}

STATIC
EFI_STATUS
EFIAPI
PointerGetState (
  IN EFI_SIMPLE_POINTER_PROTOCOL *This,
  IN OUT EFI_SIMPLE_POINTER_STATE *State
  )
{
  if (State == NULL) {
    return EFI_INVALID_PARAMETER;
  }
  ZeroMem (State, sizeof (*State));
  return EFI_NOT_READY;
}

STATIC
BOOLEAN
FindSnappyI2c (
  VOID
  )
{
  UINTN  Bus, Dev, Fn;
  UINT16 Vid, Did;
  UINTN  I;

  for (Bus = 0; Bus < 256; Bus++) {
    for (Dev = 0; Dev < 32; Dev++) {
      for (Fn = 0; Fn < 8; Fn++) {
        Vid = PciRead16 (PCI_LIB_ADDRESS (Bus, Dev, Fn, 0));
        if (Vid != INTEL_VID) {
          continue;
        }
        Did = PciRead16 (PCI_LIB_ADDRESS (Bus, Dev, Fn, 2));
        for (I = 0; mSnappyI2cDid[I] != 0; I++) {
          if (Did == mSnappyI2cDid[I]) {
            DEBUG ((DEBUG_INFO,
                    "SnappyElan: LPSS I2C %02x:%02x.%x DID %04x (Elan 0x%02x)\n",
                    Bus, Dev, Fn, Did, SNAPPY_ELAN_ADDR));
            return TRUE;
          }
        }
      }
    }
  }
  return FALSE;
}

EFI_STATUS
EFIAPI
SnappyElanPointerEntry (
  IN EFI_HANDLE       ImageHandle,
  IN EFI_SYSTEM_TABLE *SystemTable
  )
{
  EFI_STATUS Status;

  mFoundI2c = FindSnappyI2c ();
  DEBUG ((DEBUG_INFO,
          "SnappyElanPointerDxe: i2c %a elan=0x%02x probe_bus=%d\n",
          mFoundI2c ? "yes" : "no", SNAPPY_ELAN_ADDR, PROBE_BUS));

  ZeroMem (&mMode, sizeof (mMode));
  mMode.ResolutionX = 8;
  mMode.ResolutionY = 8;
  mMode.LeftButton  = TRUE;
  mMode.RightButton = TRUE;

  mPointer.Reset    = PointerReset;
  mPointer.GetState = PointerGetState;
  mPointer.Mode     = &mMode;

  Status = gBS->CreateEvent (
                  EVT_NOTIFY_WAIT,
                  TPL_NOTIFY,
                  WaitCallback,
                  NULL,
                  &mWaitEvent
                  );
  if (EFI_ERROR (Status)) {
    return Status;
  }
  mPointer.WaitForInput = mWaitEvent;

  mHandle = NULL;
  return gBS->InstallMultipleProtocolInterfaces (
                &mHandle,
                &gEfiSimplePointerProtocolGuid,
                &mPointer,
                NULL
                );
}
