/*
 *  MaplyActiveScreenMarker.mm
 *  WhirlyGlobe-MaplyComponent
 *
 *  Created by Steve Gifford on 5/21/13.
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

#import "MaplyActiveScreenMarker.h"
#import "WhirlyGlobe.h"
#import "MaplyActiveObject_private.h"

using namespace Eigen;
using namespace WhirlyKit;

@implementation MaplyActiveScreenMarker
{
    bool changed;
    NSDictionary *desc;
    UIImage *image;
    SimpleIdentity texID;
    SimpleIDSet screenIDs;
}

- (id)initWithDesc:(NSDictionary *)descDict
{
    self = [super init];
    if (!self)
        return nil;
    
    changed = false;
    desc = descDict;
    
    return self;
}

- (void)setScreenMarker:(MaplyScreenMarker *)newScreenMarker
{
    if ([NSThread currentThread] != [NSThread mainThread])
        return;
    
    changed = true;
    _screenMarker = newScreenMarker;
}

- (bool)hasUpdate
{
    return changed;
}

// Flush out changes to the scene
- (void)updateForFrame:(WhirlyKitRendererFrameInfo *)frameInfo
{
    if (!changed)
        return;
    
    std::vector<ChangeRequest *> changes;

    // Remove the old marker
    for (SimpleIDSet::iterator it = screenIDs.begin();it != screenIDs.end(); ++it)
        changes.push_back(new ScreenSpaceGeneratorRemRequest(scene->getScreenSpaceGeneratorID(), *it));
    screenIDs.clear();
    
    // And possibly the image
    if (image != _screenMarker.image)
    {
        if (texID != EmptyIdentity)
        {
            changes.push_back(new RemTextureReq(texID));
            texID = EmptyIdentity;
        }
        image = nil;
    }
    
    if (_screenMarker)
    {
        // New image
        if (_screenMarker.image)
        {
            Texture *tex = new Texture("Active ScreenLabel",_screenMarker.image);
            texID = tex->getId();
            image = _screenMarker.image;
            changes.push_back(new AddTextureReq(tex));
        }

        // Need the WK version of the Marker
        WhirlyKitMarker *marker = [[WhirlyKitMarker alloc] init];
        marker.loc = GeoCoord(_screenMarker.loc.x,_screenMarker.loc.y);
        if (texID != EmptyIdentity)
            marker.texIDs.push_back(texID);
        marker.width = _screenMarker.size.width;
        marker.height = _screenMarker.size.height;
        
        // Note: Deal with selection
        
        // Build the marker
        WhirlyKitMarkerInfo *markerInfo = [[WhirlyKitMarkerInfo alloc] initWithMarkers:nil desc:desc];
        std::vector<ScreenSpaceGenerator::ConvexShape *> screenShapes;
        
        // Build the rectangle for this one
        Point3f pts[4];
        Vector3f norm;
        float width2 = (marker.width == 0.0 ? markerInfo.width : marker.width)/2.0;
        float height2 = (marker.height == 0.0 ? markerInfo.height : marker.height)/2.0;
        
        Point3f localPt = scene->getCoordAdapter()->getCoordSystem()->geographicToLocal(marker.loc);
        norm = scene->getCoordAdapter()->normalForLocal(localPt);
        
        pts[0] = Point3f(-width2,-height2,0.0);
        pts[1] = Point3f(width2,-height2,0.0);
        pts[2] = Point3f(width2,height2,0.0);
        pts[3] = Point3f(-width2,height2,0.0);

        // Build one set of texture coordinates
        std::vector<TexCoord> texCoord;
        texCoord.resize(4);
        texCoord[0].u() = 0.0;  texCoord[0].v() = 0.0;
        texCoord[1].u() = 1.0;  texCoord[1].v() = 0.0;
        texCoord[2].u() = 1.0;  texCoord[2].v() = 1.0;
        texCoord[3].u() = 0.0;  texCoord[3].v() = 1.0;

        ScreenSpaceGenerator::SimpleGeometry smGeom;
        smGeom.texID = texID;
        smGeom.color = [markerInfo.color asRGBAColor];
        for (unsigned int ii=0;ii<4;ii++)
        {
            smGeom.coords.push_back(Point2f(pts[ii].x(),pts[ii].y()));
            smGeom.texCoords.push_back(texCoord[ii]);
        }
        ScreenSpaceGenerator::ConvexShape *shape = new ScreenSpaceGenerator::ConvexShape();
        if (marker.isSelectable && marker.selectID != EmptyIdentity)
            shape->setId(marker.selectID);
        shape->worldLoc = scene->getCoordAdapter()->localToDisplay(localPt);
        if (marker.lockRotation)
        {
            shape->useRotation = true;
            shape->rotation = marker.rotation;
        }
        // Note: No fade
        shape->minVis = markerInfo.minVis;
        shape->maxVis = markerInfo.maxVis;
        shape->drawPriority = markerInfo.drawPriority;
        shape->geom.push_back(smGeom);
        screenShapes.push_back(shape);
        screenIDs.insert(shape->getId());

        // Add all the screen space markers at once
        if (!screenShapes.empty())
            changes.push_back(new ScreenSpaceGeneratorAddRequest(scene->getScreenSpaceGeneratorID(),screenShapes));
        screenShapes.clear();
    }
    
    
    scene->addChangeRequests(changes);
    
    changed = false;
}

- (void)shutdown
{
    std::vector<ChangeRequest *> changes;
    
    // Get rid of drawables and screen objects
    for (SimpleIDSet::iterator it = screenIDs.begin();it != screenIDs.end(); ++it)
        changes.push_back(new ScreenSpaceGeneratorRemRequest(scene->getScreenSpaceGeneratorID(), *it));
    if (texID != EmptyIdentity)
        changes.push_back(new RemTextureReq(texID));
    
    scene->addChangeRequests(changes);
}


@end

