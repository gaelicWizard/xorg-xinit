XCOMM!SHELL_CMD

XCOMM
XCOMM This is the standard implementation of a slightly less primitive
XCOMM interface than xinit.  It looks for user .xinitrc and .xserverrc
XCOMM files, then system xinitrc and xserverrc files, else lets xinit choose
XCOMM its default.  The system xinitrc probably does things like check
XCOMM for .Xresources files and merge them in, start up a window manager,
XCOMM and pop a clock and several xterms.
XCOMM
XCOMM Site administrators have never in history written nicer versions.
XCOMM

unset DBUS_SESSION_BUS_ADDRESS
unset SESSION_MANAGER

#ifdef __APPLE__

XCOMM Check for /usr/bin/X11 and BINDIR in the path, if not add them.
XCOMM This allows startx to be placed in a place like /usr/bin or /usr/local/bin
XCOMM and people may use X without changing their PATH.
XCOMM Note that we put our own bin directory at the front of the path, and
XCOMM the standard system path at the back, since if you are using the Xorg
XCOMM server there's a pretty good chance you want to bias the Xorg clients
XCOMM over the old system's clients.

XCOMM First our compiled path
bindir=__bindir__

case $PATH in
    *:$bindir | *:$bindir:* | $bindir:*) ;;
    *) PATH=$bindir:$PATH ;;
esac

XCOMM Now the "old" compiled path
oldbindir=/usr/X11R6/bin

if [ -d "$oldbindir" ] ; then
    case $PATH in
        *:$oldbindir | *:$oldbindir:* | $oldbindir:*) ;;
        *) PATH=$PATH:$oldbindir ;;
    esac
fi
#endif

userclientrc="$HOME/.xinitrc"
sysclientrc=XINITDIR/xinitrc

userserverrc="$HOME/.xserverrc"
sysserverrc=XINITDIR/xserverrc
defaultclient=XTERM
defaultserver=XSERVER
defaultclientargs=""
defaultserverargs=""
defaultdisplay=""
clientargs=""
serverargs=""
vtarg=""

#ifdef __APPLE__

if [ "x$X11_PREFS_DOMAIN" = x ] ; then
    export X11_PREFS_DOMAIN=BUNDLE_ID_PREFIX".X11"
fi

XCOMM First, start caching fonts
: ${X11_CACHE_FONTS:=$(defaults read $X11_PREFS_DOMAIN cache_fonts 2>/dev/null)}
if [ "x${X11_CACHE_FONTS:=1}" = x1 ] ; then
	if [ -x $bindir/font_cache ] ; then
		$bindir/font_cache &
	elif [ -x $bindir/font_cache.sh ] ; then
		$bindir/font_cache.sh &
	elif [ -x $bindir/fc-cache ] ; then
		$bindir/fc-cache &
	fi
fi

if [ -x __libexecdir__/privileged_startx ] ; then
	XCOMM Don't push this into the background because it can cause
	XCOMM a race to create /tmp/.X11-unix
	__libexecdir__/privileged_startx
fi

: ${X11_NO_AUTH:=$(defaults read $X11_PREFS_DOMAIN no_auth 2>/dev/null)}
if [ "x${X11_NO_AUTH:=0}" = x0 ] ; then
    enable_xauth=1
    EXEC=exec
else
    enable_xauth=0
    EXEC=
fi

: ${X11_NOLISTEN_TCP:=$(defaults read $X11_PREFS_DOMAIN nolisten_tcp 2>/dev/null)}
if [ "x${X11_NOLISTEN_TCP:=1}" = x1 ] ; then
    defaultserverargs="$defaultserverargs -nolisten tcp"
fi

: ${X11_ENABLE_IGLX:=$(defaults read $X11_PREFS_DOMAIN enable_iglx 2>/dev/null)}
if [ "x${X11_ENABLE_IGLX:=0}" = x1 ] ; then
	defaultserverargs="$defaultserverargs +iglx +extension GLX"
else
    defaultserverargs="$defaultserverargs -iglx"
fi

: ${X11_DPI:=$(defaults read $X11_PREFS_DOMAIN dpi 2>/dev/null)}
if [ "x${X11_DPI:-}" != x ] ; then
    defaultserverargs="$defaultserverargs -dpi $X11_DPI"
fi

#else
enable_xauth=1
#endif

XCOMM Automatically determine an unused $DISPLAY
d=0
while true ; do
    [ -e "/tmp/.X$d-lock" -o -S "/tmp/.X11-unix/X$d" ] || break
    d=$(($d + 1))
done
defaultdisplay=":$d"
unset d

whoseargs="client"
while [ x"$1" != x ]; do
    case "$1" in
    XCOMM '' required to prevent cpp from treating "/*" as a C comment.
    /''*|\./''*)
	if [ "$whoseargs" = "client" ]; then
	    if [ x"$client" = x ] && [ x"$clientargs" = x ]; then
		client="$1"
	    else
		clientargs="$clientargs $1"
	    fi
	else
	    if [ x"$server" = x ] && [ x"$serverargs" = x ]; then
		server="$1"
	    else
		serverargs="$serverargs $1"
	    fi
	fi
	;;
    --)
	whoseargs="server"
	;;
    *)
	if [ "$whoseargs" = "client" ]; then
	    clientargs="$clientargs $1"
	else
	    XCOMM display must be the FIRST server argument
	    if [ x"$serverargs" = x ] && @@
		 expr "$1" : ':[0-9][0-9]*$' > /dev/null 2>&1; then
		display="$1"
	    else
		serverargs="$serverargs $1"
	    fi
	fi
	;;
    esac
    shift
done

XCOMM process client arguments
if [ x"$client" = x ]; then
    client=$defaultclient

    XCOMM For compatibility reasons, only use startxrc if there were no client command line arguments
    if [ x"$clientargs" = x ]; then
        if [ -f "$userclientrc" ]; then
            client=$userclientrc
        elif [ -f "$sysclientrc" ]; then
            client=$sysclientrc
        fi
    fi
fi

XCOMM if no client arguments, use defaults
if [ x"$clientargs" = x ]; then
    clientargs=$defaultclientargs
fi

XCOMM process server arguments
if [ x"$server" = x ]; then
    server=$defaultserver

#ifdef __linux__
    XCOMM When starting the defaultserver start X on the current tty to avoid
    XCOMM the startx session being seen as inactive:
    XCOMM "https://bugzilla.redhat.com/show_bug.cgi?id=806491"
    tty=$(tty)
    if expr "$tty" : '/dev/tty[0-9][0-9]*$' > /dev/null; then
		tty_num=${tty#/dev/tty}
        vtarg="vt$tty_num -keeptty"
    fi
#endif

    XCOMM For compatibility reasons, only use xserverrc if there were no server command line arguments
    if [ x"$serverargs" = x -a x"$display" = x ]; then
	if [ -f "$userserverrc" ]; then
	    server=$userserverrc
	elif [ -f "$sysserverrc" ]; then
	    server=$sysserverrc
	fi
    fi
fi

XCOMM if no server arguments, use defaults
if [ x"$serverargs" = x ]; then
    serverargs=$defaultserverargs
fi

XCOMM if no vt is specified add vtarg (which may be empty)
have_vtarg="no"
for i in $serverargs; do
    if expr "$i" : 'vt[0-9][0-9]*$' > /dev/null; then
        have_vtarg="yes"
    fi
done
if [ "$have_vtarg" = "no" ]; then
    serverargs="$serverargs $vtarg"
fi

XCOMM if no display, use default
if [ x"$display" = x ]; then
    display=$defaultdisplay
fi

if [ x"$enable_xauth" = x1 ] ; then
	: ${XAUTHORITY:=$HOME/.Xauthority}
	export XAUTHORITY

    removelist=

    XCOMM set up default Xauth info for this machine
	hostname=`uname -n`

    authdisplay=${display:-:0}
#if defined(HAS_COOKIE_MAKER) && defined(MK_COOKIE)
    mcookie=`MK_COOKIE`
#else
    if [ -r /dev/urandom ]; then
        mcookie=`dd if=/dev/urandom bs=16 count=1 2>/dev/null | hexdump -e \\"%08x\\"`
    else
        mcookie=`dd if=/dev/random bs=16 count=1 2>/dev/null | hexdump -e \\"%08x\\"`
    fi
#endif
    if test x"$mcookie" = x; then
        echo "Couldn't create cookie"
        exit 1
    fi
    dummy=0

    XCOMM create a file with auth information for the server. ':0' is a dummy.
    xserverauthfile="${XDG_RUNTIME_DIR:-${TMPDIR:-$HOME}}/.Xauthority.$$"
    trap "rm -f '$xserverauthfile'" HUP INT QUIT ILL TRAP KILL BUS TERM
    xauth -q -f "$xserverauthfile" << EOF
add :$dummy . $mcookie
EOF
#if defined(__CYGWIN__)
    xserverauthfilequoted=$(echo ${xserverauthfile} | sed "s/'/'\\\\''/g")
    serverargs=${serverargs}" -auth '"${xserverauthfilequoted}"'"
#else
    serverargs=${serverargs}" -auth "${xserverauthfile}
#endif

    XCOMM now add the same credentials to the client authority file
    XCOMM if '$displayname' already exists do not overwrite it as another
    XCOMM server may need it. Add them to the '$xserverauthfile' instead.
    for displayname in $authdisplay $hostname$authdisplay; do
        authcookie=`XAUTH list "$displayname" @@
        | sed -n "s/.*$displayname[[:space:]*].*[[:space:]*]//p"` 2>/dev/null;
        if [ "z${authcookie}" = "z" ] ; then
            XAUTH -q << EOF 
add $displayname . $mcookie
EOF
        removelist="$displayname $removelist"
        else
            dummy=$(($dummy+1));
            XAUTH -q -f "$xserverauthfile" << EOF
add :$dummy . $authcookie
EOF
        fi
    done
fi

#if defined(__CYGWIN__)
eval $EXEC XINIT \"$client\" $clientargs -- \"$server\" $display $serverargs
#else
$EXEC XINIT "$client" $clientargs -- "$server" $display $serverargs
#endif
retval=$?

if [ x"$enable_xauth" = x1 ] ; then
    if [ x"$removelist" != x ]; then
        XAUTH remove $removelist
    fi
    if [ x"$xserverauthfile" != x ]; then
        rm -f "$xserverauthfile"
    fi
fi

/*
 * various machines need special cleaning up
 */
#ifdef __linux__
if command -v deallocvt > /dev/null 2>&1; then
    deallocvt
fi
#endif

#if defined(sun)
kbd_mode -a
#endif

exit $retval

