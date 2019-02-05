#!/bin/sh
rm -rf freewrl.AppDir
cd linuxsolibbundler
make clean
rm Makefile
rm solibbundler
cd ..
rm appimagetool
rm *.AppImage
rm excludelist

