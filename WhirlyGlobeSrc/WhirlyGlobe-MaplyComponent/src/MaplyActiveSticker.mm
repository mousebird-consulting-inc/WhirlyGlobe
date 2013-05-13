/*
 *  MaplyActiveSticker.mm
 *  WhirlyGlobe-MaplyComponent
 *
 *  Created by Steve Gifford on 4/8/13.
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

#import "MaplyActiveSticker.h"
#import "WhirlyGlobe.h"
#import "MaplyActiveObject_private.h"

using namespace WhirlyKit;

@implementation MaplyActiveSticker
{
    bool changed;
    SimpleIDSet drawIDs;
    NSDictionary *desc;
    UIImage *image;
    SimpleIdentity texId;
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

- (void)setSticker:(MaplySticker *)newSticker
{
    if ([NSThread currentThread] != [NSThread mainThread])
        return;
    
    changed = true;
    _sticker = newSticker;
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

    CoordSystemDisplayAdapter *coordAdapter = scene->getCoordAdapter();
    
    std::vector<ChangeRequest *> changes;
    // Get rid of the old drawables
    for (SimpleIDSet::iterator it = drawIDs.begin();
         it != drawIDs.end(); ++it)
        changes.push_back(new RemDrawableReq(*it));
    drawIDs.clear();

    SimpleIdentity removeTexId = EmptyIdentity;
    if (_sticker)
    {
        // Possibly get rid of the image too
        if (image && _sticker.image != image)
        {
            removeTexId = texId;
            texId = EmptyIdentity;
            image = nil;
        }
        
        // And make a new one
        if (_sticker.image != image)
        {
            Texture *tex = new Texture("Active Sticker",_sticker.image);
            texId = tex->getId();
            image = _sticker.image;
            changes.push_back(new AddTextureReq(tex));
            
            // Note: Debugging
//            NSData *imageData = UIImagePNGRepresentation(image);
//            if (imageData)
//            {
//                NSArray *myPathList = NSSearchPathForDirectoriesInDomains(NSCachesDirectory, NSUserDomainMask, YES);
//                NSString *myPath    = [myPathList  objectAtIndex:0];
//                [imageData writeToFile:[NSString stringWithFormat:@"%@/%ld.png",myPath,texId] atomically:YES];
//            }
        }
        
        // Make some new drawables
        WhirlyKitSphericalChunk *chunk = [[WhirlyKitSphericalChunk alloc] init];
        GeoMbr geoMbr = GeoMbr(GeoCoord(_sticker.ll.x,_sticker.ll.y), GeoCoord(_sticker.ur.x,_sticker.ur.y));
        chunk.mbr = geoMbr;
        chunk.texId = texId;
        chunk.drawOffset = [desc[@"drawOffset"] floatValue];
        chunk.drawPriority = [desc[@"drawPriority"] floatValue];
        chunk.sampleX = [desc[@"sampleX"] intValue];
        chunk.sampleY = [desc[@"sampleY"] intValue];
        chunk.rotation = _sticker.rotation;
        
        BasicDrawable *drawable=nil,*skirtDrawable=nil;
        [chunk buildDrawable:&drawable skirtDraw:&skirtDrawable enabled:true adapter:coordAdapter];
        if (drawable)
        {
            changes.push_back(new AddDrawableReq(drawable));
            drawIDs.insert(drawable->getId());
        }
        if (skirtDrawable)
        {
            changes.push_back(new AddDrawableReq(skirtDrawable));
            drawIDs.insert(skirtDrawable->getId());
        }
    }

    if (removeTexId != EmptyIdentity)
        changes.push_back(new RemTextureReq(removeTexId));
    
    scene->addChangeRequests(changes);
    
    changed = false;
}

- (void)shutdown
{
    std::vector<ChangeRequest *> changes;
    // Get rid of the old drawables
    for (SimpleIDSet::iterator it = drawIDs.begin();
         it != drawIDs.end(); ++it)
        changes.push_back(new RemDrawableReq(*it));
    drawIDs.clear();
    if (texId != EmptyIdentity)
    {
        changes.push_back(new RemTextureReq(texId));
        texId = EmptyIdentity;
    }

    scene->addChangeRequests(changes);
}

@end
