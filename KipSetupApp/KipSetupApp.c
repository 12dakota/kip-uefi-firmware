/* Minimal ESC setup shell. Not Visual BIOS / Click BIOS.
 * v1: keyboard + USB mouse if SimplePointer exists.
 * Pad: wait for CrosEcPointerDxe.
 */
#include <Uefi.h>
#include <Library/UefiLib.h>
#include <Library/UefiBootServicesTableLib.h>
#include <Library/PrintLib.h>
#include <Protocol/GraphicsOutput.h>
#include <Protocol/SimplePointer.h>

EFI_STATUS
EFIAPI
KipSetupAppMain (
  IN EFI_HANDLE        ImageHandle,
  IN EFI_SYSTEM_TABLE  *SystemTable
  )
{
  EFI_SIMPLE_POINTER_PROTOCOL *Mouse = NULL;
  EFI_STATUS Status;
  UINTN i;
  EFI_INPUT_KEY Key;

  Print (L"kip setup v0.1  (ESC or Q = quit to firmware)\r\n");
  Print (L"1 Windows boot order hint   2 Secure Boot text");
  Print (L"   3 Reboot\r\n");

  Status = gBS->LocateProtocol (
                  &gEfiSimplePointerProtocolGuid,
                  NULL,
                  (VOID **)&Mouse
                  );
  if (!EFI_ERROR (Status) && Mouse != NULL) {
    Print (L"pointer: present (USB or EC DXE)\r\n");
  } else {
    Print (L"pointer: none (plug USB mouse or wait for CrosEcPointerDxe)\r\n");
  }

  for (;;) {
    Status = gST->ConIn->ReadKeyStroke (gST->ConIn, &Key);
    if (EFI_ERROR (Status)) {
      gBS->Stall (50000);
      if (Mouse != NULL) {
        EFI_SIMPLE_POINTER_STATE St;
        if (!EFI_ERROR (Mouse->GetState (Mouse, &St))) {
          if (St.RelativeMovementX || St.RelativeMovementY ||
              St.LeftButton || St.RightButton) {
            Print (L"  ptr dx=%d dy=%d L=%d R=%d\r\n",
                   St.RelativeMovementX, St.RelativeMovementY,
                   St.LeftButton, St.RightButton);
          }
        }
      }
      continue;
    }
    if (Key.ScanCode == SCAN_ESC || Key.UnicodeChar == L'q' ||
        Key.UnicodeChar == L'Q') {
      return EFI_SUCCESS;
    }
    if (Key.UnicodeChar == L'3') {
      gRT->ResetSystem (EfiResetCold, EFI_SUCCESS, 0, NULL);
    }
    Print (L"key %c\r\n", Key.UnicodeChar ? Key.UnicodeChar : L'?');
    i = 0; (VOID)i;
  }
}
