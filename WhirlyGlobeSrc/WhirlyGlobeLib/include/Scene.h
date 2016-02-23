/*
 *  Scene.h
 *  WhirlyGlobeLib
 *
 *  Created by Steve Gifford on 1/3/11.
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
#import "WhirlyVector.h"
#import "Texture.h"
#import "Cullable.h"
#import "BasicDrawableInstance.h"
#import "Generator.h"
#import "FontTextureManager.h"
// Note: Porting
//#import "ActiveModel.h"
#import "CoordSystem.h"
#import "OpenGLES2Program.h"

/// How the scene refers to the default triangle shader (and how you replace it)
#define kSceneDefaultTriShader "Default Triangle Shader"
/// How the scene refers to the default line shader (and how you replace it)
#define kSceneDefaultLineShader "Default Line Shader"

namespace WhirlyKit
{
class SceneRendererES;
}

// Note: Porting
//@class WhirlyKitFontTextureManager;

namespace WhirlyKit
{
    
class Scene;
class SubTexture;
class ScreenSpaceGenerator;
class ViewPlacementGenerator;

/// Request that the renderer add the given texture.
/// This will make it available for use, referenced by ID.
class AddTextureReq : public ChangeRequest
{
public:
    /// Construct with a texture.
    /// You are not responsible for deleting the texture after this.
	AddTextureReq(TextureBase *tex) : tex(tex) { }
    /// If the texture hasn't been added to the renderer, clean it up.
	~AddTextureReq() { if (tex) delete tex; tex = NULL; }

    /// Texture creation generally wants a flush
    virtual bool needsFlush() { return true; }
    
    /// Create the texture on its native thread
    virtual void setupGL(WhirlyKitGLSetupInfo *setupInfo,OpenGLMemManager *memManager) { if (tex) tex->createInGL(memManager); };

	/// Add to the renderer.  Never call this.
	void execute(Scene *scene,WhirlyKit::SceneRendererES *renderer,WhirlyKit::View *view);
	
    /// Only use this if you've thought it out
    TextureBase *getTex() { return tex; }

protected:
	TextureBase *tex;
};

/// Remove a texture referred to by ID
class RemTextureReq : public ChangeRequest
{
public:
    /// Construct with the ID
	RemTextureReq(SimpleIdentity texId) : texture(texId) { }

    /// Remove from the renderer.  Never call this.
	void execute(Scene *scene,WhirlyKit::SceneRendererES *renderer,WhirlyKit::View *view);
	
protected:
	SimpleIdentity texture;
};

/// Ask the renderer to add the drawable to the scene
class AddDrawableReq : public ChangeRequest
{
public:
    /// Construct with a drawable.  You're not responsible for deletion
	AddDrawableReq(Drawable *drawable) : drawable(drawable) { }
    /// If the drawable wasn't used, delete it
	~AddDrawableReq() { if (drawable) delete drawable; drawable = NULL; }
    
    /// Drawable creation generally wants a flush
    virtual bool needsFlush() { return true; }
    
    /// Create the drawable on its native thread
    virtual void setupGL(WhirlyKitGLSetupInfo *setupInfo,OpenGLMemManager *memManager) { if (drawable) drawable->setupGL(setupInfo, memManager); };

	/// Add to the renderer.  Never call this
	void execute(Scene *scene,WhirlyKit::SceneRendererES *renderer,WhirlyKit::View *view);	
	
protected:
	Drawable *drawable;
};

/// Ask the renderer to remove the drawable from the scene
class RemDrawableReq : public ChangeRequest
{
public:
    /// Construct with the drawable ID and an optional fade interval
	RemDrawableReq(SimpleIdentity drawId) : drawable(drawId) { }

    /// Remove the drawable.  Never call this
	void execute(Scene *scene,WhirlyKit::SceneRendererES *renderer,WhirlyKit::View *view);
	
protected:	
	SimpleIdentity drawable;
};	

/// Add a Drawable Generator to the scene
class AddGeneratorReq : public ChangeRequest
{
public:
    /// Construct with the generator
    AddGeneratorReq(Generator *generator) : generator(generator) { }

    /// Add to the renderer.  Never call this.
    void execute(Scene *scene,WhirlyKit::SceneRendererES *renderer,WhirlyKit::View *view);
    
protected:
    Generator *generator;
};
    
/// Remove the Drawable Generator from the scene
class RemGeneratorReq : public ChangeRequest
{
public:
    /// Construct with the generator ID
    RemGeneratorReq(SimpleIdentity genId) : genId(genId) { }
    
    /// Remove from the renderer.  Never call this.
    void execute(Scene *scene,WhirlyKit::SceneRendererES *renderer,WhirlyKit::View *view);
    
protected:
    SimpleIdentity genId;
};
    
/// Add an OpenGL ES 2.0 program to the scene for user later
class AddProgramReq : public ChangeRequest
{
public:
    // Construct with the program to add
    AddProgramReq(const std::string &sceneName,OpenGLES2Program *prog) : sceneName(sceneName), program(prog) { }
    ~AddProgramReq() { if (program) delete program; program = NULL; }
    
    /// Remove from the renderer.  Never call this.
    void execute(Scene *scene,WhirlyKit::SceneRendererES *renderer,WhirlyKit::View *view);

protected:
    std::string sceneName;
    OpenGLES2Program *program;
};
    
/// Remove an OpenGL ES 2.0 program from the scene
class RemProgramReq : public ChangeRequest
{
public:
    /// Construct with the program ID
    RemProgramReq(SimpleIdentity progId) : programId(progId) { }
    
    /// Remove from the renderer.  Never call this.
    void execute(Scene *scene,WhirlyKit::SceneRendererES *renderer,WhirlyKit::View *view);

protected:
    SimpleIdentity programId;
};
    
/// Ask the renderer to remove the drawable from the scene
class SetProgramValueReq : public ChangeRequest
{
public:
    /// Construct with the drawable ID and an optional fade interval
    SetProgramValueReq(SimpleIdentity progID,const std::string &name,float u_val) : progID(progID), u_name(name), u_val(u_val) { }
    
    /// Remove the drawable.  Never call this
    void execute(Scene *scene,WhirlyKit::SceneRendererES *renderer,WhirlyKit::View *view);
    
protected:
    SimpleIdentity progID;
    std::string u_name;
    float u_val;
};
    
// Note: Porting
///// Remove a GL buffer ID, presumably because we needed other things cleaned up first
//class RemBufferReq : public ChangeRequest
//{
//public:
//    /// Construct with the buffer we want to delete
//    RemBufferReq(GLuint bufID) : bufID(bufID) { }
//    
//    /// Actually run the remove.  Never call this.
//    void execute(Scene *scene,WhirlyKit::SceneRendererES *renderer,WhirlyKit::View *view);
//    
//protected:
//    GLuint bufID;
//};
    
/// Send out a notification (on the main thread) when
///  we get this request.  Used to figure out when something
///  has been completely loaded.  Do not overuse.
// Note: Porting
//class NotificationReq : public ChangeRequest
//{
//public:
//    /// The notification name is required, the objection optional
//    NotificationReq(NSString *noteName,NSObject *noteObj);
//    virtual ~NotificationReq();
//    
//    /// Send out the notification
//    void execute(Scene *scene,WhirlyKit::SceneRendererES *renderer,WhirlyKit::View *view);
//    
//protected:
//    NSString * __strong noteName;
//    NSObject * __strong noteObj;
//};
    
/// Sorted set of generators
typedef std::set<Generator *,IdentifiableSorter> GeneratorSet;
    
typedef std::set<DrawableRef,IdentifiableRefSorter> DrawableRefSet;

typedef std::set<OpenGLES2Program *,IdentifiableSorter> OpenGLES2ProgramSet;
typedef std::map<std::string,OpenGLES2Program *> OpenGLES2ProgramMap;
    
/** The scene manager is a base class for various functionality managers
    associated with a scene.  These are the objects that build geometry,
    manage layout, selection, and so forth for a scene.  They typically
    do their work off of the main thread and then merge data back into
    the scene on the main thread.
 */
class SceneManager
{
public:
    SceneManager() : scene(NULL), renderer(NULL) { }
    virtual ~SceneManager() { };
    
    /// Set (or reset) the current renderer
    virtual void setRenderer(SceneRendererES *inRenderer) { renderer = inRenderer; }

    /// Set the scene we're part of
    virtual void setScene(Scene *inScene) { scene = inScene; }
    
    /// Return the scene this is part of
    Scene *getScene() { return scene; }
    
protected:
    Scene *scene;
    SceneRendererES *renderer;
};

/** This is the top level scene object for WhirlyKit.
    It keeps track of the drawables by sorting them into
     cullables and it handles the change requests, which
     consist of pretty much everything that can happen.
 */
class Scene : public DelayedDeletable
{
	friend class ChangeRequest;
public:
	virtual ~Scene();
    
    /// Return the coordinate system adapter we're using.
    /// You can get the coordinate system we're using from that.
    WhirlyKit::CoordSystemDisplayAdapter *getCoordAdapter();
    
    /// Full set of Generators
    const GeneratorSet *getGenerators() { return &generators; }
    
    /// Search for a Generator by name.
    /// This is not thread safe, so do this in the main thread
    SimpleIdentity getGeneratorIDByName(const std::string &name);

	/// Add a single change request.  You can call this from any thread, it locks.
    /// If you have more than one, don't iterate, use the other version.
	void addChangeRequest(ChangeRequest *newChange);
    /// Add a list of change requets.  You can call this from any thread.
    /// This is the faster option if you have more than one change request
	void addChangeRequests(const ChangeSet &newchanges);
	
	/// Look for a valid texture
    /// If it's missing, we probably won't draw the associated geometry
	GLuint getGLTexture(SimpleIdentity texIdent);
	
	/// Process change requests
	/// Only the renderer should call this in the rendering thread
	void processChanges(WhirlyKit::View *view,WhirlyKit::SceneRendererES *renderer);
    
    /// True if there are pending updates
    bool hasChanges();
    
    /// Add sub texture mappings.
    /// These are mappings from images to parts of texture atlases.
    /// They're here so we can use SimpleIdentity's to point into larger
    ///  textures.  Layer side only.  The rendering engine doesn't use them.
    void addSubTexture(const SubTexture &);
    void addSubTextures(const std::vector<SubTexture> &);
    
    /// Remove a subtexture mapping
    void removeSubTexture(SimpleIdentity subTexID);
    
    /// Remove several subtexture mappings
    void removeSubTextures(const std::vector<SimpleIdentity> &subTexIDs);

    /// Return a sub texture by ID.  The idea being we can use these
    ///  the same way we use full texture IDs.
    SubTexture getSubTexture(SimpleIdentity subTexId);
    
    /// Add a drawable to the scene.
    /// A subclass can override this to control how this interacts with cullabes.
    /// The scene is responsible for the Drawable after this call.
    virtual void addDrawable(DrawableRef drawable) = 0;
    
    /// Remove a drawable from the scene
    virtual void remDrawable(DrawableRef drawable) = 0;
    
    /// We create one screen space generator on startup.  This is its ID.
    SimpleIdentity getScreenSpaceGeneratorID();
    
    /// Get the screen space generator (be careful).
    ScreenSpaceGenerator *getScreenSpaceGenerator() { return ssGen; }
    
    /// Get the UIView placement generator.  Only use this in the main thread.
    ViewPlacementGenerator *getViewPlacementGenerator() { return vpGen; }
    
    /// Called once by the renderer so we can reset any managers that care
    void setRenderer(SceneRendererES *renderer);
    
    /// Return the given manager.  This is thread safe;
    SceneManager *getManager(const char *name);
    
    /// Add the given manager.  The scene is now responsible for deletion.  This is thread safe.
    void addManager(const char *name,SceneManager *manager);
    
    /// Add an active model.  Only call this on the main thread.
    // Note: Porting
//    void addActiveModel(NSObject<WhirlyKitActiveModel> *);
    
    /// Remove an active model (if it's in here).  Only call this on the main thread.
    // Note: Porting
//    void removeActiveModel(NSObject<WhirlyKitActiveModel> *);
    
    /// Return the top level cullable
    CullTree *getCullTree() { return cullTree; }
    
    /// Explicitly tear everything down in OpenGL ES.
    /// We're assuming the context has been set.
    void teardownGL();
    
    /// Get the renderer's buffer/texture ID manager.
    /// You can use this on any thread.  The calls are protected.
    OpenGLMemManager *getMemManager() { return &memManager; }
    
    /// Return a dispatch queue that we can use for... stuff.
    /// The idea here is we'll wait for these to drain when we tear down.
    // Note: Porting
//    dispatch_queue_t getDispatchQueue() { return dispatchQueue; }
	
    // Return all the drawables in a list.  Only call this on the main thread.
    const DrawableRefSet &getDrawables();

    /// Dump out stats on what is currently in the scene.
    /// Use this sparingly, as it writes to the log.
    void dumpStats();
	
public:
    /// Don't be calling this
    void setDisplayAdapter(CoordSystemDisplayAdapter *newCoordAdapter);
    
    pthread_mutex_t coordAdapterLock;
    /// The coordinate system display adapter converts from the local space
    ///  to display coordinates.
    WhirlyKit::CoordSystemDisplayAdapter *coordAdapter;
    
    /// Look for a Draw Generator by ID
    Generator *getGenerator(SimpleIdentity genId);
	
	/// Look for a Drawable by ID
	DrawableRef getDrawable(SimpleIdentity drawId);
	
	/// Look for a Texture by ID
	TextureBase *getTexture(SimpleIdentity texId);
    
    /// Add a texture to the scene
    void addTexture(TextureBase *tex);
    
    /// All the active models
    // Note: Porting
//    NSMutableArray *activeModels;
    
    /// All the drawable generators we've been handed, sorted by ID
    GeneratorSet generators;

    /// Top level of Cullable quad tree
    CullTree *cullTree;
	
	/// All the drawables we've been handed, sorted by ID
	DrawableRefSet drawables;
	
	typedef std::set<TextureBase *,IdentifiableSorter> TextureSet;
	/// Textures, sorted by ID
	TextureSet textures;
    
    /// Mutex for accessing textures
    pthread_mutex_t textureLock;
	
	pthread_mutex_t changeRequestLock;
	/// We keep a list of change requests to execute
	/// This can be accessed in multiple threads, so we lock it
	ChangeSet changeRequests;
    
    pthread_mutex_t subTexLock;
    typedef std::set<SubTexture> SubTextureSet;
    /// Mappings from images to parts of texture atlases
    SubTextureSet subTextureMap;
    
    /// ID for screen space generator
    SimpleIdentity screenSpaceGeneratorID;
    
    /// Lock for accessing the generators
    pthread_mutex_t generatorLock;
    
    /// Memory manager, really buffer and texture ID manager
    OpenGLMemManager memManager;
    
    /// Dispatch queue(s) we'll use for... things
    // Note: Porting
//    dispatch_queue_t dispatchQueue;
    
    /// Screen space generator created on startup
    ScreenSpaceGenerator *ssGen;
    
    /// UIView placement generator created on startup
    ViewPlacementGenerator *vpGen;
    
    /// Lock for accessing managers
    pthread_mutex_t managerLock;

    /// Managers for various functionality
    std::map<std::string,SceneManager *> managers;
    
    /// Returns the font texture manager, which is thread safe
    FontTextureManager *getFontTextureManager() { return fontTextureManager; }
    
    /// Set up the font texture manager.  Don't call this yourself.
    void setFontTextureManager(FontTextureManager *newManager) { if (fontTextureManager)  delete fontTextureManager; fontTextureManager = newManager; }
    
    /// Font texture manager (created on startup)
    // Note: Porting
//    WhirlyKitFontTextureManager *fontTexManager;
    
    /// Lock for accessing programs
    pthread_mutex_t programLock;
    
    /// Search for a shader program by ID (our ID, not OpenGL's)
    OpenGLES2Program *getProgram(SimpleIdentity programId);
    
    /// Search for a shader program by the scene name (not the program name)
    OpenGLES2Program *getProgramBySceneName(const std::string &sceneName);

    /// Look for the given program by scene name and return the ID
    SimpleIdentity getProgramIDBySceneName(const std::string &sceneName);
    
    /// Search for a shader program by its name (not the scene name)
    OpenGLES2Program *getProgramByName(const std::string &name);

    /// Search for a shader program by its name (not the scene name)
    SimpleIdentity getProgramIDByName(const std::string &name);
    
    /// Add a shader for reference, but not with a scene name.
    /// Presumably you'll call setSceneProgram() shortly.
    void addProgram(OpenGLES2Program *prog);
    
    /// Add a shader referred to by the scene name.  The scene name is
    ///  different from the program name.
    void addProgram(const std::string &sceneName,OpenGLES2Program *prog);
    
    /// Set the given scene name to refer to the given program ID
    void setSceneProgram(const std::string &sceneName,SimpleIdentity programId);
    
    /// Remove the given program by ID (ours, not OpenGL's)
    void removeProgram(SimpleIdentity progId);
        
protected:
    /// Only the subclasses are allowed to create these
    Scene();

	/// Construct with the depth of the cullable quad tree,
    ///  the coordinate system we're using, and the MBR of the
    ///  top level.
    /// The earth will be recursively divided into a quad tree of given depth.
    /// Init call used by the base class to set things up
    void Init(WhirlyKit::CoordSystemDisplayAdapter *adapter,Mbr localMbr,unsigned int depth);

    /// All the OpenGL ES 2.0 shader programs we know about
    OpenGLES2ProgramSet glPrograms;
    
    /// A map from the scene names to the various programs
    OpenGLES2ProgramMap glProgramMap;
    
    // The font texture manager is created at startup
    FontTextureManager *fontTextureManager;
};
	
}
