/*
 *  OpenGLES2Program.mm
 *  WhirlyGlobeLib
 *
 *  Created by Steve Gifford on 10/23/12.
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

#import <string>
#import "OpenGLES2Program.h"
#import "Lighting.h"
#import "GLUtils.h"

using namespace Eigen;

namespace WhirlyKit
{
    
OpenGLES2Program::OpenGLES2Program()
    : lightsLastUpdated(0.0)
{
}
    
OpenGLES2Program::~OpenGLES2Program()
{
    for (std::set<OpenGLESUniform *,UniformNameSortStruct>::iterator it = uniforms.begin();
         it != uniforms.end(); ++it)
        delete *it;
    uniforms.clear();
    for (std::set<OpenGLESAttribute *,AttributeNameSortStruct>::iterator it = attrs.begin();
         it != attrs.end(); ++it)
        delete *it;
    attrs.clear();
}
    
bool OpenGLES2Program::setUniform(const std::string &name,float val)
{
    OpenGLESUniform *uni = findUniform(name);
    if (!uni)
        return false;
    
    if (uni->type != GL_FLOAT)
        return false;
    
    if (uni->isSet && uni->val.fVals[0] == val)
        return true;
    
    glUniform1f(uni->index,val);
    CheckGLError("OpenGLES2Program::setUniform() glUniform1f");
    uni->isSet = true;
    uni->val.fVals[0] = val;
    
    return true;
}

bool OpenGLES2Program::setUniform(const std::string &name,int val)
{
    OpenGLESUniform *uni = findUniform(name);
    if (!uni)
        return false;
    
    if (uni->type != GL_INT && uni->type != GL_SAMPLER_2D && uni->type != GL_UNSIGNED_INT && uni->type != GL_BOOL)
        return false;
    
    if (uni->isSet && uni->val.iVals[0] == val)
        return true;
    
    glUniform1i(uni->index,val);
    CheckGLError("BigDrawable::draw() glUniform1i");
    uni->isSet = true;
    uni->val.iVals[0] = val;
    
    return true;
}
    
bool OpenGLES2Program::setTexture(const std::string &name,GLuint val)
{
    OpenGLESUniform *uni = findUniform(name);
    if (!uni)
        return false;
    
    if (uni->type != GL_INT && uni->type != GL_SAMPLER_2D && uni->type != GL_UNSIGNED_INT && uni->type != GL_BOOL)
        return false;
    
    uni->isTexture = true;
    uni->isSet = true;
    uni->val.iVals[0] = val;
    
    return true;
}


bool OpenGLES2Program::setUniform(const std::string &name,const Eigen::Vector2f &vec)
{
    OpenGLESUniform *uni = findUniform(name);
    if (!uni)
        return false;
    
    if (uni->type != GL_FLOAT_VEC2)
        return false;
    
    if (uni->isSet && uni->val.fVals[0] == vec.x() && uni->val.fVals[1] == vec.y())
        return true;
    
    glUniform2f(uni->index, vec.x(), vec.y());
    CheckGLError("BigDrawable::draw() glUniform2f");
    uni->isSet = true;
    uni->val.fVals[0] = vec.x();  uni->val.fVals[1] = vec.y();
    
    return true;
}

bool OpenGLES2Program::setUniform(const std::string &name,const Eigen::Vector3f &vec)
{
    OpenGLESUniform *uni = findUniform(name);
    if (!uni)
        return false;
    
    if (uni->type != GL_FLOAT_VEC3)
        return false;
    if (uni->isSet && uni->val.fVals[0] == vec.x() && uni->val.fVals[1] == vec.y() && uni->val.fVals[2] == vec.z())
        return true;
    
    glUniform3f(uni->index, vec.x(), vec.y(), vec.z());
    CheckGLError("BigDrawable::draw() glUniform3f");
    uni->isSet = true;
    uni->val.fVals[0] = vec.x();  uni->val.fVals[1] = vec.y();  uni->val.fVals[2] = vec.z();
    
    return true;
}
    

bool OpenGLES2Program::setUniform(const std::string &name,const Eigen::Vector4f &vec)
{
    OpenGLESUniform *uni = findUniform(name);
    if (!uni)
        return false;
    
    if (uni->type != GL_FLOAT_VEC4)
        return false;
    if (uni->isSet && uni->val.fVals[0] == vec.x() && uni->val.fVals[1] == vec.y() &&
        uni->val.fVals[2] == vec.z() && uni->val.fVals[3] == vec.w())
        return true;
    
    glUniform4f(uni->index, vec.x(), vec.y(), vec.z(), vec.w());
    CheckGLError("BigDrawable::draw() glUniform4f");
    uni->isSet = true;
    uni->val.fVals[0] = vec.x();  uni->val.fVals[1] = vec.y();  uni->val.fVals[2] = vec.z(); uni->val.fVals[3] = vec.w();
    
    return true;
}

bool OpenGLES2Program::setUniform(const std::string &name,const Eigen::Matrix4f &mat)
{
    OpenGLESUniform *uni = findUniform(name);
    if (!uni)
        return false;
    
    if (uni->type != GL_FLOAT_MAT4)
        return false;
    
    if (uni->isSet)
    {
        bool equal = true;
        for (unsigned int ii=0;ii<16;ii++)
            if (mat.data()[ii] != uni->val.mat[ii])
            {
                equal = false;
                break;
            }
        if (equal)
            return true;
    }
    
    glUniformMatrix4fv(uni->index, 1, GL_FALSE, (GLfloat *)mat.data());
    CheckGLError("BigDrawable::draw() glUniformMatrix4fv");
    uni->isSet = true;
    for (unsigned int ii=0;ii<16;ii++)
        uni->val.mat[ii] = mat.data()[ii];
    
    return true;
}

// Helper routine to compile a shader and check return
bool compileShader(const std::string &name,const char *shaderTypeStr,GLuint *shaderId,GLenum shaderType,const std::string &shaderStr)
{
    *shaderId = glCreateShader(shaderType);
    const GLchar *sourceCStr = shaderStr.c_str();
    glShaderSource(*shaderId, 1, &sourceCStr, NULL);
    glCompileShader(*shaderId);
    
    GLint status;
    glGetShaderiv(*shaderId, GL_COMPILE_STATUS, &status);
    
    if (status != GL_TRUE)
    {
        GLint len;
        glGetShaderiv(*shaderId, GL_INFO_LOG_LENGTH, &len);
        if (len > 0)
        {
            GLchar *logStr = (GLchar *)malloc(len);
            glGetShaderInfoLog(*shaderId, len, &len, logStr);
            fprintf(stderr,"Shader compile error: %s",logStr);
//            NSLog(@"Compile error for %s shader %s:\n%s",shaderTypeStr,name.c_str(),logStr);
            free(logStr);
        }
        
        glDeleteShader(*shaderId);
        *shaderId = 0;
    }
    
    return status == GL_TRUE;
}

// Construct the program, compile and link
OpenGLES2Program::OpenGLES2Program(const std::string &inName,const std::string &vShaderString,const std::string &fShaderString)
    : name(inName), lightsLastUpdated(0.0)
{
    program = glCreateProgram();
    
    if (!compileShader(name,"vertex",&vertShader,GL_VERTEX_SHADER,vShaderString))
    {
        cleanUp();
        return;
    }
    if (!compileShader(name,"fragment",&fragShader,GL_FRAGMENT_SHADER,fShaderString))
    {
        cleanUp();
        return;
    }

    glAttachShader(program, vertShader);
    glAttachShader(program, fragShader);
    
    // Now link it
    GLint status;
    glLinkProgram(program);
    
    glGetProgramiv(program, GL_LINK_STATUS, &status);
    if (status == GL_FALSE)
    {
        GLint len;
        glGetProgramiv(program, GL_INFO_LOG_LENGTH, &len);
        if (len > 0)
        {
            GLchar *logStr = (GLchar *)malloc(len);
            glGetProgramInfoLog(program, len, &len, logStr);
            // Note: Porting
//            NSLog(@"Link error for shader program %s:\n%s",name.c_str(),logStr);
            free(logStr);
        }
        cleanUp();
        return;
    }
    
    if (vertShader)
    {
        glDeleteShader(vertShader);
        vertShader = 0;
    }
    if (fragShader)
    {
        glDeleteShader(fragShader);
        fragShader = 0;
    }
    
    // Convert the uniforms into a more friendly form
    GLint numUniform;
    glGetProgramiv(program, GL_ACTIVE_UNIFORMS, &numUniform);
    char thingName[1024];
    for (unsigned int ii=0;ii<numUniform;ii++)
    {
        OpenGLESUniform *uni = new OpenGLESUniform();
        GLint bufLen;
        thingName[0] = 0;
        glGetActiveUniform(program, ii, 1023, &bufLen, &uni->size, &uni->type, thingName);
        uni->name = thingName;
        uni->index = glGetUniformLocation(program, thingName);
        uniforms.insert(uni);
    }
    
    // Convert the attributes into a more useful form
    GLint numAttr;
    glGetProgramiv(program, GL_ACTIVE_ATTRIBUTES, &numAttr);
    for (unsigned int ii=0;ii<numAttr;ii++)
    {
        OpenGLESAttribute *attr = new OpenGLESAttribute();
        GLint bufLen;
        thingName[0] = 0;
        glGetActiveAttrib(program, ii, 1023, &bufLen, &attr->size, &attr->type, thingName);
        attr->index = glGetAttribLocation(program, thingName);
        attr->name = thingName;
        attrs.insert(attr);
    }
}
    
// Clean up oustanding OpenGL resources
void OpenGLES2Program::cleanUp()
{
    if (program)
    {
        glDeleteProgram(program);
        program = 0;
    }
    if (vertShader)
    {
        glDeleteShader(vertShader);
        vertShader = 0;
    }
    if (fragShader)
    {
        glDeleteShader(fragShader);
        fragShader = 0;
    }
    
    uniforms.clear();
    attrs.clear();
}
    
bool OpenGLES2Program::isValid()
{
    return (program != 0);
}
    

OpenGLESUniform *OpenGLES2Program::findUniform(const std::string &uniformName)
{
    OpenGLESUniform uni(uniformName);
    std::set<OpenGLESUniform *,UniformNameSortStruct>::iterator it = uniforms.find(&uni);
    if (it != uniforms.end())
        return *it;
    else
        return NULL;
}

const OpenGLESAttribute *OpenGLES2Program::findAttribute(const std::string &attrName)
{
    OpenGLESAttribute attr(attrName);
    std::set<OpenGLESAttribute *,AttributeNameSortStruct>::iterator it = attrs.find(&attr);
    if (it != attrs.end())
        return *it;
    else
        return NULL;
}
    
bool OpenGLES2Program::hasLights()
{
    OpenGLESUniform *lightAttr = findUniform("u_numLights");
    return lightAttr != NULL;
}
    
bool OpenGLES2Program::setLights(std::vector<WhirlyKitDirectionalLight*> lights, TimeInterval lastUpdate, WhirlyKitMaterial *mat,Eigen::Matrix4f &modelMat)
{
    if (lightsLastUpdated >= lastUpdate)
        return false;
    lightsLastUpdated = lastUpdate;
    
    int numLights = (int)lights.size();
    if (numLights > 8) numLights = 8;
    bool lightsSet = true;
    for (unsigned int ii=0;ii<numLights;ii++)
    {
        WhirlyKitDirectionalLight *light = lights[ii];
        lightsSet &= light->bindToProgram(this, ii, modelMat);
    }
    OpenGLESUniform *lightAttr = findUniform("u_numLights");
    if (lightAttr)
        glUniform1i(lightAttr->index, numLights);
    else
        return false;
    
    // Bind the material
    mat->bindToProgram(this);
    
    return lightsSet;
}

int OpenGLES2Program::bindTextures()
{
    int numTextures = 0;
    
    for (std::set<OpenGLESUniform *,UniformNameSortStruct>::iterator it = uniforms.begin();
         it != uniforms.end(); ++it)
    {
        OpenGLESUniform *uni = *it;
        if (uni->isTexture)
        {
            glActiveTexture(GL_TEXTURE0+numTextures);
            glBindTexture(GL_TEXTURE_2D, uni->val.iVals[0]);
            glUniform1i(uni->index,numTextures);
            numTextures++;
        }
    }
    
    return numTextures;
}


}
