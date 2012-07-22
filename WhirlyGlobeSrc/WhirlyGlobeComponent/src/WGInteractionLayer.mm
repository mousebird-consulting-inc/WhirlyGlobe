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

- (void)startWithThread:(WhirlyKitLayerThread *)inLayerThread scene:(WhirlyKit::Scene *)inScene
{
    layerThread = inLayerThread;
    scene = (WhirlyGlobe::GlobeScene *)inScene;
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
    tex->createInGL(YES, scene->getMemManager());
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
        
        [wgMarkers addObject:wgMarker];
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

@end