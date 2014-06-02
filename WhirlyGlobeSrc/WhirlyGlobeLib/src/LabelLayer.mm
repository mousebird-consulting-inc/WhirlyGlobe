/*
 *  LabelLayer.mm
 *  WhirlyGlobeLib
 *
 *  Created by Steve Gifford on 2/7/11.
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

#import "LabelLayer.h"
#import "Scene.h"

using namespace Eigen;
using namespace WhirlyKit;

@implementation WhirlyKitLabelLayer
{
    SimpleIDSet labelIDs;
	WhirlyKitLayerThread * __weak layerThread;
	WhirlyKit::Scene *scene;
}

- (void)clear
{
    scene = NULL;
}

- (void)dealloc
{
    [self clear];
}

// We only do things when called on, so nothing much to do here
- (void)startWithThread:(WhirlyKitLayerThread *)inLayerThread scene:(WhirlyKit::Scene *)inScene;
{
    layerThread = inLayerThread;
    scene = inScene;
}

// Clean out our textures and drawables
- (void)shutdown
{
    [NSObject cancelPreviousPerformRequestsWithTarget:self];
    ChangeSet changes;
    
    LabelManager *labelManager = (LabelManager *)scene->getManager(kWKLabelManager);
    labelManager->removeLabels(labelIDs, changes);
    [layerThread addChangeRequests:changes];
    
    [self clear];
}

// Pass off label creation to a routine in our own thread
- (SimpleIdentity) addLabel:(NSString *)str loc:(WhirlyKit::GeoCoord)loc desc:(NSDictionary *)desc
{
    WhirlyKitSingleLabel *theLabel = [[WhirlyKitSingleLabel alloc] init];
    theLabel.text = str;
    [theLabel setLoc:loc];
    
    return [self addLabels:@[theLabel] desc:desc];
}

- (SimpleIdentity) addLabel:(WhirlyKitSingleLabel *)label
{
    return [self addLabels:[NSMutableArray arrayWithObject:label] desc:label.desc];
}

/// Add a group of labels
- (SimpleIdentity) addLabels:(NSArray *)labels desc:(NSDictionary *)desc
{
    if (!layerThread || !scene || [NSThread currentThread] != layerThread)
    {
        NSLog(@"LabelLayer: addLabel called before initialization or in wrong thread.  Dropping labels on floor.");
        return EmptyIdentity;
    }

    SimpleIdentity labelID = EmptyIdentity;
    ChangeSet changes;
    LabelManager *labelManager = (LabelManager *)scene->getManager(kWKLabelManager);
    if (labelManager)
    {
        labelID = labelManager->addLabels(labels, desc, changes);
        labelIDs.insert(labelID);
    }
    [layerThread addChangeRequests:changes];
    
    return labelID;
}

// Change how the label is displayed
- (void)changeLabel:(WhirlyKit::SimpleIdentity)labelID desc:(NSDictionary *)desc
{
    if (!layerThread || !scene || [NSThread currentThread] != layerThread)
    {
        NSLog(@"LabelLayer: changeLabel called before initialization or in wrong thread.  Dropping request on floor.");
        return;
    }
    
    ChangeSet changes;
    LabelManager *labelManager = (LabelManager *)scene->getManager(kWKLabelManager);
    if (labelManager)
    {
        SimpleIDSet::iterator it = labelIDs.find(labelID);
        if (it != labelIDs.end())
        {
            labelManager->changeLabel(labelID, desc, changes);
            labelIDs.erase(it);
        }
    }
    [layerThread addChangeRequests:changes];
}

// Set up the label to be removed in the layer thread
- (void) removeLabel:(WhirlyKit::SimpleIdentity)labelID
{
    if (!layerThread || !scene || [NSThread currentThread] != layerThread)
    {
        NSLog(@"LabelLayer: removeLabel called before initialization or in wrong thread.  Dropping request on floor.");
        return;
    }

    ChangeSet changes;
    LabelManager *labelManager = (LabelManager *)scene->getManager(kWKLabelManager);
    if (labelManager)
    {
        SimpleIDSet::iterator it = labelIDs.find(labelID);
        if (it != labelIDs.end())
        {
            SimpleIDSet theIDs;
            theIDs.insert(labelID);
            labelManager->removeLabels(theIDs, changes);
            labelIDs.erase(it);
        }
    }
    [layerThread addChangeRequests:changes];
}

@end
