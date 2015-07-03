/*
 *  MaplyMoon.mm
 *  WhirlyGlobe-MaplyComponent
 *
 *  Created by Steve Gifford on 7/2/15.
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

#import "MaplyMoon.h"

@implementation MaplyMoon
{
    NSDate *date;
    double Bm,Lm;
}

double ipart(double x)
{
    if (x > 0)
        return floor(x);
    else
        return ceil(x);
}

double frange(double x)
{
    double b = x / 360.0;
    double a = 360 * (b - ipart(b));
    if (a < 0)
        a += 360;
    return a;
}

#define DEG2RAD(x) (x/180.0*M_PI)

// Math borrowed from: http://www.lunar-occultations.com/rlo/ephemeris.htm
- (id)initWithDate:(NSDate *)inDate
{
    self = [super init];

    date = inDate;
    // Time since 2001 and then another year
    NSDate *j2000 = [[NSDate alloc] initWithTimeIntervalSinceReferenceDate:-366*24*60*60];
    NSTimeInterval days = [date timeIntervalSinceDate:j2000];
    // Convert to days
    days /= 24*60*60;
    
    double t = days / 36525;

    // Sun math
//    double L1 = frange(280.466 + 36000.8 * t);
    double M1 = frange(357.529+35999*t - 0.0001536* t*t + t*t*t/24490000);
    double C1 = (1.915 - 0.004817* t - 0.000014* t * t)* sin(DEG2RAD(M1));
    C1 = C1 + (0.01999 - 0.000101 * t)* sin(DEG2RAD(2*M1));
    C1 = C1 + 0.00029 * sin(DEG2RAD(3*M1));
//    double V1 = M1 + C1;
//    double Ec1 = 0.01671 - 0.00004204 * t - 0.0000001236 * t*t;
//    double R1 = 0.99972 / (1 + Ec1 * cos(V1));
//    double Th1 = L1 + C1;
//    double Om1 = frange(125.04 - 1934.1 * t);
//    double Lam1 = Th1 - 0.00569 - 0.00478 * sin(Om1);
//    double Obl = (84381.448 - 46.815 * t)/3600;
//    double Ra1 = atan2(sin(Th1) * cos(Obl) - tan(0)* sin(Obl), cos(Th1));
//    double Dec1 = asin(sin(0)* cos(Obl) + cos(0)*sin(Obl)*sin(Th1));

    double F = frange(93.2721 + 483202 * t - 0.003403 * t* t - t * t * (t/3526000));
    double L2 = frange(218.316 + 481268 * t);
//    double Om2 = frange(125.045 - 1934.14 * t + 0.002071 * t * t + t * t * t/450000);
    double M2 = frange(134.963 + 477199 * t + 0.008997 * t * t + t * t * (t/69700));
    double D = frange(297.85 + 445267 * t - 0.00163 * t * t + t * t * (t/545900));
    double D2 = 2*D;
//    double R2 = 1 + (-20954 * cos(M2) - 3699 * cos(D2 - M2) - 2956 * cos(D2)) / 385000;
//    double R3 = (R2 / R1) / 379.168831168831;
    Bm = 5.128 * sin(DEG2RAD(F)) + 0.2806 * sin(DEG2RAD(M2 + F));
    Bm = Bm + 0.2777 * sin(DEG2RAD(M2 - F)) + 0.1732 * sin(DEG2RAD(D2 - F));
    Lm = 6.289 * sin(DEG2RAD(M2)) + 1.274 * sin(DEG2RAD(D2 -M2)) + 0.6583 * sin(DEG2RAD(D2));
    Lm = Lm + 0.2136 * sin(DEG2RAD(2*M2)) - 0.1851 * sin(DEG2RAD(M1)) - 0.1143 * sin(DEG2RAD(2 * F));
    Lm = Lm +0.0588 * sin(DEG2RAD(D2 - 2*M2));
    Lm = Lm + 0.0572* sin(DEG2RAD(D2 - M1 - M2)) + 0.0533* sin(DEG2RAD(D2 + M2));
    Lm = Lm + L2;
    
    return self;
}

- (MaplyCoordinate)asCoordinate
{
    return MaplyCoordinateMakeWithDegrees(Lm, Bm);
}

- (MaplyCoordinate3d)asPosition
{
    return MaplyCoordinate3dMake(Lm/180*M_PI, Bm/180*M_PI, 6.0);
}

@end
