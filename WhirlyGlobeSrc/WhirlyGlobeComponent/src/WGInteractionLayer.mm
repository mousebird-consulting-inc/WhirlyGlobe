/*
 *  WGInteractionLayer_private.h
 *  WhirlyGlobeComponent
 *
 *  Created by Steve Gifford on 7/21/12.
 *  Copyright 2011 mousebird consulting
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
    NSDictionary *desc = [NSDictionary dictionaryWithObjectsAndKeys:
                          [NSNumber numberWithBool:YES], @"screen", 
                          nil];
    SimpleIdentity markerID = [markerLayer addMarkers:wgMarkers desc:desc];
    compObj.markerIDs.insert(markerID);
    
    [userObjects addObject:compObj];
}

// Called in the main thread.
- (WGComponentObject *)addScreenMarkers:(NSArray *)markers
{
    WGComponentObject *compObj = [[WGComponentObject alloc] init];
    
    NSDictionary *argArray = [NSArray arrayWithObjects:markers, compObj, nil];    
    [self performSelector:@selector(addScreenMarkersLayerThread:) onThread:layerThread withObject:argArray waitUntilDone:NO];
    
    return compObj;
}

// Actually add the markers.
// Called in the layer thread.
- (void)addMarkersLayerThread:(NSArray *)argArray
{
    NSArray *markers = [argArray objectAtIndex:0];
    WGComponentObject *compObj = [argArray objectAtIndex:1];
    
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
    NSDictionary *desc = [NSDictionary dictionaryWithObjectsAndKeys:
                          [NSNumber numberWithInt:1], @"drawOffset",
                          nil];
    SimpleIdentity markerID = [markerLayer addMarkers:wgMarkers desc:desc];
    compObj.markerIDs.insert(markerID);
    
    [userObjects addObject:compObj];
}

// Add 3D markers
- (WGComponentObject *)addMarkers:(NSArray *)markers
{
    WGComponentObject *compObj = [[WGComponentObject alloc] init];
    
    NSDictionary *argArray = [NSArray arrayWithObjects:markers, compObj, nil];
    [self performSelector:@selector(addMarkersLayerThread:) onThread:layerThread withObject:argArray waitUntilDone:NO];
    
    return compObj;
}

// Actually add the labels.
// Called in the layer thread.
- (void)addScreenLabelsLayerThread:(NSArray *)argArray
{
    NSArray *labels = [argArray objectAtIndex:0];
    WGComponentObject *compObj = [argArray objectAtIndex:1];
    
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
    NSDictionary *desc = [NSDictionary dictionaryWithObjectsAndKeys:
                          [NSNumber numberWithBool:YES], @"screen", 
                          nil];
    SimpleIdentity labelID = [labelLayer addLabels:wgLabels desc:desc];
    compObj.labelIDs.insert(labelID);
    
    [userObjects addObject:compObj];
}

// Add screen space (2D) labels
- (WGComponentObject *)addScreenLabels:(NSArray *)labels
{
    WGComponentObject *compObj = [[WGComponentObject alloc] init];
    
    NSDictionary *argArray = [NSArray arrayWithObjects:labels, compObj, nil];    
    [self performSelector:@selector(addScreenLabelsLayerThread:) onThread:layerThread withObject:argArray waitUntilDone:NO];
    
    return compObj;
}

// Actually add the labels.
// Called in the layer thread.
- (void)addLabelsLayerThread:(NSArray *)argArray
{
    NSArray *labels = [argArray objectAtIndex:0];
    WGComponentObject *compObj = [argArray objectAtIndex:1];
    
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
    NSDictionary *desc = [NSDictionary dictionaryWithObjectsAndKeys:
                          nil];
    SimpleIdentity labelID = [labelLayer addLabels:wgLabels desc:desc];
    compObj.labelIDs.insert(labelID);
    
    [userObjects addObject:compObj];
}

// Add 3D labels
- (WGComponentObject *)addLabels:(NSArray *)labels
{
    WGComponentObject *compObj = [[WGComponentObject alloc] init];
    
    NSDictionary *argArray = [NSArray arrayWithObjects:labels, compObj, nil];    
    [self performSelector:@selector(addLabelsLayerThread:) onThread:layerThread withObject:argArray waitUntilDone:NO];
    
    return compObj;
}

// Remove the object, but do it on the layer thread
- (void)removeObjectLayerThread:(WGComponentObject *)userObj
{
    
    // First, let's make sure we're representing it
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

// Remove data associated with a user object
- (void)removeObject:(WGComponentObject *)userObj
{
    [self performSelector:@selector(removeObjectLayerThread:) onThread:layerThread withObject:userObj waitUntilDone:NO];
}

// Do the logic for a selection
// Runs in the layer thread
- (void) userDidTapLayerThread:(WhirlyGlobeTapMessage *)msg
{
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