/*
 *  SelectionLayer.mm
 *  WhirlyGlobeLib
 *
 *  Created by Steve Gifford on 10/26/11.
 *  Copyright 2011 mousebird consulting. All rights reserved.
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

#import "SelectionLayer.h"
#import "NSDictionary+Stuff.h"
#import "UIColor+Stuff.h"
#import "GlobeMath.h"

using namespace WhirlyGlobe;

bool RectSelectable::operator < (const RectSelectable &that) const
{
    return selectID < that.selectID;
}


@interface WGSelectionLayer()

@property (nonatomic,retain) WhirlyGlobeView *globeView;
@property (nonatomic,retain) SceneRendererES1 *renderer;
@property (nonatomic,assign) WhirlyGlobeLayerThread *layerThread;

@end

@implementation WGSelectionLayer

@synthesize globeView;
@synthesize renderer;
@synthesize layerThread;

- (id)initWithGlobeView:(WhirlyGlobeView *)inGlobeView renderer:(SceneRendererES1 *)inRenderer
{
    self = [super init];
    
    if (self)
    {
        self.globeView = inGlobeView;
        self.renderer = inRenderer;
    }
    
    return self;
}

- (void)dealloc
{
    self.layerThread = nil;
    self.globeView = nil;
    self.renderer = nil;
    
    selectables.clear();
    
    [super dealloc];
}

// Called in the layer thread
- (void)startWithThread:(WhirlyGlobeLayerThread *)inLayerThread scene:(WhirlyGlobe::GlobeScene *)scene
{
    self.layerThread = inLayerThread;
}

// Add a rectangle (in 3-space) available for selection
- (void)addSelectableRect:(WhirlyGlobe::SimpleIdentity)selectId rect:(Point3f *)pts
{
    if (selectId == EmptyIdentity)
        return;
    
    RectSelectable newSelect;
    newSelect.selectID = selectId;
    newSelect.minVis = newSelect.maxVis = DrawVisibleInvalid;
    newSelect.norm = (pts[1] - pts[0]).cross(pts[3]-pts[0]).normalized();
    for (unsigned int ii=0;ii<4;ii++)
        newSelect.pts[ii] = pts[ii];
    
    selectables.insert(newSelect);
}

// Add a rectangle (in 3-space) for selection, but only between the given visibilities
- (void)addSelectableRect:(WhirlyGlobe::SimpleIdentity)selectId rect:(Point3f *)pts minVis:(float)minVis maxVis:(float)maxVis
{
    if (selectId == EmptyIdentity)
        return;
    
    RectSelectable newSelect;
    newSelect.selectID = selectId;
    newSelect.minVis = minVis;  newSelect.maxVis = maxVis;
    newSelect.norm = (pts[1] - pts[0]).cross(pts[3]-pts[0]).normalized();
    for (unsigned int ii=0;ii<4;ii++)
        newSelect.pts[ii] = pts[ii];
    
    selectables.insert(newSelect);
}

// Remove the given selectable from consideration
- (void)removeSelectable:(WhirlyGlobe::SimpleIdentity)selectID
{
    RectSelectable dumbRect;
    dumbRect.selectID = selectID;
    RectSelectableSet::iterator it = selectables.find(dumbRect);
    
    if (it != selectables.end())
        selectables.erase(it);
}

/// Pass in the screen point where the user touched.  This returns the closest hit within the given distance
- (SimpleIdentity)pickObject:(Point2f)touchPt maxDist:(float)maxDist
{
    // Can only run in the layer thread
    if ([NSThread currentThread] != layerThread)
        return EmptyIdentity;
    
    float maxDist2 = maxDist * maxDist;
    
    // Precalculate the model matrix for use below
    Eigen::Affine3f modelTrans = [globeView calcModelMatrix];
    
    SimpleIdentity retId = EmptyIdentity;
    float closeDist2 = MAXFLOAT;
    
    // Work through the available features
    for (RectSelectableSet::iterator it = selectables.begin();
         it != selectables.end(); ++it)
    {
        RectSelectable sel = *it;
        if (sel.selectID != EmptyIdentity)
        {
            if (sel.minVis == DrawVisibleInvalid ||
                (sel.minVis < globeView.heightAboveGlobe && globeView.heightAboveGlobe < sel.maxVis))
            {
                std::vector<Point2f> screenPts;
                
                for (unsigned int ii=0;ii<4;ii++)
                {
                    CGPoint screenPt;
                    screenPt = [globeView pointOnScreenFromSphere:sel.pts[ii] transform:&modelTrans frameSize:Point2f(renderer.framebufferWidth,renderer.framebufferHeight)];
                    screenPts.push_back(Point2f(screenPt.x,screenPt.y));
                }
                
                // See if we fall within that polygon
                if (PointInPolygon(touchPt, screenPts))
                {
                    retId = sel.selectID;
                    break;
                }
                
                // Now for a proximit check around the edges
                for (unsigned int ii=0;ii<4;ii++)
                {
                    Point2f closePt = ClosestPointOnLineSegment(screenPts[ii],screenPts[(ii+1)%4],touchPt);
                    float dist2 = (closePt-touchPt).squaredNorm();
                    if (dist2 <= maxDist2 && (dist2 < closeDist2))
                    {
                        retId = sel.selectID;
                        closeDist2 = dist2;
                    }
                }
            }
        }
    }
    
    return retId;
}


@end