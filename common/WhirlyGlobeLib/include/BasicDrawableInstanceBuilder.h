/*
 *  BasicDrawableInstanceBuilder.h
 *  WhirlyGlobeLib
 *
 *  Created by Steve Gifford on 5/9/19.
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

#import <vector>
#import <set>
#import <map>
#import "Identifiable.h"
#import "WhirlyVector.h"
#import "GlobeView.h"
#import "BasicDrawableBuilder.h"
#import "BasicDrawableInstance.h"

namespace WhirlyKit
{
    
/** A Basic Drawable Instance replicates a basic drawable while
 tweaking some of the fields.  This is good for using the same
 geometry to implement vectors of multiple colors and line widths.
 */
class BasicDrawableInstanceBuilder : public BasicDrawableBuilder
{
public:
    EIGEN_MAKE_ALIGNED_OPERATOR_NEW;
    
    /// Construct empty
    BasicDrawableInstanceBuilder(const std::string &name,SimpleIdentity masterID,BasicDrawableInstance::Style instanceStyle);
    
    /// Set the shader program
    void setProgram(SimpleIdentity progID);
    
    /// Set the drawable we're instancing
    void setMaster(BasicDrawableRef draw) { basicDraw = draw; }
    
    /// Set this when we're representing moving geometry model instances
    void setIsMoving(bool inMoving) { moving = inMoving; }
    
    // Time we start counting from for motion
    void setStartTime(TimeInterval inStartTime) { startTime = inStartTime; }
    
    /// Add a instance to the stack of instances this instance represents (mmm, noun overload)
    void addInstances(const std::vector<BasicDrawableInstance::SingleInstance> &insts);
    
    // If set, we'll render this data where directed
    void setRenderTarget(SimpleIdentity newRenderTarget);
    
    /// Set the texture ID for a specific slot.  You get this from the Texture object.
    virtual void setTexId(unsigned int which,SimpleIdentity inId);
    
    /// Set all the textures at once
    virtual void setTexIDs(const std::vector<SimpleIdentity> &texIDs);
    
    /// Set the relative offsets for texture usage.
    /// We use these to look up parts of a texture at a higher level
    virtual void setTexRelative(int which,int size,int borderTexel,int relLevel,int relX,int relY);
    
    /// Check for the given texture coordinate entry and add it if it's not there
    virtual void setupTexCoordEntry(int which,int numReserve);
    
protected:
    BasicDrawableInstance::Style instanceStyle;
    SimpleIdentity masterID;
    BasicDrawableRef basicDraw;
    bool hasDrawPriority;
    bool hasColor;
    bool hasLineWidth;
    int numInstances;
    
    int centerSize,matSize,colorInstSize,colorSize,instSize,modelDirSize;
    TimeInterval startTime;
    bool moving;
    
    // If set, we'll instance this one multiple times
    std::vector<BasicDrawableInstance::SingleInstance> instances;
};
    
}
