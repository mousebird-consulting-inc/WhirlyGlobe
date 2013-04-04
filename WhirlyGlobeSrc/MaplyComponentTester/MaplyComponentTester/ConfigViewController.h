/*
 *  ConfigViewController.h
 *  WhirlyGlobeComponentTester
 *
 *  Created by Steve Gifford on 7/23/12.
 *  Copyright 2011-2012 mousebird consulting
 *
 *  Licensed under the Apache License, Version 2.0 (the "License");
 *  you may not use this file except in compliance with the License.
 *  You may obtain a copy of the License at
 *  http://www.apache.org/licenses/LICENSE-2.0
 *
 *  Unless required by applicable law or agreed to in writing, software
 *  distributed under the License is distributed on an "AS IS" BASIS,
 *  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *  See the License for the specific language governing permissions and
 *  limitations under the License.
 *
 */

#import <UIKit/UIKit.h>

// Configuration view lets the user decide what to turn on and off
@interface ConfigViewController : UIViewController
{
    IBOutlet UISwitch *label2DSwitch;
    IBOutlet UISwitch *label3DSwitch;
    IBOutlet UISwitch *marker2DSwitch;
    IBOutlet UISwitch *marker3DSwitch;
    IBOutlet UISwitch *zBufferSwitch;
    IBOutlet UISwitch *pinchSwitch;
    IBOutlet UISwitch *rotateSwitch;
    IBOutlet UISwitch *countrySwitch;
}

@property (nonatomic,readonly) UISwitch *label2DSwitch;
@property (nonatomic,readonly) UISwitch *label3DSwitch;
@property (nonatomic,readonly) UISwitch *marker2DSwitch;
@property (nonatomic,readonly) UISwitch *marker3DSwitch;
@property (nonatomic,readonly) UISwitch *zBufferSwitch;
@property (nonatomic,readonly) UISwitch *pinchSwitch;
@property (nonatomic,readonly) UISwitch *rotateSwitch;
@property (nonatomic,readonly) UISwitch *countrySwitch;

@end
