/** @file
  Elan I2C touchpad DXE (experimental).

  SNAPPY / ChromeOS laptops: Elan at 7-bit address 0x15 on Intel LPSS
  (DesignWare) I2C. OS uses elan_i2c / i2c-hid. This DXE:
    1. Finds Intel APL/GLK-style I2C PCI functions
    2. Optionally issues a short I2C read (disabled by default)
    3. Installs EFI_SIMPLE_POINTER_PROTOCOL

  Default build does NOT hammer I2C MMIO (PROBE_BUS=0) so setup cannot
  hang. Set PROBE_BUS 1 only on a serial-debug image.
**/

#include <Uefi.h>
#include <Library/UefiLib.h>
#include <Library/UefiBootServicesTableLib.h>
#include <Library/BaseMemoryLib.h>
#include <Library/MemoryAllocationLib.h>
#include <Library/DebugLib.h>
#include <Library/PciLib.h>
#include <Protocol/SimplePointer.h>
#include <Protocol/PciIo.h>
#include <IndustryStandard/Pci.h>

#ifndef PROBE_BUS
#define PROBE_BUS 0
#endif

#define ELAN_I2C_ADDR  0x15
#define INTEL_VID      0x8086

/* Apollo Lake / Gemini Lake LPSS I2C (subset) */
STATIC CONST UINT16 mI2cDid[] = {
  0x5AAC, 0x5AAE, 0x5AB0, 0x5AB2, 0x5AB4, 0x5AB6,
  0x31AC, 0x31AE, 0x31B0, 0x31B2, 0x31B4, 0x31B6,
  0
};

STATIC EFI_HANDLE                  mHandle;
STATIC EFI_SIMPLE_POINTER_PROTOCOL mPointer;
STATIC EFI_SIMPLE_POINTER_MODE     mMode;
STATIC EFI_EVENT                   mWaitEvent;
STATIC BOOLEAN                     mHaveController;

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
  /* Live Elan report parse not hooked up. */
  return EFI_NOT_READY;
}

STATIC
BOOLEAN
IsI2cDid (
  UINT16 Did
  )
{
  UINTN I;
  for (I = 0; mI2cDid[I] != 0; I++) {
    if (mI2cDid[I] == Did) {
      return TRUE;
    }
  }
  return FALSE;
}

STATIC
BOOLEAN
FindLpssI2c (
  VOID
  )
{
  UINTN  Bus, Dev, Fn;
  UINT16 Vid, Did;

  for (Bus = 0; Bus < 256; Bus++) {
    for (Dev = 0; Dev < 32; Dev++) {
      for (Fn = 0; Fn < 8; Fn++) {
        Vid = PciRead16 (PCI_LIB_ADDRESS (Bus, Dev, Fn, 0));
        if (Vid != INTEL_VID) {
          continue;
        }
        Did = PciRead16 (PCI_LIB_ADDRESS (Bus, Dev, Fn, 2));
        if (IsI2cDid (Did)) {
          DEBUG ((DEBUG_INFO, "ElanI2c: LPSS I2C %02x:%02x.%x DID %04x\n",
                  Bus, Dev, Fn, Did));
          return TRUE;
        }
      }
    }
  }
  return FALSE;
}

EFI_STATUS
EFIAPI
ElanI2cPointerEntry (
  IN EFI_HANDLE       ImageHandle,
  IN EFI_SYSTEM_TABLE *SystemTable
  )
{
  EFI_STATUS Status;

  mHaveController = FindLpssI2c ();
  DEBUG ((DEBUG_INFO, "ElanI2cPointerDxe: controller %a addr 0x%02x probe=%d\n",
          mHaveController ? "found" : "missing", ELAN_I2C_ADDR, PROBE_BUS));

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
