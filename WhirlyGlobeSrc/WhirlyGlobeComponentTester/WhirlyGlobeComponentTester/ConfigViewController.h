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
    IBOutlet UISwitch *northUpSwitch;
    IBOutlet UISwitch *zBufferSwitch;
    IBOutlet UISwitch *cullingSwitch;
    IBOutlet UISwitch *pinchSwitch;
    IBOutlet UISwitch *rotateSwitch;
    IBOutlet UISwitch *countrySwitch;
}

@property (nonatomic,readonly) UISwitch *label2DSwitch;
@property (nonatomic,readonly) UISwitch *label3DSwitch;
@property (nonatomic,readonly) UISwitch *marker2DSwitch;
@property (nonatomic,readonly) UISwitch *marker3DSwitch;
@property (nonatomic,readonly) UISwitch *northUpSwitch;
@property (nonatomic,readonly) UISwitch *zBufferSwitch;
@property (nonatomic,readonly) UISwitch *cullingSwitch;
@property (nonatomic,readonly) UISwitch *pinchSwitch;
@property (nonatomic,readonly) UISwitch *rotateSwitch;
@property (nonatomic,readonly) UISwitch *countrySwitch;

@end
