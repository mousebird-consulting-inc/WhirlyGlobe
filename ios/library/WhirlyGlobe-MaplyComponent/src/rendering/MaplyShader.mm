/*
 *  MaplyShader.mm
 *  WhirlyGlobe-MaplyComponent
 *
 *  Created by Steve Gifford on 2/7/13.
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

#import <UIKit/UIKit.h>
#import <string>
#import <WhirlyGlobe_iOS.h>
#import "MaplyShader_private.h"
#import "MaplyRenderController_private.h"
#import "TextureGLES_iOS.h"
#import "MaplyTexture_private.h"
#import "ProgramMTL.h"

using namespace WhirlyKit;

@implementation MaplyShader
{
    NSObject<MaplyRenderControllerProtocol> * __weak viewC;
    WhirlyKit::Scene *scene;
    SceneRendererRef renderer;
    NSString *buildError;
    // Texture we created for use in this shader
    SimpleIDSet texIDs;
    std::vector<std::string> varyings;
}

- (instancetype)initWithName:(NSString *)name vertexFile:(NSString *)vertexFileName fragmentFile:(NSString *)fragFileName viewC:(NSObject<MaplyRenderControllerProtocol> *)baseViewC
{
    if ([baseViewC getRenderControl]->sceneRenderer->getType() != SceneRenderer::RenderGLES)
    {
        NSLog(@"MaplyShader method only works with OpenGL ES");
        return nil;
    }
    
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

- (instancetype)initWithName:(NSString *)name vertex:(NSString *)vertexProg fragment:(NSString *)fragProg viewC:(NSObject<MaplyRenderControllerProtocol> *)baseViewC
{
    if ([baseViewC getRenderControl]->sceneRenderer->getType() != SceneRenderer::RenderGLES)
    {
        NSLog(@"MaplyShader method only works with OpenGL ES");
        return nil;
    }

    self = [super init];
    
    viewC = baseViewC;
    
    [self delayedSetupWithName:name vertex:vertexProg fragment:fragProg];
    
    return self;
}

- (instancetype)initMetalWithName:(NSString *)inName vertex:(id<MTLFunction>)vertexFunc fragment:(id<MTLFunction>)fragFunc viewC:(NSObject<MaplyRenderControllerProtocol> *)baseViewC
{
    if ([baseViewC getRenderControl]->sceneRenderer->getType() != SceneRenderer::RenderMetal)
    {
        NSLog(@"MaplyShader method only works with Metal");
        return nil;
    }
    
    self = [super init];
    viewC = baseViewC;
    
    std::string name = [inName cStringUsingEncoding:NSASCIIStringEncoding];
    ProgramMTLRef prog(new ProgramMTL(name,vertexFunc,fragFunc));
    _program = prog;

    MaplyRenderController *renderControl = [viewC getRenderControl];
    if (!renderControl)
        return nil;
    
    if (!_program->isValid())
    {
        buildError = @"Could not compile program.";
        _program = NULL;
        return nil;
    }
    
    scene = renderControl->scene;
    renderer = renderControl->sceneRenderer;
    
    if (renderControl->scene)
        renderControl->scene->addProgram(_program);

    _name = inName;
    
    return self;
}


- (instancetype)initWithViewC:(NSObject<MaplyRenderControllerProtocol> *)baseViewC
{
    if ([baseViewC getRenderControl]->sceneRenderer->getType() != SceneRenderer::RenderGLES)
    {
        NSLog(@"MaplyShader method only works with OpenGL ES");
        return nil;
    }

    self = [super init];

    viewC = baseViewC;

    return self;
}

- (instancetype)initWithProgram:(ProgramRef)program viewC:(NSObject<MaplyRenderControllerProtocol> * __nonnull)baseViewC
{
    if (!program)
        return nil;
    MaplyRenderController *renderControl = [baseViewC getRenderControl];
    if (!renderControl)
        return nil;

    self = [super init];
    _program = program;
    viewC = baseViewC;
    scene = renderControl->scene;
    renderer = renderControl->sceneRenderer;
    
    if (renderControl->scene)
        renderControl->scene->addProgram(_program);
    
    return self;
}

- (bool)delayedSetupWithName:(NSString *)name vertex:(NSString *)vertexProg fragment:(NSString *)fragProg
{
    if ([viewC getRenderControl]->sceneRenderer->getType() != SceneRenderer::RenderGLES)
    {
        NSLog(@"MaplyShader method only works with OpenGL ES");
        return nil;
    }

    if (!vertexProg || !fragProg)
    {
        buildError = @"Empty vertex or fragment shader program.";
        return false;
    }
    
    _name = name;
    std::string vertexStr = [vertexProg cStringUsingEncoding:NSASCIIStringEncoding];
    std::string fragStr = [fragProg cStringUsingEncoding:NSASCIIStringEncoding];
    std::string nameStr = [name cStringUsingEncoding:NSASCIIStringEncoding];
    
    MaplyRenderController *renderControl = [viewC getRenderControl];
    if (!renderControl)
        return false;
    
    CheckGLError("MaplyShader: delayedSetupWithName pre setCurrentContext");

    EAGLContext *oldContext = [EAGLContext currentContext];
    [renderControl useGLContext];
    _program = ProgramGLESRef(new ProgramGLES(nameStr,vertexStr,fragStr,(varyings.empty() ? NULL : &varyings)));
    if (oldContext)
        [EAGLContext setCurrentContext:oldContext];
    
    CheckGLError("MaplyShader: delayedSetupWithName setCurrentContext");
    
    if (!_program->isValid())
    {
        buildError = @"Could not compile program.";
        _program = NULL;
        return false;
    }
    
    scene = renderControl->scene;
    renderer = renderControl->sceneRenderer;
    
    if (renderControl->scene)
        renderControl->scene->addProgram(_program);
    
    return true;
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
 
    if (renderer->getType() != SceneRenderer::RenderGLES)
    {
        NSLog(@"MaplyShader method only works with OpenGL ES");
        return;
    }

    SceneRendererGLES_iOSRef sceneRenderGLES = std::dynamic_pointer_cast<SceneRendererGLES_iOS>(renderer);
    EAGLContext *oldContext = nil;
    if (sceneRenderGLES) {
        oldContext = [EAGLContext currentContext];
        sceneRenderGLES->useContext();
    }
    renderer->forceDrawNextFrame();

    TextureGLES_iOS *auxTex = new TextureGLES_iOS([_name cStringUsingEncoding:NSASCIIStringEncoding],auxImage);
    if ([desc[kMaplyTexMinFilter] isEqualToString:kMaplyMinFilterNearest])
        auxTex->setInterpType(TexInterpNearest);
    else if ([desc[kMaplyTexMinFilter] isEqualToString:kMaplyMinFilterLinear])
        auxTex->setInterpType(TexInterpLinear);
    SimpleIdentity auxTexId = auxTex->getId();
    auxTex->createInRenderer(renderer->getRenderSetupInfo());
    scene->addChangeRequest(new AddTextureReq(auxTex));
    if (_program)
    {
        _program->setTexture(StringIndexer::getStringID([shaderAttrName cStringUsingEncoding:NSASCIIStringEncoding]), auxTex, -1);
    }
    
    texIDs.insert(auxTexId);
    
    if (sceneRenderGLES && oldContext != [EAGLContext currentContext])
        [EAGLContext setCurrentContext:oldContext];
}

- (void)addTextureNamed:(NSString *)shaderAttrName image:(UIImage *)auxImage
{
    [self addTextureNamed:shaderAttrName image:auxImage desc:nil];
}

- (void)addVarying:(NSString *__nonnull)varyName
{
    std::string name = [varyName cStringUsingEncoding:NSASCIIStringEncoding];
    varyings.push_back(name);
}


- (bool)setUniformFloatNamed:(NSString *)uniName val:(float)val
{
    if (!_program || !scene || !renderer)
        return false;
    
    if (renderer->getType() != SceneRenderer::RenderGLES)
    {
        NSLog(@"MaplyShader method only works with OpenGL ES");
        return false;
    }

    CheckGLError("MaplyShader::setUniformFloatNamed: pre anything");

    SceneRendererGLES_iOSRef sceneRenderGLES = std::dynamic_pointer_cast<SceneRendererGLES_iOS>(renderer);
    EAGLContext *oldContext = nil;
    bool ret = false;
    if (sceneRenderGLES) {
        oldContext = [EAGLContext currentContext];
        sceneRenderGLES->useContext();
        
        renderer->forceDrawNextFrame();
        ProgramGLESRef programGLES = std::dynamic_pointer_cast<ProgramGLES>(_program);
        glUseProgram(programGLES->getProgram());
        CheckGLError("MaplyShader::setUniformFloatNamed: glUseProgram");

        std::string name = [uniName cStringUsingEncoding:NSASCIIStringEncoding];
        ret = programGLES->setUniform(StringIndexer::getStringID(name), val);
    }

    if (sceneRenderGLES && oldContext != [EAGLContext currentContext])
        [EAGLContext setCurrentContext:oldContext];
    
    return ret;
}

- (bool)setUniformFloatNamed:(NSString *__nonnull)uniName val:(float)val index:(int)idx
{
    if (!_program || !scene || !renderer)
        return false;

    if (renderer->getType() != SceneRenderer::RenderGLES)
    {
        NSLog(@"MaplyShader method only works with OpenGL ES");
        return false;
    }

    SceneRendererGLES_iOSRef sceneRenderGLES = std::dynamic_pointer_cast<SceneRendererGLES_iOS>(renderer);
    EAGLContext *oldContext = nil;
    bool ret = false;
    if (sceneRenderGLES) {
        oldContext = [EAGLContext currentContext];
        sceneRenderGLES->useContext();
        
        renderer->forceDrawNextFrame();
        ProgramGLESRef programGLES = std::dynamic_pointer_cast<ProgramGLES>(_program);
        glUseProgram(programGLES->getProgram());
        CheckGLError("MaplyShader::setUniformFloatNamed: glUseProgram");

        std::string name = [uniName cStringUsingEncoding:NSASCIIStringEncoding];
        ret = programGLES->setUniform(StringIndexer::getStringID(name), val, idx);
    }
    
    if (sceneRenderGLES && oldContext != [EAGLContext currentContext])
        [EAGLContext setCurrentContext:oldContext];
    
    return ret;
}

- (bool)setUniformIntNamed:(NSString *)uniName val:(int)val
{
    if (!_program || !scene || !renderer)
        return false;
    
    if (renderer->getType() != SceneRenderer::RenderGLES)
    {
        NSLog(@"MaplyShader method only works with OpenGL ES");
        return false;
    }

    SceneRendererGLES_iOSRef sceneRenderGLES = std::dynamic_pointer_cast<SceneRendererGLES_iOS>(renderer);
    EAGLContext *oldContext = nil;
    bool ret = false;
    if (sceneRenderGLES) {
        oldContext = [EAGLContext currentContext];
        sceneRenderGLES->useContext();
        
        renderer->forceDrawNextFrame();
        ProgramGLESRef programGLES = std::dynamic_pointer_cast<ProgramGLES>(_program);
        glUseProgram(programGLES->getProgram());
        CheckGLError("MaplyShader::setUniformIntNamed: glUseProgram");

        std::string name = [uniName cStringUsingEncoding:NSASCIIStringEncoding];
        ret = programGLES->setUniform(StringIndexer::getStringID(name), val);
    }
    
    if (sceneRenderGLES && oldContext != [EAGLContext currentContext])
        [EAGLContext setCurrentContext:oldContext];
    
    return ret;
}

- (bool)setUniformVector2Named:(NSString *)uniName x:(float)x y:(float)y
{
    if (!_program || !scene || !renderer)
        return false;
    
    if (renderer->getType() != SceneRenderer::RenderGLES)
    {
        NSLog(@"MaplyShader method only works with OpenGL ES");
        return false;
    }

    SceneRendererGLES_iOSRef sceneRenderGLES = std::dynamic_pointer_cast<SceneRendererGLES_iOS>(renderer);
    EAGLContext *oldContext = nil;
    bool ret = false;
    if (sceneRenderGLES) {
        oldContext = [EAGLContext currentContext];
        sceneRenderGLES->useContext();
        
        renderer->forceDrawNextFrame();
        ProgramGLESRef programGLES = std::dynamic_pointer_cast<ProgramGLES>(_program);
        glUseProgram(programGLES->getProgram());
        CheckGLError("MaplyShader::setUniformVector2Named: glUseProgram");

        std::string name = [uniName cStringUsingEncoding:NSASCIIStringEncoding];
        Point2f val(x,y);
        ret = programGLES->setUniform(StringIndexer::getStringID(name), val);
    }

    if (sceneRenderGLES && oldContext != [EAGLContext currentContext])
        [EAGLContext setCurrentContext:oldContext];
    
    return ret;
}

- (bool)setUniformVector3Named:(NSString *)uniName x:(float)x y:(float)y z:(float)z
{
    if (!_program || !scene || !renderer)
        return false;
    
    if (renderer->getType() != SceneRenderer::RenderGLES)
    {
        NSLog(@"MaplyShader method only works with OpenGL ES");
        return false;
    }

    SceneRendererGLES_iOSRef sceneRenderGLES = std::dynamic_pointer_cast<SceneRendererGLES_iOS>(renderer);
    EAGLContext *oldContext = nil;
    bool ret = false;
    if (sceneRenderGLES) {
        oldContext = [EAGLContext currentContext];
        sceneRenderGLES->useContext();
        
        renderer->forceDrawNextFrame();
        ProgramGLESRef programGLES = std::dynamic_pointer_cast<ProgramGLES>(_program);
        glUseProgram(programGLES->getProgram());
        CheckGLError("MaplyShader::setUniformVector3Named: glUseProgram");

        std::string name = [uniName cStringUsingEncoding:NSASCIIStringEncoding];
        Point3f val(x,y,z);
        ret = programGLES->setUniform(StringIndexer::getStringID(name), val);
    }

    if (sceneRenderGLES && oldContext != [EAGLContext currentContext])
        [EAGLContext setCurrentContext:oldContext];
    
    return ret;
}

- (bool)setUniformVector4Named:(NSString *)uniName x:(float)x y:(float)y z:(float)z w:(float)w
{    
    if (!_program || !scene || !renderer)
        return false;
    
    if (renderer->getType() != SceneRenderer::RenderGLES)
    {
        NSLog(@"MaplyShader method only works with OpenGL ES");
        return false;
    }

    SceneRendererGLES_iOSRef sceneRenderGLES = std::dynamic_pointer_cast<SceneRendererGLES_iOS>(renderer);
    EAGLContext *oldContext = nil;
    bool ret = false;
    if (sceneRenderGLES) {
        oldContext = [EAGLContext currentContext];
        sceneRenderGLES->useContext();
        
        renderer->forceDrawNextFrame();
        ProgramGLESRef programGLES = std::dynamic_pointer_cast<ProgramGLES>(_program);
        glUseProgram(programGLES->getProgram());
        CheckGLError("MaplyShader::setUniformVector4Named: glUseProgram");

        std::string name = [uniName cStringUsingEncoding:NSASCIIStringEncoding];
        Eigen::Vector4f val;
        val.x() = x;  val.y() = y;  val.z() = z;  val.w() = w;
        ret = programGLES->setUniform(StringIndexer::getStringID(name), val);
    }

    if (sceneRenderGLES && oldContext != [EAGLContext currentContext])
        [EAGLContext setCurrentContext:oldContext];
    
    return ret;
}

- (bool)setUniformVector4Named:(NSString *__nonnull)uniName x:(float)x y:(float)y z:(float)z w:(float)w index:(int)which
{
    if (!_program || !scene || !renderer)
        return false;
    
    if (renderer->getType() != SceneRenderer::RenderGLES)
    {
        NSLog(@"MaplyShader method only works with OpenGL ES");
        return false;
    }

    SceneRendererGLES_iOSRef sceneRenderGLES = std::dynamic_pointer_cast<SceneRendererGLES_iOS>(renderer);
    EAGLContext *oldContext = nil;
    bool ret = false;
    if (sceneRenderGLES) {
        oldContext = [EAGLContext currentContext];
        sceneRenderGLES->useContext();
        
        renderer->forceDrawNextFrame();
        ProgramGLESRef programGLES = std::dynamic_pointer_cast<ProgramGLES>(_program);
        glUseProgram(programGLES->getProgram());
        CheckGLError("MaplyShader::setUniformVector4Named: glUseProgram");

        std::string name = [uniName cStringUsingEncoding:NSASCIIStringEncoding];
        Eigen::Vector4f val;
        val.x() = x;  val.y() = y;  val.z() = z;  val.w() = w;
        ret = programGLES->setUniform(StringIndexer::getStringID(name), val, which);
    }
    
    if (sceneRenderGLES && oldContext != [EAGLContext currentContext])
        [EAGLContext setCurrentContext:oldContext];
    
    return ret;
}

- (bool)setUniformVector4Named:(NSString *__nonnull)uniName color:(UIColor *)color
{
    CGFloat red,green,blue,alpha;
    [color getRed:&red green:&green blue:&blue alpha:&alpha];
    return [self setUniformVector4Named:uniName x:red y:green z:blue w:alpha];
}

- (bool)setUniformVector4Named:(NSString *__nonnull)uniName color:(UIColor *)color index:(int)which
{
    CGFloat red,green,blue,alpha;
    [color getRed:&red green:&green blue:&blue alpha:&alpha];
    return [self setUniformVector4Named:uniName x:red y:green z:blue w:alpha index:which];
}

- (void)setTexture:(MaplyTexture *)tex forIndex:(int)idx
{
    if (!_program || !scene || !renderer)
        return;
    
    if (renderer->getType() != SceneRenderer::RenderMetal)
    {
        NSLog(@"MaplyShader method only works with Metal");
        return;
    }

    ProgramMTL *programMTL = (ProgramMTL *)_program.get();
    scene->addChangeRequest(new ShaderAddTextureReq(programMTL->getId(),-1,tex.texID,idx));
}

- (bool)setUniformBlock:(NSData *__nonnull)uniBlock buffer:(int)bufferID
{
    if (!_program || !scene || !renderer)
        return false;
    
    if (renderer->getType() != SceneRenderer::RenderMetal)
    {
        NSLog(@"MaplyShader method only works with Metal");
        return false;
    }
    
    ProgramMTL *programMTL = (ProgramMTL *)_program.get();
    
    RawNSDataReaderRef dataWrap = RawNSDataReaderRef(new RawNSDataReader(uniBlock));
    scene->addChangeRequest(new ProgramUniformBlockSetRequest(programMTL->getId(),dataWrap,bufferID));
    
    return true;
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

- (bool)valid
{
    return _program != NULL;
}

- (NSString *)getError
{
    return buildError;
}


@end
