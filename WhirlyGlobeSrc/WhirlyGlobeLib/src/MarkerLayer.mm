/*
 *  MarkerLayer.mm
 *  WhirlyGlobeLib
 *
 *  Created by Steve Gifford on 10/21/11.
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

#import "MarkerLayer.h"
#import "NSDictionary+Stuff.h"
#import "UIColor+Stuff.h"
#import "GlobeMath.h"
#import "MarkerGenerator.h"

using namespace WhirlyKit;

namespace WhirlyKit
{

MarkerSceneRep::MarkerSceneRep()
{
    selectID = EmptyIdentity;
}

}

@implementation WhirlyKitMarker

@synthesize isSelectable;
@synthesize selectID;
@synthesize loc;
@synthesize width,height;
@synthesize texIDs;
@synthesize period;
@synthesize timeOffset;

- (id)init
{
    self = [super init];
    
    if (self)
    {
        isSelectable = false;
        selectID = EmptyIdentity;
    }
    
    return self;
}


- (void)addTexID:(SimpleIdentity)texID
{
    texIDs.push_back(texID);
}

@end

// Used to pass marker information between threads
@interface MarkerInfo : NSObject
{
    NSArray         *markers;  // Individual marker objects
    UIColor         *color;
    int             drawOffset;
    float           minVis,maxVis;
    float           width,height;
    int             drawPriority;
    float           fade;
    SimpleIdentity  markerId;
}

@property (nonatomic) NSArray *markers;
@property (nonatomic) UIColor *color;
@property (nonatomic,assign) int drawOffset;
@property (nonatomic,assign) float minVis,maxVis;
@property (nonatomic,assign) float width,height;
@property (nonatomic,assign) int drawPriority;
@property (nonatomic,assign) float fade;
@property (nonatomic,assign) SimpleIdentity markerId;

- (id)initWithMarkers:(NSArray *)markers desc:(NSDictionary *)desc;

- (void)parseDesc:(NSDictionary *)desc;

@end

@implementation MarkerInfo

@synthesize markers;
@synthesize color;
@synthesize drawOffset;
@synthesize minVis,maxVis;
@synthesize width,height;
@synthesize drawPriority;
@synthesize fade;
@synthesize markerId;

// Initialize with an array of makers and parse out parameters
- (id)initWithMarkers:(NSArray *)inMarkers desc:(NSDictionary *)desc
{
    self = [super init];
    
    if (self)
    {
        self.markers = inMarkers;
        [self parseDesc:desc];
        
        markerId = Identifiable::genId();
    }
    
    return self;
}


- (void)parseDesc:(NSDictionary *)desc
{
    self.color = [desc objectForKey:@"color" checkType:[UIColor class] default:[UIColor whiteColor]];
    drawOffset = [desc intForKey:@"drawOffset" default:0];
    minVis = [desc floatForKey:@"minVis" default:DrawVisibleInvalid];
    maxVis = [desc floatForKey:@"maxVis" default:DrawVisibleInvalid];
    drawPriority = [desc intForKey:@"drawPriority" default:MarkerDrawPriority];
    width = [desc floatForKey:@"width" default:0.001];
    height = [desc floatForKey:@"height" default:0.001];
    fade = [desc floatForKey:@"fade" default:0.0];
}

@end


@implementation WhirlyKitMarkerLayer

@synthesize selectLayer;

- (void)clear
{
    for (MarkerSceneRepSet::iterator it = markerReps.begin();
         it != markerReps.end(); ++it)
        delete *it;
    markerReps.clear();    
}

- (void)dealloc
{
    [self clear];
}

// Called in the layer thread
- (void)startWithThread:(WhirlyKitLayerThread *)inLayerThread scene:(Scene *)inScene
{
    layerThread = inLayerThread;
    scene = inScene;    

    // Set up the generator we'll pass markers to
    MarkerGenerator *gen = new MarkerGenerator();
    generatorId = gen->getId();
    scene->addChangeRequest(new AddGeneratorReq(gen));
}

- (void)shutdown
{
    std::vector<ChangeRequest *> changeRequests;
    
    for (MarkerSceneRepSet::iterator it = markerReps.begin();
         it != markerReps.end(); ++it)
    {
        MarkerSceneRep *markerRep = *it;
        for (SimpleIDSet::iterator idIt = markerRep->drawIDs.begin();
             idIt != markerRep->drawIDs.end(); ++idIt)
            changeRequests.push_back(new RemDrawableReq(*idIt));
        
        if (!markerRep->markerIDs.empty())
        {
            std::vector<SimpleIdentity> markerIDs;
            for (SimpleIDSet::iterator idIt = markerRep->markerIDs.begin();
                 idIt != markerRep->markerIDs.end(); ++idIt)
                markerIDs.push_back(*idIt);
            changeRequests.push_back(new MarkerGeneratorRemRequest(generatorId,markerIDs));
        }
        
        if (self.selectLayer && markerRep->selectID != EmptyIdentity)
            [self.selectLayer removeSelectable:markerRep->selectID];        
    }
    
    if (generatorId != EmptyIdentity)
        changeRequests.push_back(new RemGeneratorReq(generatorId));
    
    scene->addChangeRequests(changeRequests);
    
    [self clear];
}

typedef std::map<SimpleIdentity,BasicDrawable *> DrawableMap;

// Add a bunch of markers at once
// We're in the layer thread here
- (void)runAddMarkers:(MarkerInfo *)markerInfo
{
//    CoordSystem *coordSys = scene->getCoordSystem();
    MarkerSceneRep *markerRep = new MarkerSceneRep();
    markerRep->fade = markerInfo.fade;
    markerRep->setId(markerInfo.markerId);
    markerReps.insert(markerRep);
    
    // For static markers, sort by texture
    DrawableMap drawables;
    std::vector<MarkerGenerator::Marker *> markersToAdd;
    
    for (WhirlyKitMarker *marker in markerInfo.markers)
    {
        // Build the rectangle for this one
        Point3f pts[4];
        Vector3f norm;
        float width2 = (marker.width == 0.0 ? markerInfo.width : marker.width)/2.0;
        float height2 = (marker.height == 0.0 ? markerInfo.height : marker.height)/2.0;
        
        norm = GeoCoordSystem::LocalToGeocentricish(marker.loc);
        Point3f center = norm;
        Vector3f up(0,0,1);
        Point3f horiz = up.cross(norm).normalized();
        Point3f vert = norm.cross(horiz).normalized();;        
        
        Point3f ll = center - width2*horiz - height2*vert;
        pts[0] = ll;
        pts[1] = ll + 2 * width2 * horiz;
        pts[2] = ll + 2 * width2 * horiz + 2 * height2 * vert;
        pts[3] = ll + 2 * height2 * vert;

        // While we're at it, let's add this to the selection layer
        if (selectLayer && marker.isSelectable)
        {
            // If the marker doesn't already have an ID, it needs one
            if (!marker.selectID)
                marker.selectID = Identifiable::genId();
            
            markerRep->selectID = marker.selectID;
            [selectLayer addSelectableRect:marker.selectID rect:pts];
        }
        
        // If the marker has just one texture, we can treat it as static
        if (marker.texIDs.size() <= 1)
        {        
            // Look for a texture sub mapping
            SimpleIdentity texID = (marker.texIDs.empty() ? EmptyIdentity : marker.texIDs.at(0));
            SubTexture subTex = scene->getSubTexture(texID);
            
            // We're sorting the static drawables by texture, so look for that
            DrawableMap::iterator it = drawables.find(subTex.texId);
            BasicDrawable *draw = NULL;
            if (it != drawables.end())
                draw = it->second;
            else {
                draw = new BasicDrawable();
                draw->setType(GL_TRIANGLES);
                draw->setDrawOffset(markerInfo.drawOffset);
                draw->setColor([markerInfo.color asRGBAColor]);
                draw->setDrawPriority(markerInfo.drawPriority);
                draw->setVisibleRange(markerInfo.minVis, markerInfo.maxVis);
                draw->setTexId(subTex.texId);
                drawables[subTex.texId] = draw;
                markerRep->drawIDs.insert(draw->getId());
            }

            // Build one set of texture coordinates
            std::vector<TexCoord> texCoord;
            texCoord.resize(4);
            texCoord[0].u() = 0.0;  texCoord[0].v() = 0.0;
            texCoord[1].u() = 1.0;  texCoord[1].v() = 0.0;
            texCoord[2].u() = 1.0;  texCoord[2].v() = 1.0;
            texCoord[3].u() = 0.0;  texCoord[3].v() = 1.0;
            subTex.processTexCoords(texCoord);    
            
            // Toss the geometry into the drawable
            int vOff = draw->getNumPoints();
            for (unsigned int ii=0;ii<4;ii++)
            {
                Point3f &pt = pts[ii];
                draw->addPoint(pt);
                draw->addNormal(norm);
                draw->addTexCoord(texCoord[ii]);
                GeoMbr geoMbr = draw->getGeoMbr();
                geoMbr.addGeoCoord(marker.loc);
                draw->setGeoMbr(geoMbr);
            }
            
            draw->addTriangle(BasicDrawable::Triangle(0+vOff,1+vOff,2+vOff));
            draw->addTriangle(BasicDrawable::Triangle(2+vOff,3+vOff,0+vOff));
        } else {
            // The marker changes textures, so we need to pass it to the generator
            MarkerGenerator::Marker *newMarker = new MarkerGenerator::Marker();
            newMarker->color = RGBAColor(255,255,255,255);
            newMarker->loc = marker.loc;
            for (unsigned int ii=0;ii<4;ii++)
                newMarker->pts[ii] = pts[ii];
            newMarker->norm = norm;
            newMarker->period = marker.period;
            newMarker->start = marker.timeOffset;
            newMarker->drawOffset = markerInfo.drawOffset;
            newMarker->minVis = markerInfo.minVis;
            newMarker->maxVis = markerInfo.maxVis;
            newMarker->drawPriority = markerInfo.drawPriority;
            if (markerInfo.fade > 0.0)
            {
                NSTimeInterval curTime = CFAbsoluteTimeGetCurrent();
                newMarker->fadeDown = curTime;
                newMarker->fadeUp = curTime+markerInfo.fade;
            } else {
                newMarker->fadeDown = newMarker->fadeUp= 0.0;
            }
        
            // Each set of texture coordinates may be different
            std::vector<TexCoord> texCoord;
            texCoord.resize(4);
            texCoord[0].u() = 0.0;  texCoord[0].v() = 0.0;
            texCoord[1].u() = 1.0;  texCoord[1].v() = 0.0;
            texCoord[2].u() = 1.0;  texCoord[2].v() = 1.0;
            texCoord[3].u() = 0.0;  texCoord[3].v() = 1.0;
            for (unsigned int ii=0;ii<marker.texIDs.size();ii++)
            {
                SubTexture subTex = scene->getSubTexture(marker.texIDs.at(ii));
                std::vector<TexCoord> theseTexCoord = texCoord;
                subTex.processTexCoords(theseTexCoord);
                newMarker->texCoords.push_back(theseTexCoord);
                newMarker->texIDs.push_back(subTex.texId);
            }
            
            // Send it off to the generator
            markerRep->markerIDs.insert(newMarker->getId());
            markersToAdd.push_back(newMarker);
        }
    }
    
    // Add all the new markers at once
    if (!markersToAdd.empty())
        scene->addChangeRequest(new MarkerGeneratorAddRequest(generatorId,markersToAdd));
    
    // Flush out any drawables for the static geometry
    for (DrawableMap::iterator it = drawables.begin();
         it != drawables.end(); ++it)
    {
        if (markerInfo.fade > 0.0)
        {
            NSTimeInterval curTime = CFAbsoluteTimeGetCurrent();
            it->second->setFade(curTime,curTime+markerInfo.fade);
        }
        scene->addChangeRequest(new AddDrawableReq(it->second));        
    }
    drawables.clear();
}

// Remove the given marker(s)
- (void)runRemoveMarkers:(NSNumber *)num
{
    SimpleIdentity markerId = [num unsignedIntValue];
    
    MarkerSceneRep dummyRep;
    dummyRep.setId(markerId);
    MarkerSceneRepSet::iterator it = markerReps.find(&dummyRep);
    if (it != markerReps.end())
    {
        MarkerSceneRep *markerRep = *it;
        
        if (markerRep->fade > 0.0)
        {
            NSTimeInterval curTime = CFAbsoluteTimeGetCurrent();
            for (SimpleIDSet::iterator idIt = markerRep->drawIDs.begin();
                 idIt != markerRep->drawIDs.end(); ++idIt)
                scene->addChangeRequest(new FadeChangeRequest(*idIt,curTime,curTime+markerRep->fade));

            if (!markerRep->markerIDs.empty())
            {
                std::vector<SimpleIdentity> markerIDs;
                for (SimpleIDSet::iterator idIt = markerRep->markerIDs.begin();
                     idIt != markerRep->markerIDs.end(); ++idIt)
                    markerIDs.push_back(*idIt);
                scene->addChangeRequest(new MarkerGeneratorFadeRequest(generatorId,markerIDs,curTime,curTime+markerRep->fade));            
            }
            
            [self performSelector:@selector(runRemoveMarkers:) withObject:num afterDelay:markerRep->fade];
            markerRep->fade = 0.0;
        } else {
            // Just delete everything
            for (SimpleIDSet::iterator idIt = markerRep->drawIDs.begin();
                 idIt != markerRep->drawIDs.end(); ++idIt)
                scene->addChangeRequest(new RemDrawableReq(*idIt));

            if (!markerRep->markerIDs.empty())
            {
                std::vector<SimpleIdentity> markerIDs;
                for (SimpleIDSet::iterator idIt = markerRep->markerIDs.begin();
                     idIt != markerRep->markerIDs.end(); ++idIt)
                    markerIDs.push_back(*idIt);
                scene->addChangeRequest(new MarkerGeneratorRemRequest(generatorId,markerIDs));
            }
            
            if (self.selectLayer && markerRep->selectID != EmptyIdentity)
                [self.selectLayer removeSelectable:markerRep->selectID];
            
            markerReps.erase(it);
            delete markerRep;
        }
    }
}


// Add a single marker 
- (SimpleIdentity) addMarker:(WhirlyKitMarker *)marker desc:(NSDictionary *)desc
{
    MarkerInfo *markerInfo = [[MarkerInfo alloc] initWithMarkers:[NSArray arrayWithObject:marker] desc:desc];
    
    if (!layerThread || ([NSThread currentThread] == layerThread))
        [self runAddMarkers:markerInfo];
    else
        [self performSelector:@selector(runAddMarkers:) onThread:layerThread withObject:markerInfo waitUntilDone:NO];
    
    return markerInfo.markerId;
}

// Add a group of markers
- (SimpleIdentity) addMarkers:(NSArray *)markers desc:(NSDictionary *)desc
{
    MarkerInfo *markerInfo = [[MarkerInfo alloc] initWithMarkers:markers desc:desc];

    if (!layerThread || ([NSThread currentThread] == layerThread))
        [self runAddMarkers:markerInfo];
    else
        [self performSelector:@selector(runAddMarkers:) onThread:layerThread withObject:markerInfo waitUntilDone:NO];
    
    return markerInfo.markerId;
}

// Remove a group of markers
- (void) removeMarkers:(SimpleIdentity)markerID
{
    NSNumber *num = [NSNumber numberWithUnsignedInt:markerID];
    if (!layerThread | ([NSThread currentThread] == layerThread))
        [self runRemoveMarkers:num];
    else
        [self performSelector:@selector(runRemoveMarkers:) onThread:layerThread withObject:num waitUntilDone:NO];
}


@end
