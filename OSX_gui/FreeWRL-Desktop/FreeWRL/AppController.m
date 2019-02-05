//
//  AppController.m
//  FreeWRL
//
//  Created by Doug on 2018-05-30.
//  Copyright © 2018 freewrl.sf.net. All rights reserved.
//

#import "AppController.h"
#import "../../../freex3d/src/dllFreeWRL/cdllFreeWRL.h"

extern void* fwctx;

@implementation AppController
- (id) init
{
	self = [super init];
	if(self){
		// initialization code here
	}
	return self;
}
- (IBAction)OpenFile:(id)sender {
	NSOpenPanel* openDlg = [NSOpenPanel openPanel];
	
	[openDlg setCanChooseFiles:YES];
	
	[openDlg setAllowedFileTypes:@[@"wrl", @"x3d", @"x3dv"]];
	
	[openDlg beginWithCompletionHandler:^(NSInteger result) {
		if(result==NSFileHandlingPanelOKButton) {
			[txtLocation setStringValue: openDlg.URLs[0].relativeString];


		}
	}];
	
}
- (IBAction)Load:(id)sender {
	//dllFreeWRL_onLoad(fwctx, "/Users/doug/source2/freewrl/freewrl/tests/2.wrl");
	//dllFreeWRL_onLoad(fwctx,(char*)&txtLocation.stringValue.UTF8String[7]);
	dllFreeWRL_onLoad(fwctx,(char*)txtLocation.stringValue.UTF8String);

}
- (void) dealloc
{
	[super dealloc];
}
@end
