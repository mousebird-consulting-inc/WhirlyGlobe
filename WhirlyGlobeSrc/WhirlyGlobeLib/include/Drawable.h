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

#import "glwrapper.h"
#import <vector>
#import <set>
#import <map>
#import "RawData.h"
#import "Identifiable.h"
#import "WhirlyVector.h"
#import "WhirlyKitView.h"

namespace WhirlyKit
{
class RendererFrameInfo;
class SceneRendererES;
    
/** This is the configuration info passed to setupGL for each
 drawable.  Sometimes this will be render thread side, sometimes
 layer thread side.  The defaults should be valid.
 */
class WhirlyKitGLSetupInfo
{
public:
    WhirlyKitGLSetupInfo();
    /// If we're using drawOffset, this is the units
    float minZres;
};

class Scene;
class OpenGLES2Program;
class Drawable;

/// We'll only keep this many buffers or textures around for reuse
#define WhirlyKitOpenGLMemCacheMax 32
/// Number of buffers we allocate at once
#define WhirlyKitOpenGLMemCacheAllocUnit 32
    
// Maximum of 8 textures for the moment
#define WhirlyKitMaxTextures 8

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
		
    /// Return true if this change requires a GL Flush in the thread it was executed in
    virtual bool needsFlush() { return false; }
    
    /// Fill this in to set up whatever resources we need on the GL side
    virtual void setupGL(WhirlyKitGLSetupInfo *setupInfo,OpenGLMemManager *memManager) { };
		
	/// Make a change to the scene.  For the renderer.  Never call this.
	virtual void execute(Scene *scene,WhirlyKit::SceneRendererES *renderer,WhirlyKit::View *view) = 0;
};
    
/// Representation of a list of changes.  Might get more complex in the future.
typedef std::vector<ChangeRequest *> ChangeSet;

/** Drawable tweakers are called every frame to mess with things.
    It's up to you to make the changes, just make them quick.
  */
class DrawableTweaker : public Identifiable
{
public:
    virtual ~DrawableTweaker() { }
    /// Do your tweaking here
    virtual void tweakForFrame(Drawable *draw,RendererFrameInfo *frame) = 0;
};
    
typedef std::shared_ptr<DrawableTweaker> DrawableTweakerRef;
typedef std::set<DrawableTweakerRef> DrawableTweakerRefSet;

/** The Drawable base class.  Inherit from this and fill in the virtual
    methods.  In general, use the BasicDrawable.
 */
class Drawable : public Identifiable
{
public:
    /// Construct empty
	Drawable(const std::string &name);
	virtual ~Drawable();
	    
    /// Return the local MBR, if we're working in a non-geo coordinate system
    virtual Mbr getLocalMbr() const = 0;
	
	/// We use this to sort drawables
	virtual unsigned int getDrawPriority() const = 0;
    
    /// For OpenGLES2, this is the program to use to render this drawable.
    virtual SimpleIdentity getProgram() const = 0;
	
	/// We're allowed to turn drawables off completely
	virtual bool isOn(WhirlyKit::RendererFrameInfo *frameInfo) const = 0;
	
	/// Do any OpenGL initialization you may want.
	/// For instance, set up VBOs.
	/// We pass in the minimum Z buffer resolution (for offsets).
	virtual void setupGL(WhirlyKitGLSetupInfo *setupInfo,OpenGLMemManager *memManager) { };
	
	/// Clean up any OpenGL objects you may have (e.g. VBOs).
	virtual void teardownGL(OpenGLMemManager *memManage) { };

	/// Set up what you need in the way of context and draw.
	virtual void draw(WhirlyKit::RendererFrameInfo *frameInfo,Scene *scene) = 0;
    
    /// Return the type (or an approximation thereof).  We use this for sorting.
    virtual GLenum getType() const = 0;
    
    /// Return true if the drawable has alpha.  These will be sorted last.
    virtual bool hasAlpha(WhirlyKit::RendererFrameInfo *frameInfo) const = 0;
    
    /// Return the Matrix if there is an active one (ideally not)
    virtual const Eigen::Matrix4d *getMatrix() const { return NULL; }

    /// Check if the force Z buffer on mode is on
    virtual bool getRequestZBuffer() const { return false; }

    /// Check if we're supposed to write to the z buffer
    virtual bool getWriteZbuffer() const { return true; }
    
    /// Update anything associated with the renderer.  Probably renderUntil.
    virtual void updateRenderer(WhirlyKit::SceneRendererES *renderer) = 0;
    
    /// Add a tweaker to this list to be run each frame
    virtual void addTweaker(DrawableTweakerRef tweakRef) { tweakers.insert(tweakRef); }
    
    /// Remove a tweaker from the list
    virtual void removeTweaker(DrawableTweakerRef tweakRef) { tweakers.erase(tweakRef); }
    
    /// Run the tweakers
    virtual void runTweakers(RendererFrameInfo *frame);
    
protected:
    std::string name;
    DrawableTweakerRefSet tweakers;
};

/// Reference counted Drawable pointer
typedef std::shared_ptr<Drawable> DrawableRef;
    
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
    
    EIGEN_MAKE_ALIGNED_OPERATOR_NEW;
	
	/// This will look for the drawable by ID and then call execute2()
	void execute(Scene *scene,WhirlyKit::SceneRendererES *renderer,WhirlyKit::View *view);
	
	/// This is called by execute if there's a drawable to modify.
    /// This is the one you override.
	virtual void execute2(Scene *scene,WhirlyKit::SceneRendererES *renderer,DrawableRef draw) = 0;
	
protected:
	SimpleIdentity drawId;
};

/// Turn off visibility checking
static const float DrawVisibleInvalid = 1e10;
    
/// Maximum number of points we want in a drawable
static const unsigned int MaxDrawablePoints = ((1<<16)-1);
    
/// Maximum number of triangles we want in a drawable
static const unsigned int MaxDrawableTriangles = (MaxDrawablePoints / 3);
    
class SubTexture;

/// Data types we'll accept for attributes
typedef enum {BDFloat4Type,BDFloat3Type,BDChar4Type,BDFloat2Type,BDFloatType,BDIntType} BDAttributeDataType;
    
    
/// Used to keep track of attributes (other than points)
class VertexAttribute
{
public:
    VertexAttribute(BDAttributeDataType dataType,const std::string &name);
    VertexAttribute(const VertexAttribute &that);
    ~VertexAttribute();
    
    /// Make a copy of everything for the data
    VertexAttribute templateCopy() const;
    
    /// Return the data type
    BDAttributeDataType getDataType() const;
    
    /// Set the default color (if the type matches)
    void setDefaultColor(const RGBAColor &color);
    /// Set the default 2D vector (if the type matches)
    void setDefaultVector2f(const Eigen::Vector2f &vec);
    /// Set the default 3D vector (if the type matches)
    void setDefaultVector3f(const Eigen::Vector3f &vec);
    /// Set the default float (if the type matches)
    void setDefaultFloat(float val);
    
    /// Convenience routine to add a color (if the type matches)
    void addColor(const RGBAColor &color);
    /// Convenience routine to add a 2D vector (if the type matches)
    void addVector2f(const Eigen::Vector2f &vec);
    /// Convenience routine to add a 3D vector (if the type matches)
    void addVector3f(const Eigen::Vector3f &vec);
    /// Convenience routine to add a 4D vector (if the type matches)
    void addVector4f(const Eigen::Vector4f &vec);
    /// Convenience routine to add a float (if the type matches)
    void addFloat(float val);
    /// Convenience routine to add an int (if the type matches)
    void addInt(int val);
    
    /// Reserve size in the data array
    void reserve(int size);
    
    /// Number of elements in our array
    int numElements() const;
    
    /// Return the size of a single element
    int size() const;
    
    /// Clean out the data array
    void clear();
    
    /// Return a pointer to the given element
    void *addressForElement(int which);
    
    /// Return the number of components as needed by glVertexAttribPointer
    GLuint glEntryComponents() const;
    
    /// Return the data type as required by glVertexAttribPointer
    GLenum glType() const;
    
    /// Whether or not glVertexAttribPointer will normalize the data
    GLboolean glNormalize() const;
    
    /// Calls glVertexAttrib* for the appropriate type
    void glSetDefault(int index) const;
        
public:
    /// Data type for the attribute data
    BDAttributeDataType dataType;
    /// Name used in the shader
    std::string name;
    /// Default value to pass to OpenGL if there's no data array
    union {
        float vec4[4];
        float vec3[3];
        float vec2[2];
        float floatVal;
        unsigned char color[4];
        int intVal;
    } defaultData;
    /// std::vector of attribute data.  Type is known by the caller.
    void *data;
    /// Buffer offset within interleaved vertex
    GLuint buffer;
};
    
/** Base class for the single vertex attribute that provides
    the name and type of a vertex attribute.
  */
class SingleVertexAttributeInfo
{
public:
    /// Comparison operator for set
    bool operator < (const SingleVertexAttributeInfo &that) const
    {
        if (name == that.name)
            return type < that.type;
        return name < that.name;
    }
    
    bool operator == (const SingleVertexAttributeInfo &that) const
    {
        bool ret = (name == that.name);
        if (ret)
            return type == that.type;
        return ret;
    }
    
    /// Return the number of components as needed by glVertexAttribPointer
    GLuint glEntryComponents() const;
    
    /// Return the data type as required by glVertexAttribPointer
    GLenum glType() const;
    
    /// Whether or not glVertexAttribPointer will normalize the data
    GLboolean glNormalize() const;
    
    /// Return the size for the particular data type
    int size() const;

    /// Attribute's data type
    BDAttributeDataType type;

    /// Attribute name (e.g. "u_elev")
    std::string name;
};

typedef std::set<SingleVertexAttributeInfo> SingleVertexAttributeInfoSet;

/** The single vertex attribute holds a single typed value to
    be merged into a basic drawable's attributes arrays.
 */
class SingleVertexAttribute : public SingleVertexAttributeInfo
{
public:
    /// The actual data
    union {
        float vec4[4];
        float vec3[3];
        float vec2[2];
        float floatVal;
        unsigned char color[4];
        int intVal;
    } data;
};
    
typedef std::set<SingleVertexAttribute> SingleVertexAttributeSet;

/// Generate a vertex attribute info set from a vertex attribute set (e.g. strip out the values)
void VertexAttributeSetConvert(const SingleVertexAttributeSet &attrSet,SingleVertexAttributeInfoSet &infoSet);

/// See if the given set of attributes is compatible with the set of attributes (without data)
bool VertexAttributesAreCompatible(const SingleVertexAttributeInfoSet &infoSet,const SingleVertexAttributeSet &attrSet);
    
}
