/*
 *  DynamicDrawableAtlas.h
 *  WhirlyGlobeLib
 *
 *  Created by Steve Gifford on 3/6/13.
 *  Copyright 2011-2016 mousebird consulting
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

#import "BigDrawable.h"

namespace WhirlyKit
{

/** The dynamic drawable atlas is used to consolidate drawables into a small
    set of big drawables that can be rendered quickly.
  */
class DynamicDrawableAtlas
{
public:
    /// Construct with a name (for debugging), individual vertex size, and
    ///  number of bytes for each big drawable
    DynamicDrawableAtlas(const std::string &name,int singleElementSize,int numVertexBytes,int numElementBytes,OpenGLMemManager *memManager,BigDrawable *(*newBigDrawable)(BasicDrawable *draw,int singleElementSize,int numVertexBytes,int numElementBytes) = NULL,SimpleIdentity shaderId = EmptyIdentity);
    ~DynamicDrawableAtlas();
    
    /// Add the given drawable to the drawable atlas.
    /// Returns true on success.  Reference the drawable by its ID.
    bool addDrawable(BasicDrawable *draw,ChangeSet &changes,bool enabled=true,std::vector<SimpleIdentity> *destTexIDs=NULL,bool *addedBigDrawable=NULL,const Point3d *center=NULL,double drawSize=0.0);
    
    /// Set the fade (but not on the drawables)
    void setFade(float inFade) { fade = inFade; }
    
    /// Remove the data for a drawable by ID
    bool removeDrawable(SimpleIdentity drawId,ChangeSet &changes);
    
    /// Enable/disable a drawable we're representing
    void setEnableDrawable(SimpleIdentity drawId,bool enabled);
    
    /// Enable/disable all the big drawables we're using
    void setEnableAllDrawables(bool enabled,ChangeSet &changes);
    
    /// Change the fade for all big drawables
    void setFadeAllDrawables(float newFade,ChangeSet &changes);
    
    /// Change the draw priority of all the drawables we're using
    void setDrawPriorityAllDrawables(int drawPriority,ChangeSet &changes);
    
    /// Change the program ID for all the drawables we're using
    void setProgramIDAllDrawables(SimpleIdentity programID,ChangeSet &changes);
        
    /// Used to track the remappings we need from one set of textures to another
    class DrawTexInfo
    {
    public:
        DrawTexInfo(SimpleIdentity drawId,SimpleIdentity baseTexId) : drawId(drawId), baseTexId(baseTexId) { }
        SimpleIdentity drawId;
        SimpleIdentity baseTexId;
    };

    /// Get a list of the active drawables and their texture IDs.
    /// We need this for remapping things later
    void getDrawableTextures(std::vector<DrawTexInfo> &remaps);
    
    /// Return the list of current drawables by ID
    void getDrawableIDs(SimpleIDSet &drawIDs);
        
    /// Check if there are any active updates in any of the drawable buffers
    bool hasUpdates();
    
    /// Clear the top level update flag.  This is for additions and deletions
    ///  of dynamic drawables.  The drawables themselves may still have changes.
    void clearUpdateFlag();
    
    /// Flush out any outstanding changes and swap the drawables
    /// Pass in a target and selector to pass through to the main thread.
    /// This will be called when one or more parts of the flush have done their
    ///  thing on the main thread.  Use this to wake yourself up on another thread.
    void swap(ChangeSet &changes,BigDrawableSwap::SwapCallback *,void *swapData);
    
    /// Check if we're waiting on an active drawable buffer swap
    bool waitingOnSwap();
    
    /// Add changes to be executed with the next buffer swap.
    /// These are things like the removal of textures that we're using.
    void addSwapChanges(const ChangeSet &swapChanges);
    
    /// Remove anything associated with the drawable atlas
    void shutdown(ChangeSet &changes);
    
    /// Print some status info to the log
    void log();
    
protected:
    /// Used to track where a drawable wound up in a big drawable
    class DrawRepresent : public Identifiable
    {
    public:
        // Constructor for sorting
        DrawRepresent(SimpleIdentity theId) : Identifiable(theId), vertexPos(0), vertexSize(0), elementChunkId(EmptyIdentity), bigDrawId(EmptyIdentity) { }
        
        /// Which big drawable this is in
        SimpleIdentity bigDrawId;
        
        /// Position and size within the big drawable (in bytes)
        int vertexPos,vertexSize;
        SimpleIdentity elementChunkId;
    };

    bool hasChanges;
    bool enable;
    float fade;
    BigDrawable *(*newBigDrawable)(BasicDrawable *draw,int singleElementSize,int numVertexBytes,int numElementBytes);
    SimpleIdentity shaderId;
    OpenGLMemManager *memManager;
    std::string name;
    int singleVertexSize;
    int singleElementSize;
    int numVertexBytes,numElementBytes;
    // The vertex attributes we expect to see on a drawable
    std::vector<VertexAttribute> vertexAttributes;
    
    class BigDrawableInfo
    {
    public:
        BigDrawableInfo() { }
        BigDrawableInfo(SimpleIdentity baseTexId,BigDrawable *bigDraw) : baseTexId(baseTexId), bigDraw(bigDraw) { }
        BigDrawableInfo & operator = (const BigDrawableInfo &that) { baseTexId = that.baseTexId; bigDraw = that.bigDraw; return *this; }
        bool operator < (const BigDrawableInfo &that) const { return bigDraw->getId() > that.bigDraw->getId(); }
        // This is the texture ID we associated with the big drawable.
        SimpleIdentity baseTexId;
        BigDrawable *bigDraw;
    };
    typedef std::set<BigDrawableInfo> BigDrawableSet;
    BigDrawableSet bigDrawables;
    
    // Used to track where the individual drawables wound up
    typedef std::set<DrawRepresent> DrawRepresentSet;
    DrawRepresentSet drawables;
    
    // Changes to be swept out with the next swap
    ChangeSet swapChanges;
};
    
}

