
# Licensed to Pioneers in Engineering under one
# or more contributor license agreements.  See the NOTICE file
# distributed with this work for additional information
# regarding copyright ownership.  Pioneers in Engineering licenses
# this file to you under the Apache License, Version 2.0 (the
# "License"); you may not use this file except in compliance
#  with the License.  You may obtain a copy of the License at
#
#    http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing,
# software distributed under the License is distributed on an
# "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
# KIND, either express or implied.  See the License for the
# specific language governing permissions and limitations
# under the License

set -x
ARGS="--app $(pwd)/angel-player/src/application.ini -jsconsole -purgecaches"
if [ $(command -v firefox) ]; then
	# which doesn't really work on Windows, so use command -v
        firefox $ARGS
elif [ -d "$PROGRAMFILES" ] &&
     [ -x "$PROGRAMFILES/Mozilla Firefox/firefox.exe" ]; then
	# In *the future* the user might actually have a 64-bit windows
	# firefox.
	"$PROGRAMFILES/Mozilla Firefox/firefox.exe" $ARGS
elif [ -x "C:\\Program Files (x86)\\Mozilla Firefox\\firefox.exe" ]; then
     	# Look for the normal, 32-bit firefox in windows.
	"C:\\Program Files (x86)\\Mozilla Firefox\\firefox.exe" $ARGS
elif [ $(command -v open) ]; then
	# Maybe we're on OS X
        open -n -a Firefox --args $ARGS
else
	set +x
	echo
	echo
	echo "Error: Could not find firefox!"
	echo "Are you sure you have it installed?"
	echo
	echo "You can close this script now with ^C"
	echo
	echo
	set -x

	sleep 60
fi
