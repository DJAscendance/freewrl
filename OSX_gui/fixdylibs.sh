#!/bin/sh
../macdylibbundler/dylibbundler \
-x FreeWRL.app/Contents/MacOS/FreeWRL \
-b \
-d FreeWRL.app/Contents/Resources/lib \
-p '@executable_path/../Resources/lib' \
-of \
-od \
-i /usr/local

