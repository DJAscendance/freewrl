//
//  AppController.h
//  FreeWRL
//
//  Created by Doug on 2018-05-30.
//  Copyright © 2018 freewrl.sf.net. All rights reserved.
//

#import <Foundation/Foundation.h>

@interface AppController : NSObject {
	@private
	IBOutlet NSTextField *txtLocation;
}
- (IBAction)OpenFile:(id)sender;
- (IBAction)Load:(id)sender;
@end
