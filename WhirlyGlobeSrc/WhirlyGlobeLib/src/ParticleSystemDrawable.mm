/*
 *  ParticleSystemDrawable.mm
 *  WhirlyGlobeLib
 *
 *  Created by Steve Gifford on 4/28/15.
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

#import "ParticleSystemDrawable.h"
#import "GLUtils.h"
#import "Drawable.h"
#import "GlobeScene.h"
#import "UIImage+Stuff.h"
#import "SceneRendererES.h"
#import "TextureAtlas.h"

namespace WhirlyKit
{

ParticleSystemDrawable::ParticleSystemDrawable(const std::string &name,const std::vector<SingleVertexAttributeInfo> &inVertAttrs,int numPoints)
    : Drawable(name), enable(true), numPoints(numPoints), vertexSize(0), programId(0), drawPriority(0), pointBuffer(0), vertArrayObj(0), requestZBuffer(false), writeZBuffer(false), minVis(0.0), maxVis(10000.0)
{
    for (auto attr : inVertAttrs)
    {
        vertexSize += attr.size();
        vertAttrs.push_back(attr);
    }
}
    
ParticleSystemDrawable::~ParticleSystemDrawable()
{
}
    
void ParticleSystemDrawable::setupGL(WhirlyKitGLSetupInfo *setupInfo,OpenGLMemManager *memManager)
{
    if (pointBuffer != 0)
        return;
    
    int totalBytes = vertexSize*numPoints;
    pointBuffer = memManager->getBufferID(totalBytes,GL_DYNAMIC_DRAW);
    
    // Zero it out to avoid warnings
    // Note: Don't actually have to do this
//    glBindBuffer(GL_ARRAY_BUFFER, pointBuffer);
//    void *glMem = NULL;
//    EAGLContext *context = [EAGLContext currentContext];
//    if (context.API < kEAGLRenderingAPIOpenGLES3)
//        glMem = glMapBufferOES(GL_ARRAY_BUFFER, GL_WRITE_ONLY_OES);
//    else
//        glMem = glMapBufferRange(GL_ARRAY_BUFFER, 0, totalBytes, GL_MAP_WRITE_BIT);
//    memset(glMem, 0, totalBytes);
//    if (context.API < kEAGLRenderingAPIOpenGLES3)
//        glUnmapBufferOES(GL_ARRAY_BUFFER);
//    else
//        glUnmapBuffer(GL_ARRAY_BUFFER);
//    glBindBuffer(GL_ARRAY_BUFFER, 0);
}

void ParticleSystemDrawable::teardownGL(OpenGLMemManager *memManager)
{
    if (pointBuffer)
        memManager->removeBufferID(pointBuffer);
    pointBuffer = 0;
}
    
void ParticleSystemDrawable::updateRenderer(WhirlyKitSceneRendererES *renderer)
{
    [renderer addContinuousRenderRequest:getId()];
}
    
void ParticleSystemDrawable::addAttributeData(const std::vector<AttributeData> &attrData)
{
    if (attrData.size() != vertAttrs.size())
        return;
    
    glBindBuffer(GL_ARRAY_BUFFER, pointBuffer);
    unsigned char *glMem = NULL;
    EAGLContext *context = [EAGLContext currentContext];
    if (context.API < kEAGLRenderingAPIOpenGLES3)
        glMem = (unsigned char *)glMapBufferOES(GL_ARRAY_BUFFER, GL_WRITE_ONLY_OES);
    else
        glMem = (unsigned char *)glMapBufferRange(GL_ARRAY_BUFFER, 0, vertexSize*numPoints, GL_MAP_WRITE_BIT);
    
    // Work through the attribute blocks
    int attrOffset = 0;
    for (unsigned int ai=0;ai<vertAttrs.size();ai++)
    {
        const AttributeData &thisAttrData = attrData[ai];
        SingleVertexAttributeInfo &attrInfo = vertAttrs[ai];
        int attrSize = attrInfo.size();
        unsigned char *rawAttrData = (unsigned char *)thisAttrData.data;
        unsigned char *ptr = glMem + attrOffset;
        // Copy into each vertex
        for (unsigned int ii=0;ii<numPoints;ii++)
        {
            memcpy(ptr, rawAttrData, attrSize);
            ptr += vertexSize;
            rawAttrData += attrSize;
        }
        
        attrOffset += attrSize;
    }
    
    if (context.API < kEAGLRenderingAPIOpenGLES3)
        glUnmapBufferOES(GL_ARRAY_BUFFER);
    else
        glUnmapBuffer(GL_ARRAY_BUFFER);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
}
    
void ParticleSystemDrawable::setupVAO(OpenGLES2Program *prog)
{
    glGenVertexArraysOES(1, &vertArrayObj);
    glBindVertexArrayOES(vertArrayObj);

    glBindBuffer(GL_ARRAY_BUFFER,pointBuffer);
    CheckGLError("ParticleSystemDrawable::setupVAO() shared glBindBuffer");

    // Bind the various attributes to their offsets
    int attrOffset = 0;
    for (SingleVertexAttributeInfo &attrInfo : vertAttrs)
    {
        int attrSize = attrInfo.size();
        
        const OpenGLESAttribute *thisAttr = prog->findAttribute(attrInfo.name);
        if (thisAttr)
        {
            glVertexAttribPointer(thisAttr->index, attrInfo.glEntryComponents(), attrInfo.glType(), attrInfo.glNormalize(), vertexSize, (const GLvoid *)attrOffset);
            glEnableVertexAttribArray(thisAttr->index);
        }
        
        attrOffset += attrSize;
    }
    
    glBindVertexArrayOES(0);

    // Tear down the state
    for (SingleVertexAttributeInfo &attrInfo : vertAttrs)
    {
        const OpenGLESAttribute *thisAttr = prog->findAttribute(attrInfo.name);
        if (thisAttr)
            glDisableVertexAttribArray(thisAttr->index);
    }
    glBindBuffer(GL_ARRAY_BUFFER, 0);
}

void ParticleSystemDrawable::draw(WhirlyKitRendererFrameInfo *frameInfo,Scene *scene)
{
    OpenGLES2Program *prog = frameInfo.program;
    
    // Model/View/Projection matrix
    prog->setUniform("u_mvpMatrix", frameInfo.mvpMat);
    prog->setUniform("u_mvMatrix", frameInfo.viewAndModelMat);
    prog->setUniform("u_mvNormalMatrix", frameInfo.viewModelNormalMat);
    prog->setUniform("u_mvpNormalMatrix", frameInfo.mvpNormalMat);
    prog->setUniform("u_pMatrix", frameInfo.projMat);
    
    // If this is present, the drawable wants to do something based where the viewer is looking
    prog->setUniform("u_eyeVec", frameInfo.fullEyeVec);
    
    // If necessary, set up the VAO (once)
    if (vertArrayObj == 0)
        setupVAO(prog);
    
    prog->setUniform("u_size", pointSize);
    
    // Note: Debugging
    glBindBuffer(GL_ARRAY_BUFFER, pointBuffer);
    unsigned char *glMem = NULL;
    EAGLContext *context = [EAGLContext currentContext];
    if (context.API < kEAGLRenderingAPIOpenGLES3)
        glMem = (unsigned char *)glMapBufferOES(GL_ARRAY_BUFFER, GL_WRITE_ONLY_OES);
    else
        glMem = (unsigned char *)glMapBufferRange(GL_ARRAY_BUFFER, 0, vertexSize*numPoints, GL_MAP_WRITE_BIT);

    if (context.API < kEAGLRenderingAPIOpenGLES3)
        glUnmapBufferOES(GL_ARRAY_BUFFER);
    else
        glUnmapBuffer(GL_ARRAY_BUFFER);
    glBindBuffer(GL_ARRAY_BUFFER, 0);

    // If we're using a vertex array object, bind it and draw
    glBindVertexArrayOES(vertArrayObj);
    glDrawArrays(GL_POINTS, 0, numPoints);
    CheckGLError("BasicDrawable::drawVBO2() glDrawArrays");

    glBindVertexArrayOES(0);
}
    
static const char *vertexShaderTri =
"uniform mat4  u_mvpMatrix;"
"uniform mat4  u_mvMatrix;"
"uniform mat4  u_mvNormalMatrix;"
"uniform float u_size;"
"uniform float u_time;"
""
"attribute vec3 a_position;"
"attribute vec4 a_color;"
"attribute vec3 a_dir;"
""
"varying vec4 v_color;"
""
"void main()"
"{"
"   v_color = a_color;"
"   vec3 normal = a_position;"
""
"   vec3 thePos = a_position;"
// Convert from model space into display space
"   vec4 pt = u_mvMatrix * vec4(thePos,1.0);"
"   pt /= pt.w;"
// Make sure the object is facing the user
"   vec4 testNorm = u_mvNormalMatrix * vec4(normal,0.0);"
"   float dot_res = dot(-pt.xyz,testNorm.xyz);"
// Set the point size
"   gl_PointSize = u_size;"
// Project the point into 3-space
"   gl_Position = (dot_res > 0.0) ? u_mvpMatrix * vec4(thePos,1.0) : vec4(0.0,0.0,0.0,0.0);"
"}"
;

static const char *fragmentShaderTri =
"precision lowp float;"
""
"varying vec4      v_color;"
""
"void main()"
"{"
    "  gl_FragColor = v_color;"
"}"
;
    
OpenGLES2Program *BuildParticleSystemProgram()
{
    OpenGLES2Program *shader = new OpenGLES2Program(kParticleSystemShaderName,vertexShaderTri,fragmentShaderTri);
    if (!shader->isValid())
    {
        delete shader;
        shader = NULL;
    }
    
    if (shader)
        glUseProgram(shader->getProgram());
    
    return shader;
}
    
}
