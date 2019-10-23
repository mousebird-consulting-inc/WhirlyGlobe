/*
 *  Scene.h
 *  WhirlyGlobeLib
 *
 *  Created by Steve Gifford on 1/3/11.
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
#import <unordered_map>
#import "WhirlyVector.h"
#import "Texture.h"
#import "Program.h"
#import "BasicDrawableInstance.h"
#import "ActiveModel.h"
#import "CoordSystem.h"

namespace WhirlyKit
{
    
class SceneRenderer;
class Scene;
class SubTexture;
class ScreenSpaceGenerator;
class ViewPlacementGenerator;
class FontTextureManager;
typedef std::shared_ptr<FontTextureManager> FontTextureManagerRef;
class RenderSetupInfo;

/// Request that the renderer add the given texture.
/// This will make it available for use, referenced by ID.
class AddTextureReq : public ChangeRequest
{
public:
    /// Construct with a texture.
    /// You are not responsible for deleting the texture after this.
    AddTextureReq(TextureBase *tex) { texRef = TextureBaseRef(tex); }
    AddTextureReq(const TextureBaseRef &texRef) : texRef(texRef) { }
    /// If the texture hasn't been added to the renderer, clean it up.
    ~AddTextureReq();

    /// Texture creation generally wants a flush
    virtual bool needsFlush() { return true; }
    
    /// Create the texture on its native thread
    virtual void setupForRenderer(const RenderSetupInfo *setupInfo);

	/// Add to the renderer.  Never call this.
	void execute(Scene *scene,SceneRenderer *renderer,View *view);
	
    /// Only use this if you've thought it out
    TextureBase *getTex();

protected:
    TextureBaseRef texRef;
};

/// Remove a texture referred to by ID
class RemTextureReq : public ChangeRequest
{
public:
    /// Construct with the ID
    RemTextureReq(SimpleIdentity texId) : texture(texId) { }
    /// This version is a time deletion
    RemTextureReq(SimpleIdentity texId,TimeInterval inWhen) : texture(texId) { when = inWhen; }

    /// Remove from the renderer.  Never call this.
	void execute(Scene *scene,SceneRenderer *renderer,View *view);
	
protected:
	SimpleIdentity texture;
};

/// Ask the renderer to add the drawable to the scene
class AddDrawableReq : public ChangeRequest
{
public:
    /// Construct with a drawable.  You're not responsible for deletion
	AddDrawableReq(Drawable *drawable) : drawRef(drawable) { }
    AddDrawableReq(const DrawableRef &drawRef) : drawRef(drawRef) { }
    /// If the drawable wasn't used, delete it
    ~AddDrawableReq();
    
    /// Drawable creation generally wants a flush
    virtual bool needsFlush() { return true; }
    
    /// Create the drawable on its native thread
    virtual void setupForRenderer(const RenderSetupInfo *);

	/// Add to the renderer.  Never call this
	void execute(Scene *scene,SceneRenderer *renderer,View *view);
	
protected:
    DrawableRef drawRef;
};

/// Ask the renderer to remove the drawable from the scene
class RemDrawableReq : public ChangeRequest
{
public:
    /// Construct with the drawable ID and an optional fade interval
	RemDrawableReq(SimpleIdentity drawId) : drawable(drawId) { }
    /// This version is a timed delete
    RemDrawableReq(SimpleIdentity drawId,TimeInterval inWhen) : drawable(drawId) { when = inWhen; }

    /// Remove the drawable.  Never call this
	void execute(Scene *scene,SceneRenderer *renderer,View *view);
	
protected:	
	SimpleIdentity drawable;
};
    
/// Add an OpenGL ES 2.0 program to the scene for user later
class AddProgramReq : public ChangeRequest
{
public:
    // Construct with the program to add
    AddProgramReq(const std::string &sceneName,ProgramRef prog) : sceneName(sceneName), program(prog) { }
    ~AddProgramReq() { }
    
    /// Remove from the renderer.  Never call this.
    void execute(Scene *scene,SceneRenderer *renderer,View *view);

protected:
    std::string sceneName;
    ProgramRef program;
};
    
/// Remove an OpenGL ES 2.0 program from the scene
class RemProgramReq : public ChangeRequest
{
public:
    /// Construct with the program ID
    RemProgramReq(SimpleIdentity progId) : programId(progId) { }
    
    /// Remove from the renderer.  Never call this.
    void execute(Scene *scene,SceneRenderer *renderer,View *view);

protected:
    SimpleIdentity programId;
};

// Set a program value, but go through the scene to do it
class SetProgramValueReq : public ChangeRequest
{
public:
    /// Construct with the drawable ID and an optional fade interval
    SetProgramValueReq(SimpleIdentity progID,const std::string &name,float u_val) : progID(progID), u_name(name), u_val(u_val) { }
    
    /// Remove the drawable.  Never call this
    void execute(Scene *scene,SceneRenderer *renderer,View *view);
    
protected:
    SimpleIdentity progID;
    std::string u_name;
    float u_val;
};
    
/// Run a block of code when this change request is executed
/// We use this to merge data on the main thread after other requests have been executed.
class RunBlockReq : public ChangeRequest
{
public:
    typedef std::function<void(Scene *scene,SceneRenderer *renderer,View *view)> BlockFunc;

    // Set up with the function to run
    RunBlockReq(BlockFunc newFunc);
    virtual ~RunBlockReq();

    // This is probably adding to the change requests and so needs to run first
    bool needPreExecute() { return true; }
    
    // Run the block of code
    void execute(Scene *scene,SceneRenderer *renderer,View *view);
    
protected:
    BlockFunc func;
};
        
typedef std::unordered_map<SimpleIdentity,DrawableRef> DrawableRefSet;

typedef std::map<SimpleIdentity,ProgramRef> ProgramSet;

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
    virtual void setRenderer(SceneRenderer *inRenderer) { renderer = inRenderer; }

    /// Set the scene we're part of
    virtual void setScene(Scene *inScene) { scene = inScene; }
    
    /// Return the scene this is part of
    Scene *getScene() { return scene; }

    /// Return the renderer
    SceneRenderer *getSceneRenderer() { return renderer; }
    
protected:
    Scene *scene;
    SceneRenderer *renderer;
};

/** This is the top level scene object for WhirlyKit.
    It keeps track of the drawables and the change requests, which
     consist of pretty much everything that can happen.
 */
class Scene : public DelayedDeletable
{
	friend class ChangeRequest;
public:
    EIGEN_MAKE_ALIGNED_OPERATOR_NEW;

    Scene(CoordSystemDisplayAdapter *adapter);
    virtual ~Scene();
    
    /// Return the coordinate system adapter we're using.
    /// You can get the coordinate system we're using from that.
    CoordSystemDisplayAdapter *getCoordAdapter();
    
    /// Add a single change request.  You can call this from any thread, it locks.
    /// If you have more than one, don't iterate, use the other version.
    void addChangeRequest(ChangeRequest *newChange);
    /// Add a list of change requets.  You can call this from any thread.
    /// This is the faster option if you have more than one change request
    void addChangeRequests(const ChangeSet &newchanges);
    
    /// Process change requests
    /// Only the renderer should call this in the rendering thread
    void processChanges(View *view,SceneRenderer *renderer,TimeInterval now);
    
    /// Some changes generate other changes, so they go first
    int preProcessChanges(View *view,SceneRenderer *renderer,TimeInterval now);
    
    /// True if there are pending updates
    bool hasChanges(TimeInterval now);
    
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
    virtual void addDrawable(DrawableRef drawable);
    
    /// Remove a drawable from the scene
    virtual void remDrawable(DrawableRef drawable);

    /// Called once by the renderer so we can reset any managers that care
    void setRenderer(SceneRenderer *renderer);
    
    /// Return the given manager.  This is thread safe;
    SceneManager *getManager(const char *name);
    /// This one can only be called during scene initialization
    SceneManager *getManagerNoLock(const char *name);
    
    /// Add the given manager.  The scene is now responsible for deletion.  This is thread safe.
    void addManager(const char *name,SceneManager *manager);
    
    /// Add an active model.  Only call this on the main thread.
    void addActiveModel(ActiveModelRef);
    
    /// Remove an active model (if it's in here).  Only call this on the main thread.
    void removeActiveModel(ActiveModelRef);
    
    /// Return a dispatch queue that we can use for... stuff.
    /// The idea here is we'll wait for these to drain when we tear down.
//    dispatch_queue_t getDispatchQueue() { return dispatchQueue; }
    
    // Return all the drawables in a list.  Only call this on the main thread.
    const DrawableRefSet &getDrawables();
    
    // Used for offline frame by frame rendering
    void setCurrentTime(TimeInterval newTime);
    
    // In general, this is just the system time.
    // But in offline render mode, we control this carefully
    TimeInterval getCurrentTime();
    
    // Used to track overlaps at the edges of a viewable area
    void addLocalMbr(const Mbr &localMbr);
	
    /// Dump out stats on what is currently in the scene.
    /// Use this sparingly, as it writes to the log.
    void dumpStats();
    
    /// Tear down renderer related assets
    virtual void teardown() = 0;
	
public:
    /// Don't be calling this
    void setDisplayAdapter(CoordSystemDisplayAdapter *newCoordAdapter);
    
    /// Passed around to setup and teardown renderer assets
    const RenderSetupInfo *setupInfo;
    
    std::mutex coordAdapterLock;
    /// The coordinate system display adapter converts from the local space
    ///  to display coordinates.
    CoordSystemDisplayAdapter *coordAdapter;
    
    /// Look for a Drawable by ID
    DrawableRef getDrawable(SimpleIdentity drawId);
    
    /// Look for a Texture by ID
    TextureBase *getTexture(SimpleIdentity texId);
        
    /// All the active models
    std::vector<ActiveModelRef> activeModels;
    
    /// All the drawables we've been handed, sorted by ID
    DrawableRefSet drawables;
    
    typedef std::unordered_map<SimpleIdentity,TextureBaseRef> TextureRefSet;
    /// Textures, sorted by ID
    TextureRefSet textures;
    
    /// Mutex for accessing textures
    std::mutex textureLock;
    
    std::mutex changeRequestLock;
    /// We keep a list of change requests to execute
    /// This can be accessed in multiple threads, so we lock it
    ChangeSet changeRequests;
    SortedChangeSet timedChangeRequests;
    
    std::mutex subTexLock;
    typedef std::set<SubTexture> SubTextureSet;
    /// Mappings from images to parts of texture atlases
    SubTextureSet subTextureMap;
    
    /// Lock for accessing managers
    std::mutex managerLock;
    
    /// Managers for various functionality
    std::map<std::string,SceneManager *> managers;
    
    /// Returns the font texture manager, which is thread safe
    FontTextureManager *getFontTextureManager() { return fontTextureManager.get(); }
    
    /// Set up the font texture manager.  Don't call this yourself.
    void setFontTextureManager(FontTextureManagerRef newManager);
        
    /// Lock for accessing programs
    std::mutex programLock;
    
    /// Search for a shader program by ID (our ID, not OpenGL's)
    Program *getProgram(SimpleIdentity programId);
    
    /// Look for a program by its name (last to first)
    Program *findProgramByName(const std::string &name);
    
    /// Add a shader for reference, but not with a scene name.
    /// Presumably you'll call setSceneProgram() shortly.
    void addProgram(ProgramRef prog);
    
    /// Remove the given program by ID (ours, not OpenGL's)
    void removeProgram(SimpleIdentity progId);
    
    /// For 2D maps we have an overlap margin based on what drawables may overlap the edges
    double getOverlapMargin() { return overlapMargin; }
    
protected:
    
    // If time is being set externally
    TimeInterval currentTime;

    /// All the OpenGL ES 2.0 shader programs we know about
    ProgramSet programs;
    
    /// Used for 2D overlap testing
    double overlapMargin;
    
    // The font texture manager is created at startup
    FontTextureManagerRef fontTextureManager;
};
	
}
