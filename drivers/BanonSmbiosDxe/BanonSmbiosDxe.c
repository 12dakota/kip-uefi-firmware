/** @file
  BANON SMBIOS Type 1/2 strings: Acer / CB3-532.
  Runs in DXE after SmbiosDxe publishes EFI_SMBIOS_PROTOCOL.
**/
#include <Uefi.h>
#include <Library/UefiBootServicesTableLib.h>
#include <Library/DebugLib.h>
#include <Library/BaseLib.h>
#include <Protocol/Smbios.h>
#include <IndustryStandard/SmBios.h>

STATIC CHAR8 mMfg[]     = "Acer";
STATIC CHAR8 mProduct[] = "CB3-532";
STATIC CHAR8 mFamily[]  = "Chromebook 15";
STATIC CHAR8 mBoard[]   = "BANON";

STATIC EFI_STATUS
PatchType (
  EFI_SMBIOS_PROTOCOL *Smbios,
  EFI_SMBIOS_TYPE Type,
  UINT8 Str1,
  CHAR8 *Val1,
  UINT8 Str2,
  CHAR8 *Val2
  )
{
  EFI_STATUS Status;
  EFI_SMBIOS_HANDLE Handle;
  EFI_SMBIOS_TABLE_HEADER *Hdr;
  UINTN N;

  Handle = SMBIOS_HANDLE_PI_RESERVED;
  Status = Smbios->GetNext (Smbios, &Handle, &Type, &Hdr, NULL);
  if (EFI_ERROR (Status) || Hdr == NULL) {
    return Status;
  }
  N = Str1;
  Status = Smbios->UpdateString (Smbios, &Handle, &N, Val1);
  if (EFI_ERROR (Status)) {
    DEBUG ((DEBUG_WARN, "BanonSmbios: type %d str %d: %r\n", Type, Str1, Status));
  }
  if (Str2 != 0 && Val2 != NULL) {
    N = Str2;
    Status = Smbios->UpdateString (Smbios, &Handle, &N, Val2);
  }
  return EFI_SUCCESS;
}

EFI_STATUS EFIAPI
BanonSmbiosEntry (IN EFI_HANDLE ImageHandle, IN EFI_SYSTEM_TABLE *SystemTable)
{
  EFI_STATUS Status;
  EFI_SMBIOS_PROTOCOL *Smbios;

  Status = gBS->LocateProtocol (&gEfiSmbiosProtocolGuid, NULL, (VOID **)&Smbios);
  if (EFI_ERROR (Status)) {
    DEBUG ((DEBUG_ERROR, "BanonSmbios: no SmbiosProtocol %r\n", Status));
    return EFI_SUCCESS;
  }
  PatchType (Smbios, SMBIOS_TYPE_SYSTEM_INFORMATION, 1, mMfg, 2, mProduct);
  PatchType (Smbios, SMBIOS_TYPE_SYSTEM_INFORMATION, 6, mFamily, 0, NULL);
  PatchType (Smbios, SMBIOS_TYPE_BASEBOARD_INFORMATION, 1, mMfg, 2, mBoard);
  return EFI_SUCCESS;
}
