#!/bin/bash
# run_mc3.sh — Modern Combat 3: Fallen Nation on Nintendo Switch (Switchroot Ubuntu / L4T)
#
# Standalone launcher for the armhf PortMaster loader, adapted for Ubuntu on Switch.
# Removes every PortMaster runtime dependency (control.txt, tasksetter, gptokeyb,
# pm_finish) — the loader reads the gamepad itself through SDL_GameController and
# exits through the in-game pause menu / SDL_QUIT.
#
# Requirements (installed by setup_mc3_switch.sh, run once as root on the Switch):
#   - Ubuntu 20.04+ (64-bit) with armhf multiarch enabled
#   - armhf: libc6, libsdl2-2.0-0, libegl1, libgles2, libgl1-mesa-dri, libgbm1,
#            libdrm2, libudev1, libasound2 (audio)
#   - a Pro Controller (USB recommended) or Joy-Con pair connected as a gamepad
#
# Game data (bring your own, Gameloft files are never distributed):
#   lib/armeabi-v7a/libModernCombat3.so          (from the v1.1.7g APK)
#   main.1120.com.gameloft.android.ANMP.GloftM3HM.obb
#   patch.11428.com.gameloft.android.ANMP.GloftM3HM.obb
# If these are missing, the launcher runs eapx.py to import them from a donor
# APK/zip dropped in the game folder (same first-boot flow as PortMaster).

set -uo pipefail

# ---------------------------------------------------------------------------
GAMEDIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
cd "$GAMEDIR" || exit 1

: > "$GAMEDIR/log.txt"
exec > "$GAMEDIR/log.txt" 2>&1

echo "Modern Combat 3 (Switch/L4T) launcher starting"

# System survey — kept short: machine, GL libs actually present.
echo "sys: uname: $(uname -rm 2>/dev/null)"
_sys_os=$(sed -n 's/^PRETTY_NAME="\{0,1\}\([^"]*\)"\{0,1\}$/\1/p' /etc/os-release 2>/dev/null | head -n 1)
echo "sys: os: ${_sys_os:-unknown}"
for _gldir in /usr/lib/arm-linux-gnueabihf /usr/local/lib/arm-linux-gnueabihf; do
  [ -d "$_gldir" ] || continue
  _sys_gl=$(ls "$_gldir" 2>/dev/null \
      | grep -E '^lib(EGL|GLESv1_CM|GLESv2|gbm\.|drm\.)' | tr '\n' ' ')
  echo "sys: gl $_gldir: ${_sys_gl:-(no GL libraries)}"
done
echo "sys: dri: $(ls -la /dev/dri 2>/dev/null | sed 1d | tr -s ' \n' ' ')"
echo "sys: mem: $(grep -E '^Mem(Total|Available)' /proc/meminfo 2>/dev/null | tr -s ' \n' ' ')"
_sys_sdl=$(ls "$GAMEDIR"/libs.armhf/libSDL2*.so* 2>/dev/null | xargs -n1 basename 2>/dev/null | tr '\n' ' ')
echo "sys: sdl bundled: ${_sys_sdl:-none (using system libsdl2-2.0-0:armhf)}"

chmod +x "$GAMEDIR/mc3" 2>/dev/null
PORT_VERSION=$(LD_LIBRARY_PATH="$GAMEDIR/libs.armhf${LD_LIBRARY_PATH:+:$LD_LIBRARY_PATH}" \
    "$GAMEDIR/mc3" --version 2>/dev/null) || PORT_VERSION=""
echo "Modern Combat 3 port v${PORT_VERSION:-unknown}"

# ---------------------------------------------------------------------------
# Game data. Same contract as the PortMaster release: the player supplies the
# v1.1.7g APK plus both OBBs; eapx stages and validates them.
GAME_SO="$GAMEDIR/lib/armeabi-v7a/libModernCombat3.so"
OBB_MAIN="$GAMEDIR/main.1120.com.gameloft.android.ANMP.GloftM3HM.obb"
OBB_PATCH="$GAMEDIR/patch.11428.com.gameloft.android.ANMP.GloftM3HM.obb"

if [ ! -f "$GAME_SO" ] || [ ! -f "$OBB_MAIN" ] || [ ! -f "$OBB_PATCH" ]; then
  echo "Game data absent; starting content-based import"
  if ! command -v python3 >/dev/null 2>&1; then
    echo "Game-data import failed: python3 is unavailable (install python3 on the Switch OS)"
    exit 1
  fi
  if [ ! -f "$GAMEDIR/eapx.py" ] || [ ! -f "$GAMEDIR/mc3.eapx.json" ]; then
    echo "Game-data import failed: eapx.py or mc3.eapx.json missing from the package"
    exit 1
  fi
  if ! python3 "$GAMEDIR/eapx.py" install \
       --recipe "$GAMEDIR/mc3.eapx.json" \
       --game-dir "$GAMEDIR" --tty none; then
    echo "Game-data import failed. Put your own v1.1.7g APK plus both OBB expansion files in:"
    echo "  $GAMEDIR"
    echo "Filenames do not matter; eapx detects them by content. See eapx.log."
    exit 1
  fi
fi

if [ ! -f "$GAME_SO" ] || [ ! -f "$OBB_MAIN" ] || [ ! -f "$OBB_PATCH" ]; then
  echo "Game-data check failed: expected files missing after import"
  [ -f "$GAME_SO" ]   || echo "  missing: $GAME_SO"
  [ -f "$OBB_MAIN" ]  || echo "  missing: $OBB_MAIN"
  [ -f "$OBB_PATCH" ] || echo "  missing: $OBB_PATCH"
  exit 1
fi

# The loader patches fixed offsets in the DRM, so only this exact build works.
EXPECTED_SHA1="be0d5e8779899e081a538b3930ec711d2df8aeb4"
GAME_SHA1=$(sha1sum "$GAME_SO" 2>/dev/null | cut -d' ' -f1)
if [ -n "$GAME_SHA1" ] && [ "$GAME_SHA1" != "$EXPECTED_SHA1" ]; then
  echo "Game-data check failed: sha1=$GAME_SHA1 expected=$EXPECTED_SHA1"
  echo "This port needs the 1.1.7g native library."
  exit 1
fi

# ---------------------------------------------------------------------------
# Runtime environment. The bundled libs.armhf are the port's own dependency
# closure; SDL2 comes from the armhf multiarch packages installed by the setup
# script. The loader's RUNPATH already points at /usr/lib/arm-linux-gnueabihf.
export LD_LIBRARY_PATH="$GAMEDIR/libs.armhf${LD_LIBRARY_PATH:+:$LD_LIBRARY_PATH}"

export LOADER_TRACE=1
# Aspect-correct scaling on panels that are not the R36S's 640x480.
export MC3_SCALE="${MC3_SCALE:-fit}"            # fit | stretch | N (integer)
export MC3_RPAD_SCALE="${MC3_RPAD_SCALE:-100}"  # camera speed, percent
export MC3_RPAD_INVERT_Y="${MC3_RPAD_INVERT_Y:-0}"
export MC3_NO_AUDIO="${MC3_NO_AUDIO:-0}"
# The portbase interposes six GLES1-table calls (glViewport, glScissor,
# glClear, glTexImage2D, glDrawArrays, glDrawElements); single dispatch routes
# them through the SDL GLES2 context instead of a separate GLES1 provider.
export MC3_GL_SINGLE_DISPATCH=1

# Optional controller mapping override (SDL2 usually recognises the Pro
# Controller / Joy-Cons from its built-in database).
if [ -f "$GAMEDIR/controller.map" ]; then
  export SDL_GAMECONTROLLERCONFIG="$(cat "$GAMEDIR/controller.map")"
fi

# Audio: route through a running user audio server if present, else ALSA dmix.
_MC3_PULSE=""
for _pulse in "${XDG_RUNTIME_DIR:-}/pulse/native" /run/pulse/native /var/run/pulse/native; do
  [ -n "$_pulse" ] && [ -S "$_pulse" ] && { _MC3_PULSE="$_pulse"; break; }
done
if [ -n "$_MC3_PULSE" ]; then
  export SDL_AUDIODRIVER=alsa
  export PULSE_SERVER="unix:$_MC3_PULSE"
  echo "Audio: routing through PulseAudio ($_MC3_PULSE)"
else
  export AUDIODEV="${AUDIODEV:-plug:dmix}"
  export SDL_AUDIODRIVER="${SDL_AUDIODRIVER:-alsa}"
  echo "Audio: ALSA dmix (no PulseAudio socket detected)"
fi

# Video backend override. SDL picks x11/wayland/kmsdrm by what the session has;
# force kmsdrm explicitly on a console-only boot if the default misbehaves:
#   SDL_VIDEODRIVER=kmsdrm ./run_mc3.sh

echo "Starting the loader: $GAMEDIR/mc3 $GAMEDIR"
"$GAMEDIR/mc3" "$GAMEDIR"
GAME_RC=$?

unset LD_LIBRARY_PATH SDL_GAMECONTROLLERCONFIG
echo "Game exited with code $GAME_RC (log in $GAMEDIR/log.txt)"
exit "$GAME_RC"
