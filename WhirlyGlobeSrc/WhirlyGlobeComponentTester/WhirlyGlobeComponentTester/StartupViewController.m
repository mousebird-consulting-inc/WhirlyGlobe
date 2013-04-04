/*
 *  StartupViewController.m
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

#import "StartupViewController.h"

@interface StartupViewController ()

@end

@implementation StartupViewController
{
    UITableView *tableView;
}

- (id)initWithNibName:(NSString *)nibNameOrNil bundle:(NSBundle *)nibBundleOrNil
{
    self = [super initWithNibName:nibNameOrNil bundle:nibBundleOrNil];
    if (self) {
        // Custom initialization
    }
    return self;
}

- (void)viewDidLoad
{
    [super viewDidLoad];
    self.title = @"Select Base Map";
    
    tableView = [[UITableView alloc] initWithFrame:self.view.bounds];
    tableView.delegate = self;
    tableView.dataSource = self;
    tableView.backgroundColor = [UIColor grayColor];
    tableView.separatorColor = [UIColor whiteColor];
    tableView.autoresizingMask = UIViewAutoresizingFlexibleWidth | UIViewAutoresizingFlexibleHeight;
    [self.view addSubview:tableView];
    self.view.autoresizesSubviews = true;
}

- (void)viewDidUnload
{
    [super viewDidUnload];
    
    tableView = nil;
}

- (BOOL)shouldAutorotateToInterfaceOrientation:(UIInterfaceOrientation)interfaceOrientation
{
	return YES;
}

- (void)viewWillAppear:(BOOL)animated
{
    [tableView reloadData];
}

#pragma mark - Table Data Source

- (NSInteger)numberOfSectionsInTableView:(UITableView *)tableView
{
    // Globe and map
    return 2;
}

- (NSInteger)tableView:(UITableView *)tableView numberOfRowsInSection:(NSInteger)section
{
    int numLayers = (section == 0 ? MaxBaseLayers : MaxBaseLayers-1);
    return numLayers;
}

- (NSString *)tableView:(UITableView *)tableView titleForHeaderInSection:(NSInteger)section
{
    NSString *title = nil;
    
    switch (section)
    {
        case 0:
            title = @"Globe";
            break;
        case 1:
            title = @"Map";
            break;
    }
    
    return title;
}

- (UITableViewCell *)tableView:(UITableView *)tableView cellForRowAtIndexPath:(NSIndexPath *)indexPath
{
    UITableViewCell *cell = [[UITableViewCell alloc] initWithStyle:UITableViewCellStyleDefault reuseIdentifier:nil];
    switch (indexPath.row)
    {
        case BlueMarbleSingleResLocal:
            cell.textLabel.text = @"Blue Marble Single Res - Local";
            break;
        case GeographyClassMBTilesLocal:
            cell.textLabel.text = @"Geography Class - MapBox Tiles - Local";
            break;
        case StamenWatercolorRemote:
            cell.textLabel.text = @"Stamen WaterColor - Remote";
            break;
        case OpenStreetmapRemote:
            cell.textLabel.text = @"OpenStreetMap - Remote";
            break;
        case MapBoxTilesSat1:
            cell.textLabel.text = @"MapBox Tiles Satellite - Remote";
            break;
        case MapBoxTilesTerrain1:
            cell.textLabel.text = @"MapBox Tiles Terrain - Remote";
            break;
        case MapBoxTilesRegular1:
            cell.textLabel.text = @"MapBox Tiles Regular - Remote";
            break;
        default:
            break;
    }
    cell.textLabel.textColor = [UIColor whiteColor];
    cell.backgroundColor = [UIColor grayColor];
    
    return cell;
}

#pragma mark - Table Delegate

- (void)tableView:(UITableView *)tableView didSelectRowAtIndexPath:(NSIndexPath *)indexPath
{
    TestViewController *viewC = [[TestViewController alloc] initWithMapType:indexPath.section  baseLayer:indexPath.row];
    [self.navigationController pushViewController:viewC animated:YES];
}

@end
