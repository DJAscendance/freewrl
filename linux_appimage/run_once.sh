#!/bin/bash
# scraped from: 
# https://github.com/AppImage/AppImageKit
# https://github.com/AppImage/AppImageKit/wiki/Creating-AppImages
# https://github.com/AppImage/AppImageKit/releases

wget "https://github.com/AppImage/AppImageKit/releases/download/continuous/appimagetool-x86_64.AppImage"
mv appimagetool-x86_64.AppImage appimagetool
chmod a+x appimagetool
wget "https://github.com/AppImage/AppImageKit/releases/download/continuous/AppRun-x86_64"
mv AppRun-x86_64 AppRun
chmod a+x AppRun
# wget  "https://raw.githubusercontent.com/AppImage/AppImages/master/excludelist" 
cp excludelist.freewrl excludelist
mkdir -p freewrl.AppDir
mkdir -p freewrl.AppDir/usr
mkdir -p freewrl.AppDir/usr/bin
mkdir -p freewrl.AppDir/usr/lib
cp ../freex3d/data/freewrl.desktop freewrl.AppDir
cp ../freex3d/data/freewrl.png freewrl.AppDir
mv AppRun freewrl.AppDir
mkdir -p freewrl.AppDir/usr/fonts
cp ../freex3d/appleOSX/OSX_Specific/fonts/* freewrl.AppDir/usr/fonts/
cd linuxsolibbundler
cp Makefile.am Makefile
make
cd ..
