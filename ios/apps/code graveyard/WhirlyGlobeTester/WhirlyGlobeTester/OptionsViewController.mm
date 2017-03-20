/*
 *  OptionsViewController.mm
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

#import "OptionsViewController.h"
#import "InteractionLayer.h"

@implementation OptionsViewController

@synthesize delegate;

+ (OptionsViewController *)loadFromNib
{
    OptionsViewController *viewC = [[OptionsViewController alloc] initWithNibName:@"OptionsView" bundle:nil];
    
    return viewC;
}

// We're keeping the parameter values global, basically
NSMutableDictionary *valueDict = nil;

+ (NSDictionary *)fetchValuesDict
{
    // Only one shared value dictionary
    if (!valueDict)
    {
        valueDict = [NSMutableDictionary dictionary];
        // Start with all the features off
        [valueDict setObject:[NSNumber numberWithInt:0] forKey:kWGCountryControl];
        [valueDict setObject:[NSNumber numberWithInt:0] forKey:kWGMarkerControl];
        [valueDict setObject:[NSNumber numberWithInt:0] forKey:kWGParticleControl];
        [valueDict setObject:[NSNumber numberWithInt:0] forKey:kWGLoftedControl];
        [valueDict setObject:[NSNumber numberWithInt:0] forKey:kWGGridControl];
        [valueDict setObject:[NSNumber numberWithInt:0] forKey:kWGStatsControl];
    }

    return [NSDictionary dictionaryWithDictionary:valueDict];
}

- (id)initWithNibName:(NSString *)nibNameOrNil bundle:(NSBundle *)nibBundleOrNil
{
    self = [super initWithNibName:nibNameOrNil bundle:nibBundleOrNil];
    if (self) {
        values = valueDict;
    }
    return self;
}

- (void)clear
{
    countryControl = nil;
    markersControl = nil;
    particlesSwitch = nil;
    loftedControl = nil;
    statsSwitch = nil;
}

- (void)dealloc
{
    [self clear];
    
}

- (void)didReceiveMemoryWarning
{
    // Releases the view if it doesn't have a superview.
    [super didReceiveMemoryWarning];
    
    // Release any cached data, images, etc that aren't in use.
}

#pragma mark - View lifecycle

// Implement viewDidLoad to do additional setup after loading the view, typically from a nib.
- (void)viewDidLoad
{
    [super viewDidLoad];
}

- (void)viewDidUnload
{
    [self clear];
    
    [super viewDidUnload];
}

- (void)viewWillAppear:(BOOL)animated
{
    countryControl.selectedSegmentIndex = [[values objectForKey:kWGCountryControl] intValue];
    markersControl.selectedSegmentIndex = [[values objectForKey:kWGMarkerControl] intValue];
    particlesSwitch.on = [[values objectForKey:kWGParticleControl] boolValue];
    loftedControl.selectedSegmentIndex = [[values objectForKey:kWGLoftedControl] intValue];
    statsSwitch.on = [[values objectForKey:kWGStatsControl] boolValue];
}

- (BOOL)shouldAutorotateToInterfaceOrientation:(UIInterfaceOrientation)interfaceOrientation
{
    // Return YES for supported orientations
	return YES;
}

// One of the controls changes, update the dictionary and send out a notification
- (IBAction)valueChangeAction:(id)sender
{
    [values setObject:[NSNumber numberWithInt:countryControl.selectedSegmentIndex] forKey:kWGCountryControl];
    [values setObject:[NSNumber numberWithInt:markersControl.selectedSegmentIndex] forKey:kWGMarkerControl];
    [values setObject:[NSNumber numberWithBool:particlesSwitch.on] forKey:kWGParticleControl];
    [values setObject:[NSNumber numberWithInt:loftedControl.selectedSegmentIndex] forKey:kWGLoftedControl];
    [values setObject:[NSNumber numberWithBool:statsSwitch.on] forKey:kWGStatsControl];

    [[NSNotificationCenter defaultCenter] postNotificationName:kWGControlChange object:values];
}

@end
