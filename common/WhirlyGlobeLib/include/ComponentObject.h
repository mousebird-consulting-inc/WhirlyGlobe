/*
 *  ComponentObject.h
 *  WhirlyGlobeLib
 *
 *  Created by Steve Gifford on 2/14/19.
 *  Copyright 2011-2019 mousebird consulting
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

#import "WhirlyVector.h"
#import "Scene.h"
#import "GlobeMath.h"
#import "VectorData.h"

namespace WhirlyKit
{

/* Component Object is a higher level container for the various
    IDs and objects associated with a single set of geometry (or whatever).
 */
class ComponentObject : public Identifiable
{
public:
    ComponentObject();
    virtual ~ComponentObject();
    
    SimpleIDSet markerIDs;
    SimpleIDSet labelIDs;
    SimpleIDSet vectorIDs;
    SimpleIDSet wideVectorIDs;
    SimpleIDSet shapeIDs;
    SimpleIDSet chunkIDs;
    SimpleIDSet loftIDs;
    SimpleIDSet billIDs;
    SimpleIDSet geomIDs;
    SimpleIDSet partSysIDs;
    SimpleIDSet selectIDs;
    SimpleIDSet drawStringIDs;
    // Note: Move vectors in here as well
    Point2d vectorOffset;
    bool isSelectable;
    bool enable;
    bool underConstruction;
    
    // Empty out references
    void clear();
};
    
typedef std::shared_ptr<ComponentObject> ComponentObjectRef;
    
}
