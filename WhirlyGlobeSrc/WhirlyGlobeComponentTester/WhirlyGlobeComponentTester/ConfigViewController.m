/*
 *  ConfigViewController.m
 *  WhirlyGlobeComponentTester
 *
 *  Created by Steve Gifford on 7/23/12.
 *  Copyright 2011-2013 mousebird consulting
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

#import "ConfigViewController.h"

@implementation ConfigSection

+ (ConfigSection *)ConfigSectionWithName:(NSString *)sectionName rows:(NSDictionary *)rowDict singleSelect:(bool)select
{
    ConfigSection *cs = [[ConfigSection alloc] init];
    cs.sectionName = sectionName;
    cs.rows = [NSMutableDictionary dictionaryWithDictionary:rowDict];
    cs.singleSelect = select;
    
    return cs;
}

@end

@interface ConfigViewController ()

@end

@implementation ConfigViewController

- (id)initWithNibName:(NSString *)nibNameOrNil bundle:(NSBundle *)nibBundleOrNil
{
    self = [super initWithNibName:nibNameOrNil bundle:nibBundleOrNil];
    if (self) {
        // Custom initialization
    }
    return self;
}

- (bool)valueForSection:(NSString *)section row:(NSString *)row
{
    for (ConfigSection *cs in _values)
        if (![cs.sectionName compare:section])
        {
            return [cs.rows[row] boolValue];
        }
    
    return false;
}

- (void)viewDidLoad
{
    [super viewDidLoad];
    // Do any additional setup after loading the view from its nib.
    
    NSMutableArray *newValues = [NSMutableArray array];
    [newValues addObject:[ConfigSection ConfigSectionWithName:kMaplyTestCategoryBaseLayers
                                                         rows:
                          @{kMaplyTestBlank: @(NO),
                                    kMaplyTestGeographyClass: @(YES),
                                         kMaplyTestBlueMarble: @(NO),
                                   kMaplyTestStamenWatercolor: @(NO),
                                                kMaplyTestOSM: @(NO),
                                          kMaplyTestMapBoxSat: @(NO),
                                      kMaplyTestMapBoxTerrain: @(NO),
                                      kMaplyTestMapBoxRegular: @(NO),
                                           kMaplyTestQuadTest: @(NO),
                                     kMaplyTestQuadVectorTest: @(NO),
                                    kMaplyTestQuadTestAnimate: @(NO)}
                                                 singleSelect:true]];
    
    // We won't let the user do some things in terrain mode (overlay, basically)
    //  or 2D flatmap mode (shapes, basically)
    switch (_configOptions)
    {
        case ConfigOptionsAll:
            [newValues addObjectsFromArray:
             @[[ConfigSection ConfigSectionWithName:kMaplyTestCategoryOverlayLayers
                                               rows:
                @{kMaplyTestUSGSOrtho: @(NO),
                  kMaplyTestOWM: @(NO),
                  kMaplyTestForecastIO: @(NO),
                  kMaplyTestMapboxStreets: @(NO),
                  kMaplyMapzenVectors: @(NO)
                  }
                                       singleSelect:false],
               [ConfigSection ConfigSectionWithName:kMaplyTestCategoryObjects
                                               rows:
                @{kMaplyTestLabel2D: @(NO),
                  kMaplyTestLabel3D: @(NO),
                  kMaplyTestMarker2D: @(NO),
                  kMaplyTestMarker3D: @(NO),
                  kMaplyTestSticker: @(NO),
                  kMaplyTestShapeCylinder: @(NO),
                  kMaplyTestShapeSphere: @(NO),
                  kMaplyTestShapeGreatCircle: @(NO),
                  kMaplyTestShapeArrows: @(NO),
                  kMaplyTestModels: @(NO),
                  kMaplyTestCountry: @(NO),
                  kMaplyTestLoftedPoly: @(NO),
                  kMaplyTestMegaMarkers: @(NO),
                  kMaplyTestLatLon: @(NO),
                  kMaplyTestRoads: @(NO)}
                                       singleSelect:false],
               [ConfigSection ConfigSectionWithName:kMaplyTestCategoryAnimation
                                               rows:
                @{kMaplyTestAnimateSphere: @(NO)}
                                       singleSelect:false]]];
            break;
        case ConfigOptionsTerrain:
            break;
        case ConfigOptionsFlat:
        [newValues addObjectsFromArray:
         @[[ConfigSection ConfigSectionWithName:kMaplyTestCategoryOverlayLayers
                                           rows:
            @{kMaplyTestUSGSOrtho: @(NO),
              kMaplyTestOWM: @(NO),
              kMaplyTestForecastIO: @(NO),
              kMaplyTestMapboxStreets: @(NO),
              kMaplyMapzenVectors: @(NO)
              }
                                   singleSelect:false],
           [ConfigSection ConfigSectionWithName:kMaplyTestCategoryObjects
                                           rows:
            @{kMaplyTestLabel2D: @(NO),
              kMaplyTestLabel3D: @(NO),
              kMaplyTestMarker2D: @(NO),
              kMaplyTestMarker3D: @(NO),
              kMaplyTestSticker: @(NO),
              kMaplyTestCountry: @(NO),
              kMaplyTestMegaMarkers: @(NO),
              kMaplyTestLatLon: @(NO),
              kMaplyTestRoads: @(NO)}
                                   singleSelect:false]]];
            break;
    }

    [newValues addObject:
     [ConfigSection ConfigSectionWithName:kMaplyTestCategoryGestures
                                     rows:
      @{kMaplyTestNorthUp: @(NO),
                          kMaplyTestPinch: @(YES),
                         kMaplyTestRotate: @(YES)}
                             singleSelect:false]];
    [newValues addObject:
     [ConfigSection ConfigSectionWithName:kMaplyTestCategoryInternal
                    rows:
                    @{kMaplyTestCulling: @(NO),
                      kMaplyTestPerf: @(NO),
                      kMaplyTestWaitLoad: @(NO)}
                             singleSelect:false]];
    
    _values = newValues;
}

- (void)viewDidUnload
{
    [super viewDidUnload];
    // Release any retained subviews of the main view.
    // e.g. self.myOutlet = nil;
}

- (BOOL)shouldAutorotateToInterfaceOrientation:(UIInterfaceOrientation)interfaceOrientation
{
	return YES;
}

#pragma marker - UITableView delegate and data source

- (NSInteger)numberOfSectionsInTableView:(UITableView *)tableView
{
    return [_values count];
}

- (NSString *)tableView:(UITableView *)tableView titleForHeaderInSection:(NSInteger)section
{
    if (section >= [_values count])
        return 0;

    ConfigSection *cs = _values[section];
    return cs.sectionName;
}

- (NSInteger)tableView:(UITableView *)tableView numberOfRowsInSection:(NSInteger)section
{
    if (section >= [_values count])
        return 0;
    
    ConfigSection *cs = _values[section];
    return [cs.rows count];
}

- (void)tableView:(UITableView *)tableView willDisplayCell:(UITableViewCell *)cell forRowAtIndexPath:(NSIndexPath *)indexPath
{
    if (indexPath.section >= [_values count])
        return;
    
    ConfigSection *cs = _values[indexPath.section];
    if (indexPath.row >= [cs.rows count])
        return;
    
    NSArray *arr = [[cs.rows allKeys] sortedArrayUsingSelector:@selector(compare:)];
    NSString *key = arr[indexPath.row];
    
    bool selected = [cs.rows[key] boolValue];
    cell.backgroundColor = selected ? [UIColor colorWithRed:0.75 green:0.75 blue:1.0 alpha:1.0] : [UIColor whiteColor];
}

- (UITableViewCell *)tableView:(UITableView *)tableView cellForRowAtIndexPath:(NSIndexPath *)indexPath
{
    if (indexPath.section >= [_values count])
        return nil;
    
    ConfigSection *cs = _values[indexPath.section];
    if (indexPath.row >= [cs.rows count])
        return nil;
    
    NSArray *arr = [[cs.rows allKeys]  sortedArrayUsingSelector:@selector(compare:)];
    UITableViewCell *cell = [[UITableViewCell alloc] initWithStyle:UITableViewCellStyleDefault reuseIdentifier:nil];
    NSString *key = arr[indexPath.row];
    cell.textLabel.text = key;
        
    return cell;
}

- (void)tableView:(UITableView *)tableView didSelectRowAtIndexPath:(NSIndexPath *)indexPath
{
    if (indexPath.section >= [_values count])
        return;
    ConfigSection *cs = _values[indexPath.section];
    if (indexPath.row >= [cs.rows count])
        return;
    NSArray *arr = [[cs.rows allKeys] sortedArrayUsingSelector:@selector(compare:)];
    NSString *key = arr[indexPath.row];
    
    bool selectState = [cs.rows[key] boolValue];
    if (cs.singleSelect)
    {
        // Turn everything else off and this one on
        if (!selectState)
        {
            for (NSString *theKey in [[cs.rows allKeys] sortedArrayUsingSelector:@selector(compare:)])
                cs.rows[theKey] = @(false);
            cs.rows[key] = @(true);
        }
        [tableView reloadSections:[NSIndexSet indexSetWithIndex:indexPath.section] withRowAnimation:UITableViewRowAnimationFade];
    } else {
        cs.rows[key] = [NSNumber numberWithBool:!selectState];
        [tableView reloadRowsAtIndexPaths:@[indexPath] withRowAnimation:UITableViewRowAnimationFade];
    }
    
}

@end
