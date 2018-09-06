//
//  LAZShader.h
//  LidarViewer
//
//  Created by Steve Gifford on 10/27/15.
//  Copyright © 2015-2017 mousebird consulting. All rights reserved.
//

#import <UIKit/UIKit.h>
#import "MaplyShader.h"
#import "MaplyBaseViewController.h"

// Name of the point size uniform attribute.
extern NSString* const kMaplyLAZShaderPointSize;

// Name of the zMin uniform attribute (for the ramp shader)
extern NSString* const kMaplyLAZShaderZMin;

// Name of the zMax uniform attribute (for the ramp shader)
extern NSString* const kMaplyLAZShaderZMax;

// This is a simple point shader that passes colors in
MaplyShader *MaplyLAZBuildPointShader(NSObject<MaplyRenderControllerProtocol> *viewC);

// This shader uses a ramp shader texture to look up colors
MaplyShader *MaplyLAZBuildRampPointShader(NSObject<MaplyRenderControllerProtocol> *viewC,UIImage *colorRamp);
