/*
 *  Drawable.mm
 *  WhirlyGlobeLib
 *
 *  Created by Steve Gifford on 2/1/11.
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

#import "GLUtils.h"
#import "Drawable.h"
#import "GlobeScene.h"
#import "UIImage+Stuff.h"
#import "SceneRendererES.h"

using namespace Eigen;

@implementation WhirlyKitGLSetupInfo

- (id)init
{
    self = [super init];
    if (!self)
        return nil;
    minZres = 0.0;
    
    return self;
}

@end

using namespace WhirlyGlobe;

namespace WhirlyKit
{
    
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

    if (size != 0)
    {
        glBindBuffer(GL_ARRAY_BUFFER, which);
        CheckGLError("BasicDrawable::setupGL() glBindBuffer");
        glBufferData(GL_ARRAY_BUFFER, size, NULL, drawType);
        CheckGLError("BasicDrawable::setupGL() glBufferData");
        glBindBuffer(GL_ARRAY_BUFFER, 0);
        CheckGLError("BasicDrawable::setupGL() glBindBuffer");
    }
    pthread_mutex_unlock(&idLock);
    
    return which;
}

void OpenGLMemManager::removeBufferID(GLuint bufID)
{
    bool doClear = false;
    
    pthread_mutex_lock(&idLock);

    // Clear out the data to save memory (Note: not sure we need this)
    glBindBuffer(GL_ARRAY_BUFFER, bufID);
    glBufferData(GL_ARRAY_BUFFER, 0, NULL, GL_STATIC_DRAW);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    buffIDs.insert(bufID);
    
    if (buffIDs.size() > WhirlyKitOpenGLMemCacheMax)
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
        glDeleteBuffers(toRemove.size(), &toRemove[0]);
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
    
    if (texIDs.size() > WhirlyKitOpenGLMemCacheMax)
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
        glDeleteTextures(toRemove.size(), &toRemove[0]);
    texIDs.clear();
    
    pthread_mutex_unlock(&idLock);    
}

void OpenGLMemManager::dumpStats()
{
    NSLog(@"MemCache: %ld buffers",buffIDs.size());
    NSLog(@"MemCache: %ld textures",texIDs.size());
}
		
void OpenGLMemManager::lock()
{
    pthread_mutex_lock(&idLock);
}

void OpenGLMemManager::unlock()
{
    pthread_mutex_unlock(&idLock);    
}

		
Drawable::Drawable()
{
}
	
Drawable::~Drawable()
{
}
	
void DrawableChangeRequest::execute(Scene *scene,WhirlyKitSceneRendererES *renderer,WhirlyKitView *view)
{
	DrawableRef theDrawable = scene->getDrawable(drawId);
	if (theDrawable)
		execute2(scene,renderer,theDrawable);
}
	
BasicDrawable::BasicDrawable()
{
	on = true;
    programId = EmptyIdentity;
    usingBuffers = false;
    isAlpha = false;
    drawPriority = 0;
    drawOffset = 0;
	type = 0;
	texId = EmptyIdentity;
    minVisible = maxVisible = DrawVisibleInvalid;
    minVisibleFadeBand = maxVisibleFadeBand = 0.0;

    fadeDown = fadeUp = 0.0;
	color.r = color.g = color.b = color.a = 255;
    lineWidth = 1.0;
    
    numTris = 0;
    numPoints = 0;
    
    pointBuffer = colorBuffer = texCoordBuffer = normBuffer = triBuffer = 0;
    sharedBuffer = 0;
    vertexSize = 0;
    vertArrayObj = 0;
    sharedBufferIsExternal = false;
    forceZBufferOn = false;

    hasMatrix = false;
}
	
BasicDrawable::BasicDrawable(unsigned int numVert,unsigned int numTri)
{
	on = true;
    programId = EmptyIdentity;
    usingBuffers = false;
    isAlpha = false;
    drawPriority = 0;
    drawOffset = 0;
	points.reserve(numVert);
	texCoords.reserve(numVert);
	norms.reserve(numVert);
	tris.reserve(numTri);
    fadeDown = fadeUp = 0.0;
	color.r = color.g = color.b = color.a = 255;
    lineWidth = 1.0;
	drawPriority = 0;
	texId = EmptyIdentity;
    minVisible = maxVisible = DrawVisibleInvalid;
    minVisibleFadeBand = maxVisibleFadeBand = 0.0;
    forceZBufferOn = false;

    numTris = 0;
    numPoints = 0;
    
    pointBuffer = colorBuffer = texCoordBuffer = normBuffer = triBuffer = 0;
    sharedBuffer = 0;
    vertexSize = 0;
    vertArrayObj = 0;
    sharedBufferIsExternal = false;

    hasMatrix = false;
}
	
BasicDrawable::~BasicDrawable()
{
}
    
bool BasicDrawable::isOn(WhirlyKitRendererFrameInfo *frameInfo) const
{
    if (minVisible == DrawVisibleInvalid || !on)
        return on;

    float visVal = [frameInfo.theView heightAboveSurface];
    
    return ((minVisible <= visVal && visVal <= maxVisible) ||
             (maxVisible <= visVal && visVal <= minVisible));
}
    
bool BasicDrawable::hasAlpha(WhirlyKitRendererFrameInfo *frameInfo) const
{
    if (isAlpha)
        return true;
    
    if (fadeDown < fadeUp)
    {
        // Heading to 1
        if (frameInfo.currentTime < fadeDown)
            return false;
        else
            if (frameInfo.currentTime > fadeUp)
                return false;
            else
                return true;
    } else
        if (fadeUp < fadeDown)
        {
            // Heading to 0
            if (frameInfo.currentTime < fadeUp)
                return false;
            else
                if (frameInfo.currentTime > fadeDown)
                    return false;
                else
                    return true;
        }
    
    // Note: Need to move this elsewhere
    if ((minVisibleFadeBand != 0.0 || maxVisibleFadeBand != 0.0) &&
        [frameInfo.theView isKindOfClass:[WhirlyGlobeView class]])
    {
        WhirlyGlobeView *globeView = (WhirlyGlobeView *)frameInfo.theView;
        float height = globeView.heightAboveGlobe;
        if (height > minVisible && height < minVisible + minVisibleFadeBand)
        {
            return true;
        } else if (height > maxVisible - maxVisibleFadeBand && height < maxVisible)
        {
            return true;
        }
    }
    
    return false;
}

// If we're fading in or out, update the rendering window
void BasicDrawable::updateRenderer(WhirlyKitSceneRendererES *renderer)
{
    [renderer setRenderUntil:fadeUp];
    [renderer setRenderUntil:fadeDown];
    
    // Let's also pull the default shaders out if need be
    if (programId == EmptyIdentity)
    {
        SimpleIdentity triShaderId,lineShaderId;
        renderer.scene->getDefaultProgramIDs(triShaderId,lineShaderId);
        if (type == GL_LINE_LOOP || type == GL_LINES)
            programId = lineShaderId;
        else
            programId = triShaderId;
    }
}
    
// Widen a line and turn it into a rectangle of the given width
void BasicDrawable::addRect(const Point3f &l0, const Vector3f &nl0, const Point3f &l1, const Vector3f &nl1,float width)
{
	Vector3f dir = l1-l0;
	if (dir.isZero())
		return;
	dir.normalize();

	float width2 = width/2.0;
	Vector3f c0 = dir.cross(nl0);
	c0.normalize();
	
	Point3f pt[3];
	pt[0] = l0 + c0 * width2;
	pt[1] = l1 + c0 * width2;
	pt[2] = l1 - c0 * width2;
	pt[3] = l0 - c0 * width2;

	unsigned short ptIdx[4];
	for (unsigned int ii=0;ii<4;ii++)
	{
		ptIdx[ii] = addPoint(pt[ii]);
		addNormal(nl0);
	}
	
	addTriangle(Triangle(ptIdx[0],ptIdx[1],ptIdx[3]));
	addTriangle(Triangle(ptIdx[3],ptIdx[1],ptIdx[2]));
}

// Size of a single vertex in an interleaved buffer
// Note: We're resetting the buffers for no good reason
GLuint BasicDrawable::singleVertexSize()
{
    GLuint singleVertSize = 0;

    if (!points.empty())
    {
        pointBuffer = singleVertSize;
        singleVertSize += 3*sizeof(GLfloat);
    }
    if (!colors.empty())
    {
        colorBuffer = singleVertSize;
        singleVertSize += 4*sizeof(GLchar);
    }
    if (!texCoords.empty())
    {
        texCoordBuffer = singleVertSize;
        singleVertSize += 2*sizeof(GLfloat);
    }
    if (!norms.empty())
    {
        normBuffer = singleVertSize;
        singleVertSize += 3*sizeof(GLfloat);
    }
    
    return singleVertSize;
}

// Adds the basic vertex data to an interleaved vertex buffer
void BasicDrawable::addPointToBuffer(unsigned char *basePtr,int which)
{
    if (!points.empty())
        memcpy(basePtr+pointBuffer, &points[which], 3*sizeof(GLfloat));
    if (!colors.empty())
        memcpy(basePtr+colorBuffer, &colors[which], 4*sizeof(GLchar));
    if (!texCoords.empty())
        memcpy(basePtr+texCoordBuffer, &texCoords[which], 2*sizeof(GLfloat));
    if (!norms.empty())
        memcpy(basePtr+normBuffer, &norms[which], 3*sizeof(GLfloat));
}
    
void BasicDrawable::setupGL(WhirlyKitGLSetupInfo *setupInfo,OpenGLMemManager *memManager)
{
    setupGL(setupInfo,memManager,0,0);
}

// Create VBOs and such
void BasicDrawable::setupGL(WhirlyKitGLSetupInfo *setupInfo,OpenGLMemManager *memManager,GLuint externalSharedBuf,GLuint externalSharedBufOffset)
{
    // If we're already setup, don't do it twice
    if (pointBuffer || sharedBuffer)
        return;
    
	// Offset the geometry upward by minZres units along the normals
	// Only do this once, obviously
    // Note: Probably replace this with a shader program at some point
	if (drawOffset != 0 && (points.size() == norms.size()))
	{
		float scale = setupInfo->minZres*drawOffset;
		for (unsigned int ii=0;ii<points.size();ii++)
		{
			Vector3f pt = points[ii];
			points[ii] = norms[ii] * scale + pt;
		}
	}
	
	pointBuffer = texCoordBuffer = normBuffer = triBuffer = 0;
    sharedBuffer = 0;
    
    // We'll set up a single buffer for everything.
    // The other buffer pointers are now strides
    // Size of a single vertex entry
    vertexSize = singleVertexSize();
    int numVerts = points.size();
    
    // We're handed an external buffer, so just use it
    if (externalSharedBuf)
	{
        sharedBuffer = externalSharedBuf;
        sharedBufferOffset = externalSharedBufOffset;
    } else {
        // Set up the buffer
        int bufferSize = vertexSize*numVerts;
        if (!tris.empty())
        {
                bufferSize += tris.size()*sizeof(Triangle);
        }
        sharedBuffer = memManager->getBufferID(bufferSize,GL_STATIC_DRAW);
        sharedBufferOffset = 0;
	}
    
    // Now copy in the data
    glBindBuffer(GL_ARRAY_BUFFER, sharedBuffer);
    void *glMem = glMapBufferOES(GL_ARRAY_BUFFER, GL_WRITE_ONLY_OES);
    unsigned char *basePtr = (unsigned char *)glMem + sharedBufferOffset;
    for (unsigned int ii=0;ii<numVerts;ii++,basePtr+=vertexSize)
        addPointToBuffer(basePtr,ii);

    // And copy in the element buffer
	if (tris.size())
	{
        triBuffer = vertexSize*numVerts;
        unsigned char *basePtr = (unsigned char *)glMem + triBuffer + sharedBufferOffset;
        for (unsigned int ii=0;ii<tris.size();ii++,basePtr+=sizeof(Triangle))
            memcpy(basePtr, &tris[ii], sizeof(Triangle));
	}
    glUnmapBufferOES(GL_ARRAY_BUFFER);

    glBindBuffer(GL_ARRAY_BUFFER, 0);
    
    // Clear out the arrays, since we won't need them again
    numPoints = points.size();
    points.clear();
    texCoords.clear();
    norms.clear();
    numTris = tris.size();
    tris.clear();
    colors.clear();
    
    usingBuffers = true;
}
	
// Tear down the VBOs we set up
void BasicDrawable::teardownGL(OpenGLMemManager *memManager)
{
    if (sharedBuffer && !sharedBufferIsExternal)
    {
        memManager->removeBufferID(sharedBuffer);
        sharedBuffer = 0;
    } else {
	if (pointBuffer)
        memManager->removeBufferID(pointBuffer);
    if (colorBuffer)
        memManager->removeBufferID(colorBuffer);
	if (texCoordBuffer)
        memManager->removeBufferID(texCoordBuffer);
	if (normBuffer)
        memManager->removeBufferID(normBuffer);
	if (triBuffer)
        memManager->removeBufferID(triBuffer);
    }
    pointBuffer = 0;
    colorBuffer = 0;
    texCoordBuffer = 0;
    normBuffer = 0;
        triBuffer = 0;
    }
	
void BasicDrawable::draw(WhirlyKitRendererFrameInfo *frameInfo,Scene *scene)
{
    if (frameInfo.oglVersion == kEAGLRenderingAPIOpenGLES1)
    {
        if (usingBuffers)
            drawVBO(frameInfo,scene);
        else
            drawReg(frameInfo,scene);
    } else
        drawOGL2(frameInfo,scene);
}
        
// Used to pass in buffer offsets
#define CALCBUFOFF(base,off) ((char *)(base) + (off))


// VBO based drawing, OpenGL 1.1
void BasicDrawable::drawVBO(WhirlyKitRendererFrameInfo *frameInfo,Scene *scene)
{
	GLuint textureId = scene->getGLTexture(texId);
    
    // Note: This is slightly bogus
    if (!sharedBuffer)
        return;
	
	if (type == GL_TRIANGLES)
		glEnable(GL_LIGHTING);
	else
		glDisable(GL_LIGHTING);
    CheckGLError("BasicDrawable::drawVBO() lighting");
	
    if (!colorBuffer)
    {
        float timeScale = 1.0;
        if (fadeDown < fadeUp)
        {
            // Heading to 1
            if (frameInfo.currentTime < fadeDown)
                timeScale = 0.0;
            else
                if (frameInfo.currentTime > fadeUp)
                    timeScale = 1.0;
                else
                    timeScale = (frameInfo.currentTime - fadeDown)/(fadeUp - fadeDown);
        } else
            if (fadeUp < fadeDown)
            {
                // Heading to 0
                if (frameInfo.currentTime < fadeUp)
                    timeScale = 1.0;
                else
                    if (frameInfo.currentTime > fadeDown)
                        timeScale = 0.0;
                    else
                        timeScale = 1.0-(frameInfo.currentTime - fadeUp)/(fadeDown - fadeUp);
            }
        
        // Note: Need to move this elsewhere
        float rangeScale = 1.0;
        if ((minVisibleFadeBand != 0.0 || maxVisibleFadeBand != 0.0) &&
            [frameInfo.theView isKindOfClass:[WhirlyGlobeView class]])
        {
            WhirlyGlobeView *globeView = (WhirlyGlobeView *)frameInfo.theView;
            float height = globeView.heightAboveGlobe;
            if (height > minVisible && height < minVisible + minVisibleFadeBand)
            {
                rangeScale = (height-minVisible)/minVisibleFadeBand;
            } else if (height > maxVisible - maxVisibleFadeBand && height < maxVisible)
            {
                rangeScale = (height-(maxVisible-maxVisibleFadeBand))/maxVisibleFadeBand;
            }
        }

        float scale = timeScale * rangeScale;
        RGBAColor newColor = color;
        newColor.r = color.r * scale;
        newColor.g = color.g * scale;
        newColor.b = color.b * scale;
        newColor.a = color.a * scale;
        glColor4ub(newColor.r, newColor.g, newColor.b, newColor.a);
        CheckGLError("BasicDrawable::drawVBO() glColor4ub");
    }
    
    if (sharedBuffer)
    {
        glBindBuffer(GL_ARRAY_BUFFER, sharedBuffer);
        CheckGLError("BasicDrawable::drawVBO() shared glBindBuffer");
        
        glEnableClientState(GL_VERTEX_ARRAY);
        CheckGLError("BasicDrawable::drawVBO() glEnableClientState");
        glVertexPointer(3, GL_FLOAT, vertexSize, CALCBUFOFF(sharedBufferOffset,0));
        CheckGLError("BasicDrawable::drawVBO() glVertexPointer");
        
        glEnableClientState(GL_NORMAL_ARRAY);
        CheckGLError("BasicDrawable::drawVBO() glEnableClientState");
        glNormalPointer(GL_FLOAT, vertexSize, CALCBUFOFF(sharedBufferOffset,normBuffer));
        CheckGLError("BasicDrawable::drawVBO() glNormalPointer");
        
        if (colorBuffer)
        {
            glEnableClientState(GL_COLOR_ARRAY);
            CheckGLError("BasicDrawable::drawVBO() glEnableClientState");
            
            glColorPointer(4, GL_UNSIGNED_BYTE, vertexSize, CALCBUFOFF(sharedBufferOffset,colorBuffer));
            CheckGLError("BasicDrawable::drawVBO() glVertexPointer");
        }
        
        if (textureId)
        {
            glEnable(GL_TEXTURE_2D);
            CheckGLError("BasicDrawable::drawVBO() glEnable");
            glBindTexture(GL_TEXTURE_2D, textureId);
            CheckGLError("BasicDrawable::drawVBO() glBindTexture");

            glEnableClientState(GL_TEXTURE_COORD_ARRAY);
            CheckGLError("BasicDrawable::drawVBO() glEnableClientState");
            glTexCoordPointer(2, GL_FLOAT, vertexSize, CALCBUFOFF(sharedBufferOffset,texCoordBuffer));
            CheckGLError("BasicDrawable::drawVBO() glTexCoordPointer");
        }
    }
    
    if (!textureId && (type == GL_TRIANGLES))
    {
//        NSLog(@"No texture for: %lu",getId());
		glDisable(GL_TEXTURE_2D);
        CheckGLError("BasicDrawable::drawVBO() glDisable");
    }
	
	switch (type)
	{
		case GL_TRIANGLES:
		{
            glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, sharedBuffer);
            CheckGLError("BasicDrawable::drawVBO() glBindBuffer");
			glDrawElements(GL_TRIANGLES, numTris*3, GL_UNSIGNED_SHORT, CALCBUFOFF(sharedBufferOffset,triBuffer));
            CheckGLError("BasicDrawable::drawVBO() glDrawElements");
		}
			break;
		case GL_POINTS:
		case GL_LINES:
		case GL_LINE_STRIP:
		case GL_LINE_LOOP:
            glLineWidth(lineWidth);
			glDrawArrays(type, 0, numPoints);
            glLineWidth(1.0);
            CheckGLError("BasicDrawable::drawVBO() glDrawArrays");
			break;
	}
    
    if (colorBuffer)
    {
        glDisableClientState(GL_COLOR_ARRAY);
        CheckGLError("BasicDrawable::drawVBO() glDisableClientState");
    }
	
	if (textureId)
	{
		glDisable(GL_TEXTURE_2D);
        CheckGLError("BasicDrawable::drawVBO() glDisable");
		glDisableClientState(GL_TEXTURE_COORD_ARRAY);
        CheckGLError("BasicDrawable::drawVBO() glDisableClientState");
	}
	glDisableClientState(GL_VERTEX_ARRAY);
    CheckGLError("BasicDrawable::drawVBO() glDisableClientState");
	glDisableClientState(GL_NORMAL_ARRAY);
    CheckGLError("BasicDrawable::drawVBO() glDisableClientState");

	glBindBuffer(GL_ARRAY_BUFFER, 0);
    CheckGLError("BasicDrawable::drawVBO() glBindBuffer");
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
    CheckGLError("BasicDrawable::drawVBO() glBindBuffer");
	
	glDisable(GL_LIGHTING);
    CheckGLError("BasicDrawable::drawVBO() glDisable");
}

// Non-VBO based drawing, OpenGL 1.1
void BasicDrawable::drawReg(WhirlyKitRendererFrameInfo *frameInfo,Scene *scene)
{
	if (type == GL_TRIANGLES)
		glEnable(GL_LIGHTING);
	else
		glDisable(GL_LIGHTING);

	GLuint textureId = scene->getGLTexture(texId);
	
	if (textureId)
	{
		glEnable(GL_TEXTURE_2D);
		glEnableClientState(GL_TEXTURE_COORD_ARRAY);
	} else {
		glDisable(GL_TEXTURE_2D);
	}

	glEnableClientState(GL_VERTEX_ARRAY);
    if (!norms.empty())
        glEnableClientState(GL_NORMAL_ARRAY);
    if (!colors.empty())
        glEnableClientState(GL_COLOR_ARRAY);
	
	glVertexPointer(3, GL_FLOAT, 0, &points[0]);
    if (!norms.empty())
        glNormalPointer(GL_FLOAT, 0, &norms[0]);
    if (!colors.empty())
    {
        glColorPointer(4, GL_UNSIGNED_BYTE, 0, &colors[0]);
    }
	if (textureId)
	{
		glTexCoordPointer(2, GL_FLOAT, 0, &texCoords[0]);
		glBindTexture(GL_TEXTURE_2D, textureId);
	}
    if (colors.empty())
        glColor4ub(color.r, color.g, color.b, color.a);
	
	switch (type)
	{
		case GL_TRIANGLES:
			glDrawElements(GL_TRIANGLES, tris.size()*3, GL_UNSIGNED_SHORT, (unsigned short *)&tris[0]);
			break;
		case GL_POINTS:
		case GL_LINES:
		case GL_LINE_STRIP:
		case GL_LINE_LOOP:
            glLineWidth(lineWidth);
			glDrawArrays(type, 0, points.size());
            glLineWidth(1.0);
			break;
	}
	
	if (textureId)
	{
		glDisable(GL_TEXTURE_2D);
		glDisableClientState(GL_TEXTURE_COORD_ARRAY);
	}
	glDisableClientState(GL_VERTEX_ARRAY);
    if (!norms.empty())
        glDisableClientState(GL_NORMAL_ARRAY);
    if (!colors.empty())
        glDisableClientState(GL_COLOR_ARRAY);
    
    glDisable(GL_LIGHTING);
}
    
// Called once to set up a Vertex Array Object
void BasicDrawable::setupVAO(OpenGLES2Program *prog)
{
    const OpenGLESAttribute *vertAttr = prog->findAttribute("a_position");
    const OpenGLESAttribute *texAttr = prog->findAttribute("a_texCoord");
    bool hasTexCoords = (texCoordBuffer != 0 || !texCoords.empty());
    const OpenGLESAttribute *colorAttr = prog->findAttribute("a_color");
    bool hasColors = (colorBuffer != 0 || !colors.empty());
    const OpenGLESAttribute *normAttr = prog->findAttribute("a_normal");
    bool hasNormals = (normBuffer != 0 || !norms.empty());

    glGenVertexArraysOES(1, &vertArrayObj);
    glBindVertexArrayOES(vertArrayObj);
    
    // We're using a single buffer for all of our vertex attributes
    if (sharedBuffer)
    {
        glBindBuffer(GL_ARRAY_BUFFER,sharedBuffer);
        CheckGLError("BasicDrawable::drawVBO2() shared glBindBuffer");
    }
    
    // Vertex array
    if (vertAttr)
    {
        glVertexAttribPointer(vertAttr->index, 3, GL_FLOAT, GL_FALSE, vertexSize, CALCBUFOFF(sharedBufferOffset,0));
        glEnableVertexAttribArray ( vertAttr->index );
    }
    
    // Texture coordinates
    if (texAttr && hasTexCoords && texCoordBuffer)
    {
        glVertexAttribPointer(texAttr->index, 2, GL_FLOAT, GL_FALSE, vertexSize, CALCBUFOFF(sharedBufferOffset,texCoordBuffer));
        glEnableVertexAttribArray ( texAttr->index );
    }
    
    // Per vertex colors
    if (colorAttr && hasColors && colorBuffer)
    {
        glVertexAttribPointer(colorAttr->index, 4, GL_UNSIGNED_BYTE, GL_TRUE, vertexSize, CALCBUFOFF(sharedBufferOffset,colorBuffer));
        glEnableVertexAttribArray(colorAttr->index);
    }
    
    // Per vertex normals
    if (normAttr && hasNormals && normBuffer)
    {
        glVertexAttribPointer(normAttr->index, 3, GL_FLOAT, GL_FALSE, vertexSize, CALCBUFOFF(sharedBufferOffset,normBuffer));
        glEnableVertexAttribArray(normAttr->index);
    }

    // Bind the element array
    bool boundElements = false;
    if (type == GL_TRIANGLES && triBuffer)
    {
        boundElements = true;
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, sharedBuffer);
        CheckGLError("BasicDrawable::drawVBO2() glBindBuffer");
    }    
    
    glBindVertexArrayOES(0);

    // Let a subclass set up their own VAO state
    setupAdditionalVAO(prog,vertArrayObj);
    
    // Now tear down all that state
    if (vertAttr)
        glDisableVertexAttribArray(vertAttr->index);
    if (texAttr && hasTexCoords && texCoordBuffer)
        glDisableVertexAttribArray(texAttr->index);
    if (colorAttr && hasColors && colorBuffer)
        glDisableVertexAttribArray(colorAttr->index);
    if (normAttr && hasNormals && normBuffer)
        glDisableVertexAttribArray(normAttr->index);
    if (boundElements)
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
    if (sharedBuffer)
        glBindBuffer(GL_ARRAY_BUFFER, 0);
}
        
// Draw Vertex Buffer Objects, OpenGL 2.0
void BasicDrawable::drawOGL2(WhirlyKitRendererFrameInfo *frameInfo,Scene *scene)
{
    OpenGLES2Program *prog = frameInfo.program;

    // Figure out if we're fading in or out
    float fade = 1.0;
    if (fadeDown < fadeUp)
    {
        // Heading to 1
        if (frameInfo.currentTime < fadeDown)
            fade = 0.0;
        else
            if (frameInfo.currentTime > fadeUp)
                fade = 1.0;
            else
                fade = (frameInfo.currentTime - fadeDown)/(fadeUp - fadeDown);
    } else {
        if (fadeUp < fadeDown)
        {
            // Heading to 0
            if (frameInfo.currentTime < fadeUp)
                fade = 1.0;
            else
                if (frameInfo.currentTime > fadeDown)
                    fade = 0.0;
                else
                    fade = 1.0-(frameInfo.currentTime - fadeUp)/(fadeDown - fadeUp);
        }
    }
    
    // GL Texture ID
    GLuint glTexID = 0;
    if (texId != EmptyIdentity)
        glTexID = scene->getGLTexture(texId);
        
    // Model/View/Projection matrix
    prog->setUniform("u_mvpMatrix", frameInfo.mvpMat);
    
    // Fade is always mixed in
    prog->setUniform("u_fade", fade);
    
    // Let the shaders know if we even have a texture
    prog->setUniform("u_hasTexture", (glTexID != 0));
    
    // Texture
    const OpenGLESUniform *texUni = prog->findUniform("s_baseMap");
    bool hasTexture = glTexID != 0 && texUni;
    if (hasTexture)
    {
        glActiveTexture(GL_TEXTURE0);
        CheckGLError("BasicDrawable::drawVBO2() glActiveTexture");
        glBindTexture(GL_TEXTURE_2D, glTexID);
        CheckGLError("BasicDrawable::drawVBO2() glBindTexture");
        prog->setUniform("s_baseMap", 0);
        CheckGLError("BasicDrawable::drawVBO2() glUniform1i");
    }
    
    // If necessary, set up the VAO (once)
    if (vertArrayObj == 0 && sharedBuffer != 0)
        setupVAO(prog);

    // Figure out what we're using
    const OpenGLESAttribute *vertAttr = prog->findAttribute("a_position");
    const OpenGLESAttribute *texAttr = prog->findAttribute("a_texCoord");
    bool hasTexCoords = (texCoordBuffer != 0 || !texCoords.empty());
    const OpenGLESAttribute *colorAttr = prog->findAttribute("a_color");
    bool hasColors = (colorBuffer != 0 || !colors.empty());
    const OpenGLESAttribute *normAttr = prog->findAttribute("a_normal");
    bool hasNormals = (normBuffer != 0 || !norms.empty());
        
    // Vertex array
    bool usedLocalVertices = false;
    if (vertAttr && !(sharedBuffer || pointBuffer))
        {
        usedLocalVertices = true;
            glVertexAttribPointer(vertAttr->index, 3, GL_FLOAT, GL_FALSE, 0, &points[0]);
            glEnableVertexAttribArray ( vertAttr->index );            
        }
    
    // Texture coordinates
    bool usedLocalTexCoords = false;
    if (texAttr)
    {
        if (hasTexCoords)
        {
            if (!texCoordBuffer)
            {
                usedLocalTexCoords = true;
                glVertexAttribPointer(texAttr->index, 2, GL_FLOAT, GL_FALSE, 0, &texCoords[0]);
                glEnableVertexAttribArray ( texAttr->index );                
            }
        } else {
            glVertexAttrib2f(texAttr->index, 0.0, 0.0);
            CheckGLError("BasicDrawable::drawVBO2() glVertexAttrib2f");
        }
    }
    
    // Per vertex colors
    bool usedLocalColors = false;
    if (colorAttr)
    {
        if (hasColors)
        {
            if (!colorBuffer)
            {
                usedLocalColors = true;
                glVertexAttribPointer(colorAttr->index, 4, GL_UNSIGNED_BYTE, GL_TRUE, 0, &colors[0]);
                glEnableVertexAttribArray ( colorAttr->index );                
            }
        } else {
            glVertexAttrib4f(colorAttr->index, color.r / 255.0, color.g / 255.0, color.b / 255.0, color.a / 255.0);
            CheckGLError("BasicDrawable::drawVBO2() glVertexAttrib4f");
        }
    }
    
    // Per vertex normals
    bool usedLocalNorms = false;
    if (normAttr)
    {
        if (hasNormals)
        {
            if (!normBuffer)
            {
                usedLocalNorms = true;
                glVertexAttribPointer(normAttr->index, 3, GL_FLOAT, GL_FALSE, 0, &norms[0]);
                glEnableVertexAttribArray ( normAttr->index );                
            }
        } else {
            glVertexAttrib3f(normAttr->index, 1.0, 1.0, 1.0);
            CheckGLError("BasicDrawable::drawVBO2() glVertexAttrib3f");            
        }
    }

    // Let a subclass bind anything additional
    bindAdditionalRenderObjects(frameInfo,scene);
    
    // If we're using a vertex array object, bind it and draw
    if (vertArrayObj)
    {
        glBindVertexArrayOES(vertArrayObj);
        switch (type)
        {
            case GL_TRIANGLES:
                glDrawElements(GL_TRIANGLES, numTris*3, GL_UNSIGNED_SHORT, CALCBUFOFF(sharedBufferOffset,triBuffer));
                CheckGLError("BasicDrawable::drawVBO2() glDrawElements");
                break;
            case GL_POINTS:
            case GL_LINES:
            case GL_LINE_STRIP:
            case GL_LINE_LOOP:
                glLineWidth(lineWidth);
                glDrawArrays(type, 0, numPoints);
                glLineWidth(1.0);
                CheckGLError("BasicDrawable::drawVBO2() glDrawArrays");
                break;
        }
        glBindVertexArrayOES(0);
    } else {
        // Draw without a VAO
        switch (type)
        {
            case GL_TRIANGLES:
            {
                if (triBuffer)
                {
                    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, triBuffer);
                    CheckGLError("BasicDrawable::drawVBO2() glBindBuffer");
                    glDrawElements(GL_TRIANGLES, numTris*3, GL_UNSIGNED_SHORT, 0);
                    CheckGLError("BasicDrawable::drawVBO2() glDrawElements");
                    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
                } else {
                    glDrawElements(GL_TRIANGLES, tris.size()*3, GL_UNSIGNED_SHORT, &tris[0]);
                    CheckGLError("BasicDrawable::drawVBO2() glDrawElements");
                }
            }
                break;
            case GL_POINTS:
            case GL_LINES:
            case GL_LINE_STRIP:
            case GL_LINE_LOOP:
                glLineWidth(lineWidth);
                CheckGLError("BasicDrawable::drawVBO2() glLineWidth");
                glDrawArrays(type, 0, numPoints);
                CheckGLError("BasicDrawable::drawVBO2() glDrawArrays");
                glLineWidth(1.0);
                CheckGLError("BasicDrawable::drawVBO2() glDrawArrays");
                break;
        }        
    }
    
    // Unbind any texture
    if (hasTexture)
    {
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, 0);
    }

    // Tear down the various arrays, if we stood them up
    if (usedLocalNorms)
        glDisableVertexAttribArray(normAttr->index);
    if (usedLocalColors)
        glDisableVertexAttribArray(colorAttr->index);
    if (usedLocalTexCoords)
        glDisableVertexAttribArray(texAttr->index);
    if (usedLocalVertices)
        glDisableVertexAttribArray(vertAttr->index);

    // Let a subclass clean up any remaining state
    postDrawCallback(frameInfo,scene);
}

ColorChangeRequest::ColorChangeRequest(SimpleIdentity drawId,RGBAColor inColor)
	: DrawableChangeRequest(drawId)
{
	color[0] = inColor.r;
	color[1] = inColor.g;
	color[2] = inColor.b;
	color[3] = inColor.a;
}
	
void ColorChangeRequest::execute2(Scene *scene,WhirlyKitSceneRendererES *renderer,DrawableRef draw)
{
    BasicDrawableRef basicDrawable = boost::dynamic_pointer_cast<BasicDrawable>(draw);
	basicDrawable->setColor(color);
}
	
OnOffChangeRequest::OnOffChangeRequest(SimpleIdentity drawId,bool OnOff)
	: DrawableChangeRequest(drawId), newOnOff(OnOff)
{
	
}
	
void OnOffChangeRequest::execute2(Scene *scene,WhirlyKitSceneRendererES *renderer,DrawableRef draw)
{
    BasicDrawableRef basicDrawable = boost::dynamic_pointer_cast<BasicDrawable>(draw);
	basicDrawable->setOnOff(newOnOff);
}
    
VisibilityChangeRequest::VisibilityChangeRequest(SimpleIdentity drawId,float minVis,float maxVis)
    : DrawableChangeRequest(drawId), minVis(minVis), maxVis(maxVis)
{
}
    
void VisibilityChangeRequest::execute2(Scene *scene,WhirlyKitSceneRendererES *renderer,DrawableRef draw)
{
    BasicDrawableRef basicDrawable = boost::dynamic_pointer_cast<BasicDrawable>(draw);
    basicDrawable->setVisibleRange(minVis,maxVis);
}
    
FadeChangeRequest::FadeChangeRequest(SimpleIdentity drawId,NSTimeInterval fadeUp,NSTimeInterval fadeDown)
    : DrawableChangeRequest(drawId), fadeUp(fadeUp), fadeDown(fadeDown)
{
    
}
    
void FadeChangeRequest::execute2(Scene *scene,WhirlyKitSceneRendererES *renderer,DrawableRef draw)
{
    // Fade it out, then remove it
    BasicDrawableRef basicDrawable = boost::dynamic_pointer_cast<BasicDrawable>(draw);
    basicDrawable->setFade(fadeDown, fadeUp);
    
    // And let the renderer know
    [renderer setRenderUntil:fadeDown];
    [renderer setRenderUntil:fadeUp];
}

DrawTexChangeRequest::DrawTexChangeRequest(SimpleIdentity drawId,SimpleIdentity newTexId)
: DrawableChangeRequest(drawId), newTexId(newTexId)
{
}

void DrawTexChangeRequest::execute2(Scene *scene,WhirlyKitSceneRendererES *renderer,DrawableRef draw)
{
    BasicDrawableRef basicDrawable = boost::dynamic_pointer_cast<BasicDrawable>(draw);
    basicDrawable->setTexId(newTexId);
}

TransformChangeRequest::TransformChangeRequest(SimpleIdentity drawId,const Matrix4f *newMat)
    : DrawableChangeRequest(drawId), newMat(*newMat)
{
}

void TransformChangeRequest::execute2(Scene *scene,WhirlyKitSceneRendererES *renderer,DrawableRef draw)
{
    BasicDrawableRef basicDraw = boost::dynamic_pointer_cast<BasicDrawable>(draw);
    if (basicDraw.get())
        basicDraw->setMatrix(&newMat);
}
    
DrawPriorityChangeRequest::DrawPriorityChangeRequest(SimpleIdentity drawId,int drawPriority)
: DrawableChangeRequest(drawId), drawPriority(drawPriority)
{
}

void DrawPriorityChangeRequest::execute2(Scene *scene,WhirlyKitSceneRendererES *renderer,DrawableRef draw)
{
    BasicDrawableRef basicDrawable = boost::dynamic_pointer_cast<BasicDrawable>(draw);
    basicDrawable->setDrawPriority(drawPriority);
}

LineWidthChangeRequest::LineWidthChangeRequest(SimpleIdentity drawId,float lineWidth)
: DrawableChangeRequest(drawId), lineWidth(lineWidth)
{
}

void LineWidthChangeRequest::execute2(Scene *scene,WhirlyKitSceneRendererES *renderer,DrawableRef draw)
{
    BasicDrawableRef basicDrawable = boost::dynamic_pointer_cast<BasicDrawable>(draw);
    basicDrawable->setLineWidth(lineWidth);
}


}
