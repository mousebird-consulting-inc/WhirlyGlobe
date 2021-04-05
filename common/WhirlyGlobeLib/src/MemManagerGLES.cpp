/*  MemManagerGLES.cpp
 *  WhirlyGlobeLib
 *
 *  Created by Steve Gifford on 2/1/11.
 *  Copyright 2011-2021 mousebird consulting
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
 */

#import "MemManagerGLES.h"
#import "SceneGLES.h"
#import "WrapperGLES.h"
#import "UtilsGLES.h"
#import "WhirlyKitLog.h"

namespace WhirlyKit
{

RenderSetupInfoGLES::RenderSetupInfoGLES() :
    memManager(nullptr),
    minZres(0.0),
    glesVersion(3)
{
}
    
RenderSetupInfoGLES::RenderSetupInfoGLES(Scene *inScene) : RenderSetupInfoGLES()
{
    auto scene = (SceneGLES *)inScene;
    memManager = scene->getMemManager();
}

OpenGLMemManager::~OpenGLMemManager()
{
    std::unique_lock<std::mutex> lock(idLock, std::try_to_lock);
    if (!lock.owns_lock())
    {
        wkLogLevel(Error,"OpenGL Memory Manager destroyed while locked");
        assert(!"OpenGL Memory Manager destroyed while locked");
    }
}

GLuint OpenGLMemManager::getBufferID(unsigned int size,GLenum drawType)
{
    GLuint which = 0;
    {
        std::lock_guard<std::mutex> guardLock(idLock);
        if (buffIDs.empty())
        {
            constexpr auto count = WhirlyKitOpenGLMemCacheAllocUnit;
            GLuint newAlloc[count] = {0};
            glGenBuffers(count, newAlloc);
            if (CheckGLError("OpenGLMemManager::getBufferID() glGenBuffers"))
            {
                buffIDs.insert(&newAlloc[0], &newAlloc[count]);
            }
        }
        
        if (!buffIDs.empty())
        {
            auto it = buffIDs.begin();
            which = *it;
            buffIDs.erase(it);
        }
    }
    
    if (size != 0)
    {
        glBindBuffer(GL_ARRAY_BUFFER, which);
        CheckGLError("OpenGLMemManager::getBufferID() glBindBuffer");
        glBufferData(GL_ARRAY_BUFFER, size, nullptr, drawType);
        CheckGLError("OpenGLMemManager::getBufferID() glBufferData");
        glBindBuffer(GL_ARRAY_BUFFER, 0);
        CheckGLError("OpenGLMemManager::getBufferID() glBindBuffer");
    }
    
    //    wkLogLevel(Debug,"Returning buffer %d",which);
    
    return which;
}

// If set, we'll reuse buffers rather than allocating new ones
static const bool ReuseBuffers = true;

void OpenGLMemManager::removeBufferID(GLuint bufID)
{
    bool doClear = false;
    
    //    wkLogLevel(Debug,"Releasing buffer %d",bufID);
    {
        std::lock_guard<std::mutex> guardLock(idLock);
        
        // Clear out the data to save memory
        glBindBuffer(GL_ARRAY_BUFFER, bufID);
        glBufferData(GL_ARRAY_BUFFER, 0, nullptr, GL_STATIC_DRAW);
        glBindBuffer(GL_ARRAY_BUFFER, 0);
        buffIDs.insert(bufID);
        
        if (!ReuseBuffers || buffIDs.size() > WhirlyKitOpenGLMemCacheMax)
            doClear = true;
    }
    
    if (doClear)
        clearBufferIDs();
}

// Clear out any and all buffer IDs that we may have sitting around
void OpenGLMemManager::clearBufferIDs()
{
    std::lock_guard<std::mutex> guardLock(idLock);

    if (!buffIDs.empty())
    {
        const std::vector<GLuint> toRemove(buffIDs.begin(), buffIDs.end());
        glDeleteBuffers((GLsizei)toRemove.size(), &toRemove[0]);
        buffIDs.clear();
    }
}

GLuint OpenGLMemManager::getTexID()
{
    std::lock_guard<std::mutex> guardLock(idLock);
    
    if (texIDs.empty())
    {
        constexpr auto count = WhirlyKitOpenGLMemCacheAllocUnit;
        GLuint newAlloc[count] = {0};
        glGenTextures(count, newAlloc);
        if (CheckGLError("OpenGLMemManager::getTexID glGenTextures"))
        {
            texIDs.insert(&newAlloc[0], &newAlloc[count]);
        }
    }
    
    GLuint which = 0;
    if (!texIDs.empty())
    {
        auto it = texIDs.begin();
        which = *it;
        texIDs.erase(it);
    }
    
    return which;
}

void OpenGLMemManager::removeTexID(GLuint texID)
{
    bool doClear = false;

    // Clear out the texture data first
    glBindTexture(GL_TEXTURE_2D, texID);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, 0, 0, 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);

    {
        std::lock_guard<std::mutex> guardLock(idLock);
        texIDs.insert(texID);
        
        if (!ReuseBuffers || texIDs.size() > WhirlyKitOpenGLMemCacheMax)
            doClear = true;
    }
    
    if (doClear)
        clearTextureIDs();
}

// Clear out any and all texture IDs that we have sitting around
void OpenGLMemManager::clearTextureIDs()
{
    std::lock_guard<std::mutex> guardLock(idLock);

    if (!texIDs.empty())
    {
        const std::vector<GLuint> toRemove(texIDs.begin(), texIDs.end());
        glDeleteTextures((GLsizei)toRemove.size(), &toRemove[0]);
        texIDs.clear();
    }
}

void OpenGLMemManager::dumpStats()
{
    wkLogLevel(Verbose,"MemCache: %ld buffers",(long int)buffIDs.size());
    wkLogLevel(Verbose,"MemCache: %ld textures",(long int)texIDs.size());
}

}
