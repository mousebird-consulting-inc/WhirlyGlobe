/*
 *  BigDrawable.h
 *  WhirlyGlobeLib
 *
 *  Created by Steve Gifford on 3/6/13.
 *  Copyright 2011-2013 mousebird consulting
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

#import "Drawable.h"
#import "CoordSystem.h"

namespace WhirlyKit
{
    
class BigDrawableSwap;
    
/** The Big Drawable is a double buffered drawable we can use to make changes
    in one thread and have them reflected in the renderer without slowing
    the renderer down.
  */
class BigDrawable : public Drawable
{
public:
    /// Construct with a debuggign name and the total number of bytes
    BigDrawable(const std::string &name,int singleVertexSize,int singleElementSize,int numVertexBytes,int numElementBytes);
    ~BigDrawable();

    /// See if this big drawable can represent data in the given drawable.
    /// We check various modes (e.g. draw priority, z buffer on, etc)
    bool isCompatible(BasicDrawable *);
    
    /// Set the various drawing modes to be compatible with the given
    ///  drawable.
    void setModes(BasicDrawable *);

    /// No bounding box, since these change constantly
    Mbr getLocalMbr() const { return Mbr(); }

    /// Draw priority for ordering
    unsigned int getDrawPriority() const { return drawPriority; }
    void setDrawPriority(unsigned int newPriority) { drawPriority = newPriority; }
    
    /// Texture to use in drawing
    SimpleIdentity getTexId() const { return texId; }
    void setTexId(SimpleIdentity newTexId) { texId = newTexId; }

    /// Use the default OpenGL ES shader program
    SimpleIdentity getProgram() const { return EmptyIdentity; }

    /// Always on for the moment
    bool isOn(WhirlyKitRendererFrameInfo *frameInfo) const;

    /// Create our buffers in GL
    void setupGL(WhirlyKitGLSetupInfo *setupInfo,OpenGLMemManager *memManager);

    /// Destroy GL buffers
    void teardownGL(OpenGLMemManager *memManager);

    /// Called on the rendering thread to draw
    void draw(WhirlyKitRendererFrameInfo *frameInfo,Scene *scene);

    /// Just triangle strips for now
    GLenum getType() const { return GL_TRIANGLE_STRIP; }
    
    /// At the moment, no alpha
    bool hasAlpha(WhirlyKitRendererFrameInfo *frameInfo) const { return false; }
    
    /// For now, just using these with globe layers
    bool getForceZBufferOn() const { return forceZBuffer; }

    /// Set the visible range for all drawables we produce
    void setVisibleRange(float minVis,float maxVis);

    /// Don't need to update the renderer particularly
    void updateRenderer(WhirlyKitSceneRendererES *renderer);
    
    /// If set, we want the z buffer used
    bool getForceZBufferOn() { return forceZBuffer; }
    void setForceZBufferOn(bool enable) { forceZBuffer = enable; }

    /// Look for a region of the given size for the given data.
    /// This places the vertex data and adds the element data to the set
    ///  of element chunks we consolidate during a flush.
    SimpleIdentity addRegion(NSMutableData *vertData,int &vertPos,NSMutableData *elementData);
    
    /// Clear the region referred to by position and size (in bytes)
    void clearRegion(int vertPos,int vertSize,SimpleIdentity elementChunkId);
    
    /// Flush out changes to the inactive buffer and request a switch
    void swap(std::vector<ChangeRequest *> &changes,BigDrawableSwap *swapRequest);
    
    /// Return true if we're waiting on a buffer swap, but don't block
    bool isWaitingOnSwap();
    
    /// Check if there are outstanding changes in either buffer
    bool hasChanges();
    
    /// Return true if we're not representing anything
    bool empty();
    
    /// Only called by the renderer
    void swapBuffers(int whichBuffer);
    
    /// Run the changes in the non-active buffer (don't call this yourself)
    void executeFlush(int whichBuffer);
    
    /// Return which is the active buffer
    int getActiveBuffer() { return activeBuffer; }
    
protected:
    GLuint programId;
    SimpleIdentity texId;
    int drawPriority;
    bool forceZBuffer;
    float minVis,maxVis;
    
    typedef enum {ChangeAdd,ChangeClear} ChangeType;
    /// Used to represent an outstanding change to the buffer
    class Change
    {
    public:
        Change(ChangeType type,int whereVert,NSData *vertData,int clearLen=0);
        Change(const Change &that) : type(that.type), whereVert(that.whereVert), vertData(that.vertData), clearLen(that.clearLen) { }
        const Change & operator = (const Change &that) { type = that.type;  whereVert = that.whereVert; vertData = that.vertData;  clearLen = that.clearLen; return *this; }
        ~Change() { }
        
        // Type of the change we'll make
        ChangeType type;
        // Location (in bytes) in the vertex pool
        int whereVert;
        // For an add, the actual data
        NSData *vertData;
        // For a clear, amount of data to clear
        int clearLen;
    };
    typedef boost::shared_ptr<Change> ChangeRef;
    
    /// Used to represent a single buffer we can draw
    class Buffer
    {
    public:
        Buffer();
        // GL Id for the vertex buffer
        GLuint vertexBufferId;
        // GL Id for the element buffer
        GLuint elementBufferId;
        // Number of active elements to draw (starting from zero)
        int numElement;
        // Changes we need to make to this buffer at the next opportunity
        std::vector<ChangeRef> changes;
        // VAO we use for rendering
        GLuint vertexArrayObj;
    };
    
    int numVertexBytes,numElementBytes;
    // One of these buffers will be active at a time
    Buffer buffers[2];
    // Which buffer is currently active
    int activeBuffer;
    // Size of one vertex
    int singleVertexSize;
    // Size of one element
    int singleElementSize;
    
    pthread_cond_t useCondition;
    bool waitingOnSwap;
    pthread_mutex_t useMutex;
    
    // Free region within the vertex buffer
    class Region
    {
    public:
        Region(int pos,int len) : pos(pos), len(len) { }
        // Comparison operator
        bool operator < (const Region &that) const { return pos < that.pos; }
        // Position and length of a free buffer region
        int pos,len;
    };
    typedef std::set<Region> RegionSet;
        
    // Remove the given region from the given region set
    void removeRegion(RegionSet &regions,int pos,int size);
    
    RegionSet vertexRegions;

    // A chunk of renderable element data.
    // We consolidate these during a flush to for a coherent element buffer
    class ElementChunk : public Identifiable
    {
    public:
        ElementChunk(NSData *elementData) : elementData(elementData) { }
        ElementChunk(SimpleIdentity theId) : Identifiable(theId) { }
        NSData *elementData;
    };
    typedef std::set<ElementChunk> ElementChunkSet;
    
    // Total size of elements we already have
    int elementChunkSize;
    ElementChunkSet elementChunks;
};
            
typedef boost::shared_ptr<BigDrawable> BigDrawableRef;
            
/// Tell the renderer to swap buffers in a big drawable
class BigDrawableSwap : public ChangeRequest
{
public:
    /// Construct with the big drawable ID and the buffer to switch to
    BigDrawableSwap(NSObject * __weak target,SEL sel)
    : target(target), sel(sel) { }
    
    void addSwap(SimpleIdentity drawId,int whichBuffer)
    {
        swaps.push_back(SwapInfo(drawId,whichBuffer));
    }

    /// Run the swap.  Only the renderer calls this.
    void execute(Scene *scene,WhirlyKitSceneRendererES *renderer,WhirlyKitView *view);

    /// We'll want a flush on the layer thread side before we swap buffers on the render side
    virtual bool needsFlush() { return true; }

protected:
    NSObject * __weak target;
    SEL sel;
    class SwapInfo
    {
    public:
        SwapInfo(SimpleIdentity drawId,int whichBuffer) : drawId(drawId), whichBuffer(whichBuffer) { }
        SimpleIdentity drawId;
        int whichBuffer;
    };
    std::vector<SwapInfo> swaps;
};

/// Tell the main rendering thread to flush a given big drawable
class BigDrawableFlush : public ChangeRequest
{
public:
    BigDrawableFlush(SimpleIdentity drawId) : drawId(drawId) { }

    /// Run the flush.  The renderer calls this
    void execute(Scene *scene,WhirlyKitSceneRendererES *renderer,WhirlyKitView *view);
    
protected:
    SimpleIdentity drawId;
};

}
