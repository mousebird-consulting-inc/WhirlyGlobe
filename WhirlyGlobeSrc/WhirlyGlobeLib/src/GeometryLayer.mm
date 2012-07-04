/*
 *  GeometryLayer.mm
 *  WhirlyGlobeLib
 *
 *  Created by Steve Gifford on 6/19/12.
 *  Copyright 2012 mousebird consulting
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

#import "GeometryLayer.h"
#import "NSDictionary+Stuff.h"
#import "UIColor+Stuff.h"

using namespace Eigen;
using namespace WhirlyKit;
using namespace WhirlyGlobe;

void GeomSceneRep::removeFromScene(std::vector<WhirlyKit::ChangeRequest *> &changeRequests)
{
    for (SimpleIDSet::iterator it = drawIDs.begin();
         it != drawIDs.end(); ++it)
        changeRequests.push_back(new RemDrawableReq(*it));
}

void GeomSceneRep::fadeOutScene(std::vector<WhirlyKit::ChangeRequest *> &changeRequests)
{
    NSTimeInterval curTime = CFAbsoluteTimeGetCurrent();
    for (SimpleIDSet::iterator it = drawIDs.begin();
         it != drawIDs.end(); ++it)
        changeRequests.push_back(new FadeChangeRequest(*it, curTime, curTime+fade));
}

// Used to pass geometry around internally
@interface GeomInfo : NSObject
{
@public
    SimpleIdentity  sceneRepId;
    NSArray         *geom;
    BOOL            enable;
    UIColor         *color;
    float           fade;
}

- (void)parseDict:(NSDictionary *)dict;
@end

@implementation GeomInfo

- (id)initWithGeometry:(NSArray *)inGeom desc:(NSDictionary *)dict
{
    self = [super init];
    if (self)
    {
        geom = inGeom;
        [self parseDict:dict];
    }
    
    return self;
}

- (id)initWithSceneRepId:(SimpleIdentity)inId desc:(NSDictionary *)dict
{
    if ((self = [super init]))
    {
        sceneRepId = inId;
        [self parseDict:dict];
    }
    
    return self;
}

- (void)parseDict:(NSDictionary *)dict
{
    enable = [dict boolForKey:@"enable" default:YES];
    color = [dict objectForKey:@"color" checkType:[UIColor class] default:[UIColor whiteColor]];
    fade = [dict floatForKey:@"fade" default:0.0];
}

@end

@interface WhirlyGlobeGeometry()
- (BasicDrawable *)buildDrawable;
@end

@implementation WhirlyGlobeGeometry
// Base version does nothing
- (BasicDrawable *)buildDrawable
{
    return nil;
}
@end

@implementation WhirlyGlobeGeometryRaw

@synthesize type;
@synthesize pts;
@synthesize norms;
@synthesize texCoords;
@synthesize colors;
@synthesize triangles;
@synthesize texId;

- (bool)isValid
{
    if (type != WhirlyGlobeGeometryLines && type != WhirlyGlobeGeometryTriangles)
        return false;
    int numPoints = pts.size();
    if (numPoints == 0)
        return false;
    
    if (!norms.empty() && norms.size() != numPoints)
        return false;
    if (!texCoords.empty() && texCoords.size() != numPoints)
        return false;
    if (!colors.empty() && colors.size() != numPoints)
        return false;
    if (type == WhirlyGlobeGeometryTriangles && triangles.empty())
        return false;
    if (texId != EmptyIdentity && texCoords.empty())
        return false;
    for (unsigned int ii=0;ii<triangles.size();ii++)
    {
        RawTriangle tri = triangles[ii];
        for (unsigned int jj=0;jj<3;jj++)
            if (tri.verts[jj] >= pts.size() || tri.verts[jj] < 0)
                return false;
    }
    
    return true;
}

- (BasicDrawable *)buildDrawable
{
    if (![self isValid])
        return nil;
    
    BasicDrawable *draw = new BasicDrawable();
    switch (type)
    {
        case WhirlyGlobeGeometryLines:
            draw->setType(GL_LINES);
            break;
        case WhirlyGlobeGeometryTriangles:
            draw->setType(GL_TRIANGLES);
            break;
        default:
            break;
    }
    draw->setTexId(texId);
    for (unsigned int ii=0;ii<pts.size();ii++)
    {
        draw->addPoint(pts[ii]);
        if (!norms.empty())
            draw->addNormal(norms[ii]);
        if (texId != EmptyIdentity)
            draw->addTexCoord(texCoords[ii]);
        if (!colors.empty())
            draw->addColor(colors[ii]);
    }
    for (unsigned int ii=0;ii<triangles.size();ii++)
    {
        RawTriangle tri = triangles[ii];
        draw->addTriangle(BasicDrawable::Triangle(tri.verts[0],tri.verts[1],tri.verts[2]));
    }
    
    return draw;
}

@end

@implementation WhirlyGlobeGeometryLayer

- (void)clear
{
    geomReps.clear();    
    scene = NULL;
}

- (void)dealloc
{
    [self clear];
}

- (void)startWithThread:(WhirlyKitLayerThread *)inLayerThread scene:(WhirlyKit::Scene *)inScene
{
	scene = inScene;
    layerThread = inLayerThread;
}

- (void)shutdown
{
    [NSObject cancelPreviousPerformRequestsWithTarget:self];
    std::vector<ChangeRequest *> changeRequests;
    
    for (GeomSceneRepSet::iterator it = geomReps.begin();
         it != geomReps.end(); ++it)
        (*it)->removeFromScene(changeRequests);
        
    scene->addChangeRequests(changeRequests);
    
    [self clear];
}

// Actually create the geometry
// In the layer thread here
- (void)runAddGeometry:(GeomInfo *)geomInfo
{
    std::vector<ChangeRequest *> changeRequests;
    
    // Work through the array of geometry, building as we go
    for (WhirlyGlobeGeometry *geom in geomInfo->geom)
    {
        BasicDrawable *draw = [geom buildDrawable];
        if (draw)
            changeRequests.push_back(new AddDrawableReq(draw));
    }
    
    scene->addChangeRequests(changeRequests);
}

// Remove the representation
// In the layer thread here
- (void)runRemoveGeometry:(NSNumber *)num
{
    GeomSceneRepRef dummyRep(new GeomSceneRep());
    dummyRep->setId([num intValue]);
    GeomSceneRepSet::iterator it = geomReps.find(dummyRep);
    if (it != geomReps.end())
    {
        GeomSceneRepRef geomRep = *it;
        std::vector<WhirlyKit::ChangeRequest *> changeRequests;
        if (geomRep->fade > 0.0)
        {
            geomRep->fadeOutScene(changeRequests);
            
            // Reset the fade and remove it later
            [self performSelector:@selector(runRemoveGeometry:) withObject:num afterDelay:geomRep->fade];
            geomRep->fade = 0.0;
        } else {
            // Just remove it
            geomRep->removeFromScene(changeRequests);
            geomReps.erase(geomRep);
        }
        
        scene->addChangeRequests(changeRequests);
    }
}

/// Add a sphere at the given location
- (WhirlyKit::SimpleIdentity)addGeometry:(WhirlyGlobeGeometry *)geom desc:(NSDictionary *)desc
{
    NSArray *array = [NSArray arrayWithObject:geom];
    return [self addGeometryArray:array desc:desc];
}

/// Add a group of geometry together
- (WhirlyKit::SimpleIdentity)addGeometryArray:(NSArray *)geom desc:(NSDictionary *)desc
{
    GeomInfo *geomInfo = [[GeomInfo alloc] initWithGeometry:geom desc:desc];
    geomInfo->sceneRepId = Identifiable::genId();
    
    if (!layerThread || ([NSThread currentThread] == layerThread))
        [self runAddGeometry:geomInfo];
    else
        [self performSelector:@selector(runAddGeometry:) onThread:layerThread withObject:geomInfo waitUntilDone:NO];
    
    return geomInfo->sceneRepId;
}

/// Remove an entire set of geometry at once by its ID
- (void)removeGeometry:(WhirlyKit::SimpleIdentity)geomID
{
    if (!layerThread || ([NSThread currentThread] == layerThread))
        [self runRemoveGeometry:[NSNumber numberWithInt:geomID]];
    else
        [self performSelector:@selector(runRemoveGeometry:) onThread:layerThread withObject:[NSNumber numberWithInt:geomID] waitUntilDone:NO];
}

@end
