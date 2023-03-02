/*  ProgramGLES.cpp
 *  WhirlyGlobeLib
 *
 *  Created by Steve Gifford on 10/23/12.
 *  Copyright 2011-2022 mousebird consulting
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

#import <string>
#import "ProgramGLES.h"
#import "Lighting.h"
#import "UtilsGLES.h"
#import "SceneRendererGLES.h"
#import "TextureGLES.h"
#import "WhirlyKitLog.h"

#pragma ide diagnostic ignored "readability-make-member-function-const"

using namespace Eigen;

namespace WhirlyKit
{
static constexpr int initCapacityUniforms = 20;
static constexpr int initCapacityAttributes = 20;

ProgramGLES::ProgramGLES() :
    ProgramGLES(std::string())
{
}

ProgramGLES::ProgramGLES(const char *inName) :
    ProgramGLES(inName ? std::string(inName) : std::string())
{
}

ProgramGLES::ProgramGLES(std::string name) :
    Program(std::move(name)),
    uniforms(initCapacityUniforms),
    attrs(initCapacityAttributes)
{
}

ProgramGLES::ProgramGLES(std::string inName,
                         const std::string &vShaderString,
                         const std::string &fShaderString,
                         const std::vector<std::string> *varying) :
    ProgramGLES(std::move(inName),
                vShaderString.c_str(),
                fShaderString.c_str(),
                varying)
{
}

ProgramGLES::ProgramGLES(std::string inName,
                         const char *vShaderString,
                         const char *fShaderString,
                         const std::vector<std::string> *varying) :
    ProgramGLES(std::move(inName))
{
    init(vShaderString, fShaderString, varying);
}

ProgramGLES::~ProgramGLES()
{
    if (program)
    {
        wkLogLevel(Warn, "ProgramGLES destroyed without being cleaned up");
    }
    // Clean up anyway, may fail due to thread context
    cleanUp();
}

bool ProgramGLES::setUniform(StringIdentity nameID,float val)
{
    OpenGLESUniform *const uni = findUniform(nameID);
    if (!uni)
    {
        return false;
    }
    if (uni->type != GL_FLOAT)
    {
        wkLogLevel(Warn, "Shader uniform type mismatch: %s", StringIndexer::getString(nameID).c_str());
        return false;
    }

    if (uni->isSet && uni->val.fVals[0] == val)
        return true;
    
    glUniform1f((GLint)uni->index,val);
    CheckGLError("ProgramGLES::setUniform() glUniform1f");
    uni->isSet = true;
    uni->val.fVals[0] = val;
    
    return true;
}

bool ProgramGLES::setUniform(StringIdentity nameID,float val,int index)
{
    const std::string name = StringIndexer::getString(nameID) + "[0]";
    OpenGLESUniform *const uni = findUniform(StringIndexer::getStringID(name));
    if (!uni)
    {
        return false;
    }
    if (uni->type != GL_FLOAT)
    {
        wkLogLevel(Warn, "Shader uniform type mismatch: %s", StringIndexer::getString(nameID).c_str());
        return false;
    }
    
    glUniform1f((GLint)uni->index+index,val);
    CheckGLError("ProgramGLES::setUniform() glUniform1f");
    uni->isSet = true;
    uni->val.fVals[0] = val;
    
    return true;
}

bool ProgramGLES::setUniform(StringIdentity nameID,int val)
{
    OpenGLESUniform *const uni = findUniform(nameID);
    if (!uni)
    {
        return false;
    }

    if (uni->isSet && uni->val.iVals[0] == val)
        return true;

    switch (uni->type)
    {
        case GL_INT:
        case GL_BOOL:
        case GL_SAMPLER_2D:
        case GL_INT_SAMPLER_2D:
        case GL_UNSIGNED_INT_SAMPLER_2D:
            glUniform1i((GLint) uni->index, val);
            break;
        case GL_UNSIGNED_INT:
            glUniform1ui((GLint)uni->index,(GLuint)val);
            break;
        default:
            wkLogLevel(Warn, "Shader uniform type mismatch: %s", StringIndexer::getString(nameID).c_str());
            return false;
    }

    CheckGLError("ProgramGLES::setUniform() glUniform1i");
    uni->isSet = true;
    uni->val.iVals[0] = val;
    
    return true;
}

bool ProgramGLES::setUniform(StringIdentity nameID,bool val)
{
    OpenGLESUniform *const uni = findUniform(nameID);
    if (!uni)
    {
        return false;
    }
    if (uni->type != GL_INT && uni->type != GL_UNSIGNED_INT && uni->type != GL_BOOL)
    {
        wkLogLevel(Warn, "Shader uniform type mismatch: %s", StringIndexer::getString(nameID).c_str());
        return false;
    }

    if (uni->isSet && uni->val.iVals[0] == val)
        return true;

    glUniform1i((GLint)uni->index,val);
    CheckGLError("ProgramGLES::setUniform() glUniform1i");
    uni->isSet = true;
    uni->val.iVals[0] = val;

    return true;
}

bool ProgramGLES::setTexture(StringIdentity nameID,TextureBase *inTex,int textureSlot)
{
    const auto tex = dynamic_cast<TextureBaseGLES *>(inTex);
    if (!tex)
        return false;
    
    const GLuint val = tex->getGLId();
    OpenGLESUniform *const uni = findUniform(nameID);
    if (!uni)
    {
        return false;
    }
    if (uni->type != GL_INT && uni->type != GL_SAMPLER_2D && uni->type != GL_UNSIGNED_INT && uni->type != GL_BOOL)
    {
        wkLogLevel(Warn, "Shader uniform type mismatch: %s", StringIndexer::getString(nameID).c_str());
        return false;
    }
    
    uni->isTexture = true;
    uni->isSet = true;
    uni->val.iVals[0] = (int)val;
    
    return true;
}


void ProgramGLES::clearTexture(SimpleIdentity texID)
{
    // Note: Doesn't do anything
}

bool ProgramGLES::setUniform(StringIdentity nameID,const Eigen::Vector2f &vec)
{
    OpenGLESUniform *const uni = findUniform(nameID);
    if (!uni)
    {
        return false;
    }
    if (uni->type != GL_FLOAT_VEC2)
    {
        wkLogLevel(Warn, "Shader uniform type mismatch: %s", StringIndexer::getString(nameID).c_str());
        return false;
    }
    
    if (uni->isSet && uni->val.fVals[0] == vec.x() && uni->val.fVals[1] == vec.y())
        return true;
    
    glUniform2f((GLint)uni->index, vec.x(), vec.y());
    CheckGLError("ProgramGLES::setUniform() glUniform2f");
    uni->isSet = true;
    uni->val.fVals[0] = vec.x();  uni->val.fVals[1] = vec.y();
    
    return true;
}

bool ProgramGLES::setUniform(StringIdentity nameID,const Eigen::Vector3f &vec)
{
    OpenGLESUniform *const uni = findUniform(nameID);
    if (!uni)
    {
        return false;
    }
    if (uni->type != GL_FLOAT_VEC3)
    {
        wkLogLevel(Warn, "Shader uniform type mismatch: %s", StringIndexer::getString(nameID).c_str());
        return false;
    }
    if (uni->isSet && uni->val.fVals[0] == vec.x() && uni->val.fVals[1] == vec.y() && uni->val.fVals[2] == vec.z())
        return true;
    
    glUniform3f((GLint)uni->index, vec.x(), vec.y(), vec.z());
    CheckGLError("ProgramGLES::setUniform() glUniform3f");
    uni->isSet = true;
    uni->val.fVals[0] = vec.x();  uni->val.fVals[1] = vec.y();  uni->val.fVals[2] = vec.z();
    
    return true;
}
    

bool ProgramGLES::setUniform(StringIdentity nameID,const Eigen::Vector4f &vec)
{
    OpenGLESUniform *const uni = findUniform(nameID);
    if (!uni)
    {
        return false;
    }
    if (uni->type != GL_FLOAT_VEC4)
    {
        wkLogLevel(Warn, "Shader uniform type mismatch: %s", StringIndexer::getString(nameID).c_str());
        return false;
    }
    if (uni->isSet && uni->val.fVals[0] == vec.x() && uni->val.fVals[1] == vec.y() &&
        uni->val.fVals[2] == vec.z() && uni->val.fVals[3] == vec.w())
        return true;
    
    glUniform4f((GLint)uni->index, vec.x(), vec.y(), vec.z(), vec.w());
    CheckGLError("ProgramGLES::setUniform() glUniform4f");
    uni->isSet = true;
    uni->val.fVals[0] = vec.x();  uni->val.fVals[1] = vec.y();  uni->val.fVals[2] = vec.z(); uni->val.fVals[3] = vec.w();
    
    return true;
}

bool ProgramGLES::setUniform(StringIdentity nameID,const Eigen::Vector4f &vec,int index)
{
    const std::string name = StringIndexer::getString(nameID) + "[0]";
    OpenGLESUniform *const uni = findUniform(StringIndexer::getStringID(name));
    if (!uni)
    {
        return false;
    }
    if (uni->type != GL_FLOAT_VEC4)
    {
        wkLogLevel(Warn, "Shader uniform type mismatch: %s", StringIndexer::getString(nameID).c_str());
        return false;
    }
    if (uni->isSet && index == 0 && uni->val.fVals[0] == vec.x() && uni->val.fVals[1] == vec.y() &&
        uni->val.fVals[2] == vec.z() && uni->val.fVals[3] == vec.w())
    {
        return true;
    }
    
    glUniform4f((GLint)uni->index+index, vec.x(), vec.y(), vec.z(), vec.w());
    CheckGLError("ProgramGLES::setUniform() glUniform4f");
    uni->isSet = true;
    if (index == 0)
    {
        uni->val.fVals[0] = vec.x();
        uni->val.fVals[1] = vec.y();
        uni->val.fVals[2] = vec.z();
        uni->val.fVals[3] = vec.w();
    }
    
    return true;
}
    


bool ProgramGLES::setUniform(StringIdentity nameID,const Eigen::Matrix4f &mat)
{
    OpenGLESUniform *const uni = findUniform(nameID);
    if (!uni)
    {
        return false;
    }
    if (uni->type != GL_FLOAT_MAT4)
    {
        wkLogLevel(Warn, "Shader uniform type mismatch: %s", StringIndexer::getString(nameID).c_str());
        return false;
    }
    
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
    
    glUniformMatrix4fv((GLint)uni->index, 1, GL_FALSE, (GLfloat *)mat.data());
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

static void logError(const char *logStr, const char * sourceCStr)
{
    while (logStr && logStr[0] && sourceCStr && sourceCStr[0])
    {
        // Find an error label
        // "ERROR: 0:66:"
        const char *start = strstr(logStr, "ERROR: ");
        if (start)
        {
            // Skip the filename (missing for dynamic)
            start = strchr(start + 7, ':');
        }
        if (!start || start[0] != ':')
        {
            // Nope, keep trying
            logStr = start;
            continue;
        }

        // Found it, look for another number, that's the line number
        char *end = nullptr;
        const auto errLine = strtol(start + 1, &end, 10);

        // iterate over lines in the source
        for (auto line = 1L; sourceCStr && sourceCStr[0]; ++line)
        {
            // Find the next newline
            const auto lineLen = strcspn(sourceCStr, "\r\n");
            // Print the line, if it's relevant
            if (errLine - 3 <= line && line <= errLine + 3)
            {
                const char c = (line == errLine) ? '>' : ' ';
                wkLogLevel(Warn, "%3d %c: %.*s", line, c, lineLen, sourceCStr);
            }
            // Advance past the line
            sourceCStr += lineLen;
            // And the newline(s), if appropriate
            if (sourceCStr[0])
            {
                sourceCStr += (sourceCStr[0] == '\r' && sourceCStr[1] == '\n') ? 2 : 1;
            }
        }
    }
}

// Helper routine to compile a shader and check return
bool compileShader(const char *name,const char *shaderTypeStr,GLuint *shaderId,GLenum shaderType,const char *shaderStr)
{
    if (!name || !name[0])
    {
        return false;
    }

    *shaderId = glCreateShader(shaderType);
    if (*shaderId == 0) {
        wkLogLevel(Error,"Failed to create GL shader (%d)", shaderType);
        return false;
    }

    const GLchar *sourceCStr = shaderStr ? shaderStr : "";
    glShaderSource(*shaderId, 1, &sourceCStr, nullptr);
    glCompileShader(*shaderId);
    
    GLint status = GL_FALSE;
    glGetShaderiv(*shaderId, GL_COMPILE_STATUS, &status);
    
    if (status != GL_TRUE)
    {
        GLint len = 0;
        glGetShaderiv(*shaderId, GL_INFO_LOG_LENGTH, &len);
        if (len > 0)
        {
            std::vector<char> logStr(len+1);
            glGetShaderInfoLog(*shaderId, len, &len, &logStr[0]);
            wkLogLevel(Error,"Compile error for %s shader %s:\n%s",shaderTypeStr,name,&logStr[0]);
            logError(&logStr[0], sourceCStr);
        }
        
        glDeleteShader(*shaderId);
        *shaderId = 0;
    }
    
    return status == GL_TRUE;
}

#define DUMP_UNIFORMS 0

// Construct the program, compile and link
bool ProgramGLES::init(const char *vShaderString,
                       const char *fShaderString,
                       const std::vector<std::string> *varying)
{
    cleanUp();
    program = glCreateProgram();
    if (!CheckGLError("ProgramGLES glCreateProgram"))
    {
        return false;
    }

    if (!program)
    {
        // glCreateProgram sometimes produces zero without setting any error.
        // This seems to be related to being called without a current context.
#if defined(EGL_VERSION_1_4)
        wkLogLevel(Warn, "glCreateProgram Failed (%x,%x)", glGetError(), eglGetCurrentContext());
#else
        wkLogLevel(Warn, "glCreateProgram Failed (%x)", glGetError());
#endif
        return false;
    }
    
    if (!compileShader(name.c_str(),"vertex",&vertShader,GL_VERTEX_SHADER,vShaderString))
    {
        cleanUp();
        return false;
    }
    CheckGLError("ProgramGLES: compileShader() vertex");
    if (!compileShader(name.c_str(),"fragment",&fragShader,GL_FRAGMENT_SHADER,fShaderString))
    {
        cleanUp();
        return false;
    }
    CheckGLError("ProgramGLES: compileShader() fragment");

    glAttachShader(program, vertShader);
    if (!CheckGLError("ProgramGLES: glAttachShader() vertex"))
    {
        cleanUp();
        return false;
    }

    glAttachShader(program, fragShader);
    if (!CheckGLError("ProgramGLES: glAttachShader() fragment"))
    {
        cleanUp();
        return false;
    }

    // Designate the varyings that we want out of the shader
    if (varying && !varying->empty()) {
        std::vector<GLchar*> names(varying->size());
        for (unsigned int ii = 0; ii < varying->size(); ii++) {
            const std::string &name = varying->at(ii);

            // TODO: Do we really need copies here, when we're just going to free them?
            names[ii] = (GLchar *) malloc(sizeof(GLchar) * (name.size() + 1));
            if (names[ii]) {
                strcpy(names[ii], name.c_str());
            }
        }

        glTransformFeedbackVaryings(program, (int)varying->size(), &names[0], GL_SEPARATE_ATTRIBS);

        if (!CheckGLError("ProgramGLES: Error setting up varyings in"))
        {
            cleanUp();
            return false;
        }

        for (unsigned int ii = 0; ii < varying->size(); ii++) {
            if (names[ii]) {
                free(names[ii]);
            }
        }
    }
    
    // Now link it
    glLinkProgram(program);
    if (!CheckGLError("ProgramGLES: glLinkProgram"))
    {
        cleanUp();
        return false;
    }

    GLint status = 0;
    glGetProgramiv(program, GL_LINK_STATUS, &status);
    if (status == GL_FALSE)
    {
        GLint len = 0;
        glGetProgramiv(program, GL_INFO_LOG_LENGTH, &len);
        if (len > 0)
        {
            std::vector<char> logStr(len+1);
            glGetProgramInfoLog(program, len, &len, &logStr[0]);
            wkLogLevel(Error,"Link error for shader program %s:\n%s",name.c_str(),&logStr[0]);
        }
        cleanUp();
        return false;
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
        auto uni = std::make_shared<OpenGLESUniform>();
        GLint bufLen;
        thingName[0] = 0;
        glGetActiveUniform(program, ii, sizeof(thingName)-1, &bufLen, &uni->size, &uni->type, thingName);
        uni->nameID = StringIndexer::getStringID(thingName);
        uni->index = glGetUniformLocation(program, thingName);
        uniforms[uni->nameID] = uni;
#if DUMP_UNIFORMS
        wkLog("%s Uniform %d/%d, name=%d, idx=%d, %s", inName.c_str(), ii, numUniform, uni->nameID, uni->index, thingName);
#endif
    }
    if (!CheckGLError("ProgramGLES: glGetActiveUniform"))
    {
        cleanUp();
        return false;
    }

    // Convert the attributes into a more useful form
    GLint numAttr;
    glGetProgramiv(program, GL_ACTIVE_ATTRIBUTES, &numAttr);
    for (unsigned int ii=0;ii<numAttr;ii++)
    {
        auto attr = std::make_shared<OpenGLESAttribute>();
        GLint bufLen;
        thingName[0] = 0;
        glGetActiveAttrib(program, ii, sizeof(thingName)-1, &bufLen, &attr->size, &attr->type, thingName);
        attr->index = glGetAttribLocation(program, thingName);
        attr->nameID = StringIndexer::getStringID(thingName);
        attrs[attr->nameID] = attr;
#if DUMP_UNIFORMS
        wkLog("%s Attribute %d/%d, name=%d, idx=%d, %s", inName.c_str(), ii, numAttr, attr->nameID, attr->index, thingName);
#endif
    }
    if (!CheckGLError("ProgramGLES: glGetActiveAttrib"))
    {
        cleanUp();
        return false;
    }

    return true;
}
    
void ProgramGLES::teardownForRenderer(const RenderSetupInfo *setupInfo,Scene *scene,RenderTeardownInfoRef teardown)
{
    cleanUp();
}
    
// Clean up outstanding OpenGL resources
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
    
bool ProgramGLES::isValid() const
{
    return (program != 0);
}
    

OpenGLESUniform *ProgramGLES::findUniform(StringIdentity nameID) const
{
    auto it = uniforms.find(nameID);
    if (it == uniforms.end())
        return nullptr;
    return it->second.get();
}

const OpenGLESAttribute *ProgramGLES::findAttribute(StringIdentity nameID) const
{
    auto it = attrs.find(nameID);
    if (it == attrs.end())
        return nullptr;
    return it->second.get();
}
    
bool ProgramGLES::hasLights() const
{
    OpenGLESUniform *lightAttr = findUniform(u_numLightsNameID);
    return lightAttr != nullptr;
}
    
bool ProgramGLES::setLights(const std::vector<DirectionalLight> &lights, TimeInterval lastUpdated, const Material *mat, const Eigen::Matrix4f &modelMat)
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
        const Eigen::Vector3f dir = light.getPos().normalized();
        const Eigen::Vector3f halfPlane = (dir + Eigen::Vector3f(0,0,1)).normalized();

        setUniform(lightViewDependNameIDs[ii], (light.getViewDependent() ? 0.0f : 1.0f));
        setUniform(lightDirectionNameIDs[ii], dir);
        setUniform(lightHalfplaneNameIDs[ii], halfPlane);
        setUniform(lightAmbientNameIDs[ii], light.getAmbient());
        setUniform(lightDiffuseNameIDs[ii], light.getDiffuse());
        setUniform(lightSpecularNameIDs[ii], light.getSpecular());
    }
    OpenGLESUniform *lightAttr = findUniform(u_numLightsNameID);
    if (lightAttr)
        glUniform1i((GLint)lightAttr->index, numLights);
    else
        return false;
    
    // Bind the material
    if (mat)
    {
        setUniform(materialAmbientNameID, mat->getAmbient());
        setUniform(materialDiffuseNameID, mat->getDiffuse());
        setUniform(materialSpecularNameID, mat->getSpecular());
        setUniform(materialSpecularExponentNameID, mat->getSpecularExponent());
    }

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
            glUniform1i((GLint)uni.second->index,numTextures);
            numTextures++;
        }
    }
    
    return numTextures;
}

}
