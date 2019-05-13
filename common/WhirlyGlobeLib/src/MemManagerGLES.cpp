/*
 *  MemManagerGLES.cpp
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

#import "MemManagerGLES.h"
#import "SceneGLES.h"
#import "WrapperGLES.h"
#import "UtilsGLES.h"
#import "WhirlyKitLog.h"

namespace WhirlyKit
{
    
RenderSetupInfoGLES::RenderSetupInfoGLES()
    : memManager(NULL)
{
    minZres = 0.0;
    glesVersion = 3;
}
    
RenderSetupInfoGLES::RenderSetupInfoGLES(Scene *inScene)
{
    SceneGLES *scene = (SceneGLES *)inScene;
    
    minZres = 0.0;
    glesVersion = 3;
    memManager = scene->getMemManager();
}

OpenGLMemManager::OpenGLMemManager()
{
}

OpenGLMemManager::~OpenGLMemManager()
{
}

GLuint OpenGLMemManager::getBufferID(unsigned int size,GLenum drawType)
{
    GLuint which = 0;
    {
        std::lock_guard<std::mutex> guardLock(idLock);
        if (buffIDs.empty())
        {
            GLuint newAlloc[WhirlyKitOpenGLMemCacheAllocUnit];
            glGenBuffers(WhirlyKitOpenGLMemCacheAllocUnit, newAlloc);
            CheckGLError("OpenGLMemManager::getBufferID() glGenBuffers");
            for (unsigned int ii=0;ii<WhirlyKitOpenGLMemCacheAllocUnit;ii++)
            {
                buffIDs.insert(newAlloc[ii]);
                
                //                wkLogLevel(Debug,"Added buffer %d",newAlloc[ii]);
            }
        }
        
        if (!buffIDs.empty())
        {
            std::set<GLuint>::iterator it = buffIDs.begin();
            which = *it;
            buffIDs.erase(it);
        }
    }
    
    if (size != 0)
    {
        glBindBuffer(GL_ARRAY_BUFFER, which);
        CheckGLError("OpenGLMemManager::getBufferID() glBindBuffer");
        glBufferData(GL_ARRAY_BUFFER, size, NULL, drawType);
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
        glBufferData(GL_ARRAY_BUFFER, 0, NULL, GL_STATIC_DRAW);
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
    
    std::vector<GLuint> toRemove;
    toRemove.reserve(buffIDs.size());
    for (std::set<GLuint>::iterator it = buffIDs.begin();
         it != buffIDs.end(); ++it) {
        toRemove.push_back(*it);
        //        wkLogLevel(Debug,"Deleting buffer %d",*it);
    }
    if (!toRemove.empty())
        glDeleteBuffers((GLsizei)toRemove.size(), &toRemove[0]);
    buffIDs.clear();
}

GLuint OpenGLMemManager::getTexID()
{
    std::lock_guard<std::mutex> guardLock(idLock);
    
    if (texIDs.empty())
    {
        GLuint newAlloc[WhirlyKitOpenGLMemCacheAllocUnit];
        glGenTextures(WhirlyKitOpenGLMemCacheAllocUnit, newAlloc);
        for (unsigned int ii=0;ii<WhirlyKitOpenGLMemCacheAllocUnit;ii++)
            texIDs.insert(newAlloc[ii]);
    }
    
    GLuint which = 0;
    if (!texIDs.empty())
    {
        std::set<GLuint>::iterator it = texIDs.begin();
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
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, 0, 0, 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL);
    
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
    
    std::vector<GLuint> toRemove;
    toRemove.reserve(texIDs.size());
    for (std::set<GLuint>::iterator it = texIDs.begin();
         it != texIDs.end(); ++it)
        toRemove.push_back(*it);
    if (!toRemove.empty())
        glDeleteTextures((GLsizei)toRemove.size(), &toRemove[0]);
    texIDs.clear();
}

void OpenGLMemManager::dumpStats()
{
    wkLogLevel(Verbose,"MemCache: %ld buffers",(long int)buffIDs.size());
    wkLogLevel(Verbose,"MemCache: %ld textures",(long int)texIDs.size());
}

}
