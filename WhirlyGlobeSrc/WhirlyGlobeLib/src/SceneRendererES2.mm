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

using namespace WhirlyKit;

@implementation WhirlyKitSceneRendererES2

- (id) init
{
    self = [super initWithOpenGLESVersion:kEAGLRenderingAPIOpenGLES2];
    
    return self;
}

static const char *vertexShader =
"uniform mat4 u_mvpMatrix;                   \n"
"attribute vec3 a_position;                  \n"
"attribute vec2 a_texCoord;                  \n"
"varying vec2 v_texCoord;                    \n"
"void main()                                 \n"
"{                                           \n"
"   v_texCoord = a_texCoord;                 \n"
"   gl_Position = u_mvpMatrix * vec4(a_position,1.0);  \n"
"}                                           \n"
;
static const char *fragmentShader =
"precision mediump float;                            \n"
"uniform bool u_hasTexture;                    \n"
"varying vec2 v_texCoord;                            \n"
"uniform sampler2D s_baseMap;                        \n"
"void main()                                         \n"
"{                                                   \n"
"  vec4 baseColor = texture2D(s_baseMap, v_texCoord); \n"
"  if (baseColor.a < 0.1)                            \n"
"      discard;                                      \n"
"  gl_FragColor = u_hasTexture ? baseColor : vec4(1.0,1.0,1.0,1.0);                         \n"
"}                                                   \n"
;

// When the scene is set, we'll compile our shaders
- (void)setScene:(WhirlyKit::Scene *)inScene
{
    scene = inScene;

    EAGLContext *oldContext = [EAGLContext currentContext];
    if (oldContext != context)
        [EAGLContext setCurrentContext:context];
    
    OpenGLES2Program *mainShader = new OpenGLES2Program("Default Program",vertexShader,fragmentShader);
    if (!mainShader->isValid())
    {
        NSLog(@"SceneRendererES2: Shader didn't compile.  Nothing will work.");
        delete mainShader;
    } else {
        scene->setDefaultProgram(mainShader);
    }

    if (oldContext != context)
        [EAGLContext setCurrentContext:oldContext];
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
        perfTimer.startTiming("Render");
    
	if (frameCountStart)
		frameCountStart = CFAbsoluteTimeGetCurrent();
	
    if (perfInterval > 0)
        perfTimer.startTiming("Render Setup");
    
    EAGLContext *oldContext = [EAGLContext currentContext];
    if (oldContext != context)
        [EAGLContext setCurrentContext:context];
    CheckGLError("SceneRendererES2: setCurrentContext");
    
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
    
    Eigen::Matrix4f matrixAndViewMat = modelTrans * viewTrans;
    Eigen::Matrix4f mvpMat = projMat * (matrixAndViewMat);
    
    if (zBuffer)
    {
        glDepthMask(GL_TRUE);
        CheckGLError("SceneRendererES2: glDepthMask");
        glEnable(GL_DEPTH_TEST);
        CheckGLError("SceneRendererES2: glEnable(GL_DEPTH_TEST)");
    } else {
        glDepthMask(GL_FALSE);
        CheckGLError("SceneRendererES2: glDepthMask");
        glDisable(GL_DEPTH_TEST);
        CheckGLError("SceneRendererES2: glDisable(GL_DEPTH_TEST)");
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
        frameInfo.viewAndModelMat = matrixAndViewMat;
		
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
		
		// Snag the projection matrix so we can use it later
		Mbr viewMbr(Point2f(-1,-1),Point2f(1,1));
		
		Vector4f test1(frustLL.x(),frustLL.y(),near,1.0);
		Vector4f test2(frustUR.x(),frustUR.y(),near,1.0);
		Vector4f projA = projMat * test1;
		Vector4f projB = projMat * test2;
		Vector3f projA_3(projA.x()/projA.w(),projA.y()/projA.w(),projA.z()/projA.w());
		Vector3f projB_3(projB.x()/projB.w(),projB.y()/projB.w(),projB.z()/projB.w());

        // If we're looking at a globe, run the culling
        std::set<DrawableRef> toDraw;
        int drawablesConsidered = 0;
        CullTree *cullTree = scene->getCullTree();
        if (globeView)
        {
            // Recursively search for the drawables that overlap the screen
            Mbr screenMbr;
            // Stretch the screen MBR a little for safety
            screenMbr.addPoint(Point2f(-ScreenOverlap*framebufferWidth,-ScreenOverlap*framebufferHeight));
            screenMbr.addPoint(Point2f((1+ScreenOverlap)*framebufferWidth,(1+ScreenOverlap)*framebufferHeight));
            [self findDrawables:cullTree->getTopCullable() view:globeView frameSize:Point2f(framebufferWidth,framebufferHeight) modelTrans:&modelTrans eyeVec:eyeVec3 frameInfo:frameInfo screenMbr:screenMbr topLevel:true toDraw:&toDraw considered:&drawablesConsidered];
        } else {
            // Otherwise copy it all over
            // Note: Do this
        }
        
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
        std::sort(drawList.begin(),drawList.end(),DrawListSortStruct(sortAlphaToEnd,frameInfo));

        // Need a valid OpenGL ES 2.0 program
        OpenGLES2Program *defaultProgram = scene->getDefaultProgram();
        if (defaultProgram)
        {
            glUseProgram(defaultProgram->getProgram());
            frameInfo.program = defaultProgram;
        } else
            return;
        
        if (perfInterval > 0)
        {
            perfTimer.addCount("Drawables considered", drawablesConsidered);
            perfTimer.addCount("Cullables", cullTree->getCount());
        }
        
        if (perfInterval > 0)
            perfTimer.stopTiming("Generators - generate");
        
        if (perfInterval > 0)
            perfTimer.startTiming("Draw Execution");
		
        bool depthMaskOn = zBuffer;
		for (unsigned int ii=0;ii<drawList.size();ii++)
		{
			Drawable *drawable = drawList[ii];
            
            // The first time we hit an explicitly alpha drawable
            //  turn off the depth buffer
            if (depthMaskOn && depthBufferOffForAlpha && drawable->hasAlpha(frameInfo))
            {
                depthMaskOn = false;
                glDisable(GL_DEPTH_TEST);
            }
            
            // If it has a transform, apply that
            const Matrix4f *thisMat = drawable->getMatrix();
            if (thisMat)
            {
                glPushMatrix();
                glMultMatrixf(thisMat->data());
            }
            
            // Draw using the given program
            drawable->draw(frameInfo,scene);
            
            if (thisMat)
                glPopMatrix();
            
            numDrawables++;
            if (perfInterval > 0)
            {
                BasicDrawable *basicDraw = dynamic_cast<BasicDrawable *>(drawable);
                if (basicDraw)
                    perfTimer.addCount("Buffer IDs", basicDraw->getPointBuffer());
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
            std::sort(drawList.begin(),drawList.end(),DrawListSortStruct(false,frameInfo));

            //
            // Note: Set up projection (or lack thereof) for 2D mode
            //
            
            for (unsigned int ii=0;ii<drawList.size();ii++)
            {
                Drawable *drawable = drawList[ii];
                
                if (drawable->isOn(frameInfo))
                {
                    // Note: Put this back together
//                    drawable->draw(frameInfo,scene);
                    numDrawables++;
                }
            }
            
            screenDrawables.clear();
            drawList.clear();
        }
        
        if (perfInterval > 0)
            perfTimer.stopTiming("Generators - Draw 2D");
    }
    
    glBindRenderbuffer(GL_RENDERBUFFER, colorRenderbuffer);
    [context presentRenderbuffer:GL_RENDERBUFFER];
        
    if (perfInterval > 0)
        perfTimer.stopTiming("Render");
    
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
