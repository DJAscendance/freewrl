[![Build Status](https://travis-ci.org/auriamg/macdylibbundler.svg?branch=master)](https://travis-ci.org/auriamg/macdylibbundler)

linux solib bundler
================


About
-----

AppImage.org introduced an innovative and very useful way to package applications : appImage bundles.
While their design has all that is needed to ease distribution of resources and frameworks, it
seems like dynamic libraries (.so) are very complicated to distribute. Sure, applications developed
specifically for the same linux won't make use of all of them, however applications ported from other versions and variants of Linux or other Unices may have
dependencies that will only compile as so libs. By default, there exists no mechanism to bundle them but some command-line utilities provided by linux - however it turns out that for a single program it is often necessary to issue dozens of commands! This often leads each porter to develop their own "home solution" wich are often hacky, poorly portable and/or nonoptimal.

**solibbundler** is a small command-line programs that aims to make bundling .so libs as easy as possible.
It automatically determines which solibs are needed by your program, copies these libraries inside the appImage bundle, and fixes both them and the executable to be ready for distribution... all this with a single command on the teminal! It will also work if your program uses plug-ins that have dependencies too.

It usually involves 2 actions :
* Creating a directory (by default called *libs*) that can be placed inside the *Contents* folder of the appImage bundle.
* Fixing the executable file so that it is aware of the new location of its dependencies.




Installation
------------
In the terminal, cd to the main directory of solibbundler and type "make". You can install with "sudo make install".


Feedback / Contact
------------------
You can contact me here on github, for instance by creating a ticket or pull request


Using solibbundler on the terminal
----------------------------------
Here is a list of flags you can pass to solibbundler on the terminal.

`-h`, `--help`
<blockquote>
displays a summary of options
</blockquote>

`-x`, `--fix-file` (executable or plug-in filepath)
<blockquote>
Fixes given executable or plug-in file (a .so lib can work too. anything on which `otool -L` works is accepted by `-x`). Solibbundler will walk through the dependencies of the specified file to build a dependency list. It will also fix the said files' dependencies so that it expects to find the libraries relative to itself (e.g. in the app bundle) instead of at an absolute path (e.g. /usr/local/lib). To pass multiple files to fix, simply specify multiple `-x` flags.
</blockquote>

`-b`, `--bundle-deps`
<blockquote>
Copies libaries to a local directory, fixes their internal name so that they are aware of their new location,
fixes dependencies where bundled libraries depend on each other. If this option is not passed, no libraries will be prepared for distribution.
</blockquote>

`-i`, `--ignore` (path)
> Solibs in (path) will be ignored. By default, solibbundler will ignore libraries installed in `/usr/lib` since they are assumed to be present by default on all linux installations.*(It is usually recommend not to install additional stuff in `/usr/`, always use ` /usr/local/` or another prefix to avoid confusion between system libs and libs you added yourself)*


`-d`, `--dest-dir` (directory)
> Sets the name of the directory in wich distribution-ready solibs will be placed, relative to the current working directory. (Default is `./lib`) For an appImage bundle, it is often conveniant to set it to something like `./MyApp.AppDir/usr/lib`.


`-p`, `--install-path` (libraries install path)
> Sets the "inner" installation path of libraries, usually inside the bundle and relative to executable. (Default is `@executable_path/../lib/`, which points to a directory named `lib` inside the `usr` directory of the bundle.)


*The difference between `-d` and `-p` is that `-d` is the location solibbundler will put files at, while `-p` is the location where the libraries will be expected to be found when you launch the app. Both are often related.*

`-of`, `--overwrite-files`
> When copying libraries to the output directory, allow overwriting files when one with the same name already exists.

`-od`, `--overwrite-dir`
> If the output directory already exists, completely erase its current content before adding anything to it. (This option implies --create-dir)

`-cd`, `--create-dir`
> If the output directory does not exist, create it.

A command may look like
`% solibbundler -od -b -x ./HelloWorld.appDir/usr/bin/helloworld -d ./HelloWorld.appDir/usr/lib/`


