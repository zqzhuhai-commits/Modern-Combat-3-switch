#!/bin/bash
# setup_mc3_switch.sh — one-time environment setup for Modern Combat 3 on
# Switchroot Ubuntu (L4T). Run as root on the Switch:
#
#   sudo bash setup_mc3_switch.sh
#
# What it does:
#   1. Adds the armhf (32-bit ARM) multiarch — the game engine is a 32-bit
#      armeabi-v7a library, so the whole loader stack must be armhf.
#   2. Installs the armhf runtime the loader needs: glibc, SDL2, Mesa EGL/GLES
#      (software rendering — llvmpipe/kms_swrast, since the closed NVIDIA L4T
#      blob ships no 32-bit GL), DRM/GBM/udev and ALSA.
#   3. Puts the current user in the video/input groups for /dev/dri and evdev.
#
# After this, copy the MC3-Switch-L4T folder anywhere (e.g. ~/mc3) and run:
#   bash ~/mc3/run_mc3.sh
#
# Ubuntu 18.04 is NOT supported: the bundled loader needs glibc >= 2.31
# (built on Debian bullseye for portability across CFWs).

set -uo pipefail

if [ "$(id -u)" -ne 0 ]; then
  echo "Run me as root: sudo bash $0"
  exit 1
fi

echo "== Modern Combat 3 Switch/L4T environment setup =="

# --- OS floor ---------------------------------------------------------------
_ver=$(sed -n 's/^VERSION_ID="\{0,1\}\([0-9.]*\)"\{0,1\}$/\1/p' /etc/os-release 2>/dev/null)
echo "Ubuntu VERSION_ID: ${_ver:-unknown}"
case "$_ver" in
  18.04) echo "ERROR: Ubuntu 18.04 has glibc 2.27; the loader needs >= 2.31. Use Ubuntu 20.04+." ; exit 1 ;;
  20.04|22.04|24.04|"" ) echo "OK: glibc floor satisfied." ;;
  *) echo "WARNING: untested Ubuntu version ($_ver); continuing anyway." ;;
esac

# --- armhf multiarch --------------------------------------------------------
echo "== Adding armhf architecture =="
dpkg --add-architecture armhf
apt-get update -qq

# --- armhf packages ---------------------------------------------------------
# libsdl2-2.0-0 pulls in its own dep tree (libasound2, libpulse0, libdrm2,
# libudev1, X11/Wayland libs...). Mesa pieces are named explicitly so the
# 32-bit EGL/GLES2 stack with software drivers is guaranteed present.
echo "== Installing armhf runtime packages =="
DEBIAN_FRONTEND=noninteractive apt-get install -y \
  libc6:armhf \
  libsdl2-2.0-0:armhf \
  libegl1:armhf \
  libgles2:armhf \
  libgl1-mesa-dri:armhf \
  libglx-mesa0:armhf \
  libgbm1:armhf \
  libdrm2:armhf \
  libudev1:armhf \
  libasound2:armhf \
  libpulse0:armhf \
  python3 \
  || { echo "apt install failed — check network/apt sources."; exit 1; }

# --- verify the 32-bit pieces are actually resolvable -----------------------
echo "== Verifying armhf GL stack =="
_LD=/lib/ld-linux-armhf.so.3
if [ ! -e "$_LD" ]; then
  echo "ERROR: $(_LD) not found — armhf base not installed."
  exit 1
fi
for _so in libEGL.so.1 libGLESv2.so.2 libSDL2-2.0.so.0 libgbm.so.1; do
  _f=$(ls /usr/lib/arm-linux-gnueabihf/$_so* 2>/dev/null | head -n1)
  echo "  armhf $_so -> ${_f:-MISSING}"
done
ls /usr/lib/arm-linux-gnueabihf/dri/kms_swrast_dri.so >/dev/null 2>&1 \
  && echo "  kms_swrast_dri.so present (software EGL)" \
  || echo "  WARNING: kms_swrast_dri.so not found — check libgl1-mesa-dri:armhf"

# --- groups -----------------------------------------------------------------
echo "== Group membership =="
for _g in video input render; do
  if getent group "$_g" >/dev/null 2>&1; then
    usermod -aG "$_g" "${SUDO_USER:-$(whoami)}" 2>/dev/null \
      && echo "  added user to group $_g" \
      || echo "  (group $_g exists; re-login to activate)"
  fi
done

cat <<'EOF'

== Done. Next steps ==
1. Re-login (or reboot) so the video/input groups take effect.
2. Put the MC3-Switch-L4T folder somewhere readable, e.g.:
     mkdir -p ~/mc3 && cp -r <package>/MC3-Switch-L4T/* ~/mc3/
3. Supply the game data (drop your v1.1.7g APK + both OBBs into ~/mc3/ —
   first launch imports them automatically), then:
     bash ~/mc3/run_mc3.sh
4. If the window does not appear, try forcing the video backend:
     SDL_VIDEODRIVER=kmsdrm bash ~/mc3/run_mc3.sh
   (run from a VT/console session for kmsdrm; x11/wayland are picked
   automatically when a desktop session is running)
EOF
