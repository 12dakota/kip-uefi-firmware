/* CrosEcPointerDxe — skeleton
 *
 * Installs EFI_SIMPLE_POINTER_PROTOCOL.
 * EcPoll() is a STUB. Do not expect the kip pad to move until
 * host commands from Linux cros_ec / Coolstar crosecbus are filled in.
 * If probe fails we still install a zero-motion pointer so USB HID
 * remains the real device; or skip install — see ProbeEc().
 */
#include <Uefi.h>
#include <Library/UefiBootServicesTableLib.h>
#include <Library/MemoryAllocationLib.h>
#include <Library/BaseMemoryLib.h>
#include <Library/UefiLib.h>
#include <Library/DebugLib.h>
#include <Protocol/SimplePointer.h>

typedef struct {
  EFI_SIMPLE_POINTER_PROTOCOL  Proto;
  EFI_SIMPLE_POINTER_MODE      Mode;
  EFI_EVENT                    Wait;
  INT32                        RelX;
  INT32                        RelY;
  BOOLEAN                      Left;
  BOOLEAN                      Right;
} CROS_EC_PTR;

STATIC CROS_EC_PTR  *mPtr = NULL;

/* TODO: LPC/MEC or I2C tunnel to Chrome EC. Return TRUE if pad talks. */
STATIC
BOOLEAN
ProbeEc (
  VOID
  )
{
  return FALSE;
}

/* TODO: read EC touch/host-event; fill RelX/RelY/buttons. */
STATIC
VOID
EcPoll (
  IN CROS_EC_PTR  *P
  )
{
  (VOID)P;
}

STATIC
VOID
EFIAPI
WaitCallback (
  IN EFI_EVENT  Event,
  IN VOID       *Context
  )
{
  EcPoll ((CROS_EC_PTR *)Context);
}

STATIC
EFI_STATUS
EFIAPI
PtrReset (
  IN EFI_SIMPLE_POINTER_PROTOCOL  *This,
  IN BOOLEAN                      Extended
  )
{
  CROS_EC_PTR *P = (CROS_EC_PTR *)This;
  (VOID)Extended;
  P->RelX = P->RelY = 0;
  P->Left = P->Right = FALSE;
  return EFI_SUCCESS;
}

STATIC
EFI_STATUS
EFIAPI
PtrGetState (
  IN EFI_SIMPLE_POINTER_PROTOCOL  *This,
  OUT EFI_SIMPLE_POINTER_STATE    *State
  )
{
  CROS_EC_PTR *P = (CROS_EC_PTR *)This;
  if (State == NULL) {
    return EFI_INVALID_PARAMETER;
  }
  EcPoll (P);
  State->RelativeMovementX = P->RelX;
  State->RelativeMovementY = P->RelY;
  State->RelativeMovementZ = 0;
  State->LeftButton  = P->Left;
  State->RightButton = P->Right;
  P->RelX = P->RelY = 0;
  return EFI_SUCCESS;
}

EFI_STATUS
EFIAPI
CrosEcPointerEntry (
  IN EFI_HANDLE        ImageHandle,
  IN EFI_SYSTEM_TABLE  *SystemTable
  )
{
  EFI_STATUS Status;
  (VOID)SystemTable;

  if (!ProbeEc ()) {
    DEBUG ((DEBUG_INFO, "CrosEcPointerDxe: EC pad not probed, skip\n"));
    return EFI_UNSUPPORTED;
  }

  mPtr = AllocateZeroPool (sizeof (*mPtr));
  if (mPtr == NULL) {
    return EFI_OUT_OF_RESOURCES;
  }

  mPtr->Mode.ResolutionX = 8;
  mPtr->Mode.ResolutionY = 8;
  mPtr->Mode.ResolutionZ = 0;
  mPtr->Mode.LeftButton  = TRUE;
  mPtr->Mode.RightButton = TRUE;
  mPtr->Proto.Reset      = PtrReset;
  mPtr->Proto.GetState   = PtrGetState;
  mPtr->Proto.Mode       = &mPtr->Mode;

  Status = gBS->CreateEvent (
                  EVT_NOTIFY_WAIT,
                  TPL_NOTIFY,
                  WaitCallback,
                  mPtr,
                  &mPtr->Wait
                  );
  if (EFI_ERROR (Status)) {
    FreePool (mPtr);
    return Status;
  }
  mPtr->Proto.WaitForInput = mPtr->Wait;

  Status = gBS->InstallMultipleProtocolInterfaces (
                  &ImageHandle,
                  &gEfiSimplePointerProtocolGuid,
                  &mPtr->Proto,
                  NULL
                  );
  return Status;
}
