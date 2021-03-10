/*
 *  MaplyBaseViewController.mm
 *  MaplyComponent
 *
 *  Created by Ranen Ghosh on 11/23/16.
 *  Copyright 2012-2019 mousebird consulting
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

#import "helpers/MaplyLocationTracker.h"
#import "control/MaplyBaseViewController.h"
#import "math/MaplyCoordinate.h"
#import "visual_objects/MaplyShape.h"
#import "control/WhirlyGlobeViewController.h"
#import "MaplyViewController.h"

@implementation MaplyLocationTracker {
    CLLocationManager *_locationManager;
    bool _didRequestWhenInUseAuth;
    MaplyCoordinate _prevLoc;
    __weak MaplyBaseViewController *_theViewC;
    __weak WhirlyGlobeViewController *_globeVC;
    __weak MaplyViewController *_mapVC;
    
    __weak NSObject<MaplyLocationTrackerDelegate> *_delegate;
    __weak NSObject<MaplyLocationSimulatorDelegate> *_simDelegate;

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
    int _markerDrawPriority;
    bool updateLocationScheduled;
}

- (nonnull instancetype)initWithViewC:(MaplyBaseViewController *__nullable)viewC
                           useHeading:(bool)useHeading
                            useCourse:(bool)useCourse {
    return [self initWithViewC:viewC delegate:nil simulator:nil simInterval:0 useHeading:useHeading useCourse:useCourse];
}

- (nonnull instancetype)initWithViewC:(MaplyBaseViewController *__nullable)viewC
                             delegate:(NSObject<MaplyLocationTrackerDelegate> *__nullable)delegate
                           useHeading:(bool)useHeading
                            useCourse:(bool)useCourse {
    return [self initWithViewC:viewC delegate:delegate simulator:nil simInterval:0 useHeading:useHeading useCourse:useCourse];
}

- (nonnull instancetype)initWithViewC:(MaplyBaseViewController *__nullable)viewC
                             delegate:(NSObject<MaplyLocationTrackerDelegate> *__nullable)delegate
                            simulator:(NSObject<MaplyLocationSimulatorDelegate> *__nullable)simulator
                          simInterval:(NSTimeInterval)simInterval
                           useHeading:(bool)useHeading
                            useCourse:(bool)useCourse {
    self = [super init];
    if (!self) {
        return nil;
    }

    _theViewC = viewC;
    if ([viewC isKindOfClass:[WhirlyGlobeViewController class]])
        _globeVC = (WhirlyGlobeViewController *)viewC;
    else if ([viewC isKindOfClass:[MaplyViewController class]])
        _mapVC = (MaplyViewController *)viewC;
    
    _delegate = delegate;
    _simDelegate = simulator;
    _useHeading = useHeading;
    _useCourse = useCourse;
    _simulate = (simulator != nil);
    _lockType = MaplyLocationLockNone;
    _forwardTrackOffset = 0;
    _prevLoc = kMaplyNullCoordinate;
    _markerMinVis = 0.0;
    _markerMaxVis = 1.0;
    _markerDrawPriority = kMaplyVectorDrawPriorityDefault+1;
    
    [self setupMarkerImages];
    if (!_simulate) {
        [self setupLocationManager];
    } else {
        if (!simInterval || simInterval <= 0) {
            simInterval = 1.0;
        }
        _simUpdateTimer = [NSTimer scheduledTimerWithTimeInterval:simInterval target:self selector:@selector(simUpdateTimeout) userInfo:nil repeats:YES];
    }
    return self;
}

- (CLLocationManager *)locationManager {
    return _locationManager;
}

- (void) teardown {
    [NSObject cancelPreviousPerformRequestsWithTarget:self selector:@selector(updateLocationInternal:) object:nil];
    updateLocationScheduled = false;
    
    if (!_simulate) {
        [self teardownLocationManager];
    }
    [_simUpdateTimer invalidate];
    _simUpdateTimer = nil;
    
    _delegate = nil;
    
    if (const auto __strong vc = _theViewC) {
        if (_markerObj) {
            [vc removeObjects:@[_markerObj] mode:MaplyThreadCurrent];
            _markerObj = nil;
        }
        if (_movingMarkerObj) {
            [vc removeObjects:@[_movingMarkerObj] mode:MaplyThreadCurrent];
            _movingMarkerObj = nil;
        }
        if (_shapeCircleObj) {
            [vc removeObjects:@[_shapeCircleObj] mode:MaplyThreadCurrent];
            _shapeCircleObj = nil;
        }
        // clear the view reference so we can't create more markers
        _theViewC = nil;
    }
}

- (void) changeLockType:(MaplyLocationLockType)lockType forwardTrackOffset:(int)forwardTrackOffset {
    _lockType = lockType;
    _forwardTrackOffset = forwardTrackOffset;
}

- (void) setupMarkerImages {
    const int size = LOC_TRACKER_POS_MARKER_SIZE*2;
    
    UIColor *color0 = [UIColor colorWithRed:1.0 green:1.0 blue:1.0 alpha:1.0];
    UIColor *color1 = [UIColor colorWithRed:0.0 green:0.75 blue:1.0 alpha:1.0];
    
    if (const auto __strong vc = _theViewC) {
        _markerImgs = [NSMutableArray array];
        _markerImgsDirectional = [NSMutableArray array];
        for (int i=0; i<16; i++) {
            const auto gradLoc = (float)(8-ABS(8-i)) / 8.0f;
            const auto radius = (float)(size-32-ABS(8-i)) / 2.0f;
            [_markerImgs addObject:
                [vc addTexture:[self radialGradientMarkerWithSize:size color0:color0 color1:color1
                                                     gradLocation:gradLoc radius:radius directional:false]
                          desc:nil mode:MaplyThreadCurrent]];
            [_markerImgsDirectional addObject:
                [vc addTexture:[self radialGradientMarkerWithSize:size color0:color0 color1:color1
                                                     gradLocation:gradLoc radius:radius directional:true]
                          desc:nil mode:MaplyThreadCurrent]];
        }
    }

    _markerDesc = [NSMutableDictionary dictionaryWithDictionary:@{
        kMaplyMinVis: @(_markerMinVis),
        kMaplyMaxVis: @(_markerMaxVis),
        kMaplyFade: @(0.0),
        kMaplyDrawPriority:@(_markerDrawPriority),
        kMaplyEnableEnd: @(MAXFLOAT)}];
    
    _movingMarkerDesc = [NSMutableDictionary dictionaryWithDictionary:@{
        kMaplyMinVis: @(_markerMinVis),
        kMaplyMaxVis: @(_markerMaxVis),
        kMaplyFade: @(0.0),
        kMaplyDrawPriority:@(_markerDrawPriority),
        kMaplyEnableStart:@(0.0)}];
    
    _shapeCircleDesc = [NSMutableDictionary dictionaryWithDictionary:@{
        kMaplyColor : [UIColor colorWithRed:0.06 green:0.06 blue:0.1 alpha:0.2],
        kMaplyFade: @(0.0),
        kMaplyDrawPriority: @(_markerDrawPriority-1),
        kMaplySampleX: @(100),
        kMaplyZBufferRead: @(false)}];
}

- (int)markerDrawPriority
{
    return _markerDrawPriority;
}

- (void)setMarkerDrawPriority:(int)markerDrawPriority
{
    _markerDrawPriority = markerDrawPriority;
    
    [self setupMarkerImages];
}

- (UIImage *)radialGradientMarkerWithSize:(int)size color0:(UIColor *)color0 color1:(UIColor *)color1
                             gradLocation:(float)gradLocation radius:(float)radius directional:(bool)directional {
    
    /// "When you are done modifying the context, you must call the UIGraphicsEndImageContext() function to clean
    /// up the bitmap drawing environment and remove the graphics context from the top of the context stack. You
    /// should not use the UIGraphicsPopContext() function to remove this type of context from the stack."
    UIGraphicsBeginImageContextWithOptions(CGSizeMake(size, size), NO, 0.0f);
    @try {
        CGContextRef ctx = UIGraphicsGetCurrentContext();
        if (!ctx) {
            return nil;
        }

        CGContextSaveGState(ctx);
        
        /// "You are responsible for releasing this object by calling CGColorSpaceRelease"
        CGColorSpaceRef baseSpace = CGColorSpaceCreateDeviceRGB();
        CGGradientRef gradient;
        @try {
            const CGFloat locations[] = { 0.0f, gradLocation };
            const CGFloat *components0 = CGColorGetComponents(color0.CGColor);
            const CGFloat *components1 = CGColorGetComponents(color1.CGColor);
            const CGFloat colorComponents[8] = {
                components0[0], components0[1], components0[2], components0[3],
                components1[0], components1[1], components1[2], components1[3],
            };
            gradient = CGGradientCreateWithColorComponents(baseSpace, colorComponents, locations, 2);
        } @finally {
            CGColorSpaceRelease(baseSpace);
        }
        
        @try {
            const CGPoint gradCenter = CGPointMake(size/2, size/2);
            
            // Draw translucent outline
            CGRect outlineRect = CGRectMake(0, 0, size, size);
            UIColor *translucentColor = [[UIColor whiteColor] colorWithAlphaComponent:0.5];
            CGContextSetFillColorWithColor(ctx, translucentColor.CGColor);
            CGContextFillEllipseInRect(ctx, outlineRect);
            
            // Draw direction indicator triangle
            if (directional) {
                float len = 20.0;
                float height = 12.0;
                /// "You are responsible for releasing this object."
                CGMutablePathRef path = CGPathCreateMutable();
                @try {
                    CGPathMoveToPoint(path, NULL,    size/2, size/2-radius-len);
                    CGPathAddLineToPoint(path, NULL, size/2-height, size/2-radius);
                    CGPathAddLineToPoint(path, NULL, size/2+height, size/2-radius);
                    CGPathCloseSubpath(path);
                    CGContextSetFillColorWithColor(ctx, color1.CGColor);
                    CGContextAddPath(ctx, path);
                    CGContextFillPath(ctx);
                } @finally {
                    CGPathRelease(path);
                }
            }

            // Draw white outline
            outlineRect = CGRectMake(size/2-radius-4, size/2-radius-4, 2*radius+8, 2*radius+8);
            CGContextSetFillColorWithColor(ctx, [UIColor whiteColor].CGColor);
            CGContextFillEllipseInRect(ctx, outlineRect);
            
            // Draw gradient center
            CGContextDrawRadialGradient(ctx, gradient, gradCenter, 0, gradCenter, radius, kCGGradientDrawsBeforeStartLocation);
        } @finally {
            CGGradientRelease(gradient);
        }
        
        CGContextRestoreGState(ctx);
        return UIGraphicsGetImageFromCurrentImageContext();
    } @finally {
        UIGraphicsEndImageContext();
    }
}


- (void) setupLocationManager {
    if (_locationManager)
        return;
    const CLAuthorizationStatus authStatus = [CLLocationManager authorizationStatus];
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
    const auto orientation = [[UIApplication sharedApplication] statusBarOrientation];
    switch (orientation) {
        case UIInterfaceOrientationPortrait:
            _locationManager.headingOrientation = CLDeviceOrientationPortrait;
            break;
        case UIInterfaceOrientationPortraitUpsideDown:
            _locationManager.headingOrientation = CLDeviceOrientationPortraitUpsideDown;
            break;
        case UIInterfaceOrientationLandscapeRight:
            _locationManager.headingOrientation = CLDeviceOrientationLandscapeRight;
            break;
        case UIInterfaceOrientationLandscapeLeft:
            _locationManager.headingOrientation = CLDeviceOrientationLandscapeLeft;
            break;
        default: // UIInterfaceOrientationUnknown
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
    
    const MaplyCoordinate coord1 = [self coordOfPointAtTrueCourse:0.0 andDistanceMeters:horizontalAccuracy fromCoord:coord];
    const MaplyCoordinate coord2 = [self coordOfPointAtTrueCourse:90.0 andDistanceMeters:horizontalAccuracy fromCoord:coord];

    const auto __strong vc = _theViewC;
    const auto __strong mvc = _mapVC;
    const auto __strong gvc = _globeVC;
    
    const MaplyCoordinate3d dispPt0 = [vc displayPointFromGeo:coord];
    const MaplyCoordinate3d dispPt1 = [vc displayPointFromGeo:coord1];
    const MaplyCoordinate3d dispPt2 = [vc displayPointFromGeo:coord2];
    
    const float d1 = sqrtf(powf(dispPt1.x-dispPt0.x, 2.0) + powf(dispPt1.y-dispPt0.y, 2.0));
    const float d2 = sqrtf(powf(dispPt2.x-dispPt0.x, 2.0) + powf(dispPt2.y-dispPt0.y, 2.0));
    shapeCircle.radius = (d1 + d2) / 2.0;
    
    float minHeight = 0.0;
    if (gvc)
        minHeight = [gvc getZoomLimitsMin];
    else {
        float maxHeight;
        [mvc getZoomLimitsMin:&minHeight max:&maxHeight];
    }
    shapeCircle.height = minHeight * 0.01;

    return shapeCircle;
}

// When using a simulated track, locations seem to come in fast and furious, overwhelming the renderer.
// This slows things down to a level that can actually be seen
- (void)updateLocation:(CLLocation *)location {
    if (!updateLocationScheduled) {
        updateLocationScheduled = true;
        [self performSelector:@selector(updateLocationInternal:) withObject:location afterDelay:0.0];
    }
}

- (void)updateLocationInternal:(CLLocation *)location {
    updateLocationScheduled = false;
    
    __strong MaplyBaseViewController *theViewC = _theViewC;
    if (!theViewC)
        return;
    
    MaplyCoordinate endLoc = MaplyCoordinateMakeWithDegrees(location.coordinate.longitude, location.coordinate.latitude);
    MaplyCoordinate startLoc;
    
    if (_markerObj || _movingMarkerObj) {
        startLoc = _prevLoc;
        if (_markerObj)
            [theViewC removeObjects:@[_markerObj] mode:MaplyThreadCurrent];
        if (_movingMarkerObj)
            [theViewC removeObjects:@[_movingMarkerObj] mode:MaplyThreadCurrent];
        _markerObj = nil;
        _movingMarkerObj = nil;
    } else
        startLoc = endLoc;
    
    if (_shapeCircleObj) {
        [theViewC removeObjects:@[_shapeCircleObj] mode:MaplyThreadCurrent];
        _shapeCircleObj = nil;
    }
    
    if (location.horizontalAccuracy >= 0) {
        MaplyShapeCircle *shapeCircle = [self shapeCircleForCoord:endLoc AndHorizontalAccuracy:location.horizontalAccuracy];
        if (shapeCircle) {
            _shapeCircleObj = [theViewC addShapes:@[shapeCircle] desc:_shapeCircleDesc mode:MaplyThreadCurrent];
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
        
        _movingMarkerObj = [theViewC addScreenMarkers:@[movingMarker] desc:_movingMarkerDesc mode:MaplyThreadCurrent];
        _markerObj = [theViewC addScreenMarkers:@[marker] desc:_markerDesc mode:MaplyThreadCurrent];
        
        [self lockToLocation:endLoc heading:(orientation ? orientation.floatValue : 0.0)];
        
        _prevLoc = endLoc;
    }
    
    __strong NSObject<MaplyLocationTrackerDelegate> *delegate = _delegate;
    if ([delegate respondsToSelector:@selector(updateLocation:)]) {
        [delegate updateLocation:location];
    }
}

- (void) lockToLocation:(MaplyCoordinate)location heading:(float)heading{
    __strong WhirlyGlobeViewController *globeVC = _globeVC;
    __strong MaplyViewController *mapVC = _mapVC;
//    __strong NSObject<MaplyLocationTrackerDelegate> *delegate = _delegate;
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
    [delegate locationManager:manager didFailWithError:error];
}

- (void) locationManager:(CLLocationManager *)manager didChangeAuthorizationStatus:(CLAuthorizationStatus)status {
    __strong NSObject<MaplyLocationTrackerDelegate> *delegate = _delegate;
    [delegate locationManager:manager didChangeAuthorizationStatus:status];

    switch (status) {
        case kCLAuthorizationStatusNotDetermined:
            break;
        case kCLAuthorizationStatusDenied:
        case kCLAuthorizationStatusRestricted:
            [self teardownLocationManager];
            break;
        case kCLAuthorizationStatusAuthorizedWhenInUse:
        case kCLAuthorizationStatusAuthorizedAlways:
            [_locationManager startUpdatingLocation];
            if (_useHeading)
                [_locationManager startUpdatingHeading];
            break;
    }
}

- (void) locationManager:(CLLocationManager *)manager didUpdateLocations:(NSArray *)locations {
    [self updateLocation:[locations lastObject]];
}

- (void) locationManager:(CLLocationManager *)manager didUpdateHeading:(nonnull CLHeading *)newHeading {
    _latestHeading = (newHeading.headingAccuracy >= 0) ? @(newHeading.trueHeading) : nil;
}

- (void) simUpdateTimeout {
    __strong NSObject<MaplyLocationSimulatorDelegate> *delegate = _simDelegate;
    if ([delegate respondsToSelector:@selector(getSimulationPoint)] &&
        (![delegate respondsToSelector:@selector(hasValidLocation)] || [delegate hasValidLocation])) {
        [self setLocation:[delegate getSimulationPoint]
                 altitude:10000.0
       horizontalAccuracy:250
         verticalAccuracy:15
                    speed:0];   // todo: calculate speed from positions
    }
}

- (MaplyCoordinate)getLocation {
    return _prevLoc;
}

- (void) setLocation:(MaplyLocationTrackerSimulationPoint)point
            altitude:(double)altitude {
    [self setLocation:point
             altitude:altitude
   horizontalAccuracy:250
     verticalAccuracy:15
                speed:0];
}

- (void) setLocation:(MaplyLocationTrackerSimulationPoint)point
            altitude:(double)altitude
  horizontalAccuracy:(double)horizontalAccuracy
    verticalAccuracy:(double)verticalAccuracy
               speed:(double)speed {
    const float lonDeg = point.lonDeg;
    const float latDeg = point.latDeg;
    const float hdgDeg = point.headingDeg;

    _latestHeading = @(hdgDeg);
    CLLocation *location = [[CLLocation alloc] initWithCoordinate:{latDeg, lonDeg}
                                                         altitude:altitude
                                               horizontalAccuracy:horizontalAccuracy
                                                 verticalAccuracy:verticalAccuracy
                                                           course:hdgDeg
                                                            speed:speed
                                                        timestamp:[NSDate date]];
    [self updateLocation:location];
}

@end

