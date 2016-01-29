/*
 *  MaplyGeomModelBuilder.h
 *  WhirlyGlobeComponent
 *
 *  Created by Steve Gifford on 1/20/16
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

#import <UIKit/UIKit.h>
#import "MaplyCoordinate.h"
#import "MaplyBaseViewController.h"
#import "MaplyGeomModel.h"

@interface MaplyGeomState : NSObject

@property (nonatomic,strong) UIColor *color;

@property (nonatomic,strong) id texture;

@property (nonatomic,strong) MaplyShader *shader;

@end

@interface MaplyGeomBuilder : NSObject

- (id)initWithViewC:(MaplyBaseViewController *)viewC;

- (void)addRectangleAroundOrigin:(MaplyCoordinateD)size state:(MaplyGeomState *)state;

- (void)addRectangleAroundOriginX:(double)x y:(double)y state:(MaplyGeomState *)state;

- (void)addRectangleAroundX:(double)x y:(double)y width:(double)width height:(double)height state:(MaplyGeomState *)state;

- (void)addAttributedString:(NSAttributedString *)str state:(MaplyGeomState *)state;

- (void)addString:(NSString *)str font:(UIFont *)font state:(MaplyGeomState *)state;

- (void)addPolygonWithPts:(MaplyCoordinate3dD *)pts numPts:(int)numPts state:(MaplyGeomState *)state;

- (void)addPolygonWithPts:(MaplyCoordinate3dD *)pts tex:(MaplyCoordinateD *)tex norms:(MaplyCoordinate3dD *)norms numPts:(int)numPts state:(MaplyGeomState *)state;

- (void)scale:(MaplyCoordinate3dD)scale;

- (void)scaleX:(double)x y:(double)y z:(double)z;

- (void)translate:(MaplyCoordinate3dD)trans;

- (void)translateX:(double)x y:(double)y z:(double)z;

- (void)rotate:(double)angle around:(MaplyCoordinate3dD)axis;

- (void)rotate:(double)angle aroundX:(double)x y:(double)y z:(double)z;

- (void)transform:(MaplyMatrix *)matrix;

- (void)addGeomFromBuilder:(MaplyGeomBuilder *)modelBuilder;

- (void)addGeomFromBuilder:(MaplyGeomBuilder *)modelBuilder transform:(MaplyMatrix *)matrix;

- (bool)getSizeLL:(MaplyCoordinate3dD *)ll ur:(MaplyCoordinate3dD *)ur;

- (MaplyGeomModel *)makeGeomModel:(MaplyThreadMode)threadMode;

@end
