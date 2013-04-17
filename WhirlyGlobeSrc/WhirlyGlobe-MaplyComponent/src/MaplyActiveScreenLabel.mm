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

#import "MaplyActiveScreenLabel.h"
#import "WhirlyGlobe.h"
#import "MaplyActiveObject_private.h"

using namespace WhirlyKit;

@implementation MaplyActiveScreenLabel
{
    bool changed;
    NSDictionary *desc;
    SimpleIDSet texIDs;
    SimpleIDSet drawIDs;
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

- (void)setScreenLabel:(MaplyScreenLabel *)newScreenLabel
{
    if ([NSThread currentThread] != [NSThread mainThread])
        return;
    
    changed = true;
    _screenLabel = newScreenLabel;
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
    
    // Remove the old label
    for (SimpleIDSet::iterator it = drawIDs.begin();
         it != drawIDs.end(); ++it)
        changes.push_back(new RemDrawableReq(*it));
    drawIDs.clear();
    for (SimpleIDSet::iterator it = texIDs.begin();
         it != texIDs.end(); ++it)
        changes.push_back(new RemTextureReq(*it));
    texIDs.clear();        

    if (_screenLabel)
    {
        WhirlyKitSingleLabel *wgLabel = [[WhirlyKitSingleLabel alloc] init];
        NSMutableDictionary *locDesc = [NSMutableDictionary dictionary];
        wgLabel.loc = GeoCoord(_screenLabel.loc.x,_screenLabel.loc.y);
        wgLabel.text = _screenLabel.text;
        if (_screenLabel.iconImage)
        {
            // Note: Not handling this case yet
        }
//        wgLabel.iconTexture = texID;
        if (_screenLabel.size.width > 0.0)
            [locDesc setObject:[NSNumber numberWithFloat:_screenLabel.size.width] forKey:@"width"];
        if (_screenLabel.size.height > 0.0)
            [locDesc setObject:[NSNumber numberWithFloat:_screenLabel.size.height] forKey:@"height"];
        if (_screenLabel.color)
            [locDesc setObject:_screenLabel.color forKey:@"textColor"];
        if (_screenLabel.layoutImportance != MAXFLOAT)
        {
            // Note: Not handling this case yet
        }
        wgLabel.screenOffset = _screenLabel.offset;
        // Note: Not allowing selection for the moment
        wgLabel.isSelectable = false;
        if ([locDesc count] > 0)
            wgLabel.desc = locDesc;

        WhirlyKitLabelInfo *labelInfo = [[WhirlyKitLabelInfo alloc] initWithStrs:[NSArray arrayWithObject:_screenLabel.text] desc:desc];

        // Set up the representation (but then hand it off)
        LabelSceneRep *labelRep = new LabelSceneRep();
        labelRep->fade = labelInfo.fade;
        labelRep->setId(labelInfo.labelId);
        
        // Set up the label renderer
        WhirlyKitLabelRenderer *labelRenderer = [[WhirlyKitLabelRenderer alloc] init];
        labelRenderer->labelInfo = labelInfo;
        labelRenderer->textureAtlasSize = 0;
        labelRenderer->coordAdapter = scene->getCoordAdapter();
        labelRenderer->labelRep = labelRep;
        labelRenderer->scene = scene;
        labelRenderer->screenGenId = scene->getScreenSpaceGeneratorID();
        
        [labelRenderer render];
        
        // Now harvest the results
        changes.insert(changes.end(), labelRenderer->changeRequests.begin(), labelRenderer->changeRequests.end());
        drawIDs = labelRep->drawIDs;
        texIDs = labelRep->texIDs;
    }
}

@end

