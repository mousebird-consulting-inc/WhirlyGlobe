//
//  MapyTestCase.h
//  AutoTester
//
//  Created by jmnavarro on 13/10/15.
//  Copyright 2015-2017 mousebird consulting.
//

#import <UIKit/UIKit.h>
#import <WhirlyGlobe/WhirlyGlobeComponent.h>
#import <WhirlyGlobe/MaplyComponent.h>
#import <WhirlyGlobe/MaplyComponentObject.h>
#import <WhirlyGlobe/MaplyRenderController.h>
#import <WhirlyGlobe/MaplyViewController.h>
#import <WhirlyGlobe/WhirlyGlobeViewController.h>

@class MaplyViewController;
@class MaplyBaseViewController;
@class WhirlyGlobeViewController;
@class MaplyTestCase;
@class MaplyCoordinateSystem;
@protocol WhirlyGlobeViewControllerDelegate;
@protocol MaplyViewControllerDelegate;
@protocol MaplyErrorReportingDelegate;

typedef void (^TestCaseResult)(MaplyTestCase * _Nonnull testCase);

typedef NS_OPTIONS(NSUInteger, MaplyTestCaseImplementations) {
	MaplyTestCaseImplementationGlobe = 1 << 1,
	MaplyTestCaseImplementationMap   = 1 << 2,
};

@interface MaplyTestCase : NSObject <WhirlyGlobeViewControllerDelegate,
                                     MaplyViewControllerDelegate,
                                     MaplyErrorReportingDelegate>

- (instancetype _Nonnull)init;
- (instancetype _Nonnull)initWithName:(NSString *_Nonnull)name supporting:(MaplyTestCaseImplementations)types;

- (void)startGlobe:(UINavigationController * __nonnull)nav;
- (void)startMap:(UINavigationController * __nonnull)nav;

// Called synchronously after the controller is created but before it's attached
- (void)preSetUpWithGlobe:(WhirlyGlobeViewController * _Nonnull)globeVC;
- (void)preSetUpWithMap:(MaplyViewController * _Nonnull)mapVC;

// Called asynchronously after the controller is attached to the view
- (void)setUpWithGlobe:(WhirlyGlobeViewController * _Nonnull)globeVC;
- (void)setUpWithMap:(MaplyViewController * _Nonnull)mapVC;

- (void)stop;
- (MaplyCoordinateSystem * _Nullable)customCoordSystem;

- (void)onError:(NSError * __nonnull)err
        withTag:(NSString * __nonnull)tag
          viewC:(NSObject<MaplyRenderControllerProtocol> * __nonnull)viewC;
- (void)onException:(NSException * __nonnull)err
            withTag:(NSString * __nonnull)tag
              viewC:(NSObject<MaplyRenderControllerProtocol> * __nonnull)viewC;

@property (nonatomic, strong) NSString * _Nonnull name;

@property (nonatomic) MaplyTestCaseImplementations implementations;

@property (nonatomic, weak) MaplyBaseViewController * _Nullable baseViewController;
@property (nonatomic, strong) WhirlyGlobeViewController *_Nullable globeViewController;
@property (nonatomic, strong) MaplyViewController * _Nullable mapViewController;

@end
