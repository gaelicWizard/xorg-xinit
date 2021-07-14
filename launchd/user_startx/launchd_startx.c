/* Copyright (c) 2011-2012 Apple Inc.
 *
 * Permission is hereby granted, free of charge, to any person
 * obtaining a copy of this software and associated documentation files
 * (the "Software"), to deal in the Software without restriction,
 * including without limitation the rights to use, copy, modify, merge,
 * publish, distribute, sublicense, and/or sell copies of the Software,
 * and to permit persons to whom the Software is furnished to do so,
 * subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be
 * included in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
 * NONINFRINGEMENT.  IN NO EVENT SHALL THE ABOVE LISTED COPYRIGHT
 * HOLDER(S) BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
 * WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
 * DEALINGS IN THE SOFTWARE.
 *
 * Except as contained in this notice, the name(s) of the above
 * copyright holders shall not be used in advertising or otherwise to
 * promote the sale, use or other dealings in this Software without
 * prior written authorization.
 */

#include <CoreFoundation/CoreFoundation.h>

#ifdef HAVE_CONFIG_H
# include "config.h"
#endif

#include <asl.h>
#include <unistd.h>
#include <stdio.h>
#include <assert.h>
#include <sys/wait.h>
#include <string.h>
#include <stdlib.h>
#include <spawn.h>

static char *
command_from_prefs(const char *key, const char *default_value);

#define DEFAULT_STARTX X11BINDIR "/startx -- " X11BINDIR "/Xquartz"

static const char *x11_prefs_domain = NULL;

int main(int argc, char **argv, char **envp) {
    aslclient aslc;
    const char *newargv[4];
    pid_t child;
    int pstat;

    if(argc < 2 || strcmp(argv[1], "--help") == 0) {
        fprintf(stderr, "Usage: %s prog [args...]\n", argv[0]);
        exit(EXIT_FAILURE);
    }

    aslc = asl_open(BUNDLE_ID_PREFIX".startx", BUNDLE_ID_PREFIX, ASL_OPT_NO_DELAY);

    asl_log_descriptor(aslc, NULL, ASL_LEVEL_INFO, STDOUT_FILENO, ASL_LOG_DESCRIPTOR_WRITE);
    asl_log_descriptor(aslc, NULL, ASL_LEVEL_NOTICE, STDERR_FILENO, ASL_LOG_DESCRIPTOR_WRITE);

    /* https://github.com/XQuartz/XQuartz/issues/114 */
    char const * const home = getenv("HOME");
    assert(home);
    chdir(home);

	if (!(x11_prefs_domain = getenv("X11_PREFS_DOMAIN")))
		setenv("X11_PREFS_DOMAIN", x11_prefs_domain = BUNDLE_ID_PREFIX".X11", FALSE);

    newargv[0] = command_from_prefs("login_shell", SHELL_CMD);
    newargv[1] = "-c";
    newargv[2] = command_from_prefs("startx_script", DEFAULT_STARTX);
    newargv[3] = NULL;

    assert(posix_spawnp(&child, newargv[0], NULL, NULL, (char *const *)newargv, envp) == 0);
    wait4(child, &pstat, 0, (struct rusage *)0);

    return pstat;
}

static char *
command_from_prefs(const char *key, const char *default_value)
{
    char *command = NULL;

    CFStringRef cfKey;
    CFPropertyListRef PlistRef;

    if (!key)
        return NULL;

    cfKey = CFStringCreateWithCString(NULL, key, kCFStringEncodingASCII);

    if (!cfKey)
        return NULL;

    PlistRef = CFPreferencesCopyAppValue(cfKey,
                                         x11_prefs_domain);

    if ((PlistRef == NULL) ||
        (CFGetTypeID(PlistRef) != CFStringGetTypeID())) {
        CFStringRef cfDefaultValue = CFStringCreateWithCString(
            NULL, default_value, kCFStringEncodingASCII);
        int len = strlen(default_value) + 1;

        if (!cfDefaultValue)
            goto command_from_prefs_out;

        CFPreferencesSetAppValue(cfKey, cfDefaultValue,
                                 x11_prefs_domain);
        CFPreferencesAppSynchronize(x11_prefs_domain);
        CFRelease(cfDefaultValue);

        command = (char *)malloc(len * sizeof(char));
        if (!command)
            goto command_from_prefs_out;
        strcpy(command, default_value);
    }
    else {
        int len = CFStringGetLength((CFStringRef)PlistRef) + 1;
        command = (char *)malloc(len * sizeof(char));
        if (!command)
            goto command_from_prefs_out;
        CFStringGetCString((CFStringRef)PlistRef, command, len,
                           kCFStringEncodingASCII);
    }

command_from_prefs_out:
    if (PlistRef)
        CFRelease(PlistRef);
    if (cfKey)
        CFRelease(cfKey);
    return command;
}
