/*
 *  Drawable.h
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
#import "RawData.h"
#import "Identifiable.h"
#import "StringIndexer.h"
#import "WhirlyVector.h"
#import "WhirlyKitView.h"
#import "ChangeRequest.h"

namespace WhirlyKit
{

class Drawable;
class RendererFrameInfo;
class Scene;
class SceneRenderer;

/** Drawable tweakers are called every frame to mess with things.
    It's up to you to make the changes, just make them quick.
  */
class DrawableTweaker : public Identifiable
{
public:
    virtual ~DrawableTweaker();
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
    EIGEN_MAKE_ALIGNED_OPERATOR_NEW;

    /// Construct empty
	Drawable(const std::string &name);
	virtual ~Drawable();

    /// We're allowed to turn drawables off completely
    virtual bool isOn(RendererFrameInfo *frameInfo) const = 0;

    /// Return the local MBR, if we're working in a non-geo coordinate system
    virtual Mbr getLocalMbr() const = 0;

	/// We use this to sort drawables
	virtual unsigned int getDrawPriority() const = 0;
    
    /// Return the Matrix if there is an active one (ideally not)
    virtual const Eigen::Matrix4d *getMatrix() const = 0;
    
    /// Check if the force Z buffer on mode is on
    virtual bool getRequestZBuffer() const = 0;

    /// Check if we're supposed to write to the z buffer
    virtual bool getWriteZbuffer() const = 0;
    
    /// Drawables can override where they're drawn.  EmptyIdentity is the regular screen.
    virtual SimpleIdentity getRenderTarget() const = 0;
    
    /// Update anything associated with the renderer.  Probably renderUntil.
    virtual void updateRenderer(SceneRenderer *renderer) = 0;
    
    /// Run the tweakers
    virtual void runTweakers(RendererFrameInfo *frame);
    
    /// Do any initialization you may want.
    /// For instance, set up VBOs.
    virtual void setupForRenderer(const RenderSetupInfo *setupInfo) = 0;
    
    /// Clean up any rendering objects you may have (e.g. VBOs).
    virtual void teardownForRenderer(const RenderSetupInfo *setupInfo,Scene *scene) = 0;

    /// If present, we'll do a pre-render calculation pass with this program set
    virtual SimpleIdentity getCalculationProgram() const = 0;
    
    /// For OpenGLES2, this is the program to use to render this drawable.
    virtual SimpleIdentity getProgram() const = 0;
    
    // Which workgroups this is in (might be in multiple if there's a calculation shader)
    SimpleIDSet workGroupIDs;

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
    EIGEN_MAKE_ALIGNED_OPERATOR_NEW;

    /// Construct with the ID of the Drawable we'll be changing
    DrawableChangeRequest(SimpleIdentity drawId) : drawId(drawId) { }
    virtual ~DrawableChangeRequest() { }
	
    /// This will look for the drawable by ID and then call execute2()
    void execute(Scene *scene,SceneRenderer *renderer,WhirlyKit::View *view);
	
    /// This is called by execute if there's a drawable to modify.
    /// This is the one you override.
    virtual void execute2(Scene *scene,SceneRenderer *renderer,DrawableRef draw) = 0;
	
protected:
    SimpleIdentity drawId;
};

/// Turn off visibility checking
static const float DrawVisibleInvalid = 1e10;
    
/// Maximum number of points we want in a drawable
static const unsigned int MaxDrawablePoints = ((1<<16)-1);
    
/// Maximum number of triangles we want in a drawable
static const unsigned int MaxDrawableTriangles = (MaxDrawablePoints / 3);

}
