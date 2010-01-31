#!/bin/sh
#
# Author: Cory Quammen
# Note: This file will be renamed to MicroscopeSimulator in MicroscopeSimulator.app/Contents/MacOS/
#

echo "Running MicroscopeSimulator executable."

MSIM_BUNDLE="`echo "$0" | sed -e 's/\/Contents\/MacOS\/MicroscopeSimulator\ [0-9]*.[0-9]*.[0-9]*//'`"
MSIM_EXECUTABLE="$MSIM_BUNDLE/Contents/Resources/bin/MicroscopeSimulator"

export "DYLD_LIBRARY_PATH=$MSIM_BUNDLE/Contents/Resources/lib/"

export "DYLD_FRAMEWORK_PATH=$MSIM_BUNDLE/Contents/Frameworks/"

exec "$MSIM_EXECUTABLE"
