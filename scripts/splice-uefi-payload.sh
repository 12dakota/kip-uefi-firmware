#!/bin/bash
# Splice a UEFIPAYLOAD.fd into a COPY of an 8MiB kip Full ROM.
# This does not make Mu official. Test only. Keep a backup.
set -euo pipefail
ROM=${1:?usage: $0 kip.rom UEFIPAYLOAD.fd}
FD=${2:?usage: $0 kip.rom UEFIPAYLOAD.fd}
OUT=${3:-kip-experimental-mu-payload.rom}
CBFSTOOL=${CBFSTOOL:-cbfstool}

cp -v "$ROM" "$OUT"
"$CBFSTOOL" "$OUT" print || true
"$CBFSTOOL" "$OUT" remove -n fallback/payload || true
"$CBFSTOOL" "$OUT" add-payload -n fallback/payload -f "$FD" -c lzma
"$CBFSTOOL" "$OUT" print
ls -l "$OUT"
echo "EXPERIMENTAL. Flash only if size is 8388608 and you have a backup + WP off."
