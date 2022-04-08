/*  MaplyMoon.mm
 *  WhirlyGlobe-MaplyComponent
 *
 *  Created by Steve Gifford on 7/2/15.
 *  Copyright 2011-2022 mousebird consulting
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
    std::unique_ptr<Moon> moon;
    Point2d moonPos;
}

// Math borrowed from: http://www.lunar-occultations.com/rlo/ephemeris.htm
- (instancetype _Nullable)initWithDate:(NSDate *)date
{
    if (!(self = [super init]))
    {
        return nil;
    }

    // Start with the Julian Date
    NSCalendar *calendar = [[NSCalendar alloc] initWithCalendarIdentifier:NSCalendarIdentifierGregorian];
    calendar.timeZone = [NSTimeZone timeZoneForSecondsFromGMT:0];
    
    const NSCalendarUnit units = NSCalendarUnitYear |
                                 NSCalendarUnitMonth |
                                 NSCalendarUnitDay |
                                 NSCalendarUnitHour |
                                 NSCalendarUnitMinute |
                                 NSCalendarUnitSecond;
    NSDateComponents *components = [calendar components:units fromDate:date];
    if (!components)
    {
        return nil;
    }
    
    moon = std::make_unique<Moon>(components.year,components.month,components.day,
                                  components.hour,components.minute,components.second);

    return self;
}

- (void)dealloc
{
    moon.reset();
}

- (MaplyCoordinate)coordinate
{
    if (moon)
    {
        return MaplyCoordinateMake(moon->moonLon,moon->moonLat);
    }
    return kMaplyNullCoordinate;
}

- (MaplyCoordinate3d)position
{
    if (moon)
    {
        const auto height = 385000000 / EarthRadius;
        return MaplyCoordinate3dMake(moon->moonLon,moon->moonLat, height);
    }
    return kMaplyNullCoordinate3d;
}

- (MaplyLight * _Nullable )makeLight
{
    return [self makeLightWithAmbient:0.0f diffuse:0.1f];
}

- (MaplyLight * _Nullable)makeLightWithAmbient:(float)ambient diffuse:(float)diffuse
{
    MaplyLight *light = [[MaplyLight alloc] init];
    const MaplyCoordinate3d dir = self.position;
    light.pos = MaplyCoordinate3dMake(dir.x, dir.z, dir.y);
    light.ambient = [UIColor colorWithRed:ambient green:ambient blue:ambient alpha:1.0];
    light.diffuse = [UIColor colorWithRed:diffuse green:diffuse blue:diffuse alpha:1.0];
    light.viewDependent = true;
    
    return light;
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
