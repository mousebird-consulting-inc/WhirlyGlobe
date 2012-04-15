/*
 *  OptionsViewController.h
 *  WhirlyGlobeTester
 *
 *  Created by Steve Gifford on 11/12/11.
 *  Copyright 2011 mousebird consulting
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

@class OptionsViewController;

// Fill this in to get results from the options controller
@protocol OptionsControllerDelegate <NSObject>

@end

/** Options View Controller
    Allows the user to turn on and off various displayed features.
 */
@interface OptionsViewController : UIViewController
{
    NSMutableDictionary *values;  // Used to store switch values
    IBOutlet UISegmentedControl *countryControl;
    IBOutlet UISegmentedControl *markersControl;
    IBOutlet UISwitch *particlesSwitch;
    IBOutlet UISegmentedControl *loftedControl;    
    IBOutlet UISwitch *statsSwitch;
    NSObject<OptionsControllerDelegate> * __weak delegate;
}

@property (nonatomic,weak) NSObject<OptionsControllerDelegate> *delegate;

// Use this to create one
+ (OptionsViewController *)loadFromNib;

// Return a copy of the current values dictionary
+ (NSDictionary *)fetchValuesDict;

// Various actions are tied to switches
- (IBAction)valueChangeAction:(id)sender;

@end
