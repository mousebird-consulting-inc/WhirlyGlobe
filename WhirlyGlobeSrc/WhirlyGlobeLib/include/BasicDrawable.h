/*
 *  BasicDrawable.h
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

#import "glwrapper.h"
#import <vector>
#import <set>
#import <map>
#import "Identifiable.h"
#import "WhirlyVector.h"
#import "GlobeView.h"
#import "Drawable.h"

namespace WhirlyKit
{

/** The Basic Drawable is the one we use the most.  It's
 a general purpose container for static geometry which
 may or may not be textured.
 */
class BasicDrawable : public Drawable
{
    friend class BigDrawableAtlas;
    friend class BasicDrawableInstance;

protected:
    // Used by subclasses
    BasicDrawable() { }
    
public:
    /// Construct empty
    BasicDrawable(const std::string &name);
    /// Construct with some idea how big things are.
    /// You can violate this, but it will reserve space
    BasicDrawable(const std::string &name, unsigned int numVert,unsigned int numTri);
    virtual ~BasicDrawable();
    
    /// For OpenGLES2, this is the program to use to render this drawable.
    virtual SimpleIdentity getProgram() const;
    
    /// For OpenGLES2, you can set the program to use in rendering
    void setProgram(SimpleIdentity progId);
    
    /// Set up the VBOs
    virtual void setupGL(WhirlyKitGLSetupInfo *setupInfo,OpenGLMemManager *memManager);
    
    /// This version can use a shared buffer instead of allocating its own
    virtual void setupGL(WhirlyKitGLSetupInfo *setupInfo,OpenGLMemManager *memManager,GLuint sharedBuf,GLuint sharedBufOffset);
    
    /// Clean up the VBOs
    virtual void teardownGL(OpenGLMemManager *memManage);
    
    /// Size of a single vertex used in creating an interleaved buffer.
    virtual GLuint singleVertexSize();
    
    /// Called render-thread side to set up a VAO
    virtual GLuint setupVAO(OpenGLES2Program *prog);
    
    /// Fill this in to draw the basic drawable
    virtual void draw(WhirlyKit::RendererFrameInfo *frameInfo,Scene *scene);
    
    /// Draw priority
    virtual unsigned int getDrawPriority() const;
    
    /// We use the on/off flag as well as a visibility check
    virtual bool isOn(WhirlyKit::RendererFrameInfo *frameInfo) const;
    /// True to turn it on, false to turn it off
    void setOnOff(bool onOff);
    
    /// Set the time range for enable
    void setEnableTimeRange(TimeInterval inStartEnable,TimeInterval inEndEnable) { startEnable = inStartEnable;  endEnable = inEndEnable; }
    
    /// Used for alpha sorting
    virtual bool hasAlpha(WhirlyKit::RendererFrameInfo *frameInfo) const;
    /// Set the alpha sorting on or off
    void setAlpha(bool onOff);
    
    /// Extents used for display culling in local coordinates, if we're using them
    virtual Mbr getLocalMbr() const;
    
    /// Set local extents
    void setLocalMbr(Mbr mbr);
    
    /// Simple triangle.  Can obviously only have 2^16 vertices
    class Triangle
    {
    public:
        Triangle() { }
        /// Construct with vertex IDs
        Triangle(unsigned short v0,unsigned short v1,unsigned short v2) { verts[0] = v0;  verts[1] = v1;  verts[2] = v2; }
        unsigned short verts[3];
    };
    
    /// Set the draw priority.  We sort by draw priority before rendering.
    virtual void setDrawPriority(unsigned int newPriority);
    virtual unsigned int getDrawPriority();
    
    /// Set the draw offset.  This is an integer offset from the base terrain.
    /// Geometry is moved upward by a certain number of units.
    virtual void setDrawOffset(float newOffset);
    virtual float getDrawOffset();
    
    /// Set the geometry type.  Probably triangles.
    virtual void setType(GLenum inType);
    virtual GLenum getType() const;
    
    /// Set the texture ID for a specific slot.  You get this from the Texture object.
    virtual void setTexId(unsigned int which,SimpleIdentity inId);
    
    /// Set all the textures at once
    virtual void setTexIDs(const std::vector<SimpleIdentity> &texIDs);
    
    /// Return the default color
    virtual RGBAColor getColor() const;
    
    /// Set the color as an RGB color
    virtual void setColor(RGBAColor inColor);
    
    /// Set the color as an array.
    virtual void setColor(unsigned char inColor[]);
    
    /// Set what range we can see this drawable within.
    /// The units are in distance from the center of the globe and
    ///  the surface of the globe as at 1.0
    virtual void setVisibleRange(float minVis,float maxVis,float minVisBand=0.0,float maxVisBand=0.0);
    /// Retrieve the visible range, just min and max
    virtual void getVisibleRange(float &minVis,float &maxVis);
    
    /// Retrieve the visible range, including bands
    virtual void getVisibleRange(float &minVis,float &maxVis,float &minVisBand,float &maxVisBand);

    /// Set the viewer based visibility
    virtual void setViewerVisibility(double minViewerDist,double maxViewerDist,const Point3d &viewerCenter);
    /// Retrieve the viewer based visibility
    virtual void getViewerVisibility(double &minViewerDist,double &maxViewerDist,Point3d &viewerCenter);
    
    /// Set the fade in and out
    virtual void setFade(TimeInterval inFadeDown,TimeInterval inFadeUp);
    
    /// Set the line width (if using lines)
    virtual void setLineWidth(float inWidth);
    
    /// Return the line width (1.0 is the default)
    virtual float getLineWidth();
    
    /// We can ask to use the z buffer
    virtual void setRequestZBuffer(bool val);
    
    /// Check if the force Z buffer on mode is on
    virtual bool getRequestZBuffer() const;
    
    /// Set the z buffer mode for this drawable
    virtual void setWriteZBuffer(bool val);
    
    /// Check if we want to write to the z buffer
    virtual bool getWriteZbuffer() const;
    
    /// Add a point when building up geometry.  Returns the index.
    virtual unsigned int addPoint(const Point3f &pt);
    virtual unsigned int addPoint(const Point3d &pt);
    
    /// Return a given point
    virtual Point3f getPoint(int which);
    
    /// Add a texture coordinate. -1 means we add the same
    ///  texture coordinate to all the available texture coordinate sets
    virtual void addTexCoord(int which,TexCoord coord);
    
    /// Add a color
    virtual void addColor(RGBAColor color);
    
    /// Add a normal
    virtual void addNormal(const Point3f &norm);
    virtual void addNormal(const Point3d &norm);
    
    /// Decide if the given list of vertex attributes is the same as the one we have
    bool compareVertexAttributes(const SingleVertexAttributeSet &attrs);
    
    /// Set up the required vertex attribute arrays from the given list
    void setVertexAttributes(const SingleVertexAttributeInfoSet &attrs);
    
    /// Add the given vertex attributes for the given vertex
    void addVertexAttributes(const SingleVertexAttributeSet &attrs);
    
    /// Add a 2D vector to the given attribute array
    virtual void addAttributeValue(int attrId,const Eigen::Vector2f &vec);
    
    /// Add a 3D vector to the given attribute array
    virtual void addAttributeValue(int attrId,const Eigen::Vector3f &vec);
    
    /// Add a 4D vector to the given attribute array
    virtual void addAttributeValue(int attrId,const Eigen::Vector4f &vec);
    
    /// Add a 4 component char array to the given attribute array
    virtual void addAttributeValue(int attrId,const RGBAColor &color);
    
    /// Add a float to the given attribute array
    virtual void addAttributeValue(int attrId,float val);
    
    /// Add a triangle.  Should point to the vertex IDs.
    virtual void addTriangle(Triangle tri);
    
    /// Return the texture ID
    virtual SimpleIdentity getTexId(unsigned int which);
    
    /// Texture ID and pointer to vertex attribute info
    class TexInfo
    {
    public:
        TexInfo() : texId(EmptyIdentity), texCoordEntry(0) { }
        /// Texture ID within the scene
        SimpleIdentity texId;
        /// Vertex attribute entry for this set of texture coordinates
        int texCoordEntry;
    };
    
    /// Return the current texture info
    virtual const std::vector<TexInfo> &getTexInfo() { return texInfo; }
    
    /// Add a new vertex related attribute.  Need a data type and the name the shader refers to
    ///  it by.  The index returned is how you will access it.
    virtual int addAttribute(BDAttributeDataType dataType,const std::string &name,int numThings = -1);
    
    /// Return the number of points added so far
    virtual unsigned int getNumPoints() const;
    
    /// Return the number of triangles added so far
    virtual unsigned int getNumTris() const;
    
    /// Reserve the extra space for points
    virtual void reserveNumPoints(int numPoints);
    
    /// Reserve the extra space for triangles
    virtual void reserveNumTris(int numTris);
    
    /// Reserve extra space for texture coordinates
    virtual void reserveNumTexCoords(unsigned int which,int numCoords);
    
    /// Reserve extra space for normals
    virtual void reserveNumNorms(int numNorms);
    
    /// Reserve extra space for colors
    virtual void reserveNumColors(int numColors);
    
    /// Set the active transform matrix
    virtual void setMatrix(const Eigen::Matrix4d *inMat);
    
    /// Return the active transform matrix, if we have one
    virtual const Eigen::Matrix4d *getMatrix() const;
    
    /// Run the texture and texture coordinates based on a SubTexture
    virtual void applySubTexture(int which,SubTexture subTex,int startingAt=0);
    
    /// Update fade up/down times in renderer (i.e. keep the renderer rendering)
    virtual void updateRenderer(WhirlyKit::SceneRendererES *renderer);
    
    /// Copy the vertex data into an NSData object and return it
    virtual RawDataRef asData(bool dupStart,bool dupEnd);
    
    /// Copy vertex and element data into appropriate NSData objects
    virtual void asVertexAndElementData(MutableRawDataRef &vertData,MutableRawDataRef &elementData,int singleElementSize,const Point3d *center);
    
    /// Assuming this is a set of triangles, convert to a triangle strip
    //    virtual void convertToTriStrip();
    
    /// Return the vertex attributes for reference
    virtual const std::vector<VertexAttribute *> &getVertexAttributes();
    
public:
    // Used by subclasses to do the standard init
    void basicDrawableInit();
    /// Check for the given texture coordinate entry and add it if it's not there
    virtual void setupTexCoordEntry(int which,int numReserve);
    /// Draw routine for OpenGL 2.0
    virtual void drawOGL2(WhirlyKit::RendererFrameInfo *frameInfo,Scene *scene);
    /// Add a single point to the GL Buffer.
    /// Override this to add your own data to interleaved vertex buffers.
    virtual void addPointToBuffer(unsigned char *basePtr,int which,const Point3d *center);
    /// Called while a new VAO is bound.  Set up your VAO-related state here.
    virtual void setupAdditionalVAO(OpenGLES2Program *prog,GLuint vertArrayObj) { }
    /// Called after the drawable has bound all its various data, but before it actually
    /// renders.  This is where you would bind your own attributes and uniforms, if you
    /// haven't already done so in setupAdditionalVAO()
    virtual void bindAdditionalRenderObjects(WhirlyKit::RendererFrameInfo *frameInfo,Scene *scene) { }
    /// Called at the end of the drawOGL2() call
    virtual void postDrawCallback(WhirlyKit::RendererFrameInfo *frameInfo,Scene *scene) { }
    
    // Attributes associated with each vertex, some standard some not
    std::vector<VertexAttribute *> vertexAttributes;
    // Entries for the standard attributes we create on startup
    int colorEntry,normalEntry;
    // Set up the standard vertex attributes we use
    void setupStandardAttributes(int numReserve=0);
    
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
    
    bool hasMatrix;
    // If the drawable has a matrix, we'll transform by that before drawing
    Eigen::Matrix4d mat;
    
    // Size for a single vertex w/ all its data.  Used by shared buffer
    int vertexSize;
    GLuint pointBuffer,triBuffer,sharedBuffer;
    GLuint vertArrayObj;
    GLuint sharedBufferOffset;
    bool sharedBufferIsExternal;
};

/** Drawable Tweaker that cycles through textures.
 Looks at the current time and decides which two textures to use.
 */
class BasicDrawableTexTweaker : public DrawableTweaker
{
public:
    BasicDrawableTexTweaker(const std::vector<SimpleIdentity> &texIDs,TimeInterval startTime,double period);
    
    /// Modify the active texture IDs
    void tweakForFrame(Drawable *draw,WhirlyKit::RendererFrameInfo *frame);
protected:
    std::vector<SimpleIdentity> texIDs;
    TimeInterval startTime;
    double period;
};

/** Calculates important values for the screen space texture application and
    sets the results in the shader.
  */
class BasicDrawableScreenTexTweaker : public DrawableTweaker
{
public:
    BasicDrawableScreenTexTweaker(const Point3d &centerPt,const Point2d &texScale);
    
    /// Modify the active shader
    void tweakForFrame(Drawable *draw,WhirlyKit::RendererFrameInfo *frame);
protected:
    Point3d centerPt;
    Point2d texScale;
};

/// Reference counted version of BasicDrawable
typedef std::shared_ptr<BasicDrawable> BasicDrawableRef;

/// Ask the renderer to change a drawable's color
class ColorChangeRequest : public DrawableChangeRequest
{
public:
    ColorChangeRequest(SimpleIdentity drawId,RGBAColor color);
    
    void execute2(Scene *scene,WhirlyKit::SceneRendererES *renderer,DrawableRef draw);
    
protected:
    unsigned char color[4];
};

/// Turn a given drawable on or off.  This doesn't delete it.
class OnOffChangeRequest : public DrawableChangeRequest
{
public:
    OnOffChangeRequest(SimpleIdentity drawId,bool OnOff);
    
    void execute2(Scene *scene,WhirlyKit::SceneRendererES *renderer,DrawableRef draw);
    
protected:
    bool newOnOff;
};

/// Change the visibility distances for the given drawable
class VisibilityChangeRequest : public DrawableChangeRequest
{
public:
    VisibilityChangeRequest(SimpleIdentity drawId,float minVis,float maxVis);
    
    void execute2(Scene *scene,WhirlyKit::SceneRendererES *renderer,DrawableRef draw);
    
protected:
    float minVis,maxVis;
};

/// Change the fade times for a given drawable
class FadeChangeRequest : public DrawableChangeRequest
{
public:
    FadeChangeRequest(SimpleIdentity drawId,TimeInterval fadeUp,TimeInterval fadeDown);
    
    void execute2(Scene *scene,WhirlyKit::SceneRendererES *renderer,DrawableRef draw);
    
protected:
    TimeInterval fadeUp,fadeDown;
};

/// Change the texture used by a drawable
class DrawTexChangeRequest : public DrawableChangeRequest
{
public:
    DrawTexChangeRequest(SimpleIdentity drawId,unsigned int which,SimpleIdentity newTexId);
    
    void execute2(Scene *scene,WhirlyKit::SceneRendererES *renderer,DrawableRef draw);
    
protected:
    unsigned int which;
    SimpleIdentity newTexId;
};

/// Change the textures used by a drawable
class DrawTexturesChangeRequest : public DrawableChangeRequest
{
public:
    DrawTexturesChangeRequest(SimpleIdentity drawId,const std::vector<SimpleIdentity> &newTexIDs);
    
    void execute2(Scene *scene,WhirlyKit::SceneRendererES *renderer,DrawableRef draw);
    
protected:
    const std::vector<SimpleIdentity> newTexIDs;
};

/// Change the transform matrix on a drawable
class TransformChangeRequest : public DrawableChangeRequest
{
public:
    TransformChangeRequest(SimpleIdentity drawId,const Eigen::Matrix4d *newMat);
    
    void execute2(Scene *scene,WhirlyKit::SceneRendererES *renderer,DrawableRef draw);
    
protected:
    Eigen::Matrix4d newMat;
};

/// Change the drawPriority on a drawable
class DrawPriorityChangeRequest : public DrawableChangeRequest
{
public:
    DrawPriorityChangeRequest(SimpleIdentity drawId,int drawPriority);
    
    void execute2(Scene *scene,WhirlyKit::SceneRendererES *renderer,DrawableRef draw);
    
protected:
    int drawPriority;
};

/// Change the line width on a drawable
class LineWidthChangeRequest : public DrawableChangeRequest
{
public:
    LineWidthChangeRequest(SimpleIdentity drawId,float lineWidth);
    
    void execute2(Scene *scene,WhirlyKit::SceneRendererES *renderer,DrawableRef draw);
    
protected:
    float lineWidth;
};
    
}
