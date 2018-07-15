#!/bin/sh
# pull freewrl main executable if not done manually
cp /usr/local/bin/freewrl freewrl.AppDir/usr/bin
# pull NEEDED dependencies
./linuxsolibbundler/solibbundler \
-x freewrl.AppDir/usr/bin/freewrl \
-b \
-d freewrl.AppDir/usr/lib \
-p '@executable_path/../lib' \
-of \
-od 
# -i /usr/lib
# package into .appimage
./appimagetool freewrl.AppDir
