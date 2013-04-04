/*
 *  ShapeDisplay.h
 *  WhirlyGlobeLib
 *
 *  Created by Steve Gifford on 1/26/11.
 *  Copyright 2011-2012 mousebird consulting
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

#import <math.h>
#import <vector>
#import <set>
#import <map>
#import <Foundation/Foundation.h>
#import "Drawable.h"
#import "DataLayer.h"
#import "VectorData.h"
#import "GlobeMath.h"
#import "LayerThread.h"
#import "DrawCost.h"

namespace WhirlyKit
{

/*  This is the representation of a group of vectors
    in the scene.  You do not want to create individual
    vector features on the globe one by one, that's too expensive.
    It needs to be batched and that's how we do this.
    The VectorSceneRep keeps track of what shapes are being
    represented by what drawables in the scene.  We use this
    for modifications or deletions later on.
 */
class VectorSceneRep : public Identifiable
{
public:
    VectorSceneRep() { }
    VectorSceneRep(ShapeSet &inShapes) : shapes(inShapes) { };
    
    ShapeSet shapes;  // Shapes associated with this
    SimpleIDSet drawIDs;    // The drawables we created
    float fade;       // If set, the amount of time to fade out before deletion
};
typedef std::map<SimpleIdentity,VectorSceneRep *> VectorSceneRepMap;

}

/** The Vector Display Layer will add vector objects on top of the
    globe as requested by a caller.  To keep things efficient, you
    should add a whole group of shapes at once.  Basically, the larger
    the group you can add, the better.  The vector layer will handle
    chunking them out into multiple drawables if needed.
    When you add a group of shapes you will get back a unique ID that
    can be used to modify them or delete them later on.
 
    Any of the valid methods can be called in any thread.  We check to
    see which one we're in and pass the appropriate message to the layer
    thread and execute the work in there.
 
    When adding a set of shapes, you can pass in an optional dictionary
    describing how they'll look.  That can have any of these key/value pairs:
    <list type="bullet">
    <item>enable      [NSNumber bool]
    <item>drawOffset  [NSNumber int]
    <item>color       [UIColor]
    <item>priority    [NSNumber int]
    <item>minVis      [NSNumber float]
    <item>maxVis      [NSNumber float]
    <item>fade        [NSNumber float]
    <item>width       [NSNumber float]
    <item>filled      [NSNumber bool]
    <item>sample      [NSNumber float]
    </list>
  */
@interface WhirlyKitVectorLayer : NSObject<WhirlyKitLayer>
{
@private
    WhirlyKit::Scene *scene;
    WhirlyKitLayerThread * __weak layerThread;
    
    // Visual representations of vectors
    WhirlyKit::VectorSceneRepMap vectorReps;    
}

/// Called in the layer thread
- (void)startWithThread:(WhirlyKitLayerThread *)layerThread scene:(WhirlyKit::Scene *)scene;

/// Remove any outstanding features
- (void)shutdown;

/// Create geometry from the given vector.
/// The dictionary controls how the vector will appear.
/// We refer to that vector by the returned ID.
/// Call the other version if you have more than one.
- (WhirlyKit::SimpleIdentity)addVector:(WhirlyKit::VectorShapeRef)shape desc:(NSDictionary *)dict;

/// Create geometry for the given group of vectors
/// The dictionary controls how the vectors will appear
///  and you can refer to the vectors in later calls
///  with the returned ID.
- (WhirlyKit::SimpleIdentity)addVectors:(WhirlyKit::ShapeSet *)shapes desc:(NSDictionary *)dict;

/// This is exactly the same as removeVector/addVector except it times the remove to coincide
///  with the add.
- (WhirlyKit::SimpleIdentity)replaceVector:(WhirlyKit::SimpleIdentity)oldVecID withVectors:(WhirlyKit::ShapeSet *)shapes desc:(NSDictionary *)dict;

/// This lets you change how a set of vectors is represented visually.
/// You specify a dictionary to change particular attributues
/// Only enable, color, line width, draw priority, and visibility range are supported
- (void)changeVector:(WhirlyKit::SimpleIdentity)vecID desc:(NSDictionary *)dict;

/// Removes a group of vectors from the display
- (void)removeVector:(WhirlyKit::SimpleIdentity)vecID;

/// Returns a cost estimate for the given vectors referred to by
///  ID.  This must be called in the layer thread
- (WhirlyKitDrawCost *)getCost:(WhirlyKit::SimpleIdentity)vecID;

@end
