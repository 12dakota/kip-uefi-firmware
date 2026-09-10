/** @file
  BANON (Acer Chromebook 15 CB3-532, Braswell) firmware pointer.

  Probes Chrome EC LPC (0x800/0x804), Braswell LPSS I2C (8086:22Cx),
  Elan at 7-bit 0x15. Always installs EFI_SIMPLE_POINTER_PROTOCOL.
  Timeouts on LPC/MMIO so a missing pad cannot hang BDS.
  Windows/Linux do not load this DXE; OS uses Coolstar / elan_i2c.
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

#define BANON_ELAN_ADDR 0x15
#define INTEL_VID       0x8086
#define EC_LPC_CMD      0x800
#define EC_LPC_TIMEOUT  20000
#define DW_TIMEOUT      50000
#define DW_IC_CON       0x00
#define DW_IC_TAR       0x04
#define DW_IC_DATA_CMD  0x10
#define DW_IC_ENABLE    0x6c
#define DW_IC_STATUS    0x70
#define DW_STATUS_RFNE  0x08
#define DW_STATUS_TFNF  0x02
#define ETP_I2C_DESC    0x0001
#define ETP_I2C_STAND   0x0005

STATIC CONST UINT16 mI2cDid[] = {
  0x22C1, 0x22C2, 0x22C3, 0x22C4, 0x22C5, 0x22C6, 0x22C7, 0x22C8, 0
};

STATIC EFI_HANDLE mHandle;
STATIC EFI_SIMPLE_POINTER_PROTOCOL mPointer;
STATIC EFI_SIMPLE_POINTER_MODE mMode;
STATIC EFI_EVENT mWaitEvent;
STATIC BOOLEAN mEcOk, mI2cOk, mElanOk;
STATIC UINTN mBar;

STATIC VOID EFIAPI WaitCallback (IN EFI_EVENT E, IN VOID *C) {}

STATIC VOID Spin (UINTN N)
{
  volatile UINTN I;
  for (I = 0; I < N; I++) { IoRead8 (0x80); }
}

STATIC BOOLEAN EcPresent (VOID)
{
  UINTN T;
  IoWrite8 (EC_LPC_CMD, 0x00);
  for (T = 0; T < EC_LPC_TIMEOUT; T++) {
    if ((IoRead8 (EC_LPC_CMD) & 0x03) == 0) { return TRUE; }
    Spin (50);
  }
  return FALSE;
}

STATIC BOOLEAN FindI2c (OUT UINTN *Bar)
{
  UINTN Bus, Dev, Fn, I;
  UINT16 Vid, Did;
  UINT32 Bar0, Cmd;
  for (Bus = 0; Bus < 256; Bus++) {
    for (Dev = 0; Dev < 32; Dev++) {
      for (Fn = 0; Fn < 8; Fn++) {
        Vid = PciRead16 (PCI_LIB_ADDRESS (Bus, Dev, Fn, 0));
        if (Vid != INTEL_VID) continue;
        Did = PciRead16 (PCI_LIB_ADDRESS (Bus, Dev, Fn, 2));
        for (I = 0; mI2cDid[I] != 0; I++) {
          if (Did != mI2cDid[I]) continue;
          Bar0 = PciRead32 (PCI_LIB_ADDRESS (Bus, Dev, Fn, 0x10));
          if ((Bar0 & 0x1) != 0 || (Bar0 & ~0xFULL) == 0) continue;
          Cmd = PciRead16 (PCI_LIB_ADDRESS (Bus, Dev, Fn, 0x04));
          PciWrite16 (PCI_LIB_ADDRESS (Bus, Dev, Fn, 0x04), (UINT16)(Cmd | 0x2));
          *Bar = (UINTN)(Bar0 & ~0xFULL);
          return TRUE;
        }
      }
    }
  }
  return FALSE;
}

STATIC BOOLEAN DwWait (UINTN Bar, UINT32 Mask, BOOLEAN Set)
{
  UINTN T;
  UINT32 S;
  for (T = 0; T < DW_TIMEOUT; T++) {
    S = MmioRead32 (Bar + DW_IC_STATUS);
    if (Set ? ((S & Mask) != 0) : ((S & Mask) == 0)) return TRUE;
    Spin (20);
  }
  return FALSE;
}

STATIC BOOLEAN ElanRead2 (UINTN Bar, UINT16 Reg, UINT8 *A, UINT8 *B)
{
  if (Bar == 0) return FALSE;
  MmioWrite32 (Bar + DW_IC_ENABLE, 0);
  Spin (200);
  MmioWrite32 (Bar + DW_IC_TAR, BANON_ELAN_ADDR);
  MmioWrite32 (Bar + DW_IC_CON, 0x65);
  MmioWrite32 (Bar + DW_IC_ENABLE, 1);
  Spin (200);
  if (!DwWait (Bar, DW_STATUS_TFNF, TRUE)) return FALSE;
  MmioWrite32 (Bar + DW_IC_DATA_CMD, (UINT32)((Reg >> 8) & 0xFF));
  MmioWrite32 (Bar + DW_IC_DATA_CMD, (UINT32)(Reg & 0xFF));
  MmioWrite32 (Bar + DW_IC_DATA_CMD, 0x100);
  MmioWrite32 (Bar + DW_IC_DATA_CMD, 0x300);
  if (!DwWait (Bar, DW_STATUS_RFNE, TRUE)) {
    MmioWrite32 (Bar + DW_IC_ENABLE, 0);
    return FALSE;
  }
  *A = (UINT8)MmioRead32 (Bar + DW_IC_DATA_CMD);
  *B = DwWait (Bar, DW_STATUS_RFNE, TRUE) ? (UINT8)MmioRead32 (Bar + DW_IC_DATA_CMD) : 0;
  MmioWrite32 (Bar + DW_IC_ENABLE, 0);
  return TRUE;
}

STATIC EFI_STATUS EFIAPI PointerReset (
  IN EFI_SIMPLE_POINTER_PROTOCOL *This, IN BOOLEAN Ext)
{
  UINT8 A, B;
  if (mI2cOk && mBar) ElanRead2 (mBar, ETP_I2C_STAND, &A, &B);
  return EFI_SUCCESS;
}

STATIC EFI_STATUS EFIAPI PointerGetState (
  IN EFI_SIMPLE_POINTER_PROTOCOL *This,
  IN OUT EFI_SIMPLE_POINTER_STATE *State)
{
  UINT8 A, B;
  if (State == NULL) return EFI_INVALID_PARAMETER;
  ZeroMem (State, sizeof (*State));
  if (!mI2cOk || !mBar) return EFI_NOT_READY;
  if (!ElanRead2 (mBar, ETP_I2C_DESC, &A, &B)) return EFI_NOT_READY;
  if (A == 0 && B == 0) return EFI_NOT_READY;
  State->RelativeMovementX = (INT32)(INT8)A;
  State->RelativeMovementY = (INT32)(INT8)B;
  mElanOk = TRUE;
  return EFI_SUCCESS;
}

EFI_STATUS EFIAPI BanonElanPointerEntry (
  IN EFI_HANDLE ImageHandle, IN EFI_SYSTEM_TABLE *SystemTable)
{
  EFI_STATUS Status;
  UINT8 A, B;
  mBar = 0;
  mEcOk = EcPresent ();
  mI2cOk = FindI2c (&mBar);
  mElanOk = FALSE;
  if (mI2cOk) mElanOk = ElanRead2 (mBar, ETP_I2C_DESC, &A, &B);
  DEBUG ((DEBUG_INFO, "BanonElan: EC=%a I2C=%a Elan=%a BAR=%lx\n",
          mEcOk ? "yes" : "no", mI2cOk ? "yes" : "no",
          mElanOk ? "yes" : "no", (UINT64)mBar));
  ZeroMem (&mMode, sizeof (mMode));
  mMode.ResolutionX = 8;
  mMode.ResolutionY = 8;
  mMode.LeftButton = TRUE;
  mMode.RightButton = TRUE;
  mPointer.Reset = PointerReset;
  mPointer.GetState = PointerGetState;
  mPointer.Mode = &mMode;
  Status = gBS->CreateEvent (EVT_NOTIFY_WAIT, TPL_NOTIFY, WaitCallback, NULL, &mWaitEvent);
  if (EFI_ERROR (Status)) return Status;
  mPointer.WaitForInput = mWaitEvent;
  mHandle = NULL;
  return gBS->InstallMultipleProtocolInterfaces (
           &mHandle, &gEfiSimplePointerProtocolGuid, &mPointer, NULL);
}
