<?xml version="1.0" encoding="UTF-8"?>
<!DOCTYPE plist PUBLIC "-//Apple Computer//DTD PLIST 1.0//EN" "http://www.apple.com/DTDs/PropertyList-1.0.dtd">
<plist version="1.0">
<dict>
	<key>Label</key>
	<string>BUNDLE_ID_PREFIX.startx</string>
	<key>ProgramArguments</key>
	<array>
#ifdef LAUNCHAGENT_XINIT_ONLY
		<string>__bindir__/xinit</string>
#else
		<string>__libexecdir__/launchd_startx</string>
		<string>__bindir__/startx</string>
#endif
		<string>--</string>
#ifdef LAUNCHAGENT_XSERVER_PATH
		<string>LAUNCHAGENT_XSERVER_PATH</string>
#endif
#ifdef LAUNCHAGENT_XAUTH
		<string>-auth</string>
		<string>LAUNCHAGENT_XAUTH</string>
#endif
	</array>
	<key>EnvironmentVariables</key>
	<dict>
		<key>X11_PREFS_DOMAIN</key>
		<string>BUNDLE_ID_PREFIX.X11</string>
		<key>XINITRC</key>
		<string>__sysconfdir__/X11/xinit/xinitrc</string>
	</dict>
	<key>Sockets</key>
	<dict>
		<key>BUNDLE_ID_PREFIX:0</key>
		<dict>
			<key>SecureSocketWithKey</key>
			<string>DISPLAY</string>
		</dict>
	</dict>
	<key>ServiceIPC</key>
	<true/>
	<key>EnableTransactions</key>
	<true/>
</dict>
</plist>
