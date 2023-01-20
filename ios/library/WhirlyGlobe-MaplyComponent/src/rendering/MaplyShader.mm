/*  MaplyShader.mm
 *  WhirlyGlobe-MaplyComponent
 *
 *  Created by Steve Gifford on 2/7/13.
 *  Copyright 2011-2023 mousebird consulting
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

#import <UIKit/UIKit.h>
#import <string>
#import <WhirlyGlobe_iOS.h>
#import "MaplyShader_private.h"
#import "MaplyRenderController_private.h"
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
    std::vector<MaplyTexture *> textures;  // Used to sit on the textures so they aren't deleted
}

- (instancetype _Nullable)initMetalWithName:(NSString *)inName
                                     vertex:(id<MTLFunction>)vertexFunc
                                   fragment:(id<MTLFunction>)fragFunc
                                      viewC:(NSObject<MaplyRenderControllerProtocol> *)baseViewC
{
    if (!vertexFunc) {
        NSLog(@"Passed in nil function to MaplyShader::initMetalWithName");
        return nil;
    }
    
    if ([baseViewC getRenderControl]->sceneRenderer->getType() != SceneRenderer::RenderMetal)
    {
        NSLog(@"MaplyShader method only works with Metal");
        return nil;
    }
    
    if (!(self = [super init]))
    {
        return nil;
    }

    viewC = baseViewC;
    
    std::string name = [inName cStringUsingEncoding:NSASCIIStringEncoding withDefault:""];
    ProgramMTLRef prog(new ProgramMTL(name,vertexFunc,fragFunc));
    _program = prog;

    MaplyRenderController *renderControl = [baseViewC getRenderControl];
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


- (instancetype _Nullable)initWithProgram:(ProgramRef)program
                                    viewC:(NSObject<MaplyRenderControllerProtocol> * __nonnull)baseViewC
{
    if (!program)
        return nil;

    MaplyRenderController *renderControl = [baseViewC getRenderControl];
    if (!renderControl)
        return nil;

    if (!(self = [super init]))
    {
        return nil;
    }

    _program = program;
    viewC = baseViewC;
    scene = renderControl->scene;
    renderer = renderControl->sceneRenderer;
    
    if (renderControl->scene)
        renderControl->scene->addProgram(_program);
    
    return self;
}

- (SimpleIdentity)getShaderID
{
    return _program ? _program->getId() : EmptyIdentity;
}

- (void)setTexture:(MaplyTexture * __nonnull)tex forIndex:(int)idx
             viewC:(NSObject<MaplyRenderControllerProtocol> * __nonnull)view
{
    if (!_program || !scene || !renderer)
        return;
    
    if (renderer->getType() != SceneRenderer::RenderMetal)
    {
        NSLog(@"MaplyShader method only works with Metal");
        return;
    }

    ProgramMTL *programMTL = (ProgramMTL *)_program.get();
    textures.push_back(tex);
    scene->addChangeRequest(new ShaderAddTextureReq(programMTL->getId(),-1,tex.texID,idx));
}

- (void)removeTexture:(MaplyTexture *)tex
                viewC:(NSObject<MaplyRenderControllerProtocol> *)viewC
{
    if (!_program || !scene || !renderer || tex.texID == EmptyIdentity)
        return;
    
    if (renderer->getType() != SceneRenderer::RenderMetal)
    {
        NSLog(@"MaplyShader method only works with Metal");
        return;
    }
    
    ProgramMTL *programMTL = (ProgramMTL *)_program.get();
    std::vector<int> entries;
    int which = 0;
    for (auto thisTex: textures) {
        if (thisTex.texID == tex.texID) {
            entries.push_back(which);
        }
        which++;
    }
    if (!entries.empty()) {
        scene->addChangeRequest(new ShaderRemTextureReq(programMTL->getId(),tex.texID));
    }
    for (auto entry = entries.rbegin(); entry != entries.rend(); ++entry)
        textures.erase(textures.begin()+*entry);
}

- (bool)setUniformBlock:(NSData *__nonnull)uniBlock buffer:(int)bufferID
{
    const auto programMTL = dynamic_cast<ProgramMTL *>(_program.get());
    if (!programMTL || !scene || !renderer)
    {
        return false;
    }
    
    if (renderer->getType() != SceneRenderer::RenderMetal)
    {
        NSLog(@"MaplyShader method only works with Metal");
        return false;
    }
    
    auto dataWrap = std::make_shared<RawNSDataReader>(uniBlock);
    auto req = new ProgramUniformBlockSetRequest(programMTL->getId(),std::move(dataWrap),bufferID);
    scene->addChangeRequest(req);

    return true;
}

- (void)setReduceMode:(bool)reduceMode
{
    if (_program)
    {
        _program->setReduceMode(reduceMode ? Program::TextureReduce : Program::None);
    }
}

// We're assuming the view controller has set the proper context
- (void)teardown
{
    _program.reset();

    if (scene)
    {
        ChangeSet changes;
        changes.reserve(texIDs.size());
        for (SimpleIDSet::iterator it = texIDs.begin();it != texIDs.end(); ++it)
        {
            changes.push_back(new RemTextureReq(*it));
        }
        scene->addChangeRequests(changes);
    }
}

- (bool)valid
{
    return _program.operator bool();
}

- (NSString *)getError
{
    return buildError;
}


@end
