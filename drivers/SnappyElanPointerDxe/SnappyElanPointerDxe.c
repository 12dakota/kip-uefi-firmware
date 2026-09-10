/** @file
  SNAPPY firmware pointer DXE (BIOS setup / Boot Manager only).

  Windows setup.exe / WinPE does NOT load this driver. After
  ExitBootServices it is gone. Installer pad = Coolstar INFs.

  Hardware: Elan @ I2C 0x15 on APL LPSS DesignWare (PCI 8086:5AAx).
  PROBE_BUS 0 = PCI scan only. Set 1 only on a serial-debug image.
**/

#include <Uefi.h>
#include <Library/UefiLib.h>
#include <Library/UefiBootServicesTableLib.h>
#include <Library/BaseMemoryLib.h>
#include <Library/DebugLib.h>
#include <Library/PciLib.h>
#include <Library/IoLib.h>
#include <Protocol/SimplePointer.h>
#include <IndustryStandard/Pci.h>

#ifndef PROBE_BUS
#define PROBE_BUS 0
#endif

#define SNAPPY_ELAN_ADDR  0x15
#define INTEL_VID         0x8086

#define DW_IC_CON        0x00
#define DW_IC_TAR        0x04
#define DW_IC_DATA_CMD   0x10
#define DW_IC_ENABLE     0x6c
#define DW_IC_STATUS     0x70
#define DW_IC_TXFLR      0x74
#define DW_IC_RXFLR      0x78

STATIC CONST UINT16 mSnappyI2cDid[] = {
  0x5AAC, 0x5AAE, 0x5AB0, 0x5AB2, 0x5AB4, 0x5AB6,
  0
};

STATIC EFI_HANDLE                  mHandle;
STATIC EFI_SIMPLE_POINTER_PROTOCOL mPointer;
STATIC EFI_SIMPLE_POINTER_MODE     mMode;
STATIC EFI_EVENT                   mWaitEvent;
STATIC BOOLEAN                     mFoundI2c;
STATIC UINTN                       mBar;

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
  /* Elan report -> RelativeMovementX/Y not implemented. */
  return EFI_NOT_READY;
}

STATIC
BOOLEAN
FindSnappyI2c (
  OUT UINTN *Bar OPTIONAL
  )
{
  UINTN  Bus, Dev, Fn;
  UINT16 Vid, Did;
  UINTN  I;
  UINT32 Bar0;

  for (Bus = 0; Bus < 256; Bus++) {
    for (Dev = 0; Dev < 32; Dev++) {
      for (Fn = 0; Fn < 8; Fn++) {
        Vid = PciRead16 (PCI_LIB_ADDRESS (Bus, Dev, Fn, 0));
        if (Vid != INTEL_VID) {
          continue;
        }
        Did = PciRead16 (PCI_LIB_ADDRESS (Bus, Dev, Fn, 2));
        for (I = 0; mSnappyI2cDid[I] != 0; I++) {
          if (Did != mSnappyI2cDid[I]) {
            continue;
          }
          Bar0 = PciRead32 (PCI_LIB_ADDRESS (Bus, Dev, Fn, 0x10));
          DEBUG ((DEBUG_INFO,
                  "SnappyElan: I2C %02x:%02x.%x DID %04x BAR0 %08x\n",
                  Bus, Dev, Fn, Did, Bar0));
          if (Bar != NULL) {
            *Bar = (UINTN)(Bar0 & ~0xFULL);
          }
          return TRUE;
        }
      }
    }
  }
  return FALSE;
}

#if PROBE_BUS
STATIC
BOOLEAN
DwStatusRfne (
  UINTN Bar
  )
{
  return (MmioRead32 (Bar + DW_IC_STATUS) & 0x08) != 0;
}
#endif

EFI_STATUS
EFIAPI
SnappyElanPointerEntry (
  IN EFI_HANDLE       ImageHandle,
  IN EFI_SYSTEM_TABLE *SystemTable
  )
{
  EFI_STATUS Status;

  mBar = 0;
  mFoundI2c = FindSnappyI2c (&mBar);
  DEBUG ((DEBUG_INFO,
          "SnappyElan: i2c=%a bar=%lx elan=0x%02x probe=%d\n",
          mFoundI2c ? "yes" : "no", (UINT64)mBar, SNAPPY_ELAN_ADDR, PROBE_BUS));

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
