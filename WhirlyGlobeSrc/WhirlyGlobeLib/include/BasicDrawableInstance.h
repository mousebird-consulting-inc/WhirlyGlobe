/*
 *  Drawable.h
 *  WhirlyGlobeLib
 *
 *  Created by Steve Gifford on 2/1/11.
 *  Copyright 2011-2015 mousebird consulting
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

#import <OpenGLES/ES1/gl.h>
#import <OpenGLES/ES1/glext.h>
#import <OpenGLES/ES2/gl.h>
#import <OpenGLES/ES2/glext.h>
#import <OpenGLES/ES3/gl.h>
#import <OpenGLES/ES3/glext.h>

#import <vector>
#import <set>
#import <map>
#import <boost/shared_ptr.hpp>
#import <boost/pointer_cast.hpp>
#import "Identifiable.h"
#import "WhirlyVector.h"
#import "GlobeView.h"
#import "BasicDrawable.h"

namespace WhirlyKit
{

/** A Basic Drawable Instance replicates a basic drawable while
 tweaking some of the fields.  This is good for using the same
 geometry to implement vectors of multiple colors and line widths.
 */
class BasicDrawableInstance : public Drawable
{
public:
    /// Construct emtpy
    BasicDrawableInstance(const std::string &name,SimpleIdentity masterID);
    
    /// Return the local MBR, if we're working in a non-geo coordinate system
    virtual Mbr getLocalMbr() const;
    
    /// We use this to sort drawables
    virtual unsigned int getDrawPriority() const;
    
    /// For OpenGLES2, this is the program to use to render this drawable.
    virtual SimpleIdentity getProgram() const;
    
    /// We're allowed to turn drawables off completely
    virtual bool isOn(WhirlyKitRendererFrameInfo *frameInfo) const;
    
    /// Do any OpenGL initialization you may want.
    virtual void setupGL(WhirlyKitGLSetupInfo *setupInfo,OpenGLMemManager *memManager);
    
    /// Clean up any OpenGL objects you may have (e.g. VBOs).
    virtual void teardownGL(OpenGLMemManager *memManage);
    
    /// Set up the vertex array object
    GLuint setupVAO(OpenGLES2Program *prog);
    
    /// Return the type (or an approximation thereof).  We use this for sorting.
    virtual GLenum getType() const;
    
    /// Return true if the drawable has alpha.  These will be sorted last.
    virtual bool hasAlpha(WhirlyKitRendererFrameInfo *frameInfo) const;
    
    /// We can ask to use the z buffer
    virtual void setRequestZBuffer(bool val) { requestZBuffer = val; }
    
    /// Set the z buffer mode for this drawable
    virtual void setWriteZBuffer(bool val) { writeZBuffer = val; }
    
    virtual bool getRequestZBuffer() const { return requestZBuffer; }
    virtual bool getWriteZbuffer() const { return writeZBuffer; }
    
    /// Update anything associated with the renderer.  Probably renderUntil.
    virtual void updateRenderer(WhirlyKitSceneRendererES *renderer);
    
    /// Fill this in to draw the basic drawable
    virtual void draw(WhirlyKitRendererFrameInfo *frameInfo,Scene *scene);
    
    /// Set the enable on/off
    void setEnable(bool newEnable) { enable = newEnable; }
    
    /// Set the time range for enable
    void setEnableTimeRange(NSTimeInterval inStartEnable,NSTimeInterval inEndEnable) { startEnable = inStartEnable;  endEnable = inEndEnable; }
    
    /// Set the min/max visible range
    void setVisibleRange(float inMinVis,float inMaxVis) { hasMinVis = true;  minVis = inMinVis;  hasMaxVis = true;  maxVis = inMaxVis; }
    
    /// Set the color
    void setColor(RGBAColor inColor) { hasColor = true; color = inColor; }
    
    /// Set the draw priority
    void setDrawPriority(int newPriority) { hasDrawPriority = true;  drawPriority = newPriority; }
    
    /// Set the line width
    void setLineWidth(int newLineWidth) { hasLineWidth = true;  lineWidth = newLineWidth; }
    
    /// Return the ID of the basic drawable we're instancing
    SimpleIdentity getMasterID() { return masterID; }
    
    /// Set the drawable we're instancing
    void setMaster(BasicDrawableRef draw) { basicDraw = draw; }
    
    /// Return the translation matrix if there is one
    const Eigen::Matrix4d *getMatrix() const;
    
    // Single geometry instance when we're doing multiple instance
    class SingleInstance
    {
    public:
        SingleInstance() : colorOverride(false) { }
        
        bool colorOverride;
        RGBAColor color;
        Eigen::Matrix4d mat;
    };
    
    /// Add a instance to the stack of instances this instance represents (mmm, noun overload)
    void addInstances(const std::vector<SingleInstance> &insts);
    
protected:
    bool requestZBuffer,writeZBuffer;
    SimpleIdentity masterID;
    BasicDrawableRef basicDraw;
    bool enable;
    NSTimeInterval startEnable,endEnable;
    bool hasDrawPriority;
    int drawPriority;
    bool hasColor;
    RGBAColor color;
    bool hasLineWidth;
    float lineWidth;
    bool hasMinVis;
    float minVis;
    bool hasMaxVis;
    float maxVis;
    int numInstances;
    GLuint instBuffer;
    GLuint vertArrayObj;
    
    // If set, we'll instance this one multiple times
    std::vector<SingleInstance> instances;
    // While rendering, which instance we're rendering
    int whichInstance;
};

/// Reference counted version of BasicDrawableInstance
typedef boost::shared_ptr<BasicDrawableInstance> BasicDrawableInstanceRef;

}
