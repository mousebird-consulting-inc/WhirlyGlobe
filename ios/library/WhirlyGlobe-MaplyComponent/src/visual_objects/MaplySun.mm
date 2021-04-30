/*  MaplySun.mm
 *  WhirlyGlobe-MaplyComponent
 *
 *  Created by Steve Gifford on 6/24/15.
 *  Copyright 2011-2021 mousebird consulting
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
 */

#import <WhirlyGlobe_iOS.h>
#import "visual_objects/MaplySun.h"
#import <AA+.h>

using namespace WhirlyKit;

@implementation MaplySun
{
    Sun *sun;
}

- (instancetype)initWithDate:(NSDate *)date
{
    self = [super init];
    
    // It all starts with the Julian date
    NSCalendar *calendar = [[NSCalendar alloc] initWithCalendarIdentifier:NSCalendarIdentifierGregorian];
    calendar.timeZone = [NSTimeZone timeZoneForSecondsFromGMT:0];
    NSDateComponents *components = [calendar components:(NSCalendarUnitYear | NSCalendarUnitMonth | NSCalendarUnitDay | NSCalendarUnitHour | NSCalendarUnitMinute | NSCalendarUnitSecond) fromDate:date];

    sun = new Sun(components.year, components.month, components.day, components.hour, components.minute, components.second);
    
    return self;
}

- (void)dealloc
{
    delete sun;
    sun = nullptr;
}

- (MaplyCoordinate3d)getDirection
{
    Point3d dir = sun->getDirection();
    return MaplyCoordinate3dMake(dir.x(), dir.y(), dir.z());
}

- (MaplyLight *)makeLight
{
    MaplyLight *sunLight = [[MaplyLight alloc] init];
    MaplyCoordinate3d dir = [self getDirection];
    sunLight.pos = MaplyCoordinate3dMake(dir.x, dir.z, dir.y);
    sunLight.ambient = [UIColor colorWithRed:0.1 green:0.1 blue:0.1 alpha:1.0];
    sunLight.diffuse = [UIColor colorWithRed:0.8 green:0.8 blue:0.8 alpha:1.0];
    sunLight.viewDependent = true;
    
    return sunLight;
}

- (MaplyCoordinate3d)asPosition
{
    const auto height = 149.6 * 1000000 * 1000 / EarthRadius;
    return MaplyCoordinate3dMake(sun->sunLon,sun->sunLat, height);
}

@end
