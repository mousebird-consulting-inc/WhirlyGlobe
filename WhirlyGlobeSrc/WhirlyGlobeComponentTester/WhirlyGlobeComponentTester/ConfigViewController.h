//
//  ConfigViewController.h
//  WhirlyGlobeComponentTester
//
//  Created by Steve Gifford on 7/23/12.
//  Copyright (c) 2012 mousebird consulting. All rights reserved.
//

#import <UIKit/UIKit.h>

// Configuration view lets the user decide what to turn on and off
@interface ConfigViewController : UIViewController
{
    IBOutlet UISwitch *label2DSwitch;
    IBOutlet UISwitch *label3DSwitch;
    IBOutlet UISwitch *marker2DSwitch;
    IBOutlet UISwitch *marker3DSwitch;
}

@property (nonatomic,readonly) UISwitch *label2DSwitch;
@property (nonatomic,readonly) UISwitch *label3DSwitch;
@property (nonatomic,readonly) UISwitch *marker2DSwitch;
@property (nonatomic,readonly) UISwitch *marker3DSwitch;

@end
