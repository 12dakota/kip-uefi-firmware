/** @file
  Experimental CrosEC Simple Pointer DXE.

  Probes LPC-style Chrome EC host command ports. If the EC is not
  talking this protocol (common on APL SNAPPY), GetState returns zeros.
  Does not spin on the EC. Not a finished Elan/Synaptics pad driver.
**/

#include <Uefi.h>
#include <Library/UefiLib.h>
#include <Library/UefiBootServicesTableLib.h>
#include <Library/IoLib.h>
#include <Library/BaseMemoryLib.h>
#include <Library/MemoryAllocationLib.h>
#include <Library/DebugLib.h>
#include <Protocol/SimplePointer.h>

#define EC_LPC_HOST_CMD   0x662
#define EC_LPC_HOST_DATA  0x666
#define EC_LPC_HOST_CMD_ALT 0x200
#define EC_LPC_HOST_DATA_ALT 0x204

STATIC EFI_HANDLE                 mHandle;
STATIC EFI_SIMPLE_POINTER_PROTOCOL mPointer;
STATIC EFI_SIMPLE_POINTER_MODE     mMode;
STATIC EFI_EVENT                   mWaitEvent;
STATIC BOOLEAN                     mEcSeen;

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
  /* Real EC motion path not implemented. USB mouse still works via UsbMouseDxe. */
  return EFI_NOT_READY;
}

STATIC
BOOLEAN
ProbeEc (
  VOID
  )
{
  UINT8 St;

  St = IoRead8 (EC_LPC_HOST_CMD);
  if (St != 0xFF) {
    return TRUE;
  }
  St = IoRead8 (EC_LPC_HOST_CMD_ALT);
  if (St != 0xFF) {
    return TRUE;
  }
  return FALSE;
}

EFI_STATUS
EFIAPI
CrosEcPointerEntry (
  IN EFI_HANDLE        ImageHandle,
  IN EFI_SYSTEM_TABLE  *SystemTable
  )
{
  EFI_STATUS Status;

  mEcSeen = ProbeEc ();
  DEBUG ((DEBUG_INFO, "CrosEcPointerDxe: EC probe %a\n", mEcSeen ? "maybe" : "no"));

  ZeroMem (&mMode, sizeof (mMode));
  mMode.ResolutionX = 8;
  mMode.ResolutionY = 8;
  mMode.ResolutionZ = 0;
  mMode.LeftButton  = TRUE;
  mMode.RightButton = TRUE;

  mPointer.Reset     = PointerReset;
  mPointer.GetState  = PointerGetState;
  mPointer.Mode      = &mMode;

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
