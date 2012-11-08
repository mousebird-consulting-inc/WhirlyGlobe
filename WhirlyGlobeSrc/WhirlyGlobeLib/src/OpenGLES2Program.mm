/*
 *  OpenGLES2Program.mm
 *  WhirlyGlobeLib
 *
 *  Created by Steve Gifford on 10/23/12.
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

#import <string>
#import "OpenGLES2Program.h"

namespace WhirlyKit
{
    
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
            NSLog(@"Compile error for %s shader %s:\n%s",shaderTypeStr,name.c_str(),logStr);
            free(logStr);
        }
        
        glDeleteShader(*shaderId);
        *shaderId = 0;
    }
    
    return status == GL_TRUE;
}

// Construct the program, compile and link
OpenGLES2Program::OpenGLES2Program(const std::string &inName,const std::string &vShaderString,const std::string &fShaderString)
    : name(inName)
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
    glValidateProgram(program);
    
    glGetProgramiv(program, GL_LINK_STATUS, &status);
    if (status == GL_FALSE)
    {
        NSLog(@"Failed to link shader %s",name.c_str());
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
        OpenGLESUniform uni;
        GLint bufLen;
        thingName[0] = 0;
        glGetActiveUniform(program, ii, 1023, &bufLen, &uni.size, &uni.type, thingName);
        uni.name = thingName;
        uni.index = glGetUniformLocation(program, thingName);
        uniforms.insert(uni);
    }
    
    // Convert the attributes into a more useful form
    GLint numAttr;
    glGetProgramiv(program, GL_ACTIVE_ATTRIBUTES, &numAttr);
    for (unsigned int ii=0;ii<numAttr;ii++)
    {
        OpenGLESAttribute attr;
        GLint bufLen;
        thingName[0] = 0;
        glGetActiveAttrib(program, ii, 1023, &bufLen, &attr.size, &attr.type, thingName);
        attr.index = glGetAttribLocation(program, thingName);
        attr.name = thingName;
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
    

const OpenGLESUniform *OpenGLES2Program::findUniform(const std::string &uniformName)
{
    OpenGLESUniform uni(uniformName);
    std::set<OpenGLESUniform>::iterator it = uniforms.find(uni);
    if (it != uniforms.end())
        return &(*it);
    else
        return NULL;
}

const OpenGLESAttribute *OpenGLES2Program::findAttribute(const std::string &attrName)
{
    OpenGLESAttribute attr(attrName);
    std::set<OpenGLESAttribute>::iterator it = attrs.find(attr);
    if (it != attrs.end())
        return &(*it);
    else
        return NULL;
}


}
