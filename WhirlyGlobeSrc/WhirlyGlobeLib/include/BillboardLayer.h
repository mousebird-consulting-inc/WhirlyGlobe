/*
 *  BillboardLayer.h
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

#import <math.h>
#import <set>
#import <map>
#import "Identifiable.h"
#import "Drawable.h"
#import "DataLayer.h"
#import "LayerThread.h"
#import "BillboardDrawable.h"
#import "BillboardManager.h"

/** The billboard layer manages the construction and destruction of
    billboards for the renderer.  Billboards are 3D objects that turn
    to face the viewer.
 */
@interface WhirlyKitBillboardLayer : NSObject<WhirlyKitLayer>

/// Add billboards for display
- (WhirlyKit::SimpleIdentity) addBillboards:(NSArray *)billboards desc:(NSDictionary *)desc;

/// Remove a group of billboards named by the given ID
- (void) removeBillboards:(WhirlyKit::SimpleIdentity)billId;

/// Replace a group of billboards with the ones given
- (WhirlyKit::SimpleIdentity) replaceBillboards:(WhirlyKit::SimpleIdentity)oldBillID withBillboards:(NSArray *)billboards desc:(NSDictionary *)desc;

@end
