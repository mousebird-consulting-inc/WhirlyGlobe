/*
 *  BasicDrawable.h
 *  WhirlyGlobeLib
 *
 *  Created by Steve Gifford on 2/1/11.
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
#import "BasicDrawable.h"
#import "WrapperGLES.h"
#import "ProgramGLES.h"
#import "SceneGLES.h"
#import "SceneRendererGLES.h"
#import "VertexAttributeGLES.h"
#import "DrawableGLES.h"

namespace WhirlyKit
{
    
/** OpenGL Version of the BasicDrawable.
  */
class BasicDrawableGLES : virtual public BasicDrawable, virtual public DrawableGLES
{
public:
    BasicDrawableGLES(const std::string &name);
    virtual ~BasicDrawableGLES();

    /// Set up local rendering structures (e.g. VBOs)
    virtual void setupForRenderer(const RenderSetupInfo *setupInfo);
    
    /// Clean up any rendering objects you may have (e.g. VBOs).
    virtual void teardownForRenderer(const RenderSetupInfo *setupInfo,Scene *scene);
    
    /// Some drawables have a pre-render phase that uses the GPU for calculation
    virtual void calculate(RendererFrameInfoGLES *frameInfo,Scene *scene) { };
    
    /// Called render-thread side to set up a VAO
    virtual GLuint setupVAO(ProgramGLES *prog);
    
    /// Fill this in to draw the basic drawable
    virtual void draw(RendererFrameInfoGLES *frameInfo,Scene *scene);
    
    /// Check if this has been set up and (more importantly) hasn't been torn down
    virtual bool isSetupInGL();
    
    /// Size of a single vertex used in creating an interleaved buffer.
    virtual unsigned int singleVertexSize();

    /// Add a single point to the GL Buffer.
    /// Override this to add your own data to interleaved vertex buffers.
    virtual void addPointToBuffer(unsigned char *basePtr,int which,const Point3d *center);

public:
    // Unprocessed data arrays
    std::vector<Eigen::Vector3f> points;
    std::vector<Triangle> tris;

    // Attribute that should be applied to the given program index if using VAOs
    class VertAttrDefault
    {
    public:
        VertAttrDefault(unsigned int progAttrIndex,const VertexAttribute &attr)
        : progAttrIndex(progAttrIndex), attr((VertexAttributeGLES &)attr) { }
        GLuint progAttrIndex;
        VertexAttributeGLES attr;
    };
    std::vector<VertAttrDefault> vertArrayDefaults;

    bool isSetupGL;  // Is setup to draw with GL (needed by the instances)
    bool usingBuffers;  // If set, we've downloaded the buffers already

    // Size for a single vertex w/ all its data.  Used by shared buffer
    int vertexSize;
    GLuint pointBuffer,triBuffer,sharedBuffer;
    GLuint vertArrayObj;
};
    
}
