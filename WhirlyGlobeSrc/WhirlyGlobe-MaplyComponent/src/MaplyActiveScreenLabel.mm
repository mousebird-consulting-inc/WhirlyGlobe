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
#import "LabelManager.h"

using namespace WhirlyKit;

@implementation MaplyActiveScreenLabel
{
    MaplyScreenLabel *screenLabel;
    NSDictionary *desc;
    UIImage *iconImage;
    SimpleIdentity iconTexId;
    SimpleIDSet labelIDs;
}

- (void)setScreenLabel:(MaplyScreenLabel *)newLabel desc:(NSDictionary *)inDesc
{
    if (dispatchQueue)
    {
        dispatch_async(dispatchQueue,
                       ^{
                           @synchronized(self)
                           {
                               screenLabel = newLabel;
                               desc = inDesc;
                               [self update];
                           }
                       }
                       );
    } else {
        @synchronized(self)
        {
            screenLabel = newLabel;
            desc = inDesc;
            [self update];
        }
    }
    
}

// Flush out changes to the scene
- (void)update
{
    ChangeSet changes;
    
    LabelManager *labelManager = (LabelManager *)scene->getManager(kWKLabelManager);
    if (labelManager)
        labelManager->removeLabels(labelIDs,changes);
    labelIDs.clear();
    
    SimpleIdentity removeTexId = EmptyIdentity;
    if (screenLabel)
    {
        // Possibly get rid of the image too
        if (iconImage && screenLabel.iconImage != iconImage)
        {
            removeTexId = iconTexId;
            iconTexId = EmptyIdentity;
            iconImage = nil;
        }

        // And make a new one
        if (screenLabel.iconImage != iconImage)
        {
            Texture *tex = new Texture("Active ScreenLabel",screenLabel.iconImage);
            iconTexId = tex->getId();
            iconImage = screenLabel.iconImage;
            changes.push_back(new AddTextureReq(tex));            
        }
        
        WhirlyKitSingleLabel *wgLabel = [[WhirlyKitSingleLabel alloc] init];
        NSMutableDictionary *locDesc = [NSMutableDictionary dictionary];
        wgLabel.loc = GeoCoord(screenLabel.loc.x,screenLabel.loc.y);
        wgLabel.text = screenLabel.text;
        wgLabel.iconTexture = iconTexId;
        wgLabel.iconSize = screenLabel.iconSize;
        if (screenLabel.size.width > 0.0)
            [locDesc setObject:[NSNumber numberWithFloat:screenLabel.size.width] forKey:@"width"];
        if (screenLabel.size.height > 0.0)
            [locDesc setObject:[NSNumber numberWithFloat:screenLabel.size.height] forKey:@"height"];
        if (screenLabel.color)
            [locDesc setObject:screenLabel.color forKey:@"textColor"];
        if (screenLabel.layoutImportance != MAXFLOAT)
        {
            [locDesc setObject:@(YES) forKey:@"layout"];
            [locDesc setObject:@(screenLabel.layoutImportance) forKey:@"layoutImportance"];
            [locDesc setObject:@(screenLabel.layoutPlacement) forKey:@"layoutPlacement"];
        }
        wgLabel.screenOffset = screenLabel.offset;
        if (screenLabel.selectable)
        {
            wgLabel.isSelectable = true;
            wgLabel.selectID = Identifiable::genId();
        }
        wgLabel.isSelectable = false;
        if ([locDesc count] > 0)
            wgLabel.desc = locDesc;
        
        if (labelManager)
        {
            SimpleIdentity labelID = labelManager->addLabels(@[wgLabel], desc, changes);
            if (labelID != EmptyIdentity)
                labelIDs.insert(labelID);
        }
    }
    
    scene->addChangeRequests(changes);
}

- (void)shutdown
{
    @synchronized(self)
    {
        screenLabel = nil;
        [self update];
    }
}


@end

