/** @file
  BANON (Acer Chromebook 15 CB3-532, Intel Braswell) Elan pointer DXE.

  Firmware setup / Boot Manager only. Windows setup does not load this.
  Elan Microelectronics pad at I2C 0x15 on Braswell LPSS I2C (8086:22Cx).
  PROBE_BUS=0: PCI scan only.
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

#define BANON_ELAN_ADDR 0x15
#define INTEL_VID       0x8086

STATIC CONST UINT16 mBanonI2cDid[] = {
  0x22C1, 0x22C2, 0x22C3, 0x22C4, 0x22C5, 0x22C6, 0x22C7, 0x22C8,
  0
};

STATIC EFI_HANDLE                  mHandle;
STATIC EFI_SIMPLE_POINTER_PROTOCOL mPointer;
STATIC EFI_SIMPLE_POINTER_MODE     mMode;
STATIC EFI_EVENT                   mWaitEvent;

STATIC VOID EFIAPI WaitCallback (IN EFI_EVENT Event, IN VOID *Context) {}

STATIC EFI_STATUS EFIAPI PointerReset (
  IN EFI_SIMPLE_POINTER_PROTOCOL *This, IN BOOLEAN Ext)
{
  return EFI_SUCCESS;
}

STATIC EFI_STATUS EFIAPI PointerGetState (
  IN EFI_SIMPLE_POINTER_PROTOCOL *This,
  IN OUT EFI_SIMPLE_POINTER_STATE *State)
{
  if (State == NULL) {
    return EFI_INVALID_PARAMETER;
  }
  ZeroMem (State, sizeof (*State));
  return EFI_NOT_READY;
}

STATIC BOOLEAN FindBanonI2c (VOID)
{
  UINTN Bus, Dev, Fn, I;
  UINT16 Vid, Did;

  for (Bus = 0; Bus < 256; Bus++) {
    for (Dev = 0; Dev < 32; Dev++) {
      for (Fn = 0; Fn < 8; Fn++) {
        Vid = PciRead16 (PCI_LIB_ADDRESS (Bus, Dev, Fn, 0));
        if (Vid != INTEL_VID) {
          continue;
        }
        Did = PciRead16 (PCI_LIB_ADDRESS (Bus, Dev, Fn, 2));
        for (I = 0; mBanonI2cDid[I] != 0; I++) {
          if (Did == mBanonI2cDid[I]) {
            DEBUG ((DEBUG_INFO, "BanonElan: I2C %02x:%02x.%x DID %04x\n",
                    Bus, Dev, Fn, Did));
            return TRUE;
          }
        }
      }
    }
  }
  return FALSE;
}

EFI_STATUS EFIAPI BanonElanPointerEntry (
  IN EFI_HANDLE ImageHandle, IN EFI_SYSTEM_TABLE *SystemTable)
{
  EFI_STATUS Status;

  DEBUG ((DEBUG_INFO, "BanonElan: i2c=%a elan=0x%02x\n",
          FindBanonI2c () ? "yes" : "no", BANON_ELAN_ADDR));

  ZeroMem (&mMode, sizeof (mMode));
  mMode.ResolutionX = 8;
  mMode.ResolutionY = 8;
  mMode.LeftButton = TRUE;
  mMode.RightButton = TRUE;
  mPointer.Reset = PointerReset;
  mPointer.GetState = PointerGetState;
  mPointer.Mode = &mMode;

  Status = gBS->CreateEvent (EVT_NOTIFY_WAIT, TPL_NOTIFY, WaitCallback,
                             NULL, &mWaitEvent);
  if (EFI_ERROR (Status)) {
    return Status;
  }
  mPointer.WaitForInput = mWaitEvent;
  mHandle = NULL;
  return gBS->InstallMultipleProtocolInterfaces (
           &mHandle, &gEfiSimplePointerProtocolGuid, &mPointer, NULL);
}
