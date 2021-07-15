XCOMM!SHELL_CMD

#ifdef __APPLE__
cd "${HOME:=~}" XCOMM Start everything inside $HOME

XCOMM if we are being run directly, and we are not a login shell, then re-exec as a login shell.
if [[ "${BASH_SOURCE[0]}" == "$0" ]] && ! shopt -q login_shell; then
	set -- "${0}" "${@}"
	case $(basename "${SHELL}") in
		bash)          exec -l "${SHELL}" --login -c 'exec "${@}"' - "${@}" ;;
		ksh|sh|zsh)    exec -l "${SHELL}" -c 'exec "${@}"' - "${@}" ;;
		csh|tcsh)      exec -l "${SHELL}" -c 'exec $argv:q' "${@}" ;;
		es|rc)         exec -l "${SHELL}" -l -c 'exec $*' "${@}" ;;
		*)             exec    "${@}" ;;
	esac
fi
#endif

userresources=${XDG_CONFIG_HOME:-$HOME/.}${XDG_CONFIG_HOME+/X11/}Xresources
usermodmap=${XDG_CONFIG_HOME:-$HOME/.}${XDG_CONFIG_HOME+/X11/}Xmodmap
sysresources=XINITDIR/.Xresources
sysmodmap=XINITDIR/.Xmodmap

XCOMM merge in defaults and keymaps

if [ -f $sysresources ]; then
    XRDB -merge $sysresources
fi

if [ -f $sysmodmap ]; then
    XMODMAP $sysmodmap
fi

if [ -f "$userresources" ]; then
    XRDB -merge "$userresources"
fi

if [ -f "$usermodmap" ]; then
    XMODMAP "$usermodmap"
fi

XCOMM start some nice programs

if [ -d XINITDIR/xinitrc.d ] ; then
	for f in XINITDIR/xinitrc.d/?*.sh ; do
		[ -x "$f" ] && . "$f"
	done
	unset f
fi

TWM &
XCLOCK -geometry 50x50-1+1 &
XTERM -geometry 80x50+494+51 &
XTERM -geometry 80x20+494-0 &
exec XTERM -geometry 80x66+0+0 -name login
