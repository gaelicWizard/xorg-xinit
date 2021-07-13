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

userresources=$HOME/.Xresources
usermodmap=$HOME/.Xmodmap
sysresources=XINITDIR/.Xresources
sysmodmap=XINITDIR/.Xmodmap

XCOMM merge in defaults and keymaps

if [ -f $sysresources ]; then
#ifdef __APPLE__
    if [ -x /usr/bin/cpp ] ; then
        XRDB -merge $sysresources
    else
        XRDB -nocpp -merge $sysresources
    fi
#else
    XRDB -merge $sysresources
#endif
fi

if [ -f $sysmodmap ]; then
    XMODMAP $sysmodmap
fi

if [ -f "$userresources" ]; then
#ifdef __APPLE__
    if [ -x /usr/bin/cpp ] ; then
        XRDB -merge "$userresources"
    else
        XRDB -nocpp -merge "$userresources"
    fi
#else
    XRDB -merge "$userresources"
#endif
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
