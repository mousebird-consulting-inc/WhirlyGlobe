/*
 *  WGInteractionLayer_private.h
 *  WhirlyGlobeComponent
 *
 *  Created by Steve Gifford on 7/21/12.
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

#import "WGInteractionLayer_private.h"
#import "WGScreenMarker.h"
#import "WGMarker.h"
#import "WGScreenLabel.h"
#import "WGLabel.h"
#import "WGVectorObject_private.h"

using namespace Eigen;
using namespace WhirlyKit;
using namespace WhirlyGlobe;

// Used to map UIImages to Texture IDs
class ImageTexture
{
public:
    ImageTexture(UIImage *image) : image(image), texID(EmptyIdentity) { }
    ImageTexture(UIImage *image,SimpleIdentity texID) : image(image), texID(texID) { }
    bool operator < (const ImageTexture &that) const { return image < that.image; }
    
    UIImage *image;
    SimpleIdentity texID;
};
typedef std::set<ImageTexture> ImageTextureSet;

@implementation WGInteractionLayer
{
    WhirlyKitLayerThread * __weak layerThread;
    WhirlyGlobe::GlobeScene *scene;
    ImageTextureSet imageTextures;
}

@synthesize markerLayer;
@synthesize labelLayer;
@synthesize vectorLayer;
@synthesize selectLayer;
@synthesize glView;
@synthesize viewController;

- (void)startWithThread:(WhirlyKitLayerThread *)inLayerThread scene:(WhirlyKit::Scene *)inScene
{
    layerThread = inLayerThread;
    scene = (WhirlyGlobe::GlobeScene *)inScene;
    userObjects = [NSMutableArray array];
}

- (void)dealloc
{    
}

/// Called by the layer thread to shut a layer down.
/// Clean all your stuff out of the scenegraph and so forth.
- (void)shutdown
{
    layerThread = nil;
    scene = NULL;
    imageTextures.clear();
}

// Add an image to the cache, or find an existing one
// Called in the layer thread
- (SimpleIdentity)addImage:(UIImage *)image
{
    // Look for an existing one
    ImageTextureSet::iterator it = imageTextures.find(ImageTexture(image));
    if (it != imageTextures.end())
        return it->texID;
    
    // Add it and download it
    Texture *tex = new Texture(image);
//    tex->createInGL(YES, scene->getMemManager());
    scene->addChangeRequest(new AddTextureReq(tex));
    
    // Add to our cache
    ImageTexture newTex(image,tex->getId());
    imageTextures.insert(newTex);
    
    return newTex.texID;
}

// Actually add the markers.
// Called in the layer thread.
- (void)addScreenMarkersLayerThread:(NSArray *)argArray
{
    NSArray *markers = [argArray objectAtIndex:0];
    WGComponentObject *compObj = [argArray objectAtIndex:1];
    NSDictionary *inDesc = [argArray objectAtIndex:2];
    
    // Convert to WG markers
    NSMutableArray *wgMarkers = [NSMutableArray array];
    for (WGScreenMarker *marker in markers)
    {
        WhirlyKitMarker *wgMarker = [[WhirlyKitMarker alloc] init];
        wgMarker.loc = GeoCoord(marker.loc.lon,marker.loc.lat);
        SimpleIdentity texID = EmptyIdentity;
        if (marker.image)
            texID = [self addImage:marker.image];
        if (texID != EmptyIdentity)
            wgMarker.texIDs.push_back(texID);
        wgMarker.width = marker.size.width;
        wgMarker.height = marker.size.height;
        wgMarker.isSelectable = true;
        wgMarker.selectID = Identifiable::genId();
        
        [wgMarkers addObject:wgMarker];
        
        selectObjectSet.insert(SelectObject(wgMarker.selectID,marker));
    }

    // Set up a description and create the markers in the marker layer
    NSMutableDictionary *desc = [NSMutableDictionary dictionaryWithDictionary:inDesc];
    [desc setObject:[NSNumber numberWithBool:YES] forKey:@"screen"];
    SimpleIdentity markerID = [markerLayer addMarkers:wgMarkers desc:desc];
    compObj.markerIDs.insert(markerID);
    
    [userObjects addObject:compObj];
}

// Called in the main thread.
- (WGComponentObject *)addScreenMarkers:(NSArray *)markers desc:(NSDictionary *)desc
{
    WGComponentObject *compObj = [[WGComponentObject alloc] init];
    
    NSArray *argArray = [NSArray arrayWithObjects:markers, compObj, desc, nil];    
    [self performSelector:@selector(addScreenMarkersLayerThread:) onThread:layerThread withObject:argArray waitUntilDone:NO];
    
    return compObj;
}

// Actually add the markers.
// Called in the layer thread.
- (void)addMarkersLayerThread:(NSArray *)argArray
{
    NSArray *markers = [argArray objectAtIndex:0];
    WGComponentObject *compObj = [argArray objectAtIndex:1];
    NSDictionary *inDesc = [argArray objectAtIndex:2];
    
    // Convert to WG markers
    NSMutableArray *wgMarkers = [NSMutableArray array];
    for (WGMarker *marker in markers)
    {
        WhirlyKitMarker *wgMarker = [[WhirlyKitMarker alloc] init];
        wgMarker.loc = GeoCoord(marker.loc.lon,marker.loc.lat);
        SimpleIdentity texID = EmptyIdentity;
        if (marker.image)
            texID = [self addImage:marker.image];
        if (texID != EmptyIdentity)
            wgMarker.texIDs.push_back(texID);
        wgMarker.width = marker.size.width;
        wgMarker.height = marker.size.height;
        wgMarker.isSelectable = true;
        wgMarker.selectID = Identifiable::genId();
        
        [wgMarkers addObject:wgMarker];
        
        selectObjectSet.insert(SelectObject(wgMarker.selectID,marker));
    }
    
    // Set up a description and create the markers in the marker layer
    SimpleIdentity markerID = [markerLayer addMarkers:wgMarkers desc:inDesc];
    compObj.markerIDs.insert(markerID);
    
    [userObjects addObject:compObj];
}

// Add 3D markers
- (WGComponentObject *)addMarkers:(NSArray *)markers desc:(NSDictionary *)desc
{
    WGComponentObject *compObj = [[WGComponentObject alloc] init];
    
    NSArray *argArray = [NSArray arrayWithObjects:markers, compObj, desc, nil];
    [self performSelector:@selector(addMarkersLayerThread:) onThread:layerThread withObject:argArray waitUntilDone:NO];
    
    return compObj;
}

// Actually add the labels.
// Called in the layer thread.
- (void)addScreenLabelsLayerThread:(NSArray *)argArray
{
    NSArray *labels = [argArray objectAtIndex:0];
    WGComponentObject *compObj = [argArray objectAtIndex:1];
    NSDictionary *inDesc = [argArray objectAtIndex:2];
    
    // Convert to WG markers
    NSMutableArray *wgLabels = [NSMutableArray array];
    for (WGScreenLabel *label in labels)
    {
        WhirlyKitSingleLabel *wgLabel = [[WhirlyKitSingleLabel alloc] init];
        NSMutableDictionary *desc = [NSMutableDictionary dictionary];
        wgLabel.loc = GeoCoord(label.loc.lon,label.loc.lat);
        wgLabel.text = label.text;
        SimpleIdentity texID = EmptyIdentity;
        if (label.iconImage)
            texID = [self addImage:label.iconImage];
        wgLabel.iconTexture = texID;
        if (label.size.width > 0.0)
            [desc setObject:[NSNumber numberWithFloat:label.size.width] forKey:@"width"];
        if (label.size.height > 0.0)
            [desc setObject:[NSNumber numberWithFloat:label.size.height] forKey:@"height"];
        wgLabel.isSelectable = true;
        wgLabel.selectID = Identifiable::genId();
        if ([desc count] > 0)
            wgLabel.desc = desc;
        
        [wgLabels addObject:wgLabel];
        
        selectObjectSet.insert(SelectObject(wgLabel.selectID,label));
    }
    
    // Set up a description and create the markers in the marker layer
    NSMutableDictionary *desc = [NSMutableDictionary dictionaryWithDictionary:inDesc];
    [desc setObject:[NSNumber numberWithBool:YES] forKey:@"screen"];
    SimpleIdentity labelID = [labelLayer addLabels:wgLabels desc:desc];
    compObj.labelIDs.insert(labelID);
    
    [userObjects addObject:compObj];
}

// Add screen space (2D) labels
- (WGComponentObject *)addScreenLabels:(NSArray *)labels desc:(NSDictionary *)desc
{
    WGComponentObject *compObj = [[WGComponentObject alloc] init];
    
    NSArray *argArray = [NSArray arrayWithObjects:labels, compObj, desc, nil];    
    [self performSelector:@selector(addScreenLabelsLayerThread:) onThread:layerThread withObject:argArray waitUntilDone:NO];
    
    return compObj;
}

// Actually add the labels.
// Called in the layer thread.
- (void)addLabelsLayerThread:(NSArray *)argArray
{
    NSArray *labels = [argArray objectAtIndex:0];
    WGComponentObject *compObj = [argArray objectAtIndex:1];
    NSDictionary *inDesc = [argArray objectAtIndex:2];
    
    // Convert to WG markers
    NSMutableArray *wgLabels = [NSMutableArray array];
    for (WGScreenLabel *label in labels)
    {
        WhirlyKitSingleLabel *wgLabel = [[WhirlyKitSingleLabel alloc] init];
        NSMutableDictionary *desc = [NSMutableDictionary dictionary];
        wgLabel.loc = GeoCoord(label.loc.lon,label.loc.lat);
        wgLabel.text = label.text;
        SimpleIdentity texID = EmptyIdentity;
        if (label.iconImage)
            texID = [self addImage:label.iconImage];
        wgLabel.iconTexture = texID;
        if (label.size.width > 0.0)
            [desc setObject:[NSNumber numberWithFloat:label.size.width] forKey:@"width"];
        if (label.size.height > 0.0)
            [desc setObject:[NSNumber numberWithFloat:label.size.height] forKey:@"height"];
        wgLabel.isSelectable = true;
        wgLabel.selectID = Identifiable::genId();
        wgLabel.desc = desc;
        
        [wgLabels addObject:wgLabel];
        
        selectObjectSet.insert(SelectObject(wgLabel.selectID,label));
    }
    
    // Set up a description and create the markers in the marker layer
    SimpleIdentity labelID = [labelLayer addLabels:wgLabels desc:inDesc];
    compObj.labelIDs.insert(labelID);
    
    [userObjects addObject:compObj];
}

// Add 3D labels
- (WGComponentObject *)addLabels:(NSArray *)labels desc:(NSDictionary *)desc
{
    WGComponentObject *compObj = [[WGComponentObject alloc] init];
    
    NSArray *argArray = [NSArray arrayWithObjects:labels, compObj, desc, nil];    
    [self performSelector:@selector(addLabelsLayerThread:) onThread:layerThread withObject:argArray waitUntilDone:NO];
    
    return compObj;
}

// Actually add the vectors.
// Called in the layer thread.
- (void)addVectorsLayerThread:(NSArray *)argArray
{
    NSArray *vectors = [argArray objectAtIndex:0];
    WGComponentObject *compObj = [argArray objectAtIndex:1];
    compObj.vectors = vectors;
    NSDictionary *inDesc = [argArray objectAtIndex:2];    
    
    ShapeSet shapes;
    for (WGVectorObject *vecObj in vectors)
    {
        shapes.insert(vecObj.shapes.begin(),vecObj.shapes.end());
    }
    
    SimpleIdentity vecID = [vectorLayer addVectors:&shapes desc:inDesc];
    compObj.vectorIDs.insert(vecID);
    
    [userObjects addObject:compObj];
}

// Add vectors
- (WGComponentObject *)addVectors:(NSArray *)vectors desc:(NSDictionary *)desc
{
    WGComponentObject *compObj = [[WGComponentObject alloc] init];
    
    NSArray *argArray = [NSArray arrayWithObjects:vectors, compObj, desc, nil];
    [self performSelector:@selector(addVectorsLayerThread:) onThread:layerThread withObject:argArray waitUntilDone:NO];
    
    return compObj;
}

// Remove the object, but do it on the layer thread
- (void)removeObjectLayerThread:(NSArray *)userObjs
{
    // First, let's make sure we're representing it
    for (WGComponentObject *userObj in userObjs)
    {
        if ([userObjects containsObject:userObj])
        {
            // Get rid of the various layer objects
            for (SimpleIDSet::iterator it = userObj.markerIDs.begin();
                 it != userObj.markerIDs.end(); ++it)
                [markerLayer removeMarkers:*it];
            for (SimpleIDSet::iterator it = userObj.labelIDs.begin();
                 it != userObj.labelIDs.end(); ++it)
                [labelLayer removeLabel:*it];
            for (SimpleIDSet::iterator it = userObj.vectorIDs.begin();
                 it != userObj.vectorIDs.end(); ++it)
                [vectorLayer removeVector:*it];
            
            [userObjects removeObject:userObj];
        }    
    }
}

// Remove data associated with a user object
- (void)removeObject:(WGComponentObject *)userObj
{
    [self performSelector:@selector(removeObjectLayerThread:) onThread:layerThread withObject:[NSArray arrayWithObject:userObj] waitUntilDone:NO];
}

// Remove a group of objects at once
- (void)removeObjects:(NSArray *)userObjs
{
    [self performSelector:@selector(removeObjectLayerThread:) onThread:layerThread withObject:userObjs waitUntilDone:NO];
}

// Search for a point inside any of our vector objects
// Runs in layer thread
- (NSObject *)findVectorInPoint:(Point2f)pt
{
    NSObject *selObj = nil;
    
    for (WGComponentObject *userObj in userObjects)
    {
        if (userObj.vectors)
        {
            for (WGVectorObject *vecObj in userObj.vectors)
            {
                WGCoordinate coord;
                coord.lon = pt.x();
                coord.lat = pt.y();
                if ([vecObj pointInAreal:coord])
                {
                    selObj = vecObj;
                    break;
                }
            }
        }
    }
    
    return selObj;
}

// Do the logic for a selection
// Runs in the layer thread
- (void) userDidTapLayerThread:(WhirlyGlobeTapMessage *)msg
{
    // First, we'll look for labels and markers
    SimpleIdentity selID = [selectLayer pickObject:Point2f(msg.touchLoc.x,msg.touchLoc.y) view:glView maxDist:10.0];

    NSObject *selObj;
    if (selID != EmptyIdentity)
    {       
        // Found something.  Now find the associated object
        SelectObjectSet::iterator it = selectObjectSet.find(SelectObject(selID));
        if (it != selectObjectSet.end())
        {
            selObj = it->obj;
        }
    } else {
        // Next, try the vectors
        selObj = [self findVectorInPoint:Point2f(msg.whereGeo.x(),msg.whereGeo.y())];
    }
    
    // Tell the view controller about it
    dispatch_async(dispatch_get_main_queue(),^
                   {
                       [viewController handleSelection:msg didSelect:selObj];
                   }
                   );
}

// Check for a selection
- (void) userDidTap:(WhirlyGlobeTapMessage *)msg
{
    // Pass it off to the layer thread
    [self performSelector:@selector(userDidTapLayerThread:) onThread:layerThread withObject:msg waitUntilDone:NO];
}

@end