/*
 *  Drawable.mm
 *  WhirlyGlobeLib
 *
 *  Created by Steve Gifford on 2/1/11.
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

#import "GLUtils.h"
#import "BasicDrawable.h"
#import "GlobeScene.h"
#import "SceneRendererES.h"
#import "TextureAtlas.h"

using namespace Eigen;

namespace WhirlyKit
{

WhirlyKitGLSetupInfo::WhirlyKitGLSetupInfo()
{
    minZres = 0.0;
}

OpenGLMemManager::OpenGLMemManager()
{
    pthread_mutex_init(&idLock,NULL);
}
    
OpenGLMemManager::~OpenGLMemManager()
{
    pthread_mutex_destroy(&idLock);
}
    
GLuint OpenGLMemManager::getBufferID(unsigned int size,GLenum drawType)
{
    pthread_mutex_lock(&idLock);
    
    if (buffIDs.empty())
    {
        GLuint newAlloc[WhirlyKitOpenGLMemCacheAllocUnit];
        glGenBuffers(WhirlyKitOpenGLMemCacheAllocUnit, newAlloc);
        for (unsigned int ii=0;ii<WhirlyKitOpenGLMemCacheAllocUnit;ii++)
        {
            buffIDs.insert(newAlloc[ii]);
    }
    }
    
    GLuint which = 0;
    if (!buffIDs.empty())
    {
        std::set<GLuint>::iterator it = buffIDs.begin();
        which = *it;
        buffIDs.erase(it);
    }
    pthread_mutex_unlock(&idLock);

    if (size != 0)
    {
        glBindBuffer(GL_ARRAY_BUFFER, which);
        CheckGLError("BasicDrawable::setupGL() glBindBuffer");
        glBufferData(GL_ARRAY_BUFFER, size, NULL, drawType);
        CheckGLError("BasicDrawable::setupGL() glBufferData");
        glBindBuffer(GL_ARRAY_BUFFER, 0);
        CheckGLError("BasicDrawable::setupGL() glBindBuffer");
    }
    
    return which;
}

// If set, we'll reuse buffers rather than allocating new ones
static const bool ReuseBuffers = true;

void OpenGLMemManager::removeBufferID(GLuint bufID)
{
    bool doClear = false;
    
    pthread_mutex_lock(&idLock);

    // Clear out the data to save memory (Note: not sure we need this)
//    glBindBuffer(GL_ARRAY_BUFFER, bufID);
//    glBufferData(GL_ARRAY_BUFFER, 0, NULL, GL_STATIC_DRAW);
//    glBindBuffer(GL_ARRAY_BUFFER, 0);
    buffIDs.insert(bufID);
    
    if (!ReuseBuffers || buffIDs.size() > WhirlyKitOpenGLMemCacheMax)
        doClear = true;

    pthread_mutex_unlock(&idLock);
    
    if (doClear)
        clearBufferIDs();
}

// Clear out any and all buffer IDs that we may have sitting around
void OpenGLMemManager::clearBufferIDs()
{
    pthread_mutex_lock(&idLock);
    
    std::vector<GLuint> toRemove;
    toRemove.reserve(buffIDs.size());
    for (std::set<GLuint>::iterator it = buffIDs.begin();
         it != buffIDs.end(); ++it)
        toRemove.push_back(*it);
    if (!toRemove.empty())
        glDeleteBuffers((GLsizei)toRemove.size(), &toRemove[0]);
    buffIDs.clear();
    
    pthread_mutex_unlock(&idLock);
}

GLuint OpenGLMemManager::getTexID()
{
    pthread_mutex_lock(&idLock);
    
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
    pthread_mutex_unlock(&idLock);
    
    return which;
}
    
void OpenGLMemManager::removeTexID(GLuint texID)
{
    bool doClear = false;
    
    pthread_mutex_lock(&idLock);

    // Clear out the texture data first
    glBindTexture(GL_TEXTURE_2D, texID);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, 0, 0, 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL);

    texIDs.insert(texID);
    
    if (!ReuseBuffers || texIDs.size() > WhirlyKitOpenGLMemCacheMax)
        doClear = true;

    pthread_mutex_unlock(&idLock);
    
    if (doClear)
        clearTextureIDs();
}

// Clear out any and all texture IDs that we have sitting around
void OpenGLMemManager::clearTextureIDs()
{
    pthread_mutex_lock(&idLock);
    
    std::vector<GLuint> toRemove;
    toRemove.reserve(texIDs.size());
    for (std::set<GLuint>::iterator it = texIDs.begin();
         it != texIDs.end(); ++it)
        toRemove.push_back(*it);
    if (!toRemove.empty())
        glDeleteTextures((GLsizei)toRemove.size(), &toRemove[0]);
    texIDs.clear();
    
    pthread_mutex_unlock(&idLock);    
}

void OpenGLMemManager::dumpStats()
{
    // Note: Porting
//    NSLog(@"MemCache: %ld buffers",buffIDs.size());
//    NSLog(@"MemCache: %ld textures",texIDs.size());
}
		
void OpenGLMemManager::lock()
{
    pthread_mutex_lock(&idLock);
}

void OpenGLMemManager::unlock()
{
    pthread_mutex_unlock(&idLock);
}

		
Drawable::Drawable(const std::string &name)
    : name(name)
{
}
	
Drawable::~Drawable()
{
}

void Drawable::runTweakers(RendererFrameInfo *frame)
{
    for (DrawableTweakerRefSet::iterator it = tweakers.begin();
         it != tweakers.end(); ++it)
        (*it)->tweakForFrame(this,frame);
}
	
void DrawableChangeRequest::execute(Scene *scene,WhirlyKit::SceneRendererES *renderer,WhirlyKit::View *view)
{
	DrawableRef theDrawable = scene->getDrawable(drawId);
	if (theDrawable)
		execute2(scene,renderer,theDrawable);
}
    
VertexAttribute::VertexAttribute(BDAttributeDataType dataType,const std::string &name)
    : dataType(dataType), name(name), data(NULL), buffer(0)
{
    defaultData.vec3[0] = 0.0;
    defaultData.vec3[1] = 0.0;
    defaultData.vec3[2] = 0.0;
    defaultData.vec4[3] = 0.0;
}
    
VertexAttribute::~VertexAttribute()
{
    clear();
}
    
VertexAttribute::VertexAttribute(const VertexAttribute &that)
    : dataType(that.dataType), name(that.name), data(NULL), buffer(that.buffer), defaultData(that.defaultData)
{
}
    
VertexAttribute VertexAttribute::templateCopy() const
{
    VertexAttribute newAttr(*this);
    return newAttr;
}
    
BDAttributeDataType VertexAttribute::getDataType() const
{
    return dataType;
}
    
void VertexAttribute::setDefaultColor(const RGBAColor &color)
{
    defaultData.color[0] = color.r;
    defaultData.color[1] = color.g;
    defaultData.color[2] = color.b;
    defaultData.color[3] = color.a;
}

void VertexAttribute::setDefaultVector2f(const Eigen::Vector2f &vec)
{
    defaultData.vec2[0] = vec.x();
    defaultData.vec2[1] = vec.y();
}

void VertexAttribute::setDefaultVector3f(const Eigen::Vector3f &vec)
{
    defaultData.vec3[0] = vec.x();
    defaultData.vec3[1] = vec.y();
    defaultData.vec3[2] = vec.z();
}

void VertexAttribute::addVector4f(const Eigen::Vector4f &vec)
{
    if (dataType != BDFloat4Type)
        return;
    
    if (!data)
        data = new std::vector<Vector4f>();
    std::vector<Vector4f> *vecs = (std::vector<Vector4f> *)data;
    (*vecs).push_back(vec);
}

void VertexAttribute::setDefaultFloat(float val)
{
    defaultData.floatVal = val;
}

void VertexAttribute::addColor(const RGBAColor &color)
{
    if (dataType != BDChar4Type)
        return;
    
    if (!data)
        data = new std::vector<RGBAColor>();
    std::vector<RGBAColor> *colors = (std::vector<RGBAColor> *)data;
    (*colors).push_back(color);
}

void VertexAttribute::addVector2f(const Eigen::Vector2f &vec)
{
    if (dataType != BDFloat2Type)
        return;
    
    if (!data)
        data = new std::vector<Vector2f>();
    std::vector<Vector2f> *vecs = (std::vector<Vector2f> *)data;
    (*vecs).push_back(vec);
}

void VertexAttribute::addVector3f(const Eigen::Vector3f &vec)
{
    if (dataType != BDFloat3Type)
        return;
    
    if (!data)
        data = new std::vector<Vector3f>();
    std::vector<Vector3f> *vecs = (std::vector<Vector3f> *)data;
    (*vecs).push_back(vec);
}

void VertexAttribute::addFloat(float val)
{
    if (dataType != BDFloatType)
        return;
    
    if (!data)
        data = new std::vector<float>();
    std::vector<float> *floats = (std::vector<float> *)data;
    (*floats).push_back(val);
}
    
void VertexAttribute::addInt(int val)
{
    if (dataType != BDIntType)
        return;
    
    if (!data)
        data = new std::vector<int>();
    std::vector<int> *ints = (std::vector<int> *)data;
    (*ints).push_back(val);
}
    
/// Reserve size in the data array
void VertexAttribute::reserve(int size)
{
    switch (dataType)
    {
        case BDFloat4Type:
        {
            if (!data)
                data = new std::vector<Vector4f>();
            std::vector<Vector4f> *vecs = (std::vector<Vector4f> *)data;
            vecs->reserve(size);
        }
            break;
        case BDFloat3Type:
        {
            if (!data)
                data = new std::vector<Vector3f>();
            std::vector<Vector3f> *vecs = (std::vector<Vector3f> *)data;
            vecs->reserve(size);
        }
            break;
        case BDFloat2Type:
        {
            if (!data)
                data = new std::vector<Vector2f>();
            std::vector<Vector2f> *vecs = (std::vector<Vector2f> *)data;
            vecs->reserve(size);
        }
            break;
        case BDChar4Type:
        {
            if (!data)
                data = new std::vector<RGBAColor>();
            std::vector<RGBAColor> *colors = (std::vector<RGBAColor> *)data;
            colors->reserve(size);
        }
            break;
        case BDFloatType:
        {
            if (!data)
                data = new std::vector<float>();
            std::vector<float> *floats = (std::vector<float> *)data;
            floats->reserve(size);
        }
            break;
        case BDIntType:
        {
            if (!data)
                data = new std::vector<int>();
            std::vector<int> *ints = (std::vector<int> *)data;
            ints->reserve(size);
        }
            break;
    }
}

/// Number of elements in our array
int VertexAttribute::numElements() const
{
    if (!data)
        return 0;
    
    switch (dataType)
    {
        case BDFloat4Type:
        {
            std::vector<Vector4f> *vecs = (std::vector<Vector4f> *)data;
            return (int)vecs->size();
        }
            break;
        case BDFloat3Type:
        {
            std::vector<Vector3f> *vecs = (std::vector<Vector3f> *)data;
            return (int)vecs->size();
        }
            break;
        case BDFloat2Type:
        {
            std::vector<Vector2f> *vecs = (std::vector<Vector2f> *)data;
            return (int)vecs->size();
        }
            break;
        case BDChar4Type:
        {
            std::vector<RGBAColor> *colors = (std::vector<RGBAColor> *)data;
            return (int)colors->size();
        }
            break;
        case BDFloatType:
        {
            std::vector<float> *floats = (std::vector<float> *)data;
            return (int)floats->size();
        }
            break;
        case BDIntType:
        {
            std::vector<int> *ints = (std::vector<int> *)data;
            return ints->size();
        }
    }
}

/// Return the size of a single element
int VertexAttribute::size() const
{
    switch (dataType)
    {
        case BDFloat4Type:
            return sizeof(GLfloat)*4;
            break;
        case BDFloat3Type:
            return sizeof(GLfloat)*3;
            break;
        case BDFloat2Type:
            return sizeof(GLfloat)*2;
            break;
        case BDChar4Type:
            return sizeof(unsigned char)*4;
            break;
        case BDFloatType:
            return sizeof(GLfloat);
            break;
        case BDIntType:
            return sizeof(GLint);
            break;
    }
}
    
int SingleVertexAttributeInfo::size() const
{
    switch (type)
    {
        case BDFloat4Type:
            return sizeof(GLfloat)*4;
            break;
        case BDFloat3Type:
            return sizeof(GLfloat)*3;
            break;
        case BDFloat2Type:
            return sizeof(GLfloat)*2;
            break;
        case BDChar4Type:
            return sizeof(unsigned char)*4;
            break;
        case BDFloatType:
            return sizeof(GLfloat);
            break;
        case BDIntType:
            return sizeof(GLint);
            break;
    }
}

/// Clean out the data array
void VertexAttribute::clear()
{
    if (data)
    {
        switch (dataType)
        {
            case BDFloat4Type:
            {
                std::vector<Vector4f> *vecs = (std::vector<Vector4f> *)data;
                delete vecs;
            }
                break;
            case BDFloat3Type:
            {
                std::vector<Vector3f> *vecs = (std::vector<Vector3f> *)data;
                delete vecs;
            }
                break;
            case BDFloat2Type:
            {
                std::vector<Vector2f> *vecs = (std::vector<Vector2f> *)data;
                delete vecs;
            }
                break;
            case BDChar4Type:
            {
                std::vector<RGBAColor> *colors = (std::vector<RGBAColor> *)data;
                delete colors;
            }
                break;
            case BDFloatType:
            {
                std::vector<float> *floats = (std::vector<float> *)data;
                delete floats;
            }
                break;
            case BDIntType:
            {
                std::vector<int> *ints = (std::vector<int> *)data;
                delete ints;
            }
                break;
        }
    }
    data = NULL;    
}

/// Return a pointer to the given element
void *VertexAttribute::addressForElement(int which)
{
    switch (dataType)
    {
        case BDFloat4Type:
        {
            std::vector<Vector4f> *vecs = (std::vector<Vector4f> *)data;
            return &(*vecs)[which];;
        }
            break;
        case BDFloat3Type:
        {
            std::vector<Vector3f> *vecs = (std::vector<Vector3f> *)data;
            return &(*vecs)[which];;
        }
            break;
        case BDFloat2Type:
        {
            std::vector<Vector2f> *vecs = (std::vector<Vector2f> *)data;
            return &(*vecs)[which];
        }
            break;
        case BDChar4Type:
        {
            std::vector<RGBAColor> *colors = (std::vector<RGBAColor> *)data;
            return &(*colors)[which];
        }
            break;
        case BDFloatType:
        {
            std::vector<float> *floats = (std::vector<float> *)data;
            return &(*floats)[which];
        }
            break;
        case BDIntType:
        {
            std::vector<int> *ints = (std::vector<int> *)data;
            return &(*ints)[which];
        }
            break;
    }
    
    return NULL;
}

/// Return the number of components as needed by glVertexAttribPointer
GLuint VertexAttribute::glEntryComponents() const
{
    switch (dataType)
    {
        case BDFloat4Type:
            return 4;
            break;
        case BDFloat3Type:
            return 3;
            break;
        case BDFloat2Type:
            return 2;
            break;
        case BDChar4Type:
            return 4;
            break;
        case BDFloatType:
            return 1;
            break;
        case BDIntType:
            return 1;
            break;
    }
    
    return 0;
}
    
/// Return the number of components as needed by glVertexAttribPointer
GLuint SingleVertexAttributeInfo::glEntryComponents() const
{
    switch (type)
    {
        case BDFloat4Type:
            return 4;
            break;
        case BDFloat3Type:
            return 3;
            break;
        case BDFloat2Type:
            return 2;
            break;
        case BDChar4Type:
            return 4;
            break;
        case BDFloatType:
            return 1;
            break;
        case BDIntType:
            return 1;
            break;
    }
    
    return 0;
}

/// Return the data type as required by glVertexAttribPointer
GLenum VertexAttribute::glType() const
{
    switch (dataType)
    {
        case BDFloat4Type:
        case BDFloat3Type:
        case BDFloat2Type:
        case BDFloatType:
            return GL_FLOAT;
            break;
        case BDChar4Type:
            return GL_UNSIGNED_BYTE;
            break;
        case BDIntType:
            return GL_INT;
            break;
    }
    return GL_UNSIGNED_BYTE;
}
    
/// Return the data type as required by glVertexAttribPointer
GLenum SingleVertexAttributeInfo::glType() const
{
    switch (type)
    {
        case BDFloat4Type:
        case BDFloat3Type:
        case BDFloat2Type:
        case BDFloatType:
            return GL_FLOAT;
            break;
        case BDChar4Type:
            return GL_UNSIGNED_BYTE;
            break;
        case BDIntType:
            return GL_INT;
            break;
    }
    return GL_UNSIGNED_BYTE;
}

/// Whether or not glVertexAttribPointer will normalize the data
GLboolean VertexAttribute::glNormalize() const
{
    switch (dataType)
    {
        case BDFloat4Type:
        case BDFloat3Type:
        case BDFloat2Type:
        case BDFloatType:
        case BDIntType:
            return GL_FALSE;
            break;
        case BDChar4Type:
            return GL_TRUE;
            break;
    }
}
    
/// Whether or not glVertexAttribPointer will normalize the data
GLboolean SingleVertexAttributeInfo::glNormalize() const
{
    switch (type)
    {
        case BDFloat4Type:
        case BDFloat3Type:
        case BDFloat2Type:
        case BDFloatType:
        case BDIntType:
            return GL_FALSE;
            break;
        case BDChar4Type:
            return GL_TRUE;
            break;
    }
}
    
void VertexAttribute::glSetDefault(int index) const
{
    switch (dataType)
    {
        case BDFloat4Type:
            glVertexAttrib4f(index, defaultData.vec4[0], defaultData.vec4[1], defaultData.vec4[2], defaultData.vec4[3]);
            break;
        case BDFloat3Type:
            glVertexAttrib3f(index, defaultData.vec3[0], defaultData.vec3[1], defaultData.vec3[2]);
            break;
        case BDFloat2Type:
            glVertexAttrib2f(index, defaultData.vec2[0], defaultData.vec2[1]);
            break;
        case BDFloatType:
            glVertexAttrib1f(index, defaultData.floatVal);
            break;
        case BDChar4Type:
            glVertexAttrib4f(index, defaultData.color[0] / 255.0, defaultData.color[1] / 255.0, defaultData.color[2] / 255.0, defaultData.color[3] / 255.0);
            break;
        case BDIntType:
            glVertexAttrib1f(index, defaultData.intVal);
            break;
    }
}
    
void VertexAttributeSetConvert(const SingleVertexAttributeSet &attrSet,SingleVertexAttributeInfoSet &infoSet)
{
    for (auto it : attrSet)
        infoSet.insert(it);
}

bool VertexAttributesAreCompatible(const SingleVertexAttributeInfoSet &infoSet,const SingleVertexAttributeSet &attrSet)
{
    if (infoSet.size() != attrSet.size())
        return false;
    
    auto itA = infoSet.begin();
    auto itB = attrSet.begin();
    for (;itA != infoSet.end(); ++itA, ++itB)
    {
        if (itA->name != itB->name)
            return false;
        if (itA->type != itB->type)
            return false;
    }
    
    return true;
}
    
}
