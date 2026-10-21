#!/bin/bash
# MC3 Switch NSP build entry. Toolchain (hosted, no Docker):
#   devkitARM   toolchain/dkp-root/opt/devkitpro   (devkitpro/devkitarm image)
#   libnx32     toolchain/libnx32/prefix           (built from vita2hos source)
#   mesa32      toolchain/mesa32-prefix/mesa32     (Mesa 20.1 aarch32 portlibs)
#   switch-tools toolchain/v2h-root                (npdmtool/build_pfs0, vita2hos image)
set -e
HERE="$(cd "$(dirname "$0")" && pwd)"
ROOT="$(dirname "$HERE")/mc3-switch"
export DEVKITPRO="$ROOT/toolchain/dkp-root/opt/devkitpro"
export DEVKITARM="$DEVKITPRO/devkitARM"
[ -d "$DEVKITPRO/libnx32" ] || ln -s "$ROOT/toolchain/libnx32/prefix" "$DEVKITPRO/libnx32"
mkdir -p "$HERE/portlibs32"
ln -sfn "$ROOT/toolchain/mesa32-prefix/mesa32/lib" "$HERE/portlibs32/lib"
ln -sfn "$ROOT/toolchain/mesa32-prefix/mesa32/include" "$HERE/portlibs32/include"
export PATH="$DEVKITARM/bin:$PATH"
cd "$HERE"
make -j"$(nproc)"
echo "==> built: $HERE/mc3_nx.nsp"
