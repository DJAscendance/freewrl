#!/bin/sh
# pull freewrl main executable if not done manually
cp /usr/local/bin/freewrl freewrl.AppDir/usr/bin
cp -r /usr/lib/x86_64-linux-gnu/imlib2 freewrl.AppDir/usr/imlib2
# pull NEEDED dependencies
./linuxsolibbundler/solibbundler \
-x freewrl.AppDir/usr/bin/freewrl \
-x freewrl.AppDir/usr/imlib2/loaders/bmp.so \
-x freewrl.AppDir/usr/imlib2/loaders/gif.so \
-x freewrl.AppDir/usr/imlib2/loaders/jpeg.so \
-x freewrl.AppDir/usr/imlib2/loaders/png.so \
-x freewrl.AppDir/usr/imlib2/loaders/tga.so \
-x freewrl.AppDir/usr/imlib2/loaders/tiff.so \
-x freewrl.AppDir/usr/imlib2/loaders/bz2.so \
-x freewrl.AppDir/usr/imlib2/loaders/zlib.so \
-x freewrl.AppDir/usr/imlib2/loaders/argb.so \
-b \
-d freewrl.AppDir/usr/lib \
-p '@executable_path/../lib' \
-of \
-od \
-e excludelist
 #-i /usr/lib
#imlib2 does runtime plugin loading, and its plugin path is hardcoded
# so we hack it with SED editor and replace /usr with ././ as per
# https://github.com/AppImage/AppImageKit/wiki/Creating-AppImages
# manual method > no hard coded paths 
mkdir freewrl.AppDir/usr/lib/x86_64-linux-gnu
mv freewrl.AppDir/usr/imlib2 freewrl.AppDir/usr/lib/x86_64-linux-gnu/imlib2
sed -i -e 's#/usr#././#g' freewrl.AppDir/usr/lib/libImlib2.so.1
# package into .appimage
./appimagetool freewrl.AppDir
