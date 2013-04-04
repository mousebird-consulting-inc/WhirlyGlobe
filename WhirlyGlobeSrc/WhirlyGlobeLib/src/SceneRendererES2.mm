/*
 *  SceneRendererES2.mm
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

#import "SceneRendererES2.h"
#import "UIColor+Stuff.h"
#import "GLUtils.h"

using namespace Eigen;
using namespace WhirlyKit;

namespace WhirlyKit
{

// Alpha stuff goes at the end
// Otherwise sort by draw priority
class DrawListSortStruct2
{
public:
    DrawListSortStruct2(bool useAlpha,bool useLines,WhirlyKitRendererFrameInfo *frameInfo) : useAlpha(useAlpha), useLines(useLines), frameInfo(frameInfo)
    {
    }
    DrawListSortStruct2() { }
    DrawListSortStruct2(const DrawListSortStruct &that) : useAlpha(that.useAlpha), useLines(that.useLines), frameInfo(that.frameInfo)
    {
    }
    DrawListSortStruct2 & operator = (const DrawListSortStruct &that)
    {
        useAlpha = that.useAlpha;
        useLines= that.useLines;
        frameInfo = that.frameInfo;
        return *this;
    }
    bool operator()(Drawable *a,Drawable *b)
    {
        if (useLines)
        {
            bool linesA = (a->getType() == GL_LINES) || (a->getType() == GL_LINE_LOOP) || (a->getType() == GL_POINTS) || a->getForceZBufferOn();
            bool linesB = (b->getType() == GL_LINES) || (b->getType() == GL_LINE_LOOP) || (b->getType() == GL_POINTS) || b->getForceZBufferOn();
            if (linesA != linesB)
                return !linesA;
        }
        // We may or may not sort all alpha containing drawables to the end
        if (useAlpha)
            if (a->hasAlpha(frameInfo) != b->hasAlpha(frameInfo))
                return !a->hasAlpha(frameInfo);
                
        return a->getDrawPriority() < b->getDrawPriority();
    }
    
    bool useAlpha,useLines;
    WhirlyKitRendererFrameInfo * __unsafe_unretained frameInfo;
};
    
}

@implementation WhirlyKitSceneRendererES2
{
    NSMutableArray *lights;
    CFTimeInterval lightsLastUpdated;
    WhirlyKitMaterial *defaultMat;
}

- (id) init
{
    self = [super initWithOpenGLESVersion:kEAGLRenderingAPIOpenGLES2];
    lights = [NSMutableArray array];
    
    // Add a simple default light
    WhirlyKitDirectionalLight *light = [[WhirlyKitDirectionalLight alloc] init];
    light->pos = Vector3f(0.75, 0.5, -1.0);
    light->viewDependent = true;
    light->ambient = Vector4f(0.6, 0.6, 0.6, 1.0);
    light->diffuse = Vector4f(0.5, 0.5, 0.5, 1.0);
    light->specular = Vector4f(0, 0, 0, 0);
    [self addLight:light];

    // And a basic material
    [self setDefaultMaterial:[[WhirlyKitMaterial alloc] init]];

    lightsLastUpdated = CFAbsoluteTimeGetCurrent();

    return self;
}

static const char *vertexShaderTri =
"struct directional_light {\n"
"  vec3 direction;\n"
"  vec3 halfplane;\n"
"  vec4 ambient;\n"
"  vec4 diffuse;\n"
"  vec4 specular;\n"
"  float viewdepend;\n"
"};\n"
"\n"
"struct material_properties {\n"
"  vec4 ambient;\n"
"  vec4 diffuse;\n"
"  vec4 specular;\n"
"  float specular_exponent;\n"
"};\n"
"\n"
"uniform mat4  u_mvpMatrix;                   \n"
"uniform float u_fade;                        \n"
"uniform int u_numLights;                      \n"
"uniform directional_light light[8];                     \n"
"uniform material_properties material;       \n"
"\n"
"attribute vec3 a_position;                  \n"
"attribute vec2 a_texCoord;                  \n"
"attribute vec4 a_color;                     \n"
"attribute vec3 a_normal;                    \n"
"\n"
"varying vec2 v_texCoord;                    \n"
"varying vec4 v_color;                       \n"
"\n"
"void main()                                 \n"
"{                                           \n"
"   v_texCoord = a_texCoord;                 \n"
"   v_color = vec4(0.0,0.0,0.0,0.0);         \n"
"   if (u_numLights > 0)                     \n"
"   {\n"
"     vec4 ambient = vec4(0.0,0.0,0.0,0.0);         \n"
"     vec4 diffuse = vec4(0.0,0.0,0.0,0.0);         \n"
"     for (int ii=0;ii<8;ii++)                 \n"
"     {\n"
"        if (ii>=u_numLights)                  \n"
"           break;                             \n"
"        vec3 adjNorm = light[ii].viewdepend > 0.0 ? normalize((u_mvpMatrix * vec4(a_normal.xyz, 0.0)).xyz) : a_normal.xzy;\n"
"        float ndotl;\n"
//"        float ndoth;\n"
"        ndotl = max(0.0, dot(adjNorm, light[ii].direction));\n"
//"        ndotl = pow(ndotl,0.5);\n"
//"        ndoth = max(0.0, dot(adjNorm, light[ii].halfplane));\n"
"        ambient += light[ii].ambient;\n"
"        diffuse += ndotl * light[ii].diffuse;\n"
"     }\n"
"     v_color = vec4(ambient.xyz * material.ambient.xyz * a_color.xyz + diffuse.xyz * a_color.xyz,a_color.a);\n"
"   } else {\n"
"     v_color = a_color;\n"
"   }\n"
"\n"
"   gl_Position = u_mvpMatrix * vec4(a_position,1.0);  \n"
"}                                           \n"
;

static const char *fragmentShaderTri =
"precision mediump float;                            \n"
"\n"
"uniform sampler2D s_baseMap;                        \n"
"uniform bool  u_hasTexture;                         \n"
"\n"
"varying vec2      v_texCoord;                       \n"
"varying vec4      v_color;                          \n"
"\n"
"void main()                                         \n"
"{                                                   \n"
"  vec4 baseColor = u_hasTexture ? texture2D(s_baseMap, v_texCoord) : vec4(1.0,1.0,1.0,1.0); \n"
//"  if (baseColor.a < 0.1)                            \n"
//"      discard;                                      \n"
"  gl_FragColor = v_color * baseColor;  \n"
"}                                                   \n"
;

static const char *vertexShaderLine =
"uniform mat4  u_mvpMatrix;                   \n"
"\n"
"attribute vec3 a_position;                  \n"
"attribute vec4 a_color;                     \n"
"\n"
"varying vec4      v_color;                          \n"
"\n"
"void main()                                 \n"
"{                                           \n"
"   v_color = a_color;                       \n"
"   gl_Position = u_mvpMatrix * vec4(a_position,1.0);  \n"
"}                                           \n"
;

static const char *fragmentShaderLine =
"precision mediump float;                            \n"
"\n"
"varying vec4      v_color;                          \n"
"\n"
"void main()                                         \n"
"{                                                   \n"
"  gl_FragColor = v_color;  \n"
"}                                                   \n"
;

// When the scene is set, we'll compile our shaders
- (void)setScene:(WhirlyKit::Scene *)inScene
{
    scene = inScene;

    EAGLContext *oldContext = [EAGLContext currentContext];
    if (oldContext != context)
        [EAGLContext setCurrentContext:context];
    
    // Provider default shader programs if we don't already have them
    SimpleIdentity triShaderID,lineShaderID;
    scene->getDefaultProgramIDs(triShaderID, lineShaderID);
    if (triShaderID == EmptyIdentity || lineShaderID == EmptyIdentity)
    {
        OpenGLES2Program *triShader = new OpenGLES2Program("Default Triangle Program",vertexShaderTri,fragmentShaderTri);
        OpenGLES2Program *lineShader = new OpenGLES2Program("Default Line Program",vertexShaderLine,fragmentShaderLine);
        if (!triShader->isValid() || !lineShader->isValid())
        {
            NSLog(@"SceneRendererES2: Shader didn't compile.  Nothing will work.");
            delete triShader;
            delete lineShader;
        } else {
            scene->setDefaultPrograms(triShader,lineShader);
        }
    }

    lightsLastUpdated = CFAbsoluteTimeGetCurrent();

    if (oldContext != context)
        [EAGLContext setCurrentContext:oldContext];
}

/// Add a light to the existing set
- (void)addLight:(WhirlyKitDirectionalLight *)light
{
    if (!lights)
        lights = [NSMutableArray array];
    [lights addObject:light];
    lightsLastUpdated = CFAbsoluteTimeGetCurrent();
    triggerDraw = true;
}

/// Replace all the lights at once. nil turns off lighting
- (void)replaceLights:(NSArray *)inLights
{
    lights = [NSMutableArray arrayWithArray:inLights];
    lightsLastUpdated = CFAbsoluteTimeGetCurrent();
    triggerDraw = true;
}

- (void)setDefaultMaterial:(WhirlyKitMaterial *)mat
{
    defaultMat = mat;
    lightsLastUpdated = CFAbsoluteTimeGetCurrent();
    triggerDraw = true;
}


- (BOOL)resizeFromLayer:(CAEAGLLayer *)layer
{
    bool ret = [super resizeFromLayer:layer];
    
    return ret;
}

// Make the screen a bit bigger for testing
static const float ScreenOverlap = 0.1;

- (void) render:(CFTimeInterval)duration
{
    if (framebufferWidth <= 0 || framebufferHeight <= 0)
        return;
    
	[theView animate];

    // Decide if we even need to draw
    if (!scene->hasChanges() && ![self viewDidChange])
        return;
    
    lastDraw = CFAbsoluteTimeGetCurrent();
    
    if (perfInterval > 0)
        perfTimer.startTiming("aaRender");
    
	if (frameCountStart)
		frameCountStart = CFAbsoluteTimeGetCurrent();
	
    if (perfInterval > 0)
        perfTimer.startTiming("Render Setup");
    
    EAGLContext *oldContext = [EAGLContext currentContext];
    if (oldContext != context)
        [EAGLContext setCurrentContext:context];
    CheckGLError("SceneRendererES2: setCurrentContext");
    
    // Turn on blending
    // Note: Only need to do this once
	glBlendFunc(GL_ONE, GL_ONE_MINUS_SRC_ALPHA);
    glEnable(GL_BLEND);

    // See if we're dealing with a globe view
    WhirlyGlobeView *globeView = nil;
    if ([theView isKindOfClass:[WhirlyGlobeView class]])
        globeView = (WhirlyGlobeView *)theView;

    glBindFramebuffer(GL_FRAMEBUFFER, defaultFramebuffer);
    CheckGLError("SceneRendererES2: glBindFramebuffer");
    glViewport(0, 0, framebufferWidth, framebufferHeight);
    CheckGLError("SceneRendererES2: glViewport");

    // Get the model and view matrices
    Eigen::Matrix4f modelTrans = [theView calcModelMatrix];
    Eigen::Matrix4f viewTrans = [theView calcViewMatrix];
    
    // Set up a projection matrix
    // Borrowed from the "OpenGL ES 2.0 Programming" book
	Point2f frustLL,frustUR;
	GLfloat near=0,far=0;
	[theView calcFrustumWidth:framebufferWidth height:framebufferHeight ll:frustLL ur:frustUR near:near far:far];
    Eigen::Matrix4f projMat;
    Point3f delta(frustUR.x()-frustLL.x(),frustUR.y()-frustLL.y(),far-near);
    projMat.setIdentity();
    projMat(0,0) = 2.0f * near / delta.x();
    projMat(1,0) = projMat(2,0) = projMat(3,0) = 0.0f;

    projMat(1,1) = 2.0f * near / delta.y();
    projMat(0,1) = projMat(2,1) = projMat(3,1) = 0.0f;

    projMat(0,2) = (frustUR.x()+frustLL.x()) / delta.x();
    projMat(1,2) = (frustUR.y()+frustLL.y()) / delta.y();
    projMat(2,2) = -(near + far ) / delta.z();
    projMat(3,2) = -1.0f;
    
    projMat(2,3) = -2.0f * near * far / delta.z();
    projMat(0,3) = projMat(1,3) = projMat(3,3) = 0.0f;
    
    Eigen::Matrix4f modelAndViewMat = viewTrans * modelTrans;
    Eigen::Matrix4f mvpMat = projMat * (modelAndViewMat);
    
    switch (zBufferMode)
    {
        case zBufferOn:
            glDepthMask(GL_TRUE);
            glEnable(GL_DEPTH_TEST);
            break;
        case zBufferOff:
            glDepthMask(GL_FALSE);
            glDisable(GL_DEPTH_TEST);
            break;
        case zBufferOffUntilLines:
            glDepthMask(GL_TRUE);
            glEnable(GL_DEPTH_TEST);
            glDepthFunc(GL_ALWAYS);
            break;
    }
    
	glClearColor(clearColor.r / 255.0, clearColor.g / 255.0, clearColor.b / 255.0, clearColor.a / 255.0);
    CheckGLError("SceneRendererES2: glClearColor");
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    CheckGLError("SceneRendererES2: glClear");

	glEnable(GL_CULL_FACE);
    CheckGLError("SceneRendererES2: glEnable(GL_CULL_FACE)");
    
    if (perfInterval > 0)
        perfTimer.stopTiming("Render Setup");
    
	if (scene)
	{
		numDrawables = 0;
        
        SimpleIdentity defaultTriShader,defaultLineShader;
        scene->getDefaultProgramIDs(defaultTriShader, defaultLineShader);
        
        WhirlyKitRendererFrameInfo *frameInfo = [[WhirlyKitRendererFrameInfo alloc] init];
        frameInfo.oglVersion = kEAGLRenderingAPIOpenGLES2;
        frameInfo.sceneRenderer = self;
        frameInfo.theView = theView;
        frameInfo.modelTrans = modelTrans;
        frameInfo.scene = scene;
        frameInfo.frameLen = duration;
        frameInfo.currentTime = CFAbsoluteTimeGetCurrent();
        frameInfo.projMat = projMat;
        frameInfo.mvpMat = mvpMat;
        frameInfo.viewAndModelMat = modelAndViewMat;
        frameInfo.lights = lights;
		
        if (perfInterval > 0)
            perfTimer.startTiming("Scene processing");
        
        // Let the active models to their thing
        // That thing had better not take too long
        for (NSObject<WhirlyKitActiveModel> *activeModel in scene->activeModels)
            [activeModel updateForFrame:frameInfo];
        
        if (perfInterval > 0)
            perfTimer.addCount("Scene changes", scene->changeRequests.size());
        
		// Merge any outstanding changes into the scenegraph
		// Or skip it if we don't acquire the lock
		scene->processChanges(theView,self);
        
        if (perfInterval > 0)
            perfTimer.stopTiming("Scene processing");
        
        if (perfInterval > 0)
            perfTimer.startTiming("Culling");
		
		// We need a reverse of the eye vector in model space
		// We'll use this to determine what's pointed away
		Eigen::Matrix4f modelTransInv = modelTrans.inverse();
		Vector4f eyeVec4 = modelTransInv * Vector4f(0,0,1,0);
		Vector3f eyeVec3(eyeVec4.x(),eyeVec4.y(),eyeVec4.z());
        frameInfo.eyeVec = eyeVec3;
		
        // If we're looking at a globe, run the culling
        std::set<DrawableRef> toDraw;
        int drawablesConsidered = 0;
        CullTree *cullTree = scene->getCullTree();
        // Recursively search for the drawables that overlap the screen
        Mbr screenMbr;
        // Stretch the screen MBR a little for safety
        screenMbr.addPoint(Point2f(-ScreenOverlap*framebufferWidth,-ScreenOverlap*framebufferHeight));
        screenMbr.addPoint(Point2f((1+ScreenOverlap)*framebufferWidth,(1+ScreenOverlap)*framebufferHeight));
        [self findDrawables:cullTree->getTopCullable() view:globeView frameSize:Point2f(framebufferWidth,framebufferHeight) modelTrans:&modelTrans eyeVec:eyeVec3 frameInfo:frameInfo screenMbr:screenMbr topLevel:true toDraw:&toDraw considered:&drawablesConsidered];
        
        // Turn these drawables in to a vector
		std::vector<Drawable *> drawList;
        //		drawList.reserve(toDraw.size());
		for (std::set<DrawableRef>::iterator it = toDraw.begin();
			 it != toDraw.end(); ++it)
        {
            Drawable *theDrawable = it->get();
            if (theDrawable)
                drawList.push_back(theDrawable);
            else
                NSLog(@"Bad drawable coming from cull tree.");
        }
        
        if (perfInterval > 0)
            perfTimer.stopTiming("Culling");
        
        if (perfInterval > 0)
            perfTimer.startTiming("Generators - generate");
        
        // Now ask our generators to make their drawables
        // Note: Not doing any culling here
        //       And we should reuse these Drawables
        std::vector<DrawableRef> generatedDrawables,screenDrawables;
        const GeneratorSet *generators = scene->getGenerators();
        for (GeneratorSet::iterator it = generators->begin();
             it != generators->end(); ++it)
            (*it)->generateDrawables(frameInfo, generatedDrawables, screenDrawables);
        
        // Add the generated drawables and sort them all together
        for (unsigned int ii=0;ii<generatedDrawables.size();ii++)
        {
            Drawable *theDrawable = generatedDrawables[ii].get();
            if (theDrawable)
                drawList.push_back(theDrawable);
        }
        bool sortLinesToEnd = (zBufferMode == zBufferOffUntilLines);
        std::sort(drawList.begin(),drawList.end(),DrawListSortStruct2(sortAlphaToEnd,sortLinesToEnd,frameInfo));
                
        if (perfInterval > 0)
        {
            perfTimer.addCount("Drawables considered", drawablesConsidered);
            perfTimer.addCount("Cullables", cullTree->getCount());
        }
        
        if (perfInterval > 0)
            perfTimer.stopTiming("Generators - generate");
        
        if (perfInterval > 0)
            perfTimer.startTiming("Draw Execution");
        
        SimpleIdentity curProgramId = EmptyIdentity;
		
        bool depthMaskOn = (zBufferMode == zBufferOn);
		for (unsigned int ii=0;ii<drawList.size();ii++)
		{
			Drawable *drawable = drawList[ii];
            
            // The first time we hit an explicitly alpha drawable
            //  turn off the depth buffer
            if (depthBufferOffForAlpha && !(zBufferMode == zBufferOffUntilLines))
            {
                if (depthMaskOn && depthBufferOffForAlpha && drawable->hasAlpha(frameInfo))
                {
                    depthMaskOn = false;
                    glDisable(GL_DEPTH_TEST);
                }
            }
            
            // For this mode we turn the z buffer off until we get our first lines
            // This assumes we're sorting lines to the end
            if (zBufferMode == zBufferOffUntilLines)
            {
                if (!depthMaskOn && (drawable->getType() == GL_LINES || drawable->getType() == GL_LINE_LOOP || drawable->getForceZBufferOn()))
                {
                    glDepthFunc(GL_LESS);
                    depthMaskOn = true;
                }
            }
            
            // If it has a transform, apply that
            // Note: Put the missing local transform back
//            const Matrix4f *thisMat = drawable->getMatrix();
//            if (thisMat)
//            {
//                glPushMatrix();
//                glMultMatrixf(thisMat->data());
//            }
            
            // Figure out the program to use for drawing
            SimpleIdentity drawProgramId = drawable->getProgram();
            if (drawProgramId == EmptyIdentity)
                drawProgramId = defaultTriShader;
            if (drawProgramId != curProgramId)
            {
                curProgramId = drawProgramId;
                OpenGLES2Program *program = scene->getProgram(drawProgramId);
                if (program)
                {
                    glUseProgram(program->getProgram());
                    // Assign the lights if we need to
                    if (program->hasLights() && ([lights count] > 0))
                        program->setLights(lights, lightsLastUpdated, defaultMat, frameInfo.mvpMat);
                    // Explicitly turn the lights on
                    program->setUniform(kWKOGLNumLights, (int)[lights count]);

                    frameInfo.program = program;
                }
            }
            if (drawProgramId == EmptyIdentity)
                continue;
            
            // Draw using the given program
            drawable->draw(frameInfo,scene);
            
//            if (thisMat)
//                glPopMatrix();
            
            numDrawables++;
            if (perfInterval > 0)
            {
                // Note: Need a better way to track buffer ID growth
//                BasicDrawable *basicDraw = dynamic_cast<BasicDrawable *>(drawable);
//                if (basicDraw)
//                    perfTimer.addCount("Buffer IDs", basicDraw->getPointBuffer());
            }
		}
        
        if (perfInterval > 0)
            perfTimer.addCount("Drawables drawn", numDrawables);
        
        if (perfInterval > 0)
            perfTimer.stopTiming("Draw Execution");
        
        // Anything generated needs to be cleaned up
        generatedDrawables.clear();
        drawList.clear();
        
        if (perfInterval > 0)
            perfTimer.startTiming("Generators - Draw 2D");
        
        // Now for the 2D display
        if (!screenDrawables.empty())
        {
            curProgramId = EmptyIdentity;
            
            glDisable(GL_DEPTH_TEST);
            // Sort by draw priority (and alpha, I guess)
            for (unsigned int ii=0;ii<screenDrawables.size();ii++)
            {
                Drawable *theDrawable = screenDrawables[ii].get();
                if (theDrawable)
                    drawList.push_back(theDrawable);
                else
                    NSLog(@"Bad drawable coming from generator.");
            }
            std::sort(drawList.begin(),drawList.end(),DrawListSortStruct2(false,false,frameInfo));

            // Build an orthographic projection
            // We flip the vertical axis and spread the window out (0,0)->(width,height)
            Eigen::Matrix4f orthoMat = Matrix4f::Identity();
            Vector3f delta(framebufferWidth,-framebufferHeight,2.0);
            orthoMat(0,0) = 2.0f / delta.x();
            orthoMat(0,3) = -(framebufferWidth) / delta.x();
            orthoMat(1,1) = 2.0f / delta.y();
            orthoMat(1,3) = -framebufferHeight / delta.y();
            orthoMat(2,2) = -2.0f / delta.z();
            orthoMat(2,3) = 0.0f;
            frameInfo.mvpMat = orthoMat;
            // Turn off lights
            frameInfo.lights = nil;
            
            for (unsigned int ii=0;ii<drawList.size();ii++)
            {
                Drawable *drawable = drawList[ii];
                
                if (drawable->isOn(frameInfo))
                {
                    // Figure out the program to use for drawing
                    SimpleIdentity drawProgramId = drawable->getProgram();
                    if (drawProgramId == EmptyIdentity)
                        drawProgramId = defaultTriShader;
                    if (drawProgramId != curProgramId)
                    {
                        curProgramId = drawProgramId;
                        OpenGLES2Program *program = scene->getProgram(drawProgramId);
                        if (program)
                        {
                            glUseProgram(program->getProgram());
                            // Explicitly turn the lights off
                            program->setUniform(kWKOGLNumLights, 0);
                            frameInfo.program = program;
                        }
                    }

                    drawable->draw(frameInfo,scene);
                    numDrawables++;
                }
            }
            
            screenDrawables.clear();
            drawList.clear();
        }
        
        if (perfInterval > 0)
            perfTimer.stopTiming("Generators - Draw 2D");
    }
    
    if (perfInterval > 0)
        perfTimer.startTiming("Present Renderbuffer");
    
    glBindRenderbuffer(GL_RENDERBUFFER, colorRenderbuffer);
    [context presentRenderbuffer:GL_RENDERBUFFER];

    if (perfInterval > 0)
        perfTimer.stopTiming("Present Renderbuffer");
    
    if (perfInterval > 0)
        perfTimer.stopTiming("aaRender");
    
	// Update the frames per sec
	if (perfInterval > 0 && frameCount++ > perfInterval)
	{
        CFTimeInterval now = CFAbsoluteTimeGetCurrent();
		NSTimeInterval howLong =  now - frameCountStart;;
		framesPerSec = frameCount / howLong;
		frameCountStart = now;
		frameCount = 0;
        
        NSLog(@"---Rendering Performance---");
        perfTimer.log();
        perfTimer.clear();
	}
    
    if (oldContext != context)
        [EAGLContext setCurrentContext:oldContext];
}

@end
