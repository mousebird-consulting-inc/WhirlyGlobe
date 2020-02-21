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
class BasicDrawableInstanceBuilder
{
public:
    EIGEN_MAKE_ALIGNED_OPERATOR_NEW;
    
    /// Construct empty
    BasicDrawableInstanceBuilder(const std::string &name);
    virtual ~BasicDrawableInstanceBuilder();
    
    /// Set the base draw ID and type
    void setMasterID(SimpleIdentity baseDrawID,BasicDrawableInstance::Style style);
    
    /// True to turn it on, false to turn it off
    void setOnOff(bool onOff);
    
    /// Set the time range for enable
    void setEnableTimeRange(TimeInterval inStartEnable,TimeInterval inEndEnable);
    
    /// Set the fade in and out
    void setFade(TimeInterval inFadeDown,TimeInterval inFadeUp);
        
    /// Set the viewer based visibility
    virtual void setViewerVisibility(double minViewerDist,double maxViewerDist,const Point3d &viewerCenter);
    
    /// Set what range we can see this drawable within.
    /// The units are in distance from the center of the globe and
    ///  the surface of the globe as at 1.0
    virtual void setVisibleRange(float minVis,float maxVis);
        
    /// Draw priority used for sorting
    virtual void setDrawPriority(unsigned int newPriority);
    
    /// Override color for the instanced drawable
    virtual void setColor(const RGBAColor &color);
    
    /// Override line width for the instanced drawable
    virtual void setLineWidth(float lineWidth);
        
    /// Resulting drawable wants the Z buffer for comparison
    virtual void setRequestZBuffer(bool val);
    
    /// Resulting drawable writes to the Z buffer
    virtual void setWriteZBuffer(bool val);
    
    // If set, we'll render this data where directed
    void setRenderTarget(SimpleIdentity newRenderTarget);
        
    /// Add a tweaker to this list to be run each frame
    void addTweaker(DrawableTweakerRef tweakRef);
    
    /// Set this when we're representing moving geometry model instances
    void setIsMoving(bool inMoving);
    
    // Time we start counting from for motion
    void setStartTime(TimeInterval inStartTime);

    /// Add a instance to the stack of instances this instance represents (mmm, noun overload)
    void addInstances(const std::vector<BasicDrawableInstance::SingleInstance> &insts);
    
    /// We can get the number of instances from a texture instead of being defined ahead of time
    /// It's easier to do this with a program than a copy, though
    void setInstanceTexSource(SimpleIdentity texID,SimpleIdentity srcProgID);
    
    /// Set the uniforms applied to the Program before rendering
    virtual void setUniforms(const SingleVertexAttributeSet &uniforms);
    
    /// Set a block of uniforms (Metal only, at the moment)
    virtual void setUniBlock(const BasicDrawable::UniformBlock &uniBlock);
    
    /// Set the texture ID for a specific slot.  You get this from the Texture object.
    virtual void setTexId(unsigned int which,SimpleIdentity inId);
    
    /// Set all the textures at once
    virtual void setTexIDs(const std::vector<SimpleIdentity> &texIDs);
    
    /// Set the relative offsets for texture usage.
    /// We use these to look up parts of a texture at a higher level
    virtual void setTexRelative(int which,int size,int borderTexel,int relLevel,int relX,int relY);
    
    /// Check for the given texture coordinate entry and add it if it's not there
    virtual void setupTexCoordEntry(int which,int numReserve);
    
    /// Set the shader program
    void setProgram(SimpleIdentity progID);
    
    /// Constructs the remaining pieces of the drawable and returns it
    /// Caller is responsible for deletion
    virtual BasicDrawableInstance *getDrawable() = 0;
    
    /// Return just the ID of the drawable being created
    /// This doesn't flush out the drawable in any way
    virtual SimpleIdentity getDrawableID();

protected:
    // Called by subclasses
    void Init();
    
    BasicDrawableInstance *drawInst;
};
    
typedef std::shared_ptr<BasicDrawableInstanceBuilder> BasicDrawableInstanceBuilderRef;
    
}
