/*
 *  Scene.h
 *  WhirlyGlobeLib
 *
 *  Created by Steve Gifford on 1/3/11.
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
#import "WhirlyVector.h"
#import "Texture.h"
#import "Cullable.h"
#import "Drawable.h"
#import "Generator.h"
#import "ActiveModel.h"
#import "CoordSystem.h"
#import "OpenGLES2Program.h"

/// @cond
@class WhirlyKitSceneRendererES;
/// @endcond

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
	AddTextureReq(Texture *tex) : tex(tex) { }
    /// If the texture hasn't been added to the renderer, clean it up.
	~AddTextureReq() { if (tex) delete tex; tex = NULL; }

	/// Add to the renderer.  Never call this.
	void execute(Scene *scene,WhirlyKitSceneRendererES *renderer,WhirlyKitView *view);
	
    /// Only use this if you've thought it out
    Texture *getTex() { return tex; }

protected:
	Texture *tex;
};

/// Remove a texture referred to by ID
class RemTextureReq : public ChangeRequest
{
public:
    /// Construct with the ID
	RemTextureReq(SimpleIdentity texId) : texture(texId) { }

    /// Remove from the renderer.  Never call this.
	void execute(Scene *scene,WhirlyKitSceneRendererES *renderer,WhirlyKitView *view);
	
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

	/// Add to the renderer.  Never call this
	void execute(Scene *scene,WhirlyKitSceneRendererES *renderer,WhirlyKitView *view);	
	
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
	void execute(Scene *scene,WhirlyKitSceneRendererES *renderer,WhirlyKitView *view);
	
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
    void execute(Scene *scene,WhirlyKitSceneRendererES *renderer,WhirlyKitView *view);
    
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
    void execute(Scene *scene,WhirlyKitSceneRendererES *renderer,WhirlyKitView *view);
    
protected:
    SimpleIdentity genId;
};
    
/// Add an OpenGL ES 2.0 program to the scene for user later
class AddProgramReq : public ChangeRequest
{
public:
    // Construct with the program to add
    AddProgramReq(OpenGLES2Program *prog) : program(prog) { }
    ~AddProgramReq() { if (program) delete program; program = NULL; }
    
    /// Remove from the renderer.  Never call this.
    void execute(Scene *scene,WhirlyKitSceneRendererES *renderer,WhirlyKitView *view);

protected:
    OpenGLES2Program *program;
};
    
/// Remove an OpenGL ES 2.0 program from the scene
class RemProgramReq : public ChangeRequest
{
public:
    /// Construct with the program ID
    RemProgramReq(SimpleIdentity progId) : programId(progId) { }
    
    /// Remove from the renderer.  Never call this.
    void execute(Scene *scene,WhirlyKitSceneRendererES *renderer,WhirlyKitView *view);

protected:
    SimpleIdentity programId;
};
    
/// Remove a GL buffer ID, presumably because we needed other things cleaned up first
class RemBufferReq : public ChangeRequest
{
public:
    /// Construct with the buffer we want to delete
    RemBufferReq(GLuint bufID) : bufID(bufID) { }
    
    /// Actually run the remove.  Never call this.
    void execute(Scene *scene,WhirlyKitSceneRendererES *renderer,WhirlyKitView *view);
    
protected:
    GLuint bufID;
};
    
/// Send out a notification (on the main thread) when
///  We get this request.  Used to figure out when something
///  has been completely loaded.  Do not overuse.
class NotificationReq : public ChangeRequest
{
public:
    /// The notification name is required, the objection optional
    NotificationReq(NSString *noteName,NSObject *noteObj);
    virtual ~NotificationReq();
    
    /// Send out the notification
    void execute(Scene *scene,WhirlyKitSceneRendererES *renderer,WhirlyKitView *view);
    
protected:
    NSString * __strong noteName;
    NSObject * __strong noteObj;
};
        
/// Sorted set of generators
typedef std::set<Generator *,IdentifiableSorter> GeneratorSet;
    
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
    WhirlyKit::CoordSystemDisplayAdapter *getCoordAdapter() { return coordAdapter; }
    
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
	void addChangeRequests(const std::vector<ChangeRequest *> &newchanges);
	
	/// Look for a valid texture
    /// If it's missing, we probably won't draw the associated geometry
	GLuint getGLTexture(SimpleIdentity texIdent);
	
	/// Process change requests
	/// Only the renderer should call this in the rendering thread
	void processChanges(WhirlyKitView *view,WhirlyKitSceneRendererES *renderer);
    
    /// True if there are pending updates
    bool hasChanges();
    
    /// Add sub texture mappings.
    /// These are mappings from images to parts of texture atlases.
    /// They're here so we can use SimpleIdentity's to point into larger
    ///  textures.  Layer side only.  The rendering engine doesn't use them.
    void addSubTexture(const SubTexture &);
    void addSubTextures(const std::vector<SubTexture> &);
    
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
    
    /// Add an active model.  Only call this on the main thread.
    void addActiveModel(NSObject<WhirlyKitActiveModel> *);
    
    /// Remove an active model (if it's in here).  Only call this on the main thread.
    void removeActiveModel(NSObject<WhirlyKitActiveModel> *);
    
    /// Return the top level cullable
    CullTree *getCullTree() { return cullTree; }
    
    /// Get the renderer's buffer/texture ID manager.
    /// You can use this on any thread.  The calls are protected.
    OpenGLMemManager *getMemManager() { return &memManager; }
	
    /// Dump out stats on what is currently in the scene.
    /// Use this sparingly, as it writes to the log.
    void dumpStats();
	
public:
    /// The coordinate system display adapter converts from the local space
    ///  to display coordinates.
    WhirlyKit::CoordSystemDisplayAdapter *coordAdapter;
    
    /// Look for a Draw Generator by ID
    Generator *getGenerator(SimpleIdentity genId);
	
	/// Look for a Drawable by ID
	DrawableRef getDrawable(SimpleIdentity drawId);
	
	/// Look for a Texture by ID
	Texture *getTexture(SimpleIdentity texId);
    
    /// All the active models
    NSMutableArray *activeModels;
    
    /// All the drawable generators we've been handed, sorted by ID
    GeneratorSet generators;

    /// Top level of Cullable quad tree
    CullTree *cullTree;
	
	typedef std::set<DrawableRef,IdentifiableRefSorter> DrawableRefSet;
	/// All the drawables we've been handed, sorted by ID
	DrawableRefSet drawables;
	
	typedef std::set<Texture *,IdentifiableSorter> TextureSet;
	/// Textures, sorted by ID
	TextureSet textures;
	
	pthread_mutex_t changeRequestLock;
	/// We keep a list of change requests to execute
	/// This can be accessed in multiple threads, so we lock it
	std::vector<ChangeRequest *> changeRequests;
    
    pthread_mutex_t subTexLock;
    typedef std::set<SubTexture> SubTextureSet;
    /// Mappings from images to parts of texture atlases
    SubTextureSet subTextureMap;
    
    /// ID for screen space generator
    SimpleIdentity screenSpaceGeneratorID;
    
    /// Memory manager, really buffer and texture ID manager
    OpenGLMemManager memManager;
    
    /// Screen space generator created on startup
    ScreenSpaceGenerator *ssGen;
    
    /// UIView placement generator created on startup
    ViewPlacementGenerator *vpGen;
    
    /// Search for a shader program by ID (our ID, not OpenGL's)
    OpenGLES2Program *getProgram(SimpleIdentity programId);
    
    /// Search for a shader program by name
    OpenGLES2Program *getProgram(const std::string &name);
    
    /// Add a shader to the mix (don't be calling this yourself).
    /// Scene is responsible for deletion.
    void addProgram(OpenGLES2Program *);
    
    /// Remove a program (by ID)
    void removeProgram(SimpleIdentity programId);
    
    /// Called during initialization after the default shader is created.
    /// Scene is responsible for deletion
    void setDefaultPrograms(OpenGLES2Program *tri,OpenGLES2Program *line);
    
    /// Get the IDs for the default programs
    void getDefaultProgramIDs(SimpleIdentity &triShader,SimpleIdentity &lineShader);
        
protected:
    /// Only the subclasses are allowed to create these
    Scene();

	/// Construct with the depth of the cullable quad tree,
    ///  the coordinate system we're using, and the MBR of the
    ///  top level.
    /// The earth will be recursively divided into a quad tree of given depth.
    /// Init call used by the base class to set things up
    void Init(WhirlyKit::CoordSystemDisplayAdapter *adapter,Mbr localMbr,unsigned int depth);
    
    /// Keep track of the OpenGL ES 2.0 shader programs here
    std::set<OpenGLES2Program *,IdentifiableSorter> glPrograms;
    /// IDs for the default programs we'll use in drawables that don't have them
    SimpleIdentity defaultProgramTri,defaultProgramLine;
};
	
}
