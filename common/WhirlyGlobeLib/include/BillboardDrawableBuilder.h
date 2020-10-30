/*
 *  BillboardDrawable.h
 *  WhirlyGlobeLib
 *
 *  Created by Steve Gifford on 5/27/13.
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

#import "Program.h"
#import "BasicDrawableBuilder.h"

namespace WhirlyKit
{
    
/** Billboards are just a little extra information and
    a shader.  They emit basic drawables.
  */
class BillboardDrawableBuilder : virtual public BasicDrawableBuilder
{
public:
    virtual void Init() override;
    
    /// Each vertex has an offset in 3-space
    void addOffset(const Point3f &offset);
    void addOffset(const Point3d &offset);
    
    /// Set whether this is eye or ground mode
    void setGroundMode(bool newVal);
    
protected:
    bool groundMode;
    int offsetIndex;
};
    
typedef std::shared_ptr<BillboardDrawableBuilder> BillboardDrawableBuilderRef;

}
