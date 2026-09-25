//
//  main.m
//  FreeWRL
//
//  Created by John Stewart on 11-07-20.
//  Copyright 2011 CRC Canada. All rights reserved.
//

#import <Cocoa/Cocoa.h>
#include <stdlib.h>

// A packaged app (tools/macos-package) carries Imlib2's image loaders in
// Contents/PlugIns/imlib2/loaders; Imlib2 otherwise looks in the directory it
// was built for (/opt/homebrew/...). An IMLIB2_LOADER_PATH set by the user wins.
static void useBundledImlib2Loaders(void)
{
    @autoreleasepool {
        NSString *dir = [[[NSBundle mainBundle] builtInPlugInsPath] stringByAppendingPathComponent:@"imlib2/loaders"];
        BOOL isDir = NO;
        if ([[NSFileManager defaultManager] fileExistsAtPath:dir isDirectory:&isDir] && isDir)
            setenv("IMLIB2_LOADER_PATH", [dir fileSystemRepresentation], 0);
    }
}

int main(int argc, char *argv[])
{
    useBundledImlib2Loaders();
    return NSApplicationMain(argc, (const char **)argv);
}
