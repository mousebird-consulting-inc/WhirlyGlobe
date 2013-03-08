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
    
/** The Big Drawable is a double buffered drawable we can use to make changes
    in one thread and have them reflected in the renderer without slowing
    the renderer down.
  */
class BigDrawable : public Drawable
{
public:
    /// Construct with a debuggign name and the total number of bytes
    BigDrawable(const std::string &name,int vertexSize,int numBytes);
    ~BigDrawable();

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
    bool isOn(WhirlyKitRendererFrameInfo *frameInfo) const { return true; }

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
    bool getForceZBufferOn() const { return false; }

    /// Don't need to update the renderer particularly
    void updateRenderer(WhirlyKitSceneRendererES *renderer);
    
    /// If set, we want the z buffer used
    bool getForceZBufferOn() { return forceZBuffer; }
    void setForceZBufferOn(bool enable) { forceZBuffer = enable; }


    /// Look for a region of the given size for the given data.
    /// If there isn't one, return false.  If there is one, we'll
    ///  use it and return the position and size of the region (in bytes).
    bool addRegion(NSData *data,int &pos,int &size);
    
    /// Clear the region referred to by position and size (in bytes)
    void clearRegion(int pos,int size);
    
    /// Flush out changes to the inactive buffer and request a switch
    void flush(std::vector<ChangeRequest *> &changes);
    
    /// Only called by the renderer
    void swapBuffers(int whichBuffer);
    
protected:
    GLuint programId;
    SimpleIdentity texId;
    int drawPriority;
    bool forceZBuffer;
    
    typedef enum {ChangeAdd,ChangeClear} ChangeType;
    /// Used to represent an outstanding change to the buffer
    class Change
    {
    public:
        Change(ChangeType type,int where,int len,NSData *data=nil);
        
        // Type of the change we'll make
        ChangeType type;
        // Location (in bytes)
        int where;
        // How much data (in bytes)
        int len;
        // For an add, the actual data
        NSData *data;
    };
    
    /// Used to represent a single buffer we can draw
    class Buffer
    {
    public:
        Buffer();
        // GL Id for the buffer
        GLuint bufferId;
        // Number of active vertices to draw (starting from zero)
        int numVertex;
        // Changes we need to make to this buffer at the next opportunity
        std::vector<Change> changes;
    };
    
    int numBytes;
    // One of these buffers will be active at a time
    Buffer buffers[2];
    // Which buffer is currently active
    int activeBuffer;
    // Vertex sizes
    int vertexSize;
    
    pthread_cond_t useCondition;
    bool waitingOnSwap;
    pthread_mutex_t useMutex;
    
    // Buffer region
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
    RegionSet regions;
};
            
typedef boost::shared_ptr<BigDrawable> BigDrawableRef;
            
/// Tell the renderer to swap buffers in a big drawable
class BigDrawableSwap : public ChangeRequest
{
public:
    /// Construct with the big drawable ID and the buffer to switch to
    BigDrawableSwap(SimpleIdentity drawId,int whichBuffer) : drawId(drawId), whichBuffer(whichBuffer) { }

    /// Run the swap.  Only the renderer calls this.
    void execute(Scene *scene,WhirlyKitSceneRendererES *renderer,WhirlyKitView *view);

protected:
    SimpleIdentity drawId;
    int whichBuffer;
};

}
