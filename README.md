# kip UEFI firmware (custom splash)

GitHub Actions builds a **MrChromebox kip Full ROM** with the slanted HP wordmark as the coreboot logo, then publishes the `.rom` on **Releases**.

Board: **kip** only (HP Chromebook 11 G3/G4, 14 G4). Do not flash this on any other board.

## Download

Open [Releases](https://github.com/12dakota/kip-uefi-firmware/releases) after the workflow finishes (first build can take 30–90 minutes).

## Flash (Linux live USB on the Chromebook)

Write-protect must be off (battery disconnected or WP screw out, depending on the unit).

```bash
cd
curl -LOf https://mrchromebox.tech/firmware-util.sh
sudo bash firmware-util.sh
```

Choose **Flash Custom Firmware** and point at the downloaded `.rom`. That path keeps VPD/HWID.

## Rebuild

Actions → **Build kip UEFI ROM** → Run workflow.
Or push a change under `logos/` or the workflow file.

## Not included

- HP Wolf Security / Sure Start
- A BIOS setup password baked into the image
- Windows flashing
- Official MrChromebox support for this file

Source firmware: [MrChromebox/coreboot](https://github.com/MrChromebox/coreboot)
Docs: https://docs.mrchromebox.tech/docs/support/compiling.html
