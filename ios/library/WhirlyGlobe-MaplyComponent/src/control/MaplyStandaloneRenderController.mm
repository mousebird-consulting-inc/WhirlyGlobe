/*
*  MaplyStandaloneRenderController.cpp
*  WhirlyGlobe
*
*  Created by Steve Gifford on 10/19/23.
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
*
*/

#import "MaplyGlobeRenderController.h"
#import "MaplyRenderController_private.h"
#import "MaplyRenderTarget_private.h"
#import "MaplyStandaloneRenderController.h"
#import "SceneRendererMTL.h"
#import <CoreGraphics/CoreGraphics.h>

using namespace Eigen;
using namespace WhirlyKit;
using namespace WhirlyGlobe;
using namespace Maply;

@implementation MaplyStandaloneRenderController
{
    Maply::FlatViewRef flatView;
    CoordSystemRef coordSys;
    CoordSystemDisplayAdapterRef coordAdapterRef;
    bool _clearToLoad2;
    MaplyShader *swizzleShader;
    TextureMTLRef srcTex,alphaTex;
    RenderTargetMTLRef alphaTarget;
}

- (instancetype __nullable)initWithSize:(CGSize)size separateAlpha:(bool)separateAlpha
{
    // For now we just use Plate Carree and cover the whole extents
    const auto ll = GeoCoordD::CoordFromDegrees(-180.0,-90.0);
    const auto ur = GeoCoordD::CoordFromDegrees(180.0,90.0);

    coordSys = std::make_shared<PlateCarreeCoordSystem>();
    coordSys->setCanBeWrapped(true);
    Point3d scale(1.0,1.0,1.0);
    Point3d center(0.0,0.0,0.0);
    coordAdapterRef = std::make_shared<GeneralCoordSystemDisplayAdapter>(coordSys.get(),
                                                                      Point3d(ll.x(),ll.y(),0.0),
                                                                      Point3d(ur.x(),ur.y(),0.0),
                                                                      center,
                                                                      scale);
    coordAdapter = coordAdapterRef.get();
    
    flatView = std::make_shared<FlatView>(coordAdapter);
    flatView->setWindow(Point2d(size.width,size.height), Point2d(0.0,0.0));
    visualView = flatView;
    
    self = [super initWithSize:size mode:MaplyRenderMetal];
    [self clearLights];
    
    auto defaultTarget = std::dynamic_pointer_cast<RenderTargetMTL>(sceneRenderer->getDefaultRenderTarget());
    if (defaultTarget) {
        defaultTarget->blendEnable = true;
        defaultTarget->clearEveryFrame = false;
    }
    _clearToLoad2 = false;
    
    // Set up a texture to render to if we're splitting out alpha
    if (separateAlpha) {
        srcTex = std::make_shared<TextureMTL>("alpha src");
        alphaTex = std::make_shared<TextureMTL>("alpha dest");
        scene->addTexture(srcTex);
        scene->addTexture(alphaTex);
    }
    
    return self;
}

- (void)setClearToLoad:(bool)clearToLoad
{
    _clearToLoad2 = clearToLoad;
    auto defaultTarget = std::dynamic_pointer_cast<RenderTargetMTL>(sceneRenderer->getDefaultRenderTarget());
    if (defaultTarget) {
        if (clearToLoad) {
            defaultTarget->blendEnable = false;
            defaultTarget->clearEveryFrame = true;
        } else {
            defaultTarget->blendEnable = true;
            defaultTarget->clearEveryFrame = false;
        }
    }
}

- (void)textureToImage:(id<MTLTexture>)texture {
    CIContext *context = [[CIContext alloc] initWithOptions:nil];
    CIImage *ciImage = [[CIImage alloc] initWithMTLTexture:texture options:nil];
    CGAffineTransform transform = CGAffineTransformMake(1, 0, 0, -1, 0, ciImage.extent.size.height);
    ciImage = [ciImage imageByApplyingTransform:transform];
    CGImageRef cgImage = [context createCGImage:ciImage fromRect:ciImage.extent];
    UIImage *image = [[UIImage alloc] initWithCGImage:cgImage];
    NSLog(@"");
}

- (BOOL)renderTo:(id<MTLTexture>)colorTexture alphaTex:(id<MTLTexture>)alphaTexture period:(NSTimeInterval)howLong
{
    SceneRendererMTL::RenderInfoMTL renderInfo;
    
    MTLRenderPassDescriptor *desc = [[MTLRenderPassDescriptor alloc] init];
    desc.colorAttachments[0].texture = colorTexture;
    if (_clearToLoad2) {
        desc.colorAttachments[0].loadAction = MTLLoadActionClear;
    } else {
        desc.colorAttachments[0].loadAction = MTLLoadActionLoad;
    }
    desc.colorAttachments[0].loadAction = MTLLoadActionLoad;
    desc.colorAttachments[0].storeAction = MTLStoreActionStore;
    desc.colorAttachments[0].clearColor = MTLClearColorMake(0.0, 0.0, 0.0, 0.0);
    renderInfo.renderPassDesc = desc;
    
    if (!sceneRenderer)
        return false;

    SceneRendererMTLRef sceneRendererMTL = std::dynamic_pointer_cast<SceneRendererMTL>(sceneRenderer);
    
    // Set up a separate target for the alpha texture where we swizzle the bits
    if (alphaTexture && alphaTex) {
        srcTex->setMTLTex(colorTexture);
        alphaTex->setMTLTex(alphaTexture);

        // Set up the target once, but we need a texture to do it
        if (!alphaTarget) {
            auto defaultTarget = std::dynamic_pointer_cast<RenderTargetMTL>(sceneRenderer->getDefaultRenderTarget());

            // Set up a render target to put the texture around
            alphaTarget = std::make_shared<RenderTargetMTL>();
            alphaTarget->clearOnce = false;
            alphaTarget->clearEveryFrame = true;
            alphaTarget->init(sceneRenderer.get(),scene,alphaTex->getId());
            sceneRenderer.get()->addRenderTarget(alphaTarget,true);

            // Set up a rectangle right over the view to render the render target
            Rectangle *newRect = new Rectangle();
            newRect->ll = Point3d(-1.0,-1.0,0.0);
            newRect->ur = Point3d(1.0,1.0,0.0);
            newRect->clipCoords = true;
            newRect->texIDs.push_back(srcTex->getId());
            std::vector<Shape *> shapes;
            shapes.push_back(newRect);

            id<MTLFunction> vertProg = [[self getMetalLibrary] newFunctionWithName:@"vertexTri_multiTex"];
            if (!vertProg) {
                NSLog(@"VariableShader: Couldn't find vertexTri_multiTex in WG Shader lib.");
                return false;
            }
            id<MTLFunction> fragProg = [[self getMetalLibrary]  newFunctionWithName:@"fragmentTri_alphaSwizzle"];
            if (!fragProg) {
                NSLog(@"VariableShader: Couldn't find fragmentTri_alphaSwizzle in Carrot shader library.");
                return false;
            }
            swizzleShader = [[MaplyShader alloc] initMetalWithName:@"Alpha Swizzle Shader" vertex:vertProg fragment:fragProg viewC:self];

            ChangeSet changes;
            ShapeInfo info;
            info.programID = [swizzleShader getShaderID];
            info.drawPriority = 10000000;
            info.drawableName = "MaplyVariableTarget-Rect";
            info.renderTargetID = alphaTarget->getId();
            info.zBufferRead = false;
            info.zBufferWrite = false;
            info.hasCenter = true;
            const auto shapeManager = scene->getManager<ShapeManager>(kWKShapeManager);
            shapeManager->addShapes(shapes, info, changes);
            scene->addChangeRequests(changes);
        }
    }
    
    // Note: Have to do this to trigger some of our callbacks
    sceneRendererMTL->hasChanges();
    
    sceneRendererMTL->forceDrawNextFrame();
    sceneRendererMTL->render(howLong,&renderInfo);
    
//    if (alphaTexture)
//        [self textureToImage:alphaTexture];
    
    return true;
}

@end

