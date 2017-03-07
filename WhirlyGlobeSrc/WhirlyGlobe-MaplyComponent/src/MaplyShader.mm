/*
 *  MaplyShader.mm
 *  WhirlyGlobe-MaplyComponent
 *
 *  Created by Steve Gifford on 2/7/13.
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

#import <UIKit/UIKit.h>
#import <string>
#import <WhirlyGlobe.h>
#import "MaplyShader_private.h"
#import "MaplyBaseViewController_private.h"

using namespace WhirlyKit;

@implementation MaplyShader
{
    WhirlyKit::Scene *scene;
    WhirlyKitSceneRendererES * __weak renderer;
    NSString *buildError;
    EAGLContext *context;
    // Texture we created for use in this shader
    SimpleIDSet texIDs;
}

- (instancetype)initWithName:(NSString *)name vertexFile:(NSString *)vertexFileName fragmentFile:(NSString *)fragFileName viewC:(MaplyBaseViewController *)baseViewC
{
    NSError *error = nil;
    _name = name;
    NSString *vertexShader = [NSString stringWithContentsOfFile:vertexFileName encoding:NSASCIIStringEncoding error:&error];
    if (!vertexShader)
    {
        vertexFileName = [[NSBundle mainBundle] pathForResource:vertexFileName ofType:@""];
        vertexShader = [NSString stringWithContentsOfFile:vertexFileName encoding:NSASCIIStringEncoding error:&error];
        if (!vertexShader)
        {
            buildError = @"Could not find vertex shader file.";
            return nil;
        }
    }

    NSString *fragShader = [NSString stringWithContentsOfFile:fragFileName encoding:NSASCIIStringEncoding error:&error];
    if (!fragShader)
    {
        fragFileName = [[NSBundle mainBundle] pathForResource:fragFileName ofType:@""];
        fragShader = [NSString stringWithContentsOfFile:fragFileName encoding:NSASCIIStringEncoding error:&error];
        if (!fragShader)
        {
            buildError = @"Could not find fragment shader file.";
            return nil;
        }
    }
    
    return [self initWithName:name vertex:vertexShader fragment:fragShader viewC:baseViewC];
}

- (instancetype)initWithName:(NSString *)name vertex:(NSString *)vertexProg fragment:(NSString *)fragProg viewC:(MaplyBaseViewController *)baseViewC
{
    if (!vertexProg || !fragProg)
    {
        buildError = @"Empty vertex or fragment shader program.";
        return nil;
    }
    
    _name = name;
    std::string vertexStr = [vertexProg cStringUsingEncoding:NSASCIIStringEncoding];
    std::string fragStr = [fragProg cStringUsingEncoding:NSASCIIStringEncoding];
    std::string nameStr = [name cStringUsingEncoding:NSASCIIStringEncoding];

    EAGLContext *oldContext = [EAGLContext currentContext];
    [baseViewC useGLContext];
    _program = new OpenGLES2Program(nameStr,vertexStr,fragStr);
    if (oldContext)
        [EAGLContext setCurrentContext:oldContext];
    
    if (!_program->isValid())
    {
        buildError = @"Could not compile program.";
        delete _program;
        _program = NULL;
        return nil;
    }
    
    scene = baseViewC->scene;
    renderer = baseViewC->sceneRenderer;
    
    if (baseViewC->scene)
        baseViewC->scene->addProgram(_program);
    
    return self;
}

- (SimpleIdentity)getShaderID
{
    if (!_program)
        return EmptyIdentity;
    
    return _program->getId();
}

- (void)addTextureNamed:(NSString *)shaderAttrName image:(UIImage *)auxImage desc:(NSDictionary *)desc
{
    if ([NSThread currentThread] != [NSThread mainThread])
    {
        NSLog(@"Tried to add texture, but not on main thread");
        return;
    }
    
    if (!scene || !renderer)
        return;
    
    EAGLContext *oldContext = [EAGLContext currentContext];
    [renderer useContext];
    [renderer forceDrawNextFrame];
    
    Texture *auxTex = new Texture([_name cStringUsingEncoding:NSASCIIStringEncoding],auxImage);
    if ([desc[kMaplyTexMinFilter] isEqualToString:kMaplyMinFilterNearest])
        auxTex->setInterpType(GL_NEAREST);
    else if ([desc[kMaplyTexMinFilter] isEqualToString:kMaplyMinFilterLinear])
        auxTex->setInterpType(GL_LINEAR);
    SimpleIdentity auxTexId = auxTex->getId();
    auxTex->createInGL(scene->getMemManager());
    GLuint glTexId = auxTex->getGLId();
    scene->addChangeRequest(new AddTextureReq(auxTex));
    OpenGLES2Program *prog = scene->getProgramBySceneName([_name cStringUsingEncoding:NSASCIIStringEncoding]);
    if (prog)
    {
        prog->setTexture([shaderAttrName cStringUsingEncoding:NSASCIIStringEncoding], (int)glTexId);
    }
    
    texIDs.insert(auxTexId);
    
    if (oldContext != [EAGLContext currentContext])
        [EAGLContext setCurrentContext:oldContext];
}

- (void)addTextureNamed:(NSString *)shaderAttrName image:(UIImage *)auxImage
{
    [self addTextureNamed:shaderAttrName image:auxImage desc:nil];
}

- (bool)setUniformFloatNamed:(NSString *)uniName val:(float)val
{
    if (!_program)
        return false;
    
    EAGLContext *oldContext = [EAGLContext currentContext];
    [renderer useContext];
    [renderer forceDrawNextFrame];
    glUseProgram(_program->getProgram());

    std::string name = [uniName cStringUsingEncoding:NSASCIIStringEncoding];
    bool ret = _program->setUniform(name, val);

    if (oldContext != [EAGLContext currentContext])
        [EAGLContext setCurrentContext:oldContext];
    
    return ret;
}

- (bool)setUniformIntNamed:(NSString *)uniName val:(int)val
{
    if (!_program)
        return false;
    
    EAGLContext *oldContext = [EAGLContext currentContext];
    [renderer useContext];
    [renderer forceDrawNextFrame];
    glUseProgram(_program->getProgram());

    std::string name = [uniName cStringUsingEncoding:NSASCIIStringEncoding];
    bool ret = _program->setUniform(name, val);
    
    if (oldContext != [EAGLContext currentContext])
        [EAGLContext setCurrentContext:oldContext];
    
    return ret;
}

- (bool)setUniformVector2Named:(NSString *)uniName x:(float)x y:(float)y
{
    if (!_program)
        return false;
    
    EAGLContext *oldContext = [EAGLContext currentContext];
    [renderer useContext];
    [renderer forceDrawNextFrame];
    glUseProgram(_program->getProgram());

    std::string name = [uniName cStringUsingEncoding:NSASCIIStringEncoding];
    Point2f val(x,y);
    bool ret = _program->setUniform(name, val);

    if (oldContext != [EAGLContext currentContext])
        [EAGLContext setCurrentContext:oldContext];
    
    return ret;
}

- (bool)setUniformVector3Named:(NSString *)uniName x:(float)x y:(float)y z:(float)z
{
    if (!_program)
        return false;
    
    EAGLContext *oldContext = [EAGLContext currentContext];
    [renderer useContext];
    [renderer forceDrawNextFrame];
    glUseProgram(_program->getProgram());

    std::string name = [uniName cStringUsingEncoding:NSASCIIStringEncoding];
    Point3f val(x,y,z);
    bool ret = _program->setUniform(name, val);

    if (oldContext != [EAGLContext currentContext])
        [EAGLContext setCurrentContext:oldContext];
    
    return ret;
}

- (bool)setUniformVector4Named:(NSString *)uniName x:(float)x y:(float)y z:(float)z w:(float)w
{    
    if (!_program)
        return false;
    
    EAGLContext *oldContext = [EAGLContext currentContext];
    [renderer useContext];
    glUseProgram(_program->getProgram());

    std::string name = [uniName cStringUsingEncoding:NSASCIIStringEncoding];
    Eigen::Vector4f val;
    val.x() = x;  val.y() = y;  val.z() = z;  val.w() = w;
    bool ret = _program->setUniform(name, val);

    if (oldContext != [EAGLContext currentContext])
        [EAGLContext setCurrentContext:oldContext];
    
    return ret;
}

- (bool)setUniformVector4Named:(NSString *__nonnull)uniName x:(float)x y:(float)y z:(float)z w:(float)w index:(int)which
{
    if (!_program)
        return false;
    
    EAGLContext *oldContext = [EAGLContext currentContext];
    [renderer useContext];
    glUseProgram(_program->getProgram());
    
    std::string name = [uniName cStringUsingEncoding:NSASCIIStringEncoding];
    Eigen::Vector4f val;
    val.x() = x;  val.y() = y;  val.z() = z;  val.w() = w;
    bool ret = _program->setUniform(name, val, which);
    
    if (oldContext != [EAGLContext currentContext])
        [EAGLContext setCurrentContext:oldContext];
    
    return ret;
}

// We're assuming the view controller has set the proper context
- (void)teardown
{
    if (_program)
    {
//        _program->cleanUp();
//        delete _program;
        _program = NULL;
    }

    if (scene)
    {
        ChangeSet changes;
        for (SimpleIDSet::iterator it = texIDs.begin();it != texIDs.end(); ++it)
            changes.push_back(new RemTextureReq(*it));
        scene->addChangeRequests(changes);
    }
}

- (void)dealloc
{
}

- (bool)valid
{
    return _program != NULL;
}

- (NSString *)getError
{
    return buildError;
}


@end
