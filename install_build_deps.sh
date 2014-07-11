#!/bin/bash -e
#
# This script will install packages required to build tenshi on Ubuntu
#

if [[ $EUID -ne 0 ]]
then
    echo "This script must be run with root privileges"
    echo "Running script with 'sudo'"
    sudo $0 $*
    exit 0
fi

## Ubuntu version checking

[[ -f /etc/lsb-release ]] || {
	cat <<EOF
This script is designed to run on Ubuntu only.

Ubuntu version detection failed, so it cannot run. Please install build
dependencies manually.
EOF
	exit 1
}

source /etc/lsb-release

[[ "$DISTRIB_ID" = "Ubuntu" ]] || {
	cat <<EOF
This script is designed to run on Ubuntu only.

Please install build dependencies manually on your system.
EOF
	exit 1
}

case "$DISTRIB_CODENAME" in
	trusty|saucy)
		echo "Installing build dependencies for $DISTRIB_ID $DISTRIB_RELEASE ($DISTRIB_CODENAME)"
		;;
	*)
		echo "This script has not been tested on $DISTRIB_ID $DISTRIB_RELEASE ($DISTRIB_CODENAME)"
		echo -n "Continue? [y/N]"
		read answer
		if [ "x$answer" = "xY" ] || [ "x$answer" = "xy" ]; then
			:
		else
			exit 1
		fi
		;;
esac

## Set up temporary directory for downloads
TMPDIR="/tmp/tenshi-build-dep.$$"
mkdir "$TMPDIR"
pushd "$TMPDIR"

## Make sure to cleanup if aborted
trap 'popd; rm -rf $TMPDIR; exit 15' 1 2 3 15

## Install misc dependencies
apt-get install xvfb openjdk-7-jre-headless coreutils build-essential software-properties-common wget zip

## Install Python and related
which pip >/dev/null || {
	apt-get install python-pip python-dev libyaml-dev
}

pip install pyyaml pep8

## Install nodejs and related
which node >/dev/null || {
	add-apt-repository ppa:chris-lea/node.js
	apt-get update
	apt-get install nodejs
}

npm install jshint csslint -g


## Install atmel toolchain

### Dependency for the toolchain
dpkg -s libmpc2 >/dev/null || {
	wget http://launchpadlibrarian.net/118496223/libmpc2_0.9-4build1_amd64.deb
	dpkg -i libmpc2_0.9-4build1_amd64.deb
}

dpkg -s atmel-toolchain-binutils-avr atmel-toolchain-gcc-avr atmel-toolchain-avr-libc > /dev/null || {
	### Set up PPA
	add-apt-repository ppa:nonolith/avr-toolchain

	if [ -f /etc/apt/sources.list.d/nonolith-avr-toolchain-trusty.list ]; then
		# The PPA is not currently up-to-date for Ubuntu 14.04, so use the
		# 13.10 sources instead
		sed "s/trusty/saucy/g" /etc/apt/sources.list.d/nonolith-avr-toolchain-trusty.list > nonolith-avr-toolchain-trusty.list
		mv nonolith-avr-toolchain-trusty.list /etc/apt/sources.list.d
	fi

	apt-get update

	### Toolchain installation
	apt-get install atmel-toolchain-binutils-avr atmel-toolchain-gcc-avr atmel-toolchain-avr-libc
}


## Install eagle

### Check for existing eagle instalation
EAGLE_INSTALLED=0
which eagle && EAGLE_INSTALLED=1
[ -x /opt/eagle-6.5.0/bin/eagle ] && EAGLE_INSTALLED=1

### If there is no existing installation, install eagle

if [[ "$EAGLE_INSTALLED" = 0 ]]; then
	set +e
	dpkg --print-foreign-architectures | grep i386
	is_i386_arch_enabled=$?
	set -e
	if [[ $is_i386_arch_enabled != 0 ]]; then
		dpkg --add-architecture i386
	fi
	apt-get update
	apt-get install libxrender1:i386 libxrandr2:i386 libxcursor1:i386 libfreetype6:i386 libfontconfig1:i386 libxi6:i386 libpng12-0:i386 libstdc++6:i386 libjpeg62:i386 libssl1.0.0:i386
	wget ftp://ftp.cadsoft.de/eagle/program/6.5/eagle-lin-6.5.0.run
	chmod +x eagle-lin-6.5.0.run
	./eagle-lin-6.5.0.run
fi

## Dependencies for running xulrunner
apt-get install libxrender1 libasound2 libgtk2.0-0

## Cleanup
popd
rm -rf $TMPDIR

## Pre-run tool extraction to avoid downloads later
[ -f "tools/extract-tools.sh" ] && tools/extract-tools.sh

## Done
echo "Done: build dependencies have been installed"
