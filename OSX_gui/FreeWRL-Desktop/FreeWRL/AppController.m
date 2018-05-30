//
//  AppController.m
//  FreeWRL
//
//  Created by Doug on 2018-05-30.
//  Copyright © 2018 freewrl.sf.net. All rights reserved.
//

#import "AppController.h"

@implementation AppController
- (id) init
{
	self = [super init];
	if(self){
		// initialization code here
	}
	return self;
}
- (IBAction)OpenLocation:(id)sender {
	NSOpenPanel* openDlg = [NSOpenPanel openPanel];
	
	[openDlg setCanChooseFiles:YES];
	
	[openDlg setAllowedFileTypes:@[@"wrl", @"x3d", @"x3dv"]];
	
	[openDlg beginWithCompletionHandler:^(NSInteger result) {
		if(result==NSFileHandlingPanelOKButton) {
			[txtLocation setStringValue: openDlg.URLs[0].relativeString];

			//for (NSURL *url in openDlg.URLs) {
		//		NSLog(@"%@", url);
		//	}
		}
	}];
/*
	// Get the main window for the document.
	//NSWindow* window = [[[self windowControllers] objectAtIndex:0] window];
 
	// Create and configure the panel.
	NSOpenPanel* panel = [NSOpenPanel openPanel];
	[panel setCanChooseDirectories:NO];
	[panel setAllowsMultipleSelection:NO];
	[panel setMessage:@"Open web3d scene"];
	[panel setAllowedFileTypes:@[@"x3d", @"wrl", @"x3dv"]];
	// Display the panel attached to the document's window.
	//[panel beginSheetModalForWindow:window completionHandler:^(NSInteger result){
	if ( [panel runModalForDirectory:nil file:nil] == NSOKButton )
	{
		//if (result == NSFileHandlingPanelOKButton) {
			NSArray* urls = [panel URLs];
			
			// Use the URLs to build a list of items to import.
			[txtLocation setStringValue: urls[0]];
			
		//}
		
	}
	//];
 */
}
- (void) dealloc
{
	[super dealloc];
}
@end
