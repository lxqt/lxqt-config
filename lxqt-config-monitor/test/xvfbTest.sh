#!/bin/sh
# Checks the X11 backend on a private X server, so real monitors are not touched.
# Xvfb has a single output and mode, so the primary flag is what can be observed changing.
# Usage: xvfb-run -a -s '-screen 0 1280x1024x24 +extension RANDR' dbus-run-session -- sh xvfbTest.sh /path/to/lxqt-config-monitor
monitor=${1:-lxqt-config-monitor}
export XDG_CONFIG_HOME
XDG_CONFIG_HOME=$(mktemp -d)
trap 'rm -rf "$XDG_CONFIG_HOME"' EXIT
mkdir -p "$XDG_CONFIG_HOME/lxqt"
conf="$XDG_CONFIG_HOME/lxqt/lxqt-config-monitor.conf"

check() {
    if [ "$(xrandr | grep -c ' connected primary')" = "$2" ]; then
        echo "ok: $1"
    else
        echo "FAIL: $1"
        xrandr
        exit 1
    fi
}

save() {
    cat > "$conf" <<EOF
[currentConfig]
settings\\size=1
settings\\1\\name=screen
settings\\1\\connected=true
settings\\1\\enabled=true
settings\\1\\primary=$1
settings\\1\\xPos=0
settings\\1\\yPos=0
settings\\1\\currentModeWidth=1280
settings\\1\\currentModeHeight=1024
settings\\1\\rotation=1
EOF
}

check "starts without a primary monitor" 0

save true
"$monitor" -l
check "saved layout is applied" 1

save false
"$monitor" -l
check "saved layout is applied again" 0

# Without anything saved, the first monitor becomes primary in extended mode.
: > "$conf"
"$monitor" -l
check "extended fallback" 1
