/*
 *  ProgramGLES.mm
 *  WhirlyGlobeLib
 *
 *  Created by Steve Gifford on 10/23/12.
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

#import <string>
#import "ProgramGLES.h"
#import "Lighting.h"
#import "UtilsGLES.h"
#import "SceneRendererGLES.h"
#import "TextureGLES.h"
#import "WhirlyKitLog.h"

using namespace Eigen;

namespace WhirlyKit
{
    
ProgramGLES::ProgramGLES()
    : lightsLastUpdated(0.0)
{
}
    
ProgramGLES::~ProgramGLES()
{
}
    
bool ProgramGLES::setUniform(StringIdentity nameID,float val)
{
    OpenGLESUniform *uni = findUniform(nameID);
    if (!uni)
        return false;
    
    if (uni->type != GL_FLOAT)
        return false;
    
    if (uni->isSet && uni->val.fVals[0] == val)
        return true;
    
    glUniform1f(uni->index,val);
    CheckGLError("ProgramGLES::setUniform() glUniform1f");
    uni->isSet = true;
    uni->val.fVals[0] = val;
    
    return true;
}

bool ProgramGLES::setUniform(StringIdentity nameID,float val,int index)
{
    std::string name = StringIndexer::getString(nameID) + "[0]";
    OpenGLESUniform *uni = findUniform(StringIndexer::getStringID(name));
    if (!uni)
        return false;

    if (uni->type != GL_FLOAT)
        return false;
    
    glUniform1f(uni->index+index,val);
    CheckGLError("ProgramGLES::setUniform() glUniform1f");
    uni->isSet = true;
    uni->val.fVals[0] = val;
    
    return true;
}

bool ProgramGLES::setUniform(StringIdentity nameID,int val)
{
    OpenGLESUniform *uni = findUniform(nameID);
    if (!uni)
        return false;
    
    if (uni->type != GL_INT && uni->type != GL_SAMPLER_2D && uni->type != GL_UNSIGNED_INT && uni->type != GL_BOOL)
        return false;
    
    if (uni->isSet && uni->val.iVals[0] == val)
        return true;
    
    glUniform1i(uni->index,val);
    CheckGLError("ProgramGLES::setUniform() glUniform1i");
    uni->isSet = true;
    uni->val.iVals[0] = val;
    
    return true;
}
    
bool ProgramGLES::setTexture(StringIdentity nameID,TextureBase *inTex,int textureSlot)
{
    TextureBaseGLES *tex = dynamic_cast<TextureBaseGLES *>(inTex);
    if (!tex)
        return false;
    
    GLuint val = tex->getGLId();
    OpenGLESUniform *uni = findUniform(nameID);
    if (!uni)
        return false;
    
    if (uni->type != GL_INT && uni->type != GL_SAMPLER_2D && uni->type != GL_UNSIGNED_INT && uni->type != GL_BOOL)
        return false;
    
    uni->isTexture = true;
    uni->isSet = true;
    uni->val.iVals[0] = val;
    
    return true;
}


void ProgramGLES::clearTexture(SimpleIdentity texID)
{
    // Note: Doesn't do anything
}

bool ProgramGLES::setUniform(StringIdentity nameID,const Eigen::Vector2f &vec)
{
    OpenGLESUniform *uni = findUniform(nameID);
    if (!uni)
        return false;
    
    if (uni->type != GL_FLOAT_VEC2)
        return false;
    
    if (uni->isSet && uni->val.fVals[0] == vec.x() && uni->val.fVals[1] == vec.y())
        return true;
    
    glUniform2f(uni->index, vec.x(), vec.y());
    CheckGLError("ProgramGLES::setUniform() glUniform2f");
    uni->isSet = true;
    uni->val.fVals[0] = vec.x();  uni->val.fVals[1] = vec.y();
    
    return true;
}

bool ProgramGLES::setUniform(StringIdentity nameID,const Eigen::Vector3f &vec)
{
    OpenGLESUniform *uni = findUniform(nameID);
    if (!uni)
        return false;
    
    if (uni->type != GL_FLOAT_VEC3)
        return false;
    if (uni->isSet && uni->val.fVals[0] == vec.x() && uni->val.fVals[1] == vec.y() && uni->val.fVals[2] == vec.z())
        return true;
    
    glUniform3f(uni->index, vec.x(), vec.y(), vec.z());
    CheckGLError("ProgramGLES::setUniform() glUniform3f");
    uni->isSet = true;
    uni->val.fVals[0] = vec.x();  uni->val.fVals[1] = vec.y();  uni->val.fVals[2] = vec.z();
    
    return true;
}
    

bool ProgramGLES::setUniform(StringIdentity nameID,const Eigen::Vector4f &vec)
{
    OpenGLESUniform *uni = findUniform(nameID);
    if (!uni)
        return false;
    
    if (uni->type != GL_FLOAT_VEC4)
        return false;
    if (uni->isSet && uni->val.fVals[0] == vec.x() && uni->val.fVals[1] == vec.y() &&
        uni->val.fVals[2] == vec.z() && uni->val.fVals[3] == vec.w())
        return true;
    
    glUniform4f(uni->index, vec.x(), vec.y(), vec.z(), vec.w());
    CheckGLError("ProgramGLES::setUniform() glUniform4f");
    uni->isSet = true;
    uni->val.fVals[0] = vec.x();  uni->val.fVals[1] = vec.y();  uni->val.fVals[2] = vec.z(); uni->val.fVals[3] = vec.w();
    
    return true;
}

bool ProgramGLES::setUniform(StringIdentity nameID,const Eigen::Vector4f &vec,int index)
{
    std::string name = StringIndexer::getString(nameID) + "[0]";
    OpenGLESUniform *uni = findUniform(StringIndexer::getStringID(name));
    if (!uni)
        return false;
    
    if (uni->type != GL_FLOAT_VEC4)
        return false;
    if (uni->isSet && uni->val.fVals[0] == vec.x() && uni->val.fVals[1] == vec.y() &&
        uni->val.fVals[2] == vec.z() && uni->val.fVals[3] == vec.w())
        return true;
    
    glUniform4f(uni->index+index, vec.x(), vec.y(), vec.z(), vec.w());
    CheckGLError("ProgramGLES::setUniform() glUniform4f");
    uni->isSet = true;
    uni->val.fVals[0] = vec.x();  uni->val.fVals[1] = vec.y();  uni->val.fVals[2] = vec.z(); uni->val.fVals[3] = vec.w();
    
    return true;
}
    


bool ProgramGLES::setUniform(StringIdentity nameID,const Eigen::Matrix4f &mat)
{
    OpenGLESUniform *uni = findUniform(nameID);
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
    CheckGLError("ProgramGLES::setUniform() glUniformMatrix4fv");
    uni->isSet = true;
    for (unsigned int ii=0;ii<16;ii++)
        uni->val.mat[ii] = mat.data()[ii];
    
    return true;
}
    
bool ProgramGLES::setUniform(const SingleVertexAttribute &attr)
{
    bool ret = false;
    
    switch (attr.type)
    {
        case BDFloat4Type:
            ret = setUniform(attr.nameID, Vector4f(attr.data.vec4[0],attr.data.vec4[1],attr.data.vec4[2],attr.data.vec4[3]));
            break;
        case BDFloat3Type:
            ret = setUniform(attr.nameID, Vector3f(attr.data.vec3[0],attr.data.vec3[1],attr.data.vec3[2]));
            break;
        case BDChar4Type:
            ret = setUniform(attr.nameID, Vector4f(attr.data.color[0],attr.data.color[1],attr.data.color[2],attr.data.color[3]));
            break;
        case BDFloat2Type:
            ret = setUniform(attr.nameID, Vector2f(attr.data.vec2[0],attr.data.vec2[1]));
            break;
        case BDFloatType:
            ret = setUniform(attr.nameID, attr.data.floatVal);
            break;
        case BDIntType:
            ret = setUniform(attr.nameID, attr.data.intVal);
            break;
        default:
            break;
    }
    
    return ret;
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
            wkLogLevel(Error,"Compile error for %s shader %s:\n%s",shaderTypeStr,name.c_str(),logStr);
            free(logStr);
        }
        
        glDeleteShader(*shaderId);
        *shaderId = 0;
    }
    
    return status == GL_TRUE;
}

// Construct the program, compile and link
ProgramGLES::ProgramGLES(const std::string &inName,const std::string &vShaderString,const std::string &fShaderString,const std::vector<std::string> *varying)
    : lightsLastUpdated(0.0)
{
    name = inName;
    program = glCreateProgram();
    
    if (!compileShader(name,"vertex",&vertShader,GL_VERTEX_SHADER,vShaderString))
    {
        cleanUp();
        return;
    }
    CheckGLError("ProgramGLES: compileShader() vertex");
    if (!compileShader(name,"fragment",&fragShader,GL_FRAGMENT_SHADER,fShaderString))
    {
        cleanUp();
        return;
    }
    CheckGLError("ProgramGLES: compileShader() fragment");

    glAttachShader(program, vertShader);
    CheckGLError("ProgramGLES: glAttachShader() vertex");
    glAttachShader(program, fragShader);
    CheckGLError("ProgramGLES: glAttachShader() fragment");

    // Designate the varyings that we want out of the shader
    if (varying) {
        GLchar **names = (GLchar **)malloc(sizeof(GLchar *)*varying->size());
        for (unsigned int ii=0;ii<varying->size();ii++) {
            const std::string &name = (*varying)[ii];
            names[ii] = (GLchar *)malloc(sizeof(GLchar)*(name.size()+1));
            strcpy(names[ii], name.c_str());
        }
        glTransformFeedbackVaryings(program, varying->size(), names, GL_SEPARATE_ATTRIBS);
        
        CheckGLError("ProgramGLES: Error setting up varyings in");
        
        for (unsigned int ii=0;ii<varying->size();ii++) {
            free(names[ii]);
        }
        free(names);
    }
    
    // Now link it
    GLint status;
    glLinkProgram(program);
    CheckGLError("ProgramGLES: glLinkProgram");

    glGetProgramiv(program, GL_LINK_STATUS, &status);
    if (status == GL_FALSE)
    {
        GLint len;
        glGetProgramiv(program, GL_INFO_LOG_LENGTH, &len);
        if (len > 0)
        {
            GLchar *logStr = (GLchar *)malloc(len);
            glGetProgramInfoLog(program, len, &len, logStr);
            wkLogLevel(Error,"Link error for shader program %s:\n%s",name.c_str(),logStr);
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
        std::shared_ptr<OpenGLESUniform> uni(new OpenGLESUniform());
        GLint bufLen;
        thingName[0] = 0;
        glGetActiveUniform(program, ii, 1023, &bufLen, &uni->size, &uni->type, thingName);
        uni->nameID = StringIndexer::getStringID(thingName);
        uni->index = glGetUniformLocation(program, thingName);
        uniforms[uni->nameID] = uni;
    }
    CheckGLError("ProgramGLES: glGetActiveUniform");

    // Convert the attributes into a more useful form
    GLint numAttr;
    glGetProgramiv(program, GL_ACTIVE_ATTRIBUTES, &numAttr);
    for (unsigned int ii=0;ii<numAttr;ii++)
    {
        std::shared_ptr<OpenGLESAttribute> attr(new OpenGLESAttribute());
        GLint bufLen;
        thingName[0] = 0;
        glGetActiveAttrib(program, ii, 1023, &bufLen, &attr->size, &attr->type, thingName);
        attr->index = glGetAttribLocation(program, thingName);
        attr->nameID = StringIndexer::getStringID(thingName);
        attrs[attr->nameID] = attr;
    }
    CheckGLError("ProgramGLES: glGetActiveAttrib");
}
    
void ProgramGLES::teardownForRenderer(const RenderSetupInfo *setupInfo,Scene *scene,RenderTeardownInfoRef teardown)
{
    cleanUp();
}
    
// Clean up oustanding OpenGL resources
void ProgramGLES::cleanUp()
{
    if (program)
    {
        glDeleteProgram(program);
        CheckGLError("ProgramGLES::cleanup() glDeleteProgram");
        program = 0;
    }
    if (vertShader)
    {
        glDeleteShader(vertShader);
        CheckGLError("ProgramGLES::cleanup() glDeleteShader vertShader");
        vertShader = 0;
    }
    if (fragShader)
    {
        glDeleteShader(fragShader);
        CheckGLError("ProgramGLES::cleanup() glDeleteShader fragShader");
        fragShader = 0;
    }
    
    uniforms.clear();
    attrs.clear();
}
    
bool ProgramGLES::isValid()
{
    return (program != 0);
}
    

OpenGLESUniform *ProgramGLES::findUniform(StringIdentity nameID)
{
    auto it = uniforms.find(nameID);
    if (it == uniforms.end())
        return NULL;
    return it->second.get();
}

const OpenGLESAttribute *ProgramGLES::findAttribute(StringIdentity nameID)
{
    auto it = attrs.find(nameID);
    if (it == attrs.end())
        return NULL;
    return it->second.get();
}
    
bool ProgramGLES::hasLights()
{
    OpenGLESUniform *lightAttr = findUniform(u_numLightsNameID);
    return lightAttr != NULL;
}
    
bool ProgramGLES::setLights(const std::vector<DirectionalLight> &lights, TimeInterval lastUpdated, Material *mat, Eigen::Matrix4f &modelMat)
{
    if (lightsLastUpdated >= lastUpdated)
        return false;
    lightsLastUpdated = lastUpdated;
    
    int numLights = (int)lights.size();
    numLights = std::min(numLights,8);
    bool lightsSet = true;
    for (unsigned int ii=0;ii<numLights;ii++)
    {
        const DirectionalLight &light = lights[ii];
        Eigen::Vector3f dir = light.pos.normalized();
        Eigen::Vector3f halfPlane = (dir + Eigen::Vector3f(0,0,1)).normalized();

        setUniform(lightViewDependNameIDs[ii], (light.viewDependent ? 0.0f : 1.0f));
        setUniform(lightDirectionNameIDs[ii], dir);
        setUniform(lightHalfplaneNameIDs[ii], halfPlane);
        setUniform(lightAmbientNameIDs[ii], light.ambient);
        setUniform(lightDiffuseNameIDs[ii], light.diffuse);
        setUniform(lightSpecularNameIDs[ii], light.specular);
    }
    OpenGLESUniform *lightAttr = findUniform(u_numLightsNameID);
    if (lightAttr)
        glUniform1i(lightAttr->index, numLights);
    else
        return false;
    
    // Bind the material
    setUniform(materialAmbientNameID, mat->ambient);
    setUniform(materialDiffuseNameID, mat->diffuse);
    setUniform(materialSpecularNameID, mat->specular);
    setUniform(materialSpecularExponentNameID, mat->specularExponent);

    return lightsSet;
}

int ProgramGLES::bindTextures()
{
    int numTextures = 0;
    
    for (const auto &uni : uniforms)
    {
        if (uni.second->isTexture)
        {
            glActiveTexture(GL_TEXTURE0+numTextures);
            glBindTexture(GL_TEXTURE_2D, uni.second->val.iVals[0]);
            glUniform1i(uni.second->index,numTextures);
            numTextures++;
        }
    }
    
    return numTextures;
}

}
