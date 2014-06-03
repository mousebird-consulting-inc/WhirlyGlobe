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

#import "RawData.h"
#import "Drawable.h"
#import "CoordSystem.h"

namespace WhirlyKit
{
    
class BigDrawableSwap;
    
/** The Big Drawable is a double buffered drawable we can use to make changes
    in one thread and have them reflected in the visuals without slowing
    the renderer down.
  */
class BigDrawable : public Drawable
{
    friend class DynamicDrawableAtlas;
public:
    /// Construct with a debugging name, a compatible drawable, and the total number of bytes
    BigDrawable(const std::string &name,int singleVertexSize,const std::vector<VertexAttribute> &templateAttributes ,int singleElementSize,int numVertexBytes,int numElementBytes);
    virtual ~BigDrawable();

    /// See if this big drawable can represent data in the given drawable.
    /// We check various modes (e.g. draw priority, z buffer on, etc)
    bool isCompatible(BasicDrawable *,const Point3d *center,double objSize);
    
    /// Set the various drawing modes to be compatible with the given
    ///  drawable.
    void setModes(BasicDrawable *);

    /// No bounding box, since these change constantly
    Mbr getLocalMbr() const { return Mbr(); }

    /// Return an offset matrix, if we need one
    const Eigen::Matrix4d *getMatrix() const;
    
    /// Set the center (needs to be done right after creation)
    void setCenter(const Point3d &newCenter);
    
    /// Return the center if there is one
    const Point3d *getCenter() { if (center.x() == 0.0 && center.y() == 0.0 && center.z() == 0.0) return NULL;  return &center; }

    /// Draw priority for ordering
    unsigned int getDrawPriority() const { return drawPriority; }
    void setDrawPriority(int newPriority) { drawPriority = newPriority; }
    
    /// Set all the texture info at once
    void setTexInfo(const std::vector<BasicDrawable::TexInfo> &newTexInfo) { texInfo = newTexInfo; }
    
    /// Set the texture ID for a given entry
    void setTexID(unsigned int which,SimpleIdentity texId);
    
    /// Program to use for rendering
    virtual SimpleIdentity getProgram() const { return programId; }
    
    /// Set the shader program.  Empty (default) by default
    virtual void setProgram(SimpleIdentity newProgId) { programId = newProgId; }

    /// Whether it's currently displaying
    bool isOn(WhirlyKit::RendererFrameInfo *frameInfo) const;
    /// True to turn it on, false to turn it off
    void setOnOff(bool onOff);

    /// Create our buffers in GL
    void setupGL(WhirlyKitGLSetupInfo *setupInfo,OpenGLMemManager *memManager);

    /// Destroy GL buffers
    void teardownGL(OpenGLMemManager *memManager);

    /// Called on the rendering thread to draw
    void draw(WhirlyKit::RendererFrameInfo *frameInfo,Scene *scene);

    /// Just triangle strips for now
    GLenum getType() const { return GL_TRIANGLE_STRIP; }
    
    /// At the moment, no alpha
    bool hasAlpha(WhirlyKit::RendererFrameInfo *frameInfo) const { return false; }
    
    /// Don't need to update the renderer particularly
    void updateRenderer(WhirlyKit::SceneRendererES *renderer);
    
    /// If set, we want to use the z buffer
    bool getRequestZBuffer() const { return requestZBuffer; }
    void setRequestZBuffer(bool enable) { requestZBuffer = enable; }

    /// If set, we want to write to the z buffer
    bool getWriteZbuffer() const { return writeZBuffer; }
    void setWriteZbuffer(bool enable) { writeZBuffer = enable; }

    /// Look for a region of the given size for the given data.
    /// This places the vertex data and adds the element data to the set
    ///  of element chunks we consolidate during a flush.
    SimpleIdentity addRegion(RawDataRef vertData,int &vertPos,RawDataRef elementData,bool enabled);
    
    /// Enable/Disable a given region
    void setEnableRegion(SimpleIdentity elementChunkId,bool enabled);
    
    /// Clear the region referred to by position and size (in bytes)
    void clearRegion(int vertPos,int vertSize,SimpleIdentity elementChunkId);
    
    /// Flush out changes to the inactive buffer and request a switch
    void swap(ChangeSet &changes,BigDrawableSwap *swapRequest);
    
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
    
    /// Count the size of vertex and element buffers we're representing
    void getUtilization(int &vertSize,int &elSize);
    
protected:
    bool enable;
    GLuint programId;
    std::vector<BasicDrawable::TexInfo> texInfo;
    int drawPriority;
    bool requestZBuffer,writeZBuffer;
    Point3d center;
    Eigen::Matrix4d transMat;
    float minVis,maxVis,minVisibleFadeBand,maxVisibleFadeBand;
    
    // The vertex attributes we're representing in the buffers
    std::vector<VertexAttribute> vertexAttributes;
    
    /// Called when a new VAO is bound.  Set up your VAO-related state here.
    virtual void setupAdditionalVAO(OpenGLES2Program *prog,GLuint vertArrayObj) { }
    
    typedef enum {ChangeAdd,ChangeClear,ChangeElements} ChangeType;
    /// Used to represent an outstanding change to the buffer
    class Change
    {
    public:
        Change(ChangeType type,int whereVert,RawDataRef vertData,int clearLen=0);
        Change(const Change &that) : type(that.type), whereVert(that.whereVert), vertData(that.vertData), clearLen(that.clearLen) { }
        const Change & operator = (const Change &that) { type = that.type;  whereVert = that.whereVert; vertData = that.vertData;  clearLen = that.clearLen; return *this; }
        ~Change() { }
        
        // Type of the change we'll make
        ChangeType type;
        // Location (in bytes) in the vertex pool
        int whereVert;
        // For an add, the actual data
        RawDataRef vertData;
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
        ElementChunk(RawDataRef elementData) : elementData(elementData), enabled(true) { }
        ElementChunk(SimpleIdentity theId) : Identifiable(theId) { }
        RawDataRef elementData;
        bool enabled;
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
    // Called when the bid drawable has swapped (on the rendering thread)
    typedef void (SwapCallback)(BigDrawableSwap *swap,void *data);

    /// Construct with the big drawable ID and the buffer to switch to
    BigDrawableSwap(SwapCallback *swapCallback,void *swapData)
    : swapCallback(swapCallback), swapData(swapData) { }
    
    void addSwap(SimpleIdentity drawId,int whichBuffer)
    {
        swaps.push_back(SwapInfo(drawId,whichBuffer));
    }

    /// Run the swap.  Only the renderer calls this.
    void execute(Scene *scene,WhirlyKit::SceneRendererES *renderer,WhirlyKit::View *view);

    /// We'll want a flush on the layer thread side before we swap buffers on the render side
    virtual bool needsFlush() { return true; }

protected:
    SwapCallback *swapCallback;
    void *swapData;
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

    /// Run the command.  The renderer calls this
    void execute(Scene *scene,WhirlyKit::SceneRendererES *renderer,WhirlyKit::View *view);
    
protected:
    SimpleIdentity drawId;
};
            
/// Change the texture used by a drawable
class BigDrawableTexChangeRequest : public ChangeRequest
{
public:
    BigDrawableTexChangeRequest(SimpleIdentity drawId,unsigned int which,SimpleIdentity newTexId) : drawId(drawId), which(which), texId(newTexId) { }
    
    /// Run the command.  The renderer calls this
    void execute(Scene *scene,WhirlyKit::SceneRendererES *renderer,WhirlyKit::View *view);
    
protected:
    SimpleIdentity drawId;
    unsigned int which;
    SimpleIdentity texId;
};
    
/// Enable/disable a whole big drawable
class BigDrawableOnOffChangeRequest : public ChangeRequest
{
public:
    BigDrawableOnOffChangeRequest(SimpleIdentity drawId,bool enable) : drawId(drawId), enable(enable) { }

    /// Run the command.  The renderer calls this
    void execute(Scene *scene,WhirlyKit::SceneRendererES *renderer,WhirlyKit::View *view);

protected:
    SimpleIdentity drawId;
    bool enable;
};

/// Change the draw priority of a big drawable
class BigDrawableDrawPriorityChangeRequest : public ChangeRequest
{
public:
    BigDrawableDrawPriorityChangeRequest(SimpleIdentity drawId,int drawPriority) : drawId(drawId), drawPriority(drawPriority) { }
    
    /// Run the command.  The renderer calls this
    void execute(Scene *scene,WhirlyKit::SceneRendererES *renderer,WhirlyKit::View *view);
    
protected:
    SimpleIdentity drawId;
    int drawPriority;
};

/// Change the draw priority of a big drawable
class BigDrawableProgramIDChangeRequest : public ChangeRequest
{
public:
    BigDrawableProgramIDChangeRequest(SimpleIdentity drawId,SimpleIdentity programID) : drawId(drawId), programID(programID) { }
    
    /// Run the command.  The renderer calls this
    void execute(Scene *scene,WhirlyKit::SceneRendererES *renderer,WhirlyKit::View *view);
    
protected:
    SimpleIdentity drawId;
    SimpleIdentity programID;
};
    
}
