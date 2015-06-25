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

#import "MaplySun.h"
#import <WhirlyGlobe.h>

@implementation MaplySun
{
    NSDate *date;
    double phi,omega,decl;
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
    NSCalendar *calendar = [[NSCalendar alloc] initWithCalendarIdentifier:NSGregorianCalendar];
    NSUInteger dayOfYear = [calendar ordinalityOfUnit:NSDayCalendarUnit
                                               inUnit:NSYearCalendarUnit forDate:date];
    NSDateComponents *components = [calendar components:(NSHourCalendarUnit | NSMinuteCalendarUnit | NSSecondCalendarUnit) fromDate:date];
    NSTimeZone *timeZone = [NSTimeZone localTimeZone];
    double hour = components.hour + -timeZone.secondsFromGMT / (60*60);
    double minute = components.minute;
    double second = components.second;
    
    double gamma = 2 * M_PI / 365.0 * (dayOfYear - 1 + ((hour) - 12)/24.0);
    double eqtime = 229.18 * (0.000075 + 0.001868 * cos(gamma) - 0.032077 * sin(gamma)
                             - 0.014615 * cos(2 * gamma) - 0.040849 * sin ( 2 * gamma));
    decl = 0.006918 - 0.399912 * cos (gamma)  + 0.070257 * sin (gamma)  - 0.006758 * cos ( 2 * gamma)
    + 0.000907 * sin (2 * gamma) - 0.002697 * cos( 3 * gamma) + 0.00148 * sin (3 * gamma);
    // longitude = 0, timezone = 0
    double time_offset = eqtime;
    double tst = hour * 60 + minute + second / 60.0 + time_offset;
    // Solar angle hour in radians
    double ha = ((tst / 4.0) - 180)/180.0 * M_PI;
    // Solar zenith angle (radians)
    double cosOfPhi = 1.0 * cos(decl) * cos(ha);
    phi = acos(cosOfPhi);
    // Solar azimuth (clockwise from north)
    omega = M_PI-acos(-sin(decl)/sin(phi));
    if (hour >= 12)
        omega += M_PI;
    
    // The latitude is just the declination
    sunLat = decl;
    // And the longitude is related to the solar angle hour (duh)
    sunLon = -ha;
    if (sunLon < -M_PI)
        sunLon += 2*M_PI;
    if (sunLon > M_PI)
        sunLon -= 2*M_PI;
}

- (MaplyCoordinate3d)getDirection
{
    WhirlyKit::Point3d pt(cos(sunLon),sin(sunLat),sin(sunLon));
    pt.normalize();
    
    return MaplyCoordinate3dMake(pt.x(), pt.y(), pt.z());
}

- (MaplyLight *)makeLight
{
    MaplyLight *sunLight = [[MaplyLight alloc] init];
    sunLight.pos = [self getDirection];
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
