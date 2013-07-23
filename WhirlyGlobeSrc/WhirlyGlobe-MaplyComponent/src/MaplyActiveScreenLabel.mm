/*
 *  MaplyActiveScreenLabel.mm
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

#import "MaplyActiveScreenLabel.h"
#import "WhirlyGlobe.h"
#import "MaplyActiveObject_private.h"
#import "LayoutManager.h"

using namespace WhirlyKit;

@implementation MaplyActiveScreenLabel
{
    bool changed;
    UIImage *iconImage;
    SimpleIDSet drawIDs;
    SimpleIDSet screenIDs;
    SimpleIdentity iconTexId;
}

- (id)initWithDesc:(NSDictionary *)descDict
{
    self = [super init];
    if (!self)
        return nil;
    
    changed = false;
    self.desc = descDict;
    
    return self;
}

- (void)setScreenLabel:(MaplyScreenLabel *)newScreenLabel
{
    if ([NSThread currentThread] != [NSThread mainThread])
        return;
    
    changed = true;
    _screenLabel = newScreenLabel;
}

- (void)setDesc:(NSDictionary *)inDesc
{
    super.desc = inDesc;
    changed = true;
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
    
    ChangeSet changes;
    
    // Remove the old label
    for (SimpleIDSet::iterator it = drawIDs.begin();
         it != drawIDs.end(); ++it)
        changes.push_back(new RemDrawableReq(*it));
    drawIDs.clear();
    for (SimpleIDSet::iterator it = screenIDs.begin();
         it != screenIDs.end(); ++it)
        changes.push_back(new ScreenSpaceGeneratorRemRequest(scene->getScreenSpaceGeneratorID(), *it));
    LayoutManager *layoutManager = (LayoutManager *)scene->getManager(kWKLayoutManager);
    if (layoutManager && !screenIDs.empty())
    {
        layoutManager->removeLayoutObjects(screenIDs);
        screenIDs.clear();
    }

    SimpleIdentity removeTexId = EmptyIdentity;
    if (_screenLabel)
    {
        // Possibly get rid of the image too
        if (iconImage && _screenLabel.iconImage != iconImage)
        {
            removeTexId = iconTexId;
            iconTexId = EmptyIdentity;
            iconImage = nil;
        }

        // And make a new one
        if (_screenLabel.iconImage != iconImage)
        {
            Texture *tex = new Texture("Active ScreenLabel",_screenLabel.iconImage);
            iconTexId = tex->getId();
            iconImage = _screenLabel.iconImage;
            changes.push_back(new AddTextureReq(tex));            
        }
        
        WhirlyKitSingleLabel *wgLabel = [[WhirlyKitSingleLabel alloc] init];
        NSMutableDictionary *locDesc = [NSMutableDictionary dictionary];
        wgLabel.loc = GeoCoord(_screenLabel.loc.x,_screenLabel.loc.y);
        wgLabel.text = _screenLabel.text;
        wgLabel.iconTexture = iconTexId;
        wgLabel.iconSize = _screenLabel.iconSize;
        if (_screenLabel.size.width > 0.0)
            [locDesc setObject:[NSNumber numberWithFloat:_screenLabel.size.width] forKey:@"width"];
        if (_screenLabel.size.height > 0.0)
            [locDesc setObject:[NSNumber numberWithFloat:_screenLabel.size.height] forKey:@"height"];
        if (_screenLabel.color)
            [locDesc setObject:_screenLabel.color forKey:@"textColor"];
        if (_screenLabel.layoutImportance != MAXFLOAT)
        {
            [locDesc setObject:@(YES) forKey:@"layout"];
            [locDesc setObject:@(_screenLabel.layoutImportance) forKey:@"layoutImportance"];
            [locDesc setObject:@(_screenLabel.layoutPlacement) forKey:@"layoutPlacement"];
        }
        wgLabel.screenOffset = _screenLabel.offset;
        // Note: Not allowing selection for the moment
        wgLabel.isSelectable = false;
        if ([locDesc count] > 0)
            wgLabel.desc = locDesc;

        WhirlyKitLabelInfo *labelInfo = [[WhirlyKitLabelInfo alloc] initWithStrs:[NSArray arrayWithObject:wgLabel] desc:self.desc];
        labelInfo.screenObject = true;

        // Set up the representation (but then hand it off)
        LabelSceneRep *labelRep = new LabelSceneRep();
        labelRep->fade = labelInfo.fade;
        
        // Set up the label renderer
        WhirlyKitLabelRenderer *labelRenderer = [[WhirlyKitLabelRenderer alloc] init];
        labelRenderer->labelInfo = labelInfo;
        labelRenderer->textureAtlasSize = 0;
        labelRenderer->coordAdapter = scene->getCoordAdapter();
        labelRenderer->labelRep = labelRep;
        labelRenderer->scene = scene;
        labelRenderer->fontTexManager = scene->getFontTextureManager();
        labelRenderer->screenGenId = scene->getScreenSpaceGeneratorID();
        
        [labelRenderer render];
        
        // Now harvest the results
        changes.insert(changes.end(), labelRenderer->changeRequests.begin(), labelRenderer->changeRequests.end());
        scene->addChangeRequests(changes);
        if (layoutManager)
            layoutManager->addLayoutObjects(labelRenderer->layoutObjects);
        labelRenderer->changeRequests.clear();
        drawIDs = labelRep->drawIDs;
        screenIDs = labelRep->screenIDs;
        // Note: Not expecting textures from the label renderer
    } else
    scene->addChangeRequests(changes);
    
    changed = false;
}

- (void)shutdown
{
    ChangeSet changes;

    // Get rid of drawables and screen objects
    for (SimpleIDSet::iterator it = drawIDs.begin();it != drawIDs.end(); ++it)
        changes.push_back(new RemDrawableReq(*it));
    drawIDs.clear();
    for (SimpleIDSet::iterator it = screenIDs.begin();it != screenIDs.end(); ++it)
        changes.push_back(new ScreenSpaceGeneratorRemRequest(scene->getScreenSpaceGeneratorID(), *it));
    if (iconTexId != EmptyIdentity)
        changes.push_back(new RemTextureReq(iconTexId));
    LayoutManager *layoutManager = (LayoutManager *)scene->getManager(kWKLayoutManager);
    if (layoutManager && !screenIDs.empty())
    {
        layoutManager->removeLayoutObjects(screenIDs);
        screenIDs.clear();
    }
    
    scene->addChangeRequests(changes);
}


@end

