/*
 *  OverlapHelper.h
 *  WhirlyGlobeLib
 *
 *  Created by Steve Gifford on 9/28/15.
 *  Copyright 2011-2015 mousebird consulting. All rights reserved.
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
#import "BasicDrawable.h"
#import "Scene.h"
#import "SceneRendererES.h"
#import "GlobeLayerViewWatcher.h"
#import "ScreenSpaceBuilder.h"
#import "SelectionManager.h"

namespace WhirlyKit
{
// We use this to avoid overlapping labels
class OverlapHelper
{
public:
    OverlapHelper(const Mbr &mbr,int sizeX,int sizeY);
    
    // Try to add an object.  Might fail (kind of the whole point).
    bool addObject(const std::vector<Point2d> &pts);
    
protected:
    // Object and its bounds
    class BoundedObject
    {
    public:
        ~BoundedObject() { }
        std::vector<Point2d> pts;
    };
    
    Mbr mbr;
    std::vector<BoundedObject> objects;
    int sizeX,sizeY;
    Point2f cellSize;
    std::vector<std::vector<int> > grid;
};
    
}
