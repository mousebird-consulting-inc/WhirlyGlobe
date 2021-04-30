/*  MaplyMoon.mm
 *  WhirlyGlobe-MaplyComponent
 *
 *  Created by Steve Gifford on 7/2/15.
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

#import "visual_objects/MaplyMoon.h"
#import <AA+.h>
#import "WhirlyGlobe_iOS.h"

using namespace WhirlyKit;

@implementation MaplyMoon
{
    WhirlyKit::Moon *moon;
    double moonLon,moonLat;
}

// Math borrowed from: http://www.lunar-occultations.com/rlo/ephemeris.htm
- (instancetype)initWithDate:(NSDate *)date
{
    self = [super init];

    // Start with the Julian Date
    NSCalendar *calendar = [[NSCalendar alloc] initWithCalendarIdentifier:NSCalendarIdentifierGregorian];
    calendar.timeZone = [NSTimeZone timeZoneForSecondsFromGMT:0];
    NSDateComponents *components = [calendar components:(NSCalendarUnitYear | NSCalendarUnitMonth | NSCalendarUnitDay | NSCalendarUnitHour | NSCalendarUnitMinute | NSCalendarUnitSecond) fromDate:date];
    
    moon = new Moon(components.year,components.month,components.day,components.hour,components.minute,components.second);

    return self;
}

- (void)dealloc
{
    delete moon;
    moon = nullptr;
}

- (MaplyCoordinate)asCoordinate
{
    return MaplyCoordinateMake(moon->moonLon,moon->moonLat);
}

- (MaplyCoordinate3d)asPosition
{
    const auto height = 385000000 / EarthRadius;
    return MaplyCoordinate3dMake(moon->moonLon,moon->moonLat, height);
}

- (double)illuminatedFraction
{
    return moon ? moon->illuminatedFraction : 0;
}

- (double)phase
{
    return moon ? moon->phase : 0;
}

@end
