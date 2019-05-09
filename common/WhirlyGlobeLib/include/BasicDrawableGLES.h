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
#import "Drawable.h"

namespace WhirlyKit
{

/** OpenGL Version of the BasicDrawable.
  */
class BasicDrawableGLES : public BasicDrawable
{
public:
    /// Set up local rendering structures (e.g. VBOs)
    virtual void setupForRenderer(RenderSetupInfo *setupInfo);
    
    /// Clean up any rendering objects you may have (e.g. VBOs).
    virtual void teardownForRenderer(RenderSetupInfo *setupInfo);
    
    /// Called render-thread side to set up a VAO
    virtual GLuint setupVAO(OpenGLES2Program *prog);
    
    /// Fill this in to draw the basic drawable
    virtual void draw(RendererFrameInfo *frameInfo,Scene *scene);
    
    /// Check if this has been set up and (more importantly) hasn't been torn down
    virtual bool isSetupInGL();
    
    /// Geometry type.  Probably triangles.
    virtual GLenum getType() const;
    
public:
    // Used by subclasses to do the standard init
    void basicDrawableInit();
    /// Check for the given texture coordinate entry and add it if it's not there
    virtual void setupTexCoordEntry(int which,int numReserve);
    
    // Attributes associated with each vertex, some standard some not
    std::vector<VertexAttribute *> vertexAttributes;
    // Entries for the standard attributes we create on startup
    int colorEntry,normalEntry;
    // Set up the standard vertex attributes we use
    virtual void setupStandardAttributes(int numReserve=0);
    
    bool isSetupGL;  // Is setup to draw with GL (needed by the instances)
    bool on;  // If set, draw.  If not, not
    TimeInterval startEnable,endEnable;
    SimpleIdentity programId;    // Program to use for rendering
    bool usingBuffers;  // If set, we've downloaded the buffers already
    TimeInterval fadeUp,fadeDown;  // Controls fade in and fade out
    unsigned int drawPriority;  // Used to sort drawables
    float drawOffset;    // Number of units of Z buffer resolution to offset upward (by the normal)
    bool isAlpha;  // Set if we want to be drawn last
    Mbr localMbr;  // Extents in a local space, if we're not using lat/lon/radius
    GLenum type;  // Primitive(s) type
    std::vector<TexInfo> texInfo;
    RGBAColor color;
    float minVisible,maxVisible;
    float minVisibleFadeBand,maxVisibleFadeBand;
    double minViewerDist,maxViewerDist;
    Point3d viewerCenter;
    float lineWidth;
    // For zBufferOffDefault mode we'll sort this to the end
    bool requestZBuffer;
    // When this is set we'll update the z buffer with our geometry.
    bool writeZBuffer;
    
    // We'll nuke the data arrays when we hand over the data to GL
    unsigned int numPoints, numTris;
    std::vector<Eigen::Vector3f> points;
    std::vector<Triangle> tris;
    SimpleIdentity renderTargetID;
    bool hasOverrideColor;  // If set, we've changed the default color
    
    bool hasMatrix;
    // If the drawable has a matrix, we'll transform by that before drawing
    Eigen::Matrix4d mat;
    // Uniforms to apply to shader
    SingleVertexAttributeSet uniforms;
    
    // Attribute that should be applied to the given program index if using VAOs
    class VertAttrDefault
    {
    public:
        VertAttrDefault(GLuint progAttrIndex,const VertexAttribute &attr)
        : progAttrIndex(progAttrIndex), attr(attr) { }
        GLuint progAttrIndex;
        VertexAttribute attr;
    };
    
    // Size for a single vertex w/ all its data.  Used by shared buffer
    int vertexSize;
    GLuint pointBuffer,triBuffer,sharedBuffer;
    GLuint vertArrayObj;
    std::vector<VertAttrDefault> vertArrayDefaults;
    
    // If set the geometry is already in OpenGL clip coordinates, so no transform
    bool clipCoords;
};
    
}
