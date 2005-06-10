XCOMM!SHELL_CMD
XCOMM $Xorg: xinitrc.cpp,v 1.3 2000/08/17 19:54:30 cpqbld Exp $

userresources=$HOME/.Xresources
usermodmap=$HOME/.Xmodmap
sysresources=XINITDIR/.Xresources
sysmodmap=XINITDIR/.Xmodmap

XCOMM merge in defaults and keymaps

if [ -f $sysresources ]; then
    BINDIR/xrdb -merge $sysresources
fi

if [ -f $sysmodmap ]; then
    BINDIR/xmodmap $sysmodmap
fi

if [ -f $userresources ]; then
    BINDIR/xrdb -merge $userresources
fi

if [ -f $usermodmap ]; then
    BINDIR/xmodmap $usermodmap
fi

XCOMM start some nice programs

#if defined(__SCO__) || defined(__UNIXWARE__)
if [ -r /etc/default/xdesktops ]; then
  . /etc/default/xdesktops
fi

if [ -r $HOME/.x11rc ]; then
  . $HOME/.x11rc
else
  if [ -r /etc/default/X11 ]; then
  . /etc/default/X11
  fi
fi

#if defined(__SCO__)
if [ -n "$XSESSION" ]; then
  case "$XSESSION" in
    [Yy][Ee][Ss])
      [ -x /usr/bin/X11/scosession ] && exec /usr/bin/X11/scosession
      ;;
  esac
fi

if [ -n "$XDESKTOP" ]; then
  exec `eval echo $"$XDESKTOP"`
else
  if [ -x /usr/bin/X11/pmwm -a -x /usr/bin/X11/scoterm ]; then
    /usr/bin/X11/scoterm 2> /dev/null &
    exec /usr/bin/X11/pmwm    2> /dev/null
  fi
fi
#elif defined(__UNIXWARE__)
if [ -n "$XDESKTOP" ]; then
  exec `eval echo $"$XDESKTOP"`
else
  if [ -x /usr/X/bin/pmwm ]; then
    exec /usr/X/bin/pmwm    2> /dev/null
  fi
fi
#endif

XCOMM This is the fallback case if nothing else is executed above
#endif /* !defined(__SCO__)  && !defined(__UNIXWARE__) */
BINDIR/twm &
BINDIR/xclock -geometry 50x50-1+1 &
BINDIR/xterm -geometry 80x50+494+51 &
BINDIR/xterm -geometry 80x20+494-0 &
exec BINDIR/xterm -geometry 80x66+0+0 -name login
