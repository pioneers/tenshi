Pioneers in Engineering Control System Next
===========================================


## UI Development Quick-Start (Angel-Player)
The following steps should work on Linux, OS X, and Windows (cygwin).

1. Install `firefox` (v30+), `git` (which comes with cygwin / git bash on
Windows).
2. `git clone [this repo]`
3. Run `dev.sh`
4. There's no need to run `build.sh`
5. Reload Angel-Player without restarting it by clicking the square at the
   bottom right, and clicking the `Reload` text that appears.


## Windows
If you're on Windows and can't run `dev.sh` (not using cygwin), run `dev.bat`


## UI Development Slow-Start (and release build instructions)
1. Install essential and angel-player dependencies below.
2. Run `build-angel-player.sh` once.
3. Run `./run-angel-player` or `./dev.sh` without `build.sh` to see changes.


## Building the Whole System
1. Install all dependencies below.
2. Run `build.sh`.


## Setting up Angel Player debugger
Angel Player can be debugged by using the Firefox remote debug tools that are
normally used to debug Thunderbird/Firefox OS. To enable this on the Firefox
side:

1. Open the developer tools (Ctrl+Shift+I)
2. Click the "Settings" sprocket in the top-left.
3. Under advanced, enable remote debugging (you may also need to enable chrome
   debugging).

To enable remote debugging in Angel Player, either launch the in-tree version
of the code (`./run-angel-player` or `./dev.sh`) or set
"devtools.debugger.remote-enabled" and "devtools.chrome.enabled" to "true" in
about:config. If for some reason you already have an application listening on
port 6000 on your computer, you can also change "devtools.debugger.remote-port"
to an alternate port.

To connect the remote debugger, restart firefox and select `Tools->Web
Developer->Connect...` in Firefox. Set the host to localhost and the port to
6000 (unless you changed the port above) and click connect. Select `Main
Process` from the list that appears.


## Full System Setup (i.e. how to run build.sh)
Are you on Ubuntu? Use `install_build_deps.sh`! (You must be connected to the
Internet for this to work.) After running install_build_deps.sh, you also need
to manually:

1. Run emcc. This creates some config files necessary for it to work.
2. Run EAGLE. This creates a directory in your home so that it doesn't prompt
   you while building (and stalling the build).
3. If you are running on Ubuntu, make sure to add yourself to the dialout group to
   access your USB serial port (usually /dev/ttyUSB0). This can be done using
   the command `sudo useradd -G dialout $USER`
4. The whole build system can be run using `./build.sh`. Build results are in
   build/artifacts.

If you're not on Ubuntu, it's more difficult. Basically, install the
equivalents of the dependencies below.


## Full System Dependencies

For the time being, `build.sh` is not modular enough, and needs all of these
dependencies to exist to do anything.

However, Angel-Player can be built with only its listed dependencies using
`./build-angel-player.sh`.

### To do anything
* PyYAML
* coreutils, build-essential, etc.
* python (2.7 recommended)
* tar, bz2, etc.
* wget

### To build release Angel-Player
* emscripten (get from http://rqou.com/emscripten-bin-18apr2014.tar.bz2)

### To lint Angel-Player
* nodejs (currently using v0.10.26, from the chris-lea/node.js PPA)
* jshint (install from npm, currently using 2.1.11)
* csslint (install from npm, currently using 0.10.0)
* Java JRE

### To test Angel-Player
* libX11, Xvfb (also used to build eda)

### To build eda
* EAGLE (currently using v6.5.0)
* libX11, Xvfb (also used to test Angel-Player)

### To build embedded software
* project-tenshi-tools (get from Jenkins)
* avr-gcc (currently using the nonolith/avr-toolchain PPA)

### To lint the build system itself
* pep8
