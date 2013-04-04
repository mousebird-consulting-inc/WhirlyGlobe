/*
 *  Drawable.h
 *  WhirlyGlobeLib
 *
 *  Created by Steve Gifford on 2/1/11.
 *  Copyright 2011-2012 mousebird consulting
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

#import <vector>
#import <set>
#import <map>
#import <boost/shared_ptr.hpp>
#import <boost/pointer_cast.hpp>
#import "Identifiable.h"
#import "WhirlyVector.h"
#import "GlobeView.h"

/// @cond
@class WhirlyKitSceneRendererES;
/// @endcond

/// @cond
@class WhirlyKitRendererFrameInfo;
/// @endcon

/** This is the configuration info passed to setupGL for each
    drawable.  Sometimes this will be render thread side, sometimes
    layer thread side.  The defaults should be valid.
  */
@interface WhirlyKitGLSetupInfo : NSObject
{
@public
    /// If we're using drawOffset, this is the units
    float minZres;
}
@end

namespace WhirlyKit
{
	
class Scene;
    class OpenGLES2Program;

/// We'll only keep this many buffers or textures around for reuse
#define WhirlyKitOpenGLMemCacheMax 32
/// Number of buffers we allocate at once
#define WhirlyKitOpenGLMemCacheAllocUnit 32

/// Used to manage OpenGL buffer IDs and such.
/// They're expensive to create and delete, so we try to do it
///  outside the renderer.
class OpenGLMemManager
{
public:
    OpenGLMemManager();
    ~OpenGLMemManager();
    
    /// Pick a buffer ID off the list or ask OpenGL for one
    GLuint getBufferID(unsigned int size=0,GLenum drawType=GL_STATIC_DRAW);
    /// Toss the given buffer ID back on the list for reuse
    void removeBufferID(GLuint bufID);

    /// Pick a texture ID off the list or ask OpenGL for one
    GLuint getTexID();
    /// Toss the given texture ID back on the list for reuse
    void removeTexID(GLuint texID);
        
    /// Clear out any and all buffer IDs that we may have sitting around
    void clearBufferIDs();
    
    /// Clear out any and all texture IDs that we have sitting around
    void clearTextureIDs();
    
    /// Print out stats about what's in the cache
    void dumpStats();
        
    /// Locks the mutex.  Don't be using this.
    void lock();
    
    /// Unlocks the mutix.  Seriously, not for you.  Don't call this.
    void unlock();
        
protected:
    pthread_mutex_t idLock;

    std::set<GLuint> buffIDs;
    std::set<GLuint> texIDs;
};

/// Mapping from Simple ID to an int
typedef std::map<SimpleIdentity,SimpleIdentity> TextureIDMap;
	
/** This is the base clase for a change request.  Change requests
    are how we modify things in the scene.  The renderer is running
    on the main thread and we want to keep our interaction with it
    very simple.  So instead of deleting things or modifying them
    directly, we ask the renderer to do so through a change request.
 */
class ChangeRequest
{
public:
	ChangeRequest() { }
	virtual ~ChangeRequest() { }
		
	/// Make a change to the scene.  For the renderer.  Never call this.
	virtual void execute(Scene *scene,WhirlyKitSceneRendererES *renderer,WhirlyKitView *view) = 0;
};	

/** The Drawable base class.  Inherit from this and fill in the virtual
    methods.  In general, use the BasicDrawable.
 */
class Drawable : public Identifiable
{
public:
    /// Construct empty
	Drawable();
	virtual ~Drawable();
	    
    /// Return the local MBR, if we're working in a non-geo coordinate system
    virtual Mbr getLocalMbr() const = 0;
	
	/// We use this to sort drawables
	virtual unsigned int getDrawPriority() const = 0;
    
    /// For OpenGLES2, this is the program to use to render this drawable.
    virtual SimpleIdentity getProgram() const = 0;
	
	/// We're allowed to turn drawables off completely
	virtual bool isOn(WhirlyKitRendererFrameInfo *frameInfo) const = 0;
	
	/// Do any OpenGL initialization you may want.
	/// For instance, set up VBOs.
	/// We pass in the minimum Z buffer resolution (for offsets).
	virtual void setupGL(WhirlyKitGLSetupInfo *setupInfo,OpenGLMemManager *memManager) { };
	
	/// Clean up any OpenGL objects you may have (e.g. VBOs).
	virtual void teardownGL(OpenGLMemManager *memManage) { };

	/// Set up what you need in the way of context and draw.
	virtual void draw(WhirlyKitRendererFrameInfo *frameInfo,Scene *scene) = 0;
    
    /// Return the type (or an approximation thereof).  We use this for sorting.
    virtual GLenum getType() const = 0;
    
    /// Return true if the drawable has alpha.  These will be sorted last.
    virtual bool hasAlpha(WhirlyKitRendererFrameInfo *frameInfo) const = 0;
    
    /// Return the Matrix if there is an active one (ideally not)
    virtual const Eigen::Matrix4f *getMatrix() const { return NULL; }

    /// Check if the force Z buffer on mode is on
    virtual bool getForceZBufferOn() const { return false; }
    
    /// Update anything associated with the renderer.  Probably renderUntil.
    virtual void updateRenderer(WhirlyKitSceneRendererES *renderer) = 0;
};

/// Reference counted Drawable pointer
typedef boost::shared_ptr<Drawable> DrawableRef;
    
/** Drawable Change Request is a subclass of the change request
    for drawables.  This is, itself, subclassed for specific
    change requests.
 */
class DrawableChangeRequest : public ChangeRequest
{
public:
    /// Construct with the ID of the Drawable we'll be changing
	DrawableChangeRequest(SimpleIdentity drawId) : drawId(drawId) { }
	virtual ~DrawableChangeRequest() { }
	
	/// This will look for the drawable by ID and then call execute2()
	void execute(Scene *scene,WhirlyKitSceneRendererES *renderer,WhirlyKitView *view);
	
	/// This is called by execute if there's a drawable to modify.
    /// This is the one you override.
	virtual void execute2(Scene *scene,WhirlyKitSceneRendererES *renderer,DrawableRef draw) = 0;
	
protected:
	SimpleIdentity drawId;
};

/// Turn off visibility checking
static const float DrawVisibleInvalid = 1e10;
    
/// Maximum number of points we want in a drawable
static const unsigned int MaxDrawablePoints = ((1<<16)-1);
    
/// Maximum number of triangles we want in a drawable
static const unsigned int MaxDrawableTriangles = (MaxDrawablePoints / 3);
    
/** The Basic Drawable is the one we use the most.  It's
    a general purpose container for static geometry which
    may or may not be textured.
 */
class BasicDrawable : public Drawable
{
public:
    /// Construct empty
	BasicDrawable();
	/// Construct with some idea how big things are.
    /// You can violate this, but it will reserve space
	BasicDrawable(unsigned int numVert,unsigned int numTri);
	virtual ~BasicDrawable();

    /// For OpenGLES2, this is the program to use to render this drawable.
    virtual SimpleIdentity getProgram() const { return programId; }

    /// For OpenGLES2, you can set the program to use in rendering
    void setProgram(SimpleIdentity progId) { programId = progId; }

	/// Set up the VBOs
	virtual void setupGL(WhirlyKitGLSetupInfo *setupInfo,OpenGLMemManager *memManager);

    /// This version can use a shared buffer instead of allocating its own
	virtual void setupGL(WhirlyKitGLSetupInfo *setupInfo,OpenGLMemManager *memManager,GLuint sharedBuf,GLuint sharedBufOffset);
	
	/// Clean up the VBOs
	virtual void teardownGL(OpenGLMemManager *memManage);	
	
    /// Size of a single vertex used in creating an interleaved buffer.
    /// Override this if you want to include your own data in the interleaved buffer.
    virtual GLuint singleVertexSize();
        
    /// Called render-thread side to set up a VAO
    /// Override this to set up
    virtual void setupVAO(OpenGLES2Program *prog);
	
	/// Fill this in to draw the basic drawable
	virtual void draw(WhirlyKitRendererFrameInfo *frameInfo,Scene *scene);
	
	/// Draw priority
	virtual unsigned int getDrawPriority() const { return drawPriority; }
	
	/// We use the on/off flag as well as a visibility check
	virtual bool isOn(WhirlyKitRendererFrameInfo *frameInfo) const;
	/// True to turn it on, false to turn it off
	void setOnOff(bool onOff) { on = onOff; }
    
    /// Used for alpha sorting
    virtual bool hasAlpha(WhirlyKitRendererFrameInfo *frameInfo) const;
    /// Set the alpha sorting on or off
    void setAlpha(bool onOff) { isAlpha = onOff; }
	    
    /// Extents used for display culling in local coordinates, if we're using them
    virtual Mbr getLocalMbr() const  { return localMbr; }
	    
    /// Set local extents
    void setLocalMbr(Mbr mbr) { localMbr = mbr; }
	
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
	void setDrawPriority(unsigned int newPriority) { drawPriority = newPriority; }
	unsigned int getDrawPriority() { return drawPriority; }

    /// Set the draw offset.  This is an integer offset from the base terrain.
    /// Geometry is moved upward by a certain number of units.
	void setDrawOffset(float newOffset) { drawOffset = newOffset; }
	float getDrawOffset() { return drawOffset; }

	/// Set the geometry type.  Probably triangles.
	void setType(GLenum inType) { type = inType; }
	GLenum getType() const { return type; }

    /// Set the texture ID.  You get this from the Texture object.
	void setTexId(SimpleIdentity inId) { texId = inId; }

    /// Set the color as an RGB color
	void setColor(RGBAColor inColor) { color = inColor; }

    /// Set the color as an array.
	void setColor(unsigned char inColor[]) { color.r = inColor[0];  color.g = inColor[1];  color.b = inColor[2];  color.a = inColor[3]; }
    RGBAColor getColor() const { return color; }

    /// Set what range we can see this drawable within.
    /// The units are in distance from the center of the globe and
    ///  the surface of the globe as at 1.0
    void setVisibleRange(float minVis,float maxVis,float minVisBand=0.0,float maxVisBand=0.0) { minVisible = minVis;  maxVisible = maxVis;  minVisibleFadeBand = minVisBand; maxVisibleFadeBand = maxVisBand; }
    
    /// Retrieve the visible range, just min and max
    void getVisibleRange(float &minVis,float &maxVis) { minVis = minVisible;  maxVis = maxVisible; }
    
    /// Retrieve the visible range, including bands
    void getVisibleRange(float &minVis,float &maxVis,float &minVisBand,float &maxVisBand) { minVis = minVisible; maxVis = maxVisible;  minVisBand = minVisibleFadeBand; maxVisBand = maxVisibleFadeBand; }
    
    /// Set the fade in and out
    void setFade(NSTimeInterval inFadeDown,NSTimeInterval inFadeUp) { fadeUp = inFadeUp;  fadeDown = inFadeDown; }
    
    /// Set the line width (if using lines)
    void setLineWidth(float inWidth) { lineWidth = inWidth; }
    
    /// Return the line width (1.0 is the default)
    float getLineWidth() { return lineWidth; }

    /// Used to sort a Drawable in with the lines in zBufferOffUntilLines mode
    void setForceZBufferOn(bool val) { forceZBufferOn = val; }
    
    /// Check if the force Z buffer on mode is on
    bool getForceZBufferOn() const { return forceZBufferOn; }

	/// Add a point when building up geometry.  Returns the index.
	unsigned int addPoint(Point3f pt) { points.push_back(pt); return points.size()-1; }
    
    /// Return a given point
    Point3f getPoint(int which) { if (which >= points.size()) return Point3f(0,0,0);  return points[which]; }

    /// Add a texture coordinate.
	void addTexCoord(TexCoord coord) { texCoords.push_back(coord); }
    
    /// Add a color
    void addColor(RGBAColor color) { colors.push_back(color); }

    /// Add a normal
	void addNormal(Point3f norm) { norms.push_back(norm); }
    
    /// Return a given normal
    Point3f getNormal(int which) { if (which <= norms.size()) return Point3f(0,0,1);  return norms[which]; }

    /// Add a triangle.  Should point to the vertex IDs.
	void addTriangle(Triangle tri) { tris.push_back(tri); }
    
    /// Return the texture ID
    SimpleIdentity getTexId() { return texId; }
    
    /// Return the number of points added so far
    unsigned int getNumPoints() const { return points.size(); }
    
    /// Return the number of triangles added so far
    unsigned int getNumTris() const { return tris.size(); }
    
    /// Return the number of normals added so far
    unsigned int getNumNorms() const { return norms.size(); }
    
    /// Return the number of texture coordinates added so far
    unsigned int getNumTexCoords() const { return texCoords.size(); }
    
    /// Reserve the extra space for points
    void reserveNumPoints(int numPoints) { points.reserve(points.size()+numPoints); }
    
    /// Reserve the extra space for triangles
    void reserveNumTris(int numTris) { tris.reserve(tris.size()+numTris); }
    
    /// Reserve extra space for texture coordinates
    void reserveNumTexCoords(int numCoords) { texCoords.reserve(texCoords.size()+numCoords); }
    
    /// Reserve extra space for normals
    void reserveNumNorms(int numNorms) { norms.reserve(norms.size()+numNorms); }
    
    /// Reserve extra space for colors
    void reserveNumColors(int numColors) { colors.reserve(colors.size()+numColors); }
	
	/// Widen a line and turn it into a rectangle of the given width
	void addRect(const Point3f &l0, const Eigen::Vector3f &ln0, const Point3f &l1, const Eigen::Vector3f &ln1,float width);
    
    /// Set the active transform matrix
    void setMatrix(const Eigen::Matrix4f *inMat) { mat = *inMat; hasMatrix = true; }

    /// Return the active transform matrix, if we have one
    const Eigen::Matrix4f *getMatrix() const { if (hasMatrix) return &mat;  return NULL; }

    /// Update fade up/down times in renderer (i.e. keep the renderer rendering)
    virtual void updateRenderer(WhirlyKitSceneRendererES *renderer);
    
protected:
    /// OpenGL ES 1.1 drawing routine
	virtual void drawReg(WhirlyKitRendererFrameInfo *frameInfo,Scene *scene);
    /// OpenGL ES 1.1 drawing routine
	virtual void drawVBO(WhirlyKitRendererFrameInfo *frameInfo,Scene *scene);
    
    /// Draw routine for OpenGL 2.0
    virtual void drawOGL2(WhirlyKitRendererFrameInfo *frameInfo,Scene *scene);
    /// Add a single point to the GL Buffer.
    /// Override this to add your own data to interleaved vertex buffers.
    virtual void addPointToBuffer(unsigned char *basePtr,int which);
    /// Called while a new VAO is bound.  Set up your VAO-related state here.
    virtual void setupAdditionalVAO(OpenGLES2Program *prog,GLuint vertArrayObj) { }
    /// Called after the drawable has bound all its various data, but before it actually
    /// renders.  This is where you would bind your own attributes and uniforms, if you
    /// haven't already done so in setupAdditionalVAO()
    virtual void bindAdditionalRenderObjects(WhirlyKitRendererFrameInfo *frameInfo,Scene *scene) { }
    /// Called at the end of the drawOGL2() call
    virtual void postDrawCallback(WhirlyKitRendererFrameInfo *frameInfo,Scene *scene) { }
	
	bool on;  // If set, draw.  If not, not
    SimpleIdentity programId;    // Program to use for rendering
    bool usingBuffers;  // If set, we've downloaded the buffers already
    NSTimeInterval fadeUp,fadeDown;  // Controls fade in and fade out
	unsigned int drawPriority;  // Used to sort drawables
	float drawOffset;    // Number of units of Z buffer resolution to offset upward (by the normal)
    bool isAlpha;  // Set if we want to be drawn last
    Mbr localMbr;  // Extents in a local space, if we're not using lat/lon/radius
	GLenum type;  // Primitive(s) type
	SimpleIdentity texId;  // ID for Texture (in scene)
	RGBAColor color;
    float minVisible,maxVisible;
    float minVisibleFadeBand,maxVisibleFadeBand;
    float lineWidth;
    // For zBufferOffUntilLines mode we'll sort this with the lines
    bool forceZBufferOn;
    // We'll nuke the data arrays when we hand over the data to GL
    unsigned int numPoints, numTris;
	std::vector<Eigen::Vector3f> points;
    std::vector<RGBAColor> colors;
	std::vector<Eigen::Vector2f> texCoords;
	std::vector<Eigen::Vector3f> norms;
	std::vector<Triangle> tris;
    
    bool hasMatrix;
    // If the drawable has a matrix, we'll transform by that before drawing
    Eigen::Matrix4f mat;
	
    // Size for a single vertex w/ all its data.  Used by shared buffer
    int vertexSize;
	GLuint pointBuffer,colorBuffer,texCoordBuffer,normBuffer,triBuffer,sharedBuffer;
    GLuint vertArrayObj;
    GLuint sharedBufferOffset;
    bool sharedBufferIsExternal;
};
    
/// Reference counted version of BasicDrawable
typedef boost::shared_ptr<BasicDrawable> BasicDrawableRef;

/// Ask the renderer to change a drawable's color
class ColorChangeRequest : public DrawableChangeRequest
{
public:
	ColorChangeRequest(SimpleIdentity drawId,RGBAColor color);
	
	void execute2(Scene *scene,WhirlyKitSceneRendererES *renderer,DrawableRef draw);
	
protected:
	unsigned char color[4];
};
	
/// Turn a given drawable on or off.  This doesn't delete it.
class OnOffChangeRequest : public DrawableChangeRequest
{
public:
	OnOffChangeRequest(SimpleIdentity drawId,bool OnOff);
	
	void execute2(Scene *scene,WhirlyKitSceneRendererES *renderer,DrawableRef draw);
	
protected:
	bool newOnOff;
};	

/// Change the visibility distances for the given drawable
class VisibilityChangeRequest : public DrawableChangeRequest
{
public:
    VisibilityChangeRequest(SimpleIdentity drawId,float minVis,float maxVis);
    
    void execute2(Scene *scene,WhirlyKitSceneRendererES *renderer,DrawableRef draw);
    
protected:
    float minVis,maxVis;
};
    
/// Change the fade times for a given drawable
class FadeChangeRequest : public DrawableChangeRequest
{
public:
    FadeChangeRequest(SimpleIdentity drawId,NSTimeInterval fadeUp,NSTimeInterval fadeDown);
    
    void execute2(Scene *scene,WhirlyKitSceneRendererES *renderer,DrawableRef draw);
    
protected:
    NSTimeInterval fadeUp,fadeDown;
};
    
/// Change the texture used by a drawable
class DrawTexChangeRequest : public DrawableChangeRequest
{
public:
    DrawTexChangeRequest(SimpleIdentity drawId,SimpleIdentity newTexId);
    
    void execute2(Scene *scene,WhirlyKitSceneRendererES *renderer,DrawableRef draw);
    
protected:
    SimpleIdentity newTexId;
};
    
/// Change the transform matrix on a drawable
class TransformChangeRequest : public DrawableChangeRequest
{
public:
    TransformChangeRequest(SimpleIdentity drawId,const Eigen::Matrix4f *newMat);
    
    void execute2(Scene *scene,WhirlyKitSceneRendererES *renderer,DrawableRef draw);
    
protected:
    Eigen::Matrix4f newMat;
};
    
/// Change the drawPriority on a drawable
class DrawPriorityChangeRequest : public DrawableChangeRequest
{
public:
    DrawPriorityChangeRequest(SimpleIdentity drawId,int drawPriority);
    
    void execute2(Scene *scene,WhirlyKitSceneRendererES *renderer,DrawableRef draw);
    
protected:
    int drawPriority;
};
    
/// Change the line width on a drawable
class LineWidthChangeRequest : public DrawableChangeRequest
{
public:
    LineWidthChangeRequest(SimpleIdentity drawId,float lineWidth);
    
    void execute2(Scene *scene,WhirlyKitSceneRendererES *renderer,DrawableRef draw);
    
protected:
    float lineWidth;
};
    
}
