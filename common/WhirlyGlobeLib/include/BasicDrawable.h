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
#import "VertexAttribute.h"

namespace WhirlyKit
{

/** The Basic Drawable is the one we use the most.  It's
 a general purpose container for static geometry which
 may or may not be textured.
 */
class BasicDrawable : public Drawable
{
friend class BasicDrawableInstance;

protected:
    // Used by subclasses
    BasicDrawable() { }
    
public:
    EIGEN_MAKE_ALIGNED_OPERATOR_NEW;

    /// Construct empty
    BasicDrawable(const std::string &name);
    /// Construct with some idea how big things are.
    /// You can violate this, but it will reserve space
    BasicDrawable(const std::string &name, unsigned int numVert,unsigned int numTri);
    virtual ~BasicDrawable();
    
    /// For OpenGLES2, this is the program to use to render this drawable.
    virtual SimpleIdentity getProgram() const;

    /// Set up local rendering structures (e.g. VBOs)
    virtual void setupForRenderer(RenderSetupInfo *setupInfo);
    
    /// Clean up any rendering objects you may have (e.g. VBOs).
    virtual void teardownForRenderer(RenderSetupInfo *setupInfo);

    /// Fill this in to draw the basic drawable
    virtual void draw(RendererFrameInfo *frameInfo,Scene *scene);
    
    /// Draw priority
    virtual unsigned int getDrawPriority() const;
    
    /// We use the on/off flag as well as a visibility check
    virtual bool isOn(RendererFrameInfo *frameInfo) const;
    
    /// Used for alpha sorting
    virtual bool hasAlpha(RendererFrameInfo *frameInfo) const;
    
    /// Extents used for display culling in local coordinates, if we're using them
    virtual Mbr getLocalMbr() const;
        
    /// We sort by draw priority before rendering.
    virtual unsigned int getDrawPriority();
    
    /// Set the draw offset.  This is an integer offset from the base terrain.
    /// Geometry is moved upward by a certain number of units.
    virtual float getDrawOffset();
    
    /// Return the default color
    virtual RGBAColor getColor() const;
    
    /// Set what range we can see this drawable within.
    /// The units are in distance from the center of the globe and
    ///  the surface of the globe as at 1.0
    virtual void getVisibleRange(float &minVis,float &maxVis);
    
    /// Retrieve the visible range, including bands
    virtual void getVisibleRange(float &minVis,float &maxVis,float &minVisBand,float &maxVisBand);

    /// Set the viewer based visibility
    virtual void setViewerVisibility(double minViewerDist,double maxViewerDist,const Point3d &viewerCenter);
    /// Retrieve the viewer based visibility
    virtual void getViewerVisibility(double &minViewerDist,double &maxViewerDist,Point3d &viewerCenter);
    
    /// Return the line width (1.0 is the default)
    virtual float getLineWidth();
    
    /// Check if the force Z buffer on mode is on
    virtual bool getRequestZBuffer() const;
    
    /// Check if we want to write to the z buffer
    virtual bool getWriteZbuffer() const;
    
    /// Return the texture ID
    virtual SimpleIdentity getTexId(unsigned int which);
    
    /// Texture ID and pointer to vertex attribute info
    class TexInfo
    {
    public:
        TexInfo() : texId(EmptyIdentity), texCoordEntry(0),
                    relLevel(0), relX(0), relY(0),
                    size(0), borderTexel(0) { }
        /// Texture ID within the scene
        SimpleIdentity texId;
        /// Vertex attribute entry for this set of texture coordinates
        int texCoordEntry;
        /// Our use of this texture relative to its native resolution
        int relLevel,relX,relY;
        /// Size of a texture side
        int size;
        /// Border texels to avoid.  Used for blending.
        int borderTexel;
    };
    
    /// Return the current texture info
    virtual const std::vector<TexInfo> &getTexInfo();
    
    /// Return the active transform matrix, if we have one
    virtual const Eigen::Matrix4d *getMatrix() const;
    
    // EmptyIdentity is the standard view, anything else is custom render target
    SimpleIdentity getRenderTarget();

    /// Update fade up/down times in renderer (i.e. keep the renderer rendering)
    virtual void updateRenderer(SceneRenderer *renderer);
        
    ////////////////////////////////
    /// ---- Changeable values ----
    
    /// Set the draw offset.  This is an integer offset from the base terrain.
    /// Geometry is moved upward by a certain number of units.
    virtual void setDrawOffset(float newOffset);
    
    /// Draw priority used for sorting
    virtual void setDrawPriority(unsigned int newPriority);
    
    // If set, we'll render this data where directed
    void setRenderTarget(SimpleIdentity newRenderTarget) { renderTargetID = newRenderTarget; }
    
    /// Set the alpha sorting on or off
    void setAlpha(bool onOff);

    /// Set the texture ID for a specific slot.  You get this from the Texture object.
    virtual void setTexId(unsigned int which,SimpleIdentity inId);
    
    /// Set all the textures at once
    virtual void setTexIDs(const std::vector<SimpleIdentity> &texIDs);

    /// Used to override a color that's already been built in (by changeVector:)
    virtual void setOverrideColor(RGBAColor inColor);
    virtual void setOverrideColor(unsigned char inColor[]);

    /// Set the time range for enable
    void setEnableTimeRange(TimeInterval inStartEnable,TimeInterval inEndEnable) { startEnable = inStartEnable;  endEnable = inEndEnable; }
    
    /// True to turn it on, false to turn it off
    void setOnOff(bool onOff);

    /// Set the fade in and out
    virtual void setFade(TimeInterval inFadeDown,TimeInterval inFadeUp);

    /// Set the line width (if using lines)
    virtual void setLineWidth(float inWidth);
    
    /// Set what range we can see this drawable within.
    /// The units are in distance from the center of the globe and
    ///  the surface of the globe as at 1.0
    virtual void setVisibleRange(float minVis,float maxVis,float minVisBand=0.0,float maxVisBand=0.0);
    
    /// We can ask to use the z buffer
    virtual void setRequestZBuffer(bool val);
    
    /// Set the z buffer mode for this drawable
    virtual void setWriteZBuffer(bool val);

    /// For OpenGLES2, you can set the program to use in rendering
    void setProgram(SimpleIdentity progId);
    
    /// Set the relative offsets for texture usage.
    /// We use these to look up parts of a texture at a higher level
    virtual void setTexRelative(int which,int size,int borderTexel,int relLevel,int relX,int relY);

public:
    // Used by subclasses to do the standard init
    void basicDrawableInit();
    /// Check for the given texture coordinate entry and add it if it's not there
    virtual void setupTexCoordEntry(int which,int numReserve);
    
    // Attributes associated with each vertex, some standard some not
    std::vector<VertexAttribute *> vertexAttributes;
    // Entries for the standard attributes we create on startup
    int colorEntry,normalEntry;
    
    bool on;  // If set, draw.  If not, not
    TimeInterval startEnable,endEnable;
    SimpleIdentity programId;    // Program to use for rendering
    TimeInterval fadeUp,fadeDown;  // Controls fade in and fade out
    unsigned int drawPriority;  // Used to sort drawables
    float drawOffset;    // Number of units of Z buffer resolution to offset upward (by the normal)
    bool isAlpha;  // Set if we want to be drawn last
    Mbr localMbr;  // Extents in a local space, if we're not using lat/lon/radius
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
        VertAttrDefault(unsigned int progAttrIndex,const VertexAttribute &attr)
        : progAttrIndex(progAttrIndex), attr(attr) { }
        unsigned int progAttrIndex;
        VertexAttribute attr;
    };
    
    std::vector<VertAttrDefault> vertArrayDefaults;
    
    // If set the geometry is already in OpenGL clip coordinates, so no transform
    bool clipCoords;
};

/** Drawable Tweaker that cycles through textures.
 Looks at the current time and decides which two textures to use.
 */
class BasicDrawableTexTweaker : public DrawableTweaker
{
public:
    BasicDrawableTexTweaker(const std::vector<SimpleIdentity> &texIDs,TimeInterval startTime,double period);
    
    /// Modify the active texture IDs
    void tweakForFrame(Drawable *draw,RendererFrameInfo *frame);
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
    EIGEN_MAKE_ALIGNED_OPERATOR_NEW;

    BasicDrawableScreenTexTweaker(const Point3d &centerPt,const Point2d &texScale);
    
    /// Modify the active shader
    void tweakForFrame(Drawable *draw,RendererFrameInfo *frame);
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
    
    void execute2(Scene *scene,SceneRenderer *renderer,DrawableRef draw);
    
protected:
    unsigned char color[4];
};

/// Turn a given drawable on or off.  This doesn't delete it.
class OnOffChangeRequest : public DrawableChangeRequest
{
public:
    OnOffChangeRequest(SimpleIdentity drawId,bool OnOff);
    
    void execute2(Scene *scene,SceneRenderer *renderer,DrawableRef draw);
    
protected:
    bool newOnOff;
};

/// Change the visibility distances for the given drawable
class VisibilityChangeRequest : public DrawableChangeRequest
{
public:
    VisibilityChangeRequest(SimpleIdentity drawId,float minVis,float maxVis);
    
    void execute2(Scene *scene,SceneRenderer *renderer,DrawableRef draw);
    
protected:
    float minVis,maxVis;
};

/// Change the fade times for a given drawable
class FadeChangeRequest : public DrawableChangeRequest
{
public:
    FadeChangeRequest(SimpleIdentity drawId,TimeInterval fadeUp,TimeInterval fadeDown);
    
    void execute2(Scene *scene,SceneRenderer *renderer,DrawableRef draw);
    
protected:
    TimeInterval fadeUp,fadeDown;
};

/// Change the texture used by a drawable
class DrawTexChangeRequest : public DrawableChangeRequest
{
public:
    DrawTexChangeRequest(SimpleIdentity drawId,unsigned int which,SimpleIdentity newTexId);
    DrawTexChangeRequest(SimpleIdentity drawId,unsigned int which,SimpleIdentity newTexId,int size,int borderTexel,int relLevel,int relX,int relY);
    
    void execute2(Scene *scene,SceneRenderer *renderer,DrawableRef draw);
    
protected:
    unsigned int which;
    SimpleIdentity newTexId;
    bool relSet;
    int size,borderTexel;
    int relLevel,relX,relY;
};

/// Change the textures used by a drawable
class DrawTexturesChangeRequest : public DrawableChangeRequest
{
public:
    DrawTexturesChangeRequest(SimpleIdentity drawId,const std::vector<SimpleIdentity> &newTexIDs);
    
    void execute2(Scene *scene,SceneRenderer *renderer,DrawableRef draw);
    
protected:
    const std::vector<SimpleIdentity> newTexIDs;
};

/// Change the transform matrix on a drawable
class TransformChangeRequest : public DrawableChangeRequest
{
public:
    EIGEN_MAKE_ALIGNED_OPERATOR_NEW;

    TransformChangeRequest(SimpleIdentity drawId,const Eigen::Matrix4d *newMat);
    
    void execute2(Scene *scene,SceneRenderer *renderer,DrawableRef draw);
    
protected:
    Eigen::Matrix4d newMat;
};

/// Change the drawPriority on a drawable
class DrawPriorityChangeRequest : public DrawableChangeRequest
{
public:
    DrawPriorityChangeRequest(SimpleIdentity drawId,int drawPriority);
    
    void execute2(Scene *scene,SceneRenderer *renderer,DrawableRef draw);
    
protected:
    int drawPriority;
};

/// Change the line width on a drawable
class LineWidthChangeRequest : public DrawableChangeRequest
{
public:
    LineWidthChangeRequest(SimpleIdentity drawId,float lineWidth);
    
    void execute2(Scene *scene,SceneRenderer *renderer,DrawableRef draw);
    
protected:
    float lineWidth;
};
    
/// Reset the uniforms passed into a shader for a specific drawable
class DrawUniformsChangeRequest : public DrawableChangeRequest
{
public:
    DrawUniformsChangeRequest(SimpleIdentity drawID,const SingleVertexAttributeSet &attrs);
    
    void execute2(Scene *scene,SceneRenderer *renderer,DrawableRef draw);
    
protected:
    SingleVertexAttributeSet attrs;
};

/// Change the render target (or clear it)
class RenderTargetChangeRequest : public DrawableChangeRequest
{
public:
    RenderTargetChangeRequest(SimpleIdentity drawId,SimpleIdentity );
    
    void execute2(Scene *scene,SceneRenderer *renderer,DrawableRef draw);
    
protected:
    SimpleIdentity targetID;
};
    
}
