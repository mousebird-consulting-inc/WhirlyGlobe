/*
 *  ShapeLayer.h
 *  WhirlyGlobeLib
 *
 *  Created by Steve Gifford on 9/28/12.
 *  Copyright 2011-2013 mousebird consulting
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

#import "Identifiable.h"
#import "WhirlyVector.h"
#import "DataLayer.h"
#import "layerThread.h"
#import "SelectionManager.h"
#import "ShapeManager.h"

/**  The Shape Layer displays a set of shapes on the globe or map in specified
     locations.  The type of the object determines the sort of shape displayed.
 
     Location and visual information for a Shape is controlled by the associated object.
     Other attributes are in the NSDictionary passed in on creation.
     <list type="bullet">
     <item>minVis        [NSNumber float]
     <item>maxVis        [NSNumber float]
     <item>color         [UIColor]
     <item>drawPriority  [NSNumber int]
     <item>drawOffset    [NSNumber int]
     <item>fade          [NSNumber float]
     <item>shader        [NSNumber int]
     </list>
  */
@interface WhirlyKitShapeLayer : NSObject<WhirlyKitLayer>

/// Called in the layer thread
- (void)startWithThread:(WhirlyKitLayerThread *)layerThread scene:(WhirlyKit::Scene *)scene;

/// Called in the layer thread
- (void)shutdown;

/// Add a single shape.  The returned ID can be used to remove or modify the shape.
- (WhirlyKit::SimpleIdentity) addShape:(WhirlyKitShape *)shapes desc:(NSDictionary *)desc;

/// Add an array of shapes.  The returned ID can be used to remove or modify the group of shapes.
- (WhirlyKit::SimpleIdentity) addShapes:(NSArray *)shapes desc:(NSDictionary *)desc;

/// Remove a group of shapes named by the given ID
- (void) removeShapes:(WhirlyKit::SimpleIdentity)shapeID;

@end
