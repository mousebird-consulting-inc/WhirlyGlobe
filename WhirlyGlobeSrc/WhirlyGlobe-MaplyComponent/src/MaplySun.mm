/*
 *  MaplySun.h
 *  WhirlyGlobe-MaplyComponent
 *
 *  Created by Steve Gifford on 6/24/15.
 *  Copyright 2011-2015 mousebird consulting
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

#import <WhirlyGlobe.h>
#import "MaplySun.h"
#import <AA+.h>

using namespace WhirlyKit;

@implementation MaplySun
{
    NSDate *date;
    double sunLon,sunLat;
}

- (id)initWithDate:(NSDate *)inDate
{
    self = [super init];
    date = inDate;
    if (!date)
        date = [NSDate date];
    
    [self runCalculation];
    
    return self;
}

// Calculation from: http://www.esrl.noaa.gov/gmd/grad/solcalc/solareqns.PDF
- (void)runCalculation
{
    // It all starts with the Julian date
    NSCalendar *calendar = [[NSCalendar alloc] initWithCalendarIdentifier:NSGregorianCalendar];
    calendar.timeZone = [NSTimeZone timeZoneForSecondsFromGMT:0];
    NSDateComponents *components = [calendar components:(NSYearCalendarUnit | NSMonthCalendarUnit | NSDayCalendarUnit | NSHourCalendarUnit | NSMinuteCalendarUnit | NSSecondCalendarUnit) fromDate:date];
    CAADate aaDate(components.year,components.month,components.day,components.hour,components.minute,components.second,true);
    double jdSun = CAADynamicalTime::UTC2TT(aaDate.Julian());
    
    // Position of the sun in equatorial
    double sunEclipticLong = CAASun::ApparentEclipticLongitude(jdSun);
    double sunEclipticLat = CAASun::ApparentEclipticLatitude(jdSun);
    double obliquity = CAANutation::TrueObliquityOfEcliptic(jdSun);
    CAA2DCoordinate sunEquatorial = CAACoordinateTransformation::Ecliptic2Equatorial(sunEclipticLong,sunEclipticLat,obliquity);
    
    double siderealTime = CAASidereal::MeanGreenwichSiderealTime(jdSun);
    
    sunLon = CAACoordinateTransformation::DegreesToRadians(15*(sunEquatorial.X-siderealTime));
    sunLat = CAACoordinateTransformation::DegreesToRadians(sunEquatorial.Y);
}

- (MaplyCoordinate3d)getDirection
{
//    WhirlyKit::Point3d pt(cos(sunLon),sin(sunLat),sin(sunLon));

    double z = sin(sunLat);
    double rad = sqrt(1.0-z*z);
    Point3d pt(rad*cos(sunLon),rad*sin(sunLon),z);

    return MaplyCoordinate3dMake(pt.x(), pt.y(), pt.z());
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

- (MaplyCoordinate)asPosition
{
    return MaplyCoordinateMake(sunLon,sunLat);
}

@end
