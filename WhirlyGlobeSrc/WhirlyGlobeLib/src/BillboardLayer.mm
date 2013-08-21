/*
 *  BillboardLayer.mm
 *  WhirlyGlobeLib
 *
 *  Created by Steve Gifford on 5/27/12.
 *  Copyright 2011-2013 mousebird consulting. All rights reserved.
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

#import "BillboardLayer.h"
#import "NSDictionary+Stuff.h"
#import "UIColor+Stuff.h"

using namespace Eigen;
using namespace WhirlyKit;


@implementation WhirlyKitBillboardLayer
{
    /// Layer thread this belongs to
    WhirlyKitLayerThread * __weak layerThread;
    /// Scene the marker layer is modifying
    WhirlyKit::Scene *scene;
    SimpleIdentity billboardProgram;

    SimpleIDSet billIDs;
}

- (void)clear
{
    billIDs.clear();
}

- (void)startWithThread:(WhirlyKitLayerThread *)inLayerThread scene:(Scene *)inScene
{
    layerThread = inLayerThread;
    scene = inScene;
    
    OpenGLES2Program *prog = BuildBillboardProgram();
    if (prog)
    {
        scene->addProgram("Billboard Shader",prog);
        billboardProgram = prog->getId();
    }
}

- (void)shutdown
{
    ChangeSet changes;

    BillboardManager *billManager = (BillboardManager *)scene->getManager(kWKBillboardManager);
    
    if (billManager)
        billManager->removeBillboards(billIDs, changes);
    
    [layerThread addChangeRequests:changes];
    
    [self clear];
}


- (void)runAddBillboards:(WhirlyKitBillboardInfo *)billboardInfo
{
}

- (void)runRemoveBillboards:(NSNumber *)num
{
}

- (SimpleIdentity) addBillboards:(NSArray *)billboards desc:(NSDictionary *)desc
{
    if (!layerThread || !scene || ([NSThread currentThread] != layerThread))
    {
        NSLog(@"BillboardLayer: Called before initialized or in wrong thread.  Dropping billboards.");
        return EmptyIdentity;
    }
    
    SimpleIdentity billID = EmptyIdentity;
    ChangeSet changes;
    BillboardManager *billManager = (BillboardManager *)scene->getManager(kWKBillboardManager);
    
    if (billManager)
    {
        billID = billManager->addBillboards(billboards, desc, billboardProgram, changes);
        if (billID)
            billIDs.insert(billID);
    }
    [layerThread addChangeRequests:changes];
    
    return billID;
}

- (SimpleIdentity) replaceBillboards:(SimpleIdentity)oldBillID withBillboards:(NSArray *)billboards desc:(NSDictionary *)desc
{
    if (!layerThread || !scene || ([NSThread currentThread] != layerThread))
    {
        NSLog(@"BillboardLayer: Called before initialized or in wrong thread.  Dropping billboards.");
        return EmptyIdentity;
    }

    SimpleIdentity billID = EmptyIdentity;
    ChangeSet changes;
    BillboardManager *billManager = (BillboardManager *)scene->getManager(kWKBillboardManager);
    
    if (billManager)
    {
        SimpleIDSet::iterator it = billIDs.find(oldBillID);
        if (it != billIDs.end())
        {
            SimpleIDSet theIDs;
            theIDs.insert(oldBillID);
            billManager->removeBillboards(theIDs, changes);
            billIDs.erase(it);
        }
        billID = billManager->addBillboards(billboards, desc, billboardProgram, changes);
        if (billID)
            billIDs.insert(billID);
    }
    [layerThread addChangeRequests:changes];
    
    return billID;
}

- (void) removeBillboards:(SimpleIdentity)billID
{
    if (!layerThread || !scene || ([NSThread currentThread] != layerThread))
    {
        NSLog(@"BillboardLayer: Called before initialized or in wrong thread.  Dropping billboards.");
        return;
    }
    
    ChangeSet changes;
    BillboardManager *billManager = (BillboardManager *)scene->getManager(kWKBillboardManager);

    if (billManager)
    {
        SimpleIDSet::iterator it = billIDs.find(billID);
        if (it != billIDs.end())
        {
            SimpleIDSet theIDs;
            theIDs.insert(billID);
            billManager->removeBillboards(theIDs, changes);
            billIDs.erase(it);
        }
    }
        
    [layerThread addChangeRequests:changes];
}

@end
