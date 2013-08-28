/*
 *  BigButtonViewController.h
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

@class BigButtonViewController;

// Fill this in to get notified when the big button is pushed
@protocol BigButtonDelegate <NSObject>
- (void)bigButtonPushed:(BigButtonViewController *)viewC;
@end

/** Big Button View Controller
    A view with a big old button.  For pressing.
 */
@interface BigButtonViewController : UIViewController
{
    NSObject<BigButtonDelegate> * __weak delegate;
}

@property(nonatomic,weak) NSObject<BigButtonDelegate> *delegate;

// Use this to create one
+ (BigButtonViewController *)loadFromNib;

// Called when button pressed (shocking)
- (IBAction)buttonPress:(id)sender;

@end
