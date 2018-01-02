#owclauncher

A Native Messaging Host (launcher) for ["Open with"](https://github.com/darktrojan/openwith/) add-on

Lightweight (17kb non-compressed binary), C-based, compatible with [Firefox](https://addons.mozilla.org/firefox/addon/open-with/), [Chrome & Opera](https://chrome.google.com/webstore/detail/open-with/cogjlncmljjnjpbgppagklanlcbchlno)

##Motivation
["Open with"](https://github.com/darktrojan/openwith/) is a great add-on for calling external programs from the browser.

It comes very handy when you need to open a page on another browser, pass a link to a specific program, etc ...

Unfortunately, starting from Firefox 57, only WebExtensions add-ons are enabled.

Which means that a 3rd party, standalone app (aka Native Messaging Host) is now necessary for the add-on to work.

The current solution implemented by "Open With" is a python Script. This (obviously) require Python and the user must use the command line to setup the script.

This project offers a lightweight standalone binary along with simple installation scripts (InstallWin.bat & UninstallWin.bat for Windows)

##Releases
See the [release page](https://github.com/mbooga/owclauncher/releases) for binary downloads and history of releases.

Currently only Windows is supported.

Porting to Linux and Mac OS is not planned, but should be straightforward.


##Building
In Windows, you can use [Mingw](https://sourceforge.net/projects/mingw-w64/files/) to build the binary.

```
gcc -s owclauncher.c -o owclauncher.exe
```

##Install & Uninstall

Use InstallWin.bat and UninstallWin.bat for easy setup & removal.

The default installation directory on Windows is:

%AppData%\owclauncher\ (usually translates to C:\Users\username\AppData\Roaming\owclauncher) The installation is user-specific. Administrator privileges are NOT required.

##Limitations
"Look for Browsers" feature is not yet implemented. You have to add other browsers manually.

##Thanks
Specials thanks to [Geoff Lankow](https://darktrojan.github.io/) for his great work on Open With, and for keeping it free and open.