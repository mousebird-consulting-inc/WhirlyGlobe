/*
 *  BasicDrawableBuilderMTL.h
 *  WhirlyGlobeLib
 *
 *  Created by Steve Gifford on 5/16/19.
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

#import "BasicDrawableBuilder.h"
#import "BasicDrawableMTL.h"

namespace WhirlyKit
{

/** Metal version of BasicDrawable Builder.
 */
class BasicDrawableBuilderMTL : virtual public BasicDrawableBuilder
{
public:
    /// Construct empty
    BasicDrawableBuilderMTL(const std::string &name);
    virtual ~BasicDrawableBuilderMTL();
    
    /// Add a new vertex related attribute.  Need a data type and the name the shader refers to
    ///  it by.  The index returned is how you will access it.
    virtual int addAttribute(BDAttributeDataType dataType,StringIdentity nameID,int numThings = -1);
    
    /// Override this to set the internal Metal buffer
    virtual void setupTexCoordEntry(int which,int numReserve=0);
    
    /// Fill out and return the drawable
    virtual BasicDrawable *getDrawable();
    
    virtual void setupStandardAttributes(int numReserve=0);
    
protected:
    bool drawableGotten;
};

}
