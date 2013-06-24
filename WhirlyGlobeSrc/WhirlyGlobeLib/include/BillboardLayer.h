/*
 *  BillboardLayer.h
 *  WhirlyGlobeLib
 *
 *  Created by Steve Gifford on 5/27/12.
 *  Copyright 2011-2013 mousebird consulting. All rights reserved.
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
#import <set>
#import <map>
#import "Identifiable.h"
#import "Drawable.h"
#import "DataLayer.h"
#import "LayerThread.h"
#import "BillboardDrawable.h"

// Used to pass parameters around between threads
@interface WhirlyKitBillboardInfo : NSObject

@property (nonatomic,assign) WhirlyKit::SimpleIdentity billboardId;
@property (nonatomic) NSArray *billboards;
@property (nonatomic) UIColor *color;
@property (nonatomic,assign) float fade;
@property (nonatomic,assign) float minVis,maxVis;
@property (nonatomic,assign) int drawPriority;
@property (nonatomic,assign) WhirlyKit::SimpleIdentity replaceId;

- (id)initWithBillboards:(NSArray *)billboards desc:(NSDictionary *)desc;

- (void)parseDesc:(NSDictionary *)desc;

@end

namespace WhirlyKit
{

/// Used internally to track billboard geometry
class BillboardSceneRep : public Identifiable
{
public:
    BillboardSceneRep();
    BillboardSceneRep(SimpleIdentity inId);
    ~BillboardSceneRep();
    
    // Clear the contents out of the scene
    void clearContents(std::vector<ChangeRequest *> &changes);

    SimpleIDSet drawIDs;  // Drawables created for this
    float fade;  // Time to fade away for removal
};
    
typedef std::set<BillboardSceneRep *,IdentifiableSorter> BillboardSceneRepSet;
  
// Used to construct billboard geometry
class BillboardDrawableBuilder
{
public:
    BillboardDrawableBuilder(Scene *scene,std::vector<ChangeRequest *> &changes,BillboardSceneRep *sceneRep,WhirlyKitBillboardInfo *billInfo,SimpleIdentity billboardProgram,SimpleIdentity texId);
    ~BillboardDrawableBuilder();
    
    void addBillboard(Point3f center,float width,float height,UIColor *color);
    
    void flush();
    
protected:
    Scene *scene;
    std::vector<ChangeRequest *> &changes;
    Mbr drawMbr;
    BillboardDrawable *drawable;
    WhirlyKitBillboardInfo *billInfo;
    BillboardSceneRep *sceneRep;
    SimpleIdentity billboardProgram;
    SimpleIdentity texId;
};

}

/** Single billboard representation.  Billboards are oriented towards
    the user.  Fill this out and hand it over to the billboard layer
    to manage.
  */
@interface WhirlyKitBillboard : NSObject

/// Center in display coordinates
@property (nonatomic,assign) WhirlyKit::Point3f center;
/// Height in display coordinates
@property (nonatomic,assign) float height;
/// Width in dispay coordinates
@property (nonatomic,assign) float width;
/// Color of geometry
@property (nonatomic,assign) UIColor *color;
/// Texture to use
@property (nonatomic,assign) WhirlyKit::SimpleIdentity texId;

@end

/** The billboard layer manages the construction and destruction of
    billboards for the renderer.  Billboards are 3D objects that turn
    to face the viewer.
 */
@interface WhirlyKitBillboardLayer : NSObject<WhirlyKitLayer>

/// Add billboards for display
- (WhirlyKit::SimpleIdentity) addBillboards:(NSArray *)billboards desc:(NSDictionary *)desc;

/// Remove a group of billboards named by the given ID
- (void) removeBillboards:(WhirlyKit::SimpleIdentity)billId;

/// Replace the existing billboards with new ones
- (WhirlyKit::SimpleIdentity) replaceBillboards:(WhirlyKit::SimpleIdentity)billId withBillboards:(NSArray *)billboards desc:(NSDictionary *)desc;

@end
