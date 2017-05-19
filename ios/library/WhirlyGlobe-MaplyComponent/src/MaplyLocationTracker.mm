/*
 *  MaplyBaseViewController.mm
 *  MaplyComponent
 *
 *  Created by Ranen Ghosh on 11/23/16.
 *  Copyright 2012-2017 mousebird consulting
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

#import "MaplyLocationTracker.h"
#import "MaplyBaseViewController.h"
#import "MaplyCoordinate.h"
#import "MaplyShape.h"
#import "WhirlyGlobeViewController.h"
#import "MaplyViewController.h"

@implementation MaplyLocationTracker {
    CLLocationManager *_locationManager;
    bool _didRequestWhenInUseAuth;
    MaplyCoordinate _prevLoc;
    __weak MaplyBaseViewController *_theViewC;
    __weak WhirlyGlobeViewController *_globeVC;
    __weak MaplyViewController *_mapVC;
    
    __weak NSObject<MaplyLocationTrackerDelegate> *_delegate;
    
    NSMutableArray *_markerImgs, *_markerImgsDirectional;
    
    MaplyComponentObject *_markerObj;
    MaplyComponentObject *_movingMarkerObj;
    MaplyComponentObject *_shapeCircleObj;
    NSMutableDictionary *_markerDesc, *_movingMarkerDesc, *_shapeCircleDesc;
    
    NSNumber *_latestHeading;
    
    NSTimer *_simUpdateTimer;
    bool _simulate;
    
    bool _useHeading, _useCourse;
    MaplyLocationLockType _lockType;
    int _forwardTrackOffset;
}

- (nonnull instancetype)initWithViewC:(MaplyBaseViewController *__nullable)viewC delegate:(NSObject<MaplyLocationTrackerDelegate> *__nullable)delegate useHeading:(bool)useHeading useCourse:(bool)useCourse simulate:(bool)simulate {
    
    self = [super init];
    if (self) {
        _theViewC = viewC;
        if ([viewC isKindOfClass:[WhirlyGlobeViewController class]])
            _globeVC = (WhirlyGlobeViewController *)viewC;
        else if ([viewC isKindOfClass:[MaplyViewController class]])
            _mapVC = (MaplyViewController *)viewC;
        
        _delegate = delegate;
        _useHeading = useHeading;
        _useCourse = useCourse;
        _simulate = simulate;
        _lockType = MaplyLocationLockNone;
        _forwardTrackOffset = 0;
        _prevLoc = kMaplyNullCoordinate;
        
        [self setupMarkerImages];
        if (!_simulate)
            [self setupLocationManager];
        else {
            _simUpdateTimer = [NSTimer scheduledTimerWithTimeInterval:1.0 target:self selector:@selector(simUpdateTimeout) userInfo:nil repeats:YES];
        }
        
    }
    return self;
}

- (CLLocationManager *)locationManager {
    return _locationManager;
}

- (void) teardown {
    if (!_simulate)
        [self teardownLocationManager];
    _delegate = nil;
    if (_markerObj) {
        [_theViewC removeObject:_markerObj];
        [_theViewC removeObject:_movingMarkerObj];
        _markerObj = nil;
        _movingMarkerObj = nil;
    }
    if (_shapeCircleObj) {
        [_theViewC removeObject:_shapeCircleObj];
        _shapeCircleObj = nil;
    }
}

- (void) changeLockType:(MaplyLocationLockType)lockType forwardTrackOffset:(int)forwardTrackOffset {
    _lockType = lockType;
    _forwardTrackOffset = forwardTrackOffset;
}

- (void) setupMarkerImages {
    int size = LOC_TRACKER_POS_MARKER_SIZE*2;
    
    UIColor *color0 = [UIColor colorWithRed:1.0 green:1.0 blue:1.0 alpha:1.0];
    UIColor *color1 = [UIColor colorWithRed:0.0 green:0.75 blue:1.0 alpha:1.0];
    
    _markerImgs = [NSMutableArray array];
    _markerImgsDirectional = [NSMutableArray array];
    for (int i=0; i<16; i++) {
        [_markerImgs addObject:[self radialGradientMarkerWithSize:size color0:color0 color1:color1 gradLocation:(0.0 + (float)(8-ABS(8-i))/8.0) radius:(float)(size-32-ABS(8-i))/2.0 directional:false]];
        [_markerImgsDirectional addObject:[self radialGradientMarkerWithSize:size color0:color0 color1:color1 gradLocation:(0.0 + (float)(8-ABS(8-i))/8.0) radius:(float)(size-32-ABS(8-i))/2.0 directional:true]];
    }
    
    _markerDesc = [NSMutableDictionary dictionaryWithDictionary:@{kMaplyMinVis: @(0.0), kMaplyMaxVis: @(1.0), kMaplyFade: @(0.0), kMaplyDrawPriority:@(kMaplyVectorDrawPriorityDefault+1), kMaplyEnableEnd: @(MAXFLOAT)}];
    
    _movingMarkerDesc = [NSMutableDictionary dictionaryWithDictionary:@{kMaplyMinVis: @(0.0), kMaplyMaxVis: @(1.0), kMaplyFade: @(0.0), kMaplyDrawPriority:@(kMaplyVectorDrawPriorityDefault+1), kMaplyEnableStart:@(0.0)}];
    
    _shapeCircleDesc = [NSMutableDictionary dictionaryWithDictionary:@{kMaplyColor : [UIColor colorWithRed:0.06 green:0.06 blue:0.1 alpha:0.2], kMaplyFade: @(0.0), kMaplyDrawPriority: @(kMaplyVectorDrawPriorityDefault), kMaplySampleX: @(100)}];
    
}


- (UIImage *)radialGradientMarkerWithSize:(int)size color0:(UIColor *)color0 color1:(UIColor *)color1 gradLocation:(float)gradLocation radius:(float)radius directional:(bool)directional {
    
    UIGraphicsBeginImageContextWithOptions(CGSizeMake(size, size), NO, 0.0f);
    CGContextRef ctx = UIGraphicsGetCurrentContext();
    CGContextSaveGState(ctx);
    
    CGColorSpaceRef baseSpace = CGColorSpaceCreateDeviceRGB();
    
    CGFloat colorComponents[8];
    const CGFloat *components0 = CGColorGetComponents(color0.CGColor);
    const CGFloat *components1 = CGColorGetComponents(color1.CGColor);
    colorComponents[0] = components0[0];
    colorComponents[1] = components0[1];
    colorComponents[2] = components0[2];
    colorComponents[3] = components0[3];
    colorComponents[4] = components1[0];
    colorComponents[5] = components1[1];
    colorComponents[6] = components1[2];
    colorComponents[7] = components1[3];
    
    CGFloat locations[] = {0.0, gradLocation};
    
    CGGradientRef gradient = CGGradientCreateWithColorComponents(baseSpace, colorComponents, locations, 2);
    CGColorSpaceRelease(baseSpace);
    
    CGPoint gradCenter = CGPointMake(size/2, size/2);
    
    // Draw translucent outline
    CGRect outlineRect = CGRectMake(0, 0, size, size);
    UIColor *translucentColor = [[UIColor whiteColor] colorWithAlphaComponent:0.5];
    CGContextSetFillColorWithColor(ctx, translucentColor.CGColor);
    CGContextFillEllipseInRect(ctx, outlineRect);
    
    // Draw direction indicator triangle
    if (directional) {
        CGMutablePathRef path = CGPathCreateMutable();
        CGPathMoveToPoint(path, NULL,    size/2, size/2-radius-20);
        CGPathAddLineToPoint(path, NULL, size/2-12, size/2-radius);
        CGPathAddLineToPoint(path, NULL, size/2+12, size/2-radius);
        CGPathCloseSubpath(path);
        CGContextSetFillColorWithColor(ctx, color1.CGColor);
        CGContextAddPath(ctx, path);
        CGContextFillPath(ctx);
        CGPathRelease(path);
    }

    // Draw white outline
    outlineRect = CGRectMake(size/2-radius-4, size/2-radius-4, 2*radius+8, 2*radius+8);
    CGContextSetFillColorWithColor(ctx, [UIColor whiteColor].CGColor);
    CGContextFillEllipseInRect(ctx, outlineRect);
    
    // Draw gradient center
    CGContextDrawRadialGradient(ctx, gradient, gradCenter, 0, gradCenter, radius, kCGGradientDrawsBeforeStartLocation);
    CGGradientRelease(gradient);
    
    CGContextRestoreGState(ctx);
    UIImage *img = UIGraphicsGetImageFromCurrentImageContext();
    UIGraphicsEndImageContext();
    
    return img;
}


- (void) setupLocationManager {
    if (_locationManager)
        return;
    CLAuthorizationStatus authStatus = [CLLocationManager authorizationStatus];
    if (authStatus == kCLAuthorizationStatusRestricted || authStatus == kCLAuthorizationStatusDenied) {
        return;
    }
    _locationManager = [[CLLocationManager alloc] init];
    _locationManager.delegate = self;
    _locationManager.desiredAccuracy = kCLLocationAccuracyBest;
    
    if ([_locationManager respondsToSelector:@selector(requestWhenInUseAuthorization)]) {
        if (!_didRequestWhenInUseAuth) {
            // Sending a message to avoid compile time error
            [[UIApplication sharedApplication] sendAction:@selector(requestWhenInUseAuthorization)
                                                       to:_locationManager
                                                     from:self
                                                 forEvent:nil];
            _didRequestWhenInUseAuth = true;
        }
    } else {
        [[UIApplication sharedApplication] sendAction:@selector(startUpdatingLocation)
                                                   to:_locationManager
                                                 from:self
                                             forEvent:nil];
        if (_useHeading)
            [[UIApplication sharedApplication] sendAction:@selector(startUpdatingHeading)
                                                       to:_locationManager
                                                     from:self
                                                 forEvent:nil];
    }
    
    if (_useHeading)
        [[NSNotificationCenter defaultCenter] addObserver:self selector:@selector(orientationChanged:) name:UIDeviceOrientationDidChangeNotification object:nil];
}

- (void)orientationChanged:(NSNotification *)notification {
    UIInterfaceOrientation orientation = [[UIApplication sharedApplication] statusBarOrientation];
    switch (orientation) {
        case UIDeviceOrientationPortrait:
            _locationManager.headingOrientation = CLDeviceOrientationPortrait;
            break;
        case UIDeviceOrientationPortraitUpsideDown:
            _locationManager.headingOrientation = CLDeviceOrientationPortraitUpsideDown;
            break;
        case UIDeviceOrientationLandscapeRight:
            _locationManager.headingOrientation = CLDeviceOrientationLandscapeRight;
            break;
        case UIDeviceOrientationLandscapeLeft:
            _locationManager.headingOrientation = CLDeviceOrientationLandscapeLeft;
            break;
        default: // UIDeviceOrientationUnknown
            _locationManager.headingOrientation = CLDeviceOrientationPortrait;
            break;
    }
    
}

- (void) teardownLocationManager {
    if (!_locationManager)
        return;
    [_locationManager stopUpdatingLocation];
    if (_useHeading)
        [_locationManager stopUpdatingHeading];
    if (_useHeading)
        [[NSNotificationCenter defaultCenter] removeObserver:self];
    _locationManager.delegate = nil;
    _locationManager = nil;
    _didRequestWhenInUseAuth = false;
}

- (MaplyCoordinate) coordOfPointAtTrueCourse:(double)tcDeg andDistanceMeters:(double)dMeters fromCoord:(MaplyCoordinate)coord;
{
    // http://www.movable-type.co.uk/scripts/latlong.html
    double tcRad = tcDeg * M_PI/180.0;
    double lat1 = coord.y;
    double lon1 = -coord.x;
    
    double dRadians = dMeters / 6.371e6;
    
    double latRad, lonRad;
    
    latRad = asin(sin(lat1)*cos(dRadians)+cos(lat1)*sin(dRadians)*cos(tcRad));
    
    if (cos(latRad) == 0)
        lonRad = lon1;
    else
        lonRad = fmod(lon1-asin(sin(tcRad)*sin(dRadians)/cos(latRad))+M_PI,2.0*M_PI)-M_PI;
    
    
    return MaplyCoordinateMake(-lonRad, latRad);
}


- (MaplyShapeCircle *)shapeCircleForCoord:(MaplyCoordinate)coord AndHorizontalAccuracy:(int)horizontalAccuracy {
    
    MaplyShapeCircle *shapeCircle = [[MaplyShapeCircle alloc] init];
    shapeCircle.center = coord;
    
    MaplyCoordinate coord1 = [self coordOfPointAtTrueCourse:0.0 andDistanceMeters:horizontalAccuracy fromCoord:coord];
    MaplyCoordinate coord2 = [self coordOfPointAtTrueCourse:90.0 andDistanceMeters:horizontalAccuracy fromCoord:coord];
    
    MaplyCoordinate3d dispPt0 = [_theViewC displayPointFromGeo:coord];
    MaplyCoordinate3d dispPt1 = [_theViewC displayPointFromGeo:coord1];
    MaplyCoordinate3d dispPt2 = [_theViewC displayPointFromGeo:coord2];
    
    float d1 = sqrtf(powf(dispPt1.x-dispPt0.x, 2.0) + powf(dispPt1.y-dispPt0.y, 2.0));
    float d2 = sqrtf(powf(dispPt2.x-dispPt0.x, 2.0) + powf(dispPt2.y-dispPt0.y, 2.0));
    shapeCircle.radius = (d1 + d2) / 2.0;
    shapeCircle.height = 0.00001;
    
    return shapeCircle;
}

- (void)updateLocation:(CLLocation *)location {
    __strong MaplyBaseViewController *theViewC = _theViewC;
    if (!theViewC)
        return;
    
    MaplyCoordinate endLoc = MaplyCoordinateMakeWithDegrees(location.coordinate.longitude, location.coordinate.latitude);
    MaplyCoordinate startLoc;
    
    if (_markerObj) {
        startLoc = _prevLoc;
        [_theViewC removeObject:_markerObj];
        [_theViewC removeObject:_movingMarkerObj];
        _markerObj = nil;
        _movingMarkerObj = nil;
    } else
        startLoc = endLoc;
    
    if (_shapeCircleObj) {
        [theViewC removeObject:_shapeCircleObj];
        _shapeCircleObj = nil;
    }
    
    if (location.horizontalAccuracy >= 0) {
        MaplyShapeCircle *shapeCircle = [self shapeCircleForCoord:endLoc AndHorizontalAccuracy:location.horizontalAccuracy];
        if (shapeCircle) {
            _shapeCircleObj = [_theViewC addShapes:@[shapeCircle] desc:_shapeCircleDesc];
        }
        
        NSNumber *orientation;
        if (_useHeading && _latestHeading)
            orientation = _latestHeading;
        else if (_useCourse && location.course >= 0)
            orientation = @(location.course);
            
        NSArray *markerImages;
        if (orientation)
            markerImages = _markerImgsDirectional;
        else
            markerImages = _markerImgs;
        
        MaplyMovingScreenMarker *movingMarker = [[MaplyMovingScreenMarker alloc] init];
        movingMarker.loc = startLoc;
        movingMarker.endLoc = endLoc;
        movingMarker.duration = 0.5;
        
        movingMarker.period = 1.0;
        movingMarker.size = CGSizeMake(LOC_TRACKER_POS_MARKER_SIZE, LOC_TRACKER_POS_MARKER_SIZE);
        if (orientation)
            movingMarker.rotation = -M_PI/180.0 * orientation.doubleValue;
        movingMarker.images = markerImages;
        movingMarker.layoutImportance = MAXFLOAT;
        
        MaplyScreenMarker *marker = [[MaplyScreenMarker alloc] init];
        marker.loc = endLoc;
        
        marker.period = 1.0;
        marker.size = CGSizeMake(LOC_TRACKER_POS_MARKER_SIZE, LOC_TRACKER_POS_MARKER_SIZE);
        if (orientation)
            marker.rotation = -M_PI/180.0 * orientation.doubleValue;
        marker.images = markerImages;
        marker.layoutImportance = MAXFLOAT;
        
        NSTimeInterval ti = [NSDate timeIntervalSinceReferenceDate]+0.5;
        _markerDesc[kMaplyEnableStart] = _movingMarkerDesc[kMaplyEnableEnd] = @(ti);
        
        _movingMarkerObj = [_theViewC addScreenMarkers:@[movingMarker] desc:_movingMarkerDesc];
        _markerObj = [_theViewC addScreenMarkers:@[marker] desc:_markerDesc];
        
        [self lockToLocation:endLoc heading:(orientation ? orientation.floatValue : 0.0)];
        
        _prevLoc = endLoc;
    }
    
    __strong NSObject<MaplyLocationTrackerDelegate> *delegate = _delegate;
    if (delegate && [delegate respondsToSelector:@selector(updateLocation:)]) {
        [delegate updateLocation:location];
    }
}

- (void) lockToLocation:(MaplyCoordinate)location heading:(float)heading{
    __strong WhirlyGlobeViewController *globeVC = _globeVC;
    __strong MaplyViewController *mapVC = _mapVC;
    __strong NSObject<MaplyLocationTrackerDelegate> *delegate = _delegate;
    if (!globeVC && !mapVC)
        return;
    
//    MaplyCoordinateD locationD = MaplyCoordinateDMakeWithMaplyCoordinate(location);
    
    switch (_lockType) {
        case MaplyLocationLockNone:
            break;
        case MaplyLocationLockNorthUp:
            if (globeVC)
                [globeVC animateToPosition:location height:[globeVC getHeight] heading:0.0 time:0.5];
            else if (mapVC)
                [mapVC animateToPosition:location height:[mapVC getHeight] heading:0.0 time:0.5];
            break;
        case MaplyLocationLockHeadingUp:
            if (globeVC)
                [globeVC animateToPosition:location height:[globeVC getHeight] heading:fmod(M_PI/180.0 * heading + 2.0*M_PI, 2.0*M_PI) time:0.5];
            else if (mapVC)
                [mapVC animateToPosition:location height:[mapVC getHeight] heading:fmod(M_PI/180.0 * heading + 2.0*M_PI, 2.0*M_PI) time:0.5];
            break;
        case MaplyLocationLockHeadingUpOffset:
            if (globeVC)
                [globeVC animateToPosition:location onScreen:CGPointMake(0, -_forwardTrackOffset) height:[globeVC getHeight] heading:fmod(M_PI/180.0 * heading + 2.0*M_PI, 2.0*M_PI) time:0.5];
            else if (mapVC)
                [mapVC animateToPosition:location onScreen:CGPointMake(0, -_forwardTrackOffset) height:[mapVC getHeight] heading:fmod(M_PI/180.0 * heading + 2.0*M_PI, 2.0*M_PI) time:0.5];
            break;
        default:
            break;
    }
    
}

#pragma mark
#pragma mark CLLocationManagerDelegate Methods

- (void) locationManager:(CLLocationManager *)manager didFailWithError:(NSError *)error {
    __strong NSObject<MaplyLocationTrackerDelegate> *delegate = _delegate;
    _prevLoc = kMaplyNullCoordinate;
    _latestHeading = nil;
    if (delegate)
        [delegate locationManager:manager didFailWithError:error];
}

- (void) locationManager:(CLLocationManager *)manager didChangeAuthorizationStatus:(CLAuthorizationStatus)status {

    __strong NSObject<MaplyLocationTrackerDelegate> *delegate = _delegate;
    if (delegate)
        [delegate locationManager:manager didChangeAuthorizationStatus:status];
    
    if (status == kCLAuthorizationStatusNotDetermined) {
        return;
    }
    if (status == kCLAuthorizationStatusDenied || status == kCLAuthorizationStatusRestricted) {
        [self teardownLocationManager];
    } else if (status == kCLAuthorizationStatusAuthorized || status == kCLAuthorizationStatusAuthorizedWhenInUse || status == kCLAuthorizationStatusAuthorizedAlways) {
        
        [_locationManager startUpdatingLocation];
        if (_useHeading)
            [_locationManager startUpdatingHeading];
    }
}

- (void) locationManager:(CLLocationManager *)manager didUpdateLocations:(NSArray *)locations {
    
    CLLocation *location = [locations lastObject];
    
    [self updateLocation:location];
    
}

- (void) locationManager:(CLLocationManager *)manager didUpdateHeading:(nonnull CLHeading *)newHeading {
    
    if (newHeading.headingAccuracy < 0)
        _latestHeading = nil;
    else
        _latestHeading = @(newHeading.trueHeading);
}

- (void) simUpdateTimeout {
    __strong NSObject<MaplyLocationTrackerDelegate> *delegate = _delegate;
    
    if (!delegate || ![delegate respondsToSelector:@selector(getSimulationPoint)])
        return;
    
    MaplyLocationTrackerSimulationPoint simPoint = [delegate getSimulationPoint];
    
    float lonDeg = simPoint.lonDeg;
    float latDeg = simPoint.latDeg;
    float hdgDeg = simPoint.headingDeg;
    
    _latestHeading = @(hdgDeg);
    CLLocation *location = [[CLLocation alloc] initWithCoordinate:(CLLocationCoordinate2D){latDeg, lonDeg} altitude:10000.0 horizontalAccuracy:250 verticalAccuracy:15 course:hdgDeg speed:0 timestamp:[NSDate date]];
    [self updateLocation:location];
}

- (MaplyCoordinate)getLocation {
    return _prevLoc;
}

@end

