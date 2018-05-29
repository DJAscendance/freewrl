//
//  FreeWRLAppDelegate.h
//  FreeWRL
//
//  Created by John Stewart on 11-07-20.
//  Copyright 2011 CRC Canada. All rights reserved.
//

#import <Cocoa/Cocoa.h>


@interface FreeWRLAppDelegate : NSObject
{
}


- (void)observeValueForKeyPath:(NSString *)keyPath
                      ofObject:(id)object
                        change:(NSDictionary *)change
                       context:(void *)context;

- (BOOL)applicationShouldTerminateAfterLastWindowClosed:(NSApplication *)theApplication;
-(void)dealloc;
- (void)applicationDidFinishLaunching:(NSNotification *)notification;

+(bool)applicationHasLaunched;

@end