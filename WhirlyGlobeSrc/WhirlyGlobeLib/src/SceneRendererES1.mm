/*
 *  SceneRendererES1.mm
 *  WhirlyGlobeLib
 *
 *  Created by Steve Gifford on 1/13/11.
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

#import "SceneRendererES1.h"
#import "UIColor+Stuff.h"
//#import <Eigen/OpenGLSupport>

using namespace Eigen;
using namespace WhirlyKit;

@interface WhirlyKitSceneRendererES1()
- (void)setupView;
@end

@implementation WhirlyKitSceneRendererES1

@synthesize delegate;

- (id) init
{
    self = [super initWithOpenGLESVersion:kEAGLRenderingAPIOpenGLES1];
    
    return self;
}

// Set up the various view parameters
- (void)setupView
{
    // If the client provided a setupView, use that
    if (delegate && [(NSObject *)delegate respondsToSelector:@selector(lightingSetup:)])
    {
        [delegate lightingSetup:self];
    } else {
        // Otherwise we'll do a default setup
        // If you make your own, just copy this to start
        const GLfloat			lightAmbient[] = {0.5, 0.5, 0.5, 1.0};
        const GLfloat			lightDiffuse[] = {0.6, 0.6, 0.6, 1.0};
        const GLfloat			matAmbient[] = {0.5, 0.5, 0.5, 1.0};
        const GLfloat			matDiffuse[] = {1.0, 1.0, 1.0, 1.0};	
        const GLfloat			matSpecular[] = {1.0, 1.0, 1.0, 1.0};
        const GLfloat			lightPosition[] = {0.75, 0.5, 1.0, 0.0}; 
        const GLfloat			lightShininess = 100.0;
        
        //Configure OpenGL lighting
        glEnable(GL_LIGHTING);
        glEnable(GL_LIGHT0);
        glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT, matAmbient);
        glMaterialfv(GL_FRONT_AND_BACK, GL_DIFFUSE, matDiffuse);
        glMaterialfv(GL_FRONT_AND_BACK, GL_SPECULAR, matSpecular);
        glMaterialf(GL_FRONT_AND_BACK, GL_SHININESS, lightShininess);
        glLightfv(GL_LIGHT0, GL_AMBIENT, lightAmbient);
        glLightfv(GL_LIGHT0, GL_DIFFUSE, lightDiffuse);
        glLightfv(GL_LIGHT0, GL_POSITION, lightPosition); 
        glShadeModel(GL_SMOOTH);
        glEnable(GL_COLOR_MATERIAL);
    }

	// Set it back to model view
	glMatrixMode(GL_MODELVIEW);	
	glEnable(GL_BLEND);	
    
	// Set a blending function to use
	glBlendFunc(GL_ONE, GL_ONE_MINUS_SRC_ALPHA);
}

- (BOOL)resizeFromLayer:(CAEAGLLayer *)layer
{
    bool ret = [super resizeFromLayer:layer];
    
    EAGLContext *oldContext = [EAGLContext currentContext];
    if (oldContext != context)
        [EAGLContext setCurrentContext:context];
    
    glMatrixMode(GL_MODELVIEW);
	[self setupView];
    
    if (oldContext != context)
        [EAGLContext setCurrentContext:oldContext];

    return ret;
}

// Make the screen a bit bigger for testing
static const float ScreenOverlap = 0.1;

- (void) render:(CFTimeInterval)duration
{
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
    
    // Deal with any lighting changes
    glMatrixMode(GL_MODELVIEW);
    if (delegate && [(NSObject *)delegate respondsToSelector:@selector(lightingChanged:)] &&
        [(NSObject *)delegate respondsToSelector:@selector(lightingSetup:)] &&
        [delegate lightingChanged:self])
    {
        [delegate lightingSetup:self];
    }
    
    glBindFramebuffer(GL_FRAMEBUFFER, defaultFramebuffer);
    glViewport(0, 0, framebufferWidth, framebufferHeight);
    
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
	Point2f frustLL,frustUR;
	GLfloat near=0,far=0;
	[theView calcFrustumWidth:framebufferWidth height:framebufferHeight ll:frustLL ur:frustUR near:near far:far];
	glFrustumf(frustLL.x(),frustUR.x(),frustLL.y(),frustUR.y(),near,far);
	
	glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
    
    // See if we're dealing with a globe view
    WhirlyGlobeView *globeView = nil;
    if ([theView isKindOfClass:[WhirlyGlobeView class]])
        globeView = (WhirlyGlobeView *)theView;
    
    // Put both model and view matrices in place
    Eigen::Matrix4f modelTrans = [theView calcModelMatrix];
    Eigen::Matrix4f viewTrans = [theView calcViewMatrix];
    glMultMatrixf(viewTrans.data());
    glMultMatrixf(modelTrans.data());
    
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
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    
	glEnable(GL_CULL_FACE);
    
    // Call the pre-frame callback
    if (delegate && [(NSObject *)delegate respondsToSelector:@selector(preFrame:)])
        [delegate preFrame:self];
    
    if (perfInterval > 0)
        perfTimer.stopTiming("Render Setup");
    
	if (scene)
	{
		numDrawables = 0;
        
        WhirlyKitRendererFrameInfo *frameInfo = [[WhirlyKitRendererFrameInfo alloc] init];
        frameInfo.oglVersion = kEAGLRenderingAPIOpenGLES1;
        frameInfo.sceneRenderer = self;
        frameInfo.theView = theView;
        frameInfo.modelTrans = modelTrans;
        frameInfo.viewTrans = viewTrans;
        frameInfo.scene = scene;
        frameInfo.frameLen = duration;
        frameInfo.currentTime = CFAbsoluteTimeGetCurrent();
        Matrix4f projMat;
        glGetFloatv(GL_PROJECTION_MATRIX,projMat.data());
        frameInfo.projMat = projMat;
        Matrix4f matrixAndViewMat;
        glGetFloatv(GL_MODELVIEW_MATRIX,matrixAndViewMat.data());
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
        
        // Recursively search for the drawables that overlap the screen
        Mbr screenMbr;
        // Stretch the screen MBR a little for safety
        screenMbr.addPoint(Point2f(-ScreenOverlap*framebufferWidth,-ScreenOverlap*framebufferHeight));
        screenMbr.addPoint(Point2f((1+ScreenOverlap)*framebufferWidth,(1+ScreenOverlap)*framebufferHeight));
        int drawablesConsidered = 0;
		std::set<DrawableRef> toDraw;
        CullTree *cullTree = scene->getCullTree();
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
        std::sort(drawList.begin(),drawList.end(),DrawListSortStruct(sortAlphaToEnd,sortLinesToEnd,frameInfo));
        
        if (perfInterval > 0)
        {
            perfTimer.addCount("Drawables considered", drawablesConsidered);
            perfTimer.addCount("Cullables", cullTree->getCount());
        }
        
        if (perfInterval > 0)
            perfTimer.stopTiming("Generators - generate");
        
        if (perfInterval > 0)
            perfTimer.startTiming("Draw Execution");
		
        bool depthMaskOn = (zBufferMode == zBufferOn);
		for (unsigned int ii=0;ii<drawList.size();ii++)
		{
			Drawable *drawable = drawList[ii];
            
            // The first time we hit an explicitly alpha drawable
            //  turn off the depth buffer
            if (depthBufferOffForAlpha)
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
            const Matrix4f *thisMat = drawable->getMatrix();
            if (thisMat)
            {
                glPushMatrix();
                glMultMatrixf(thisMat->data());
            }
            drawable->draw(frameInfo,scene);
            
            if (thisMat)
                glPopMatrix();
            
            numDrawables++;
            if (perfInterval > 0)
            {
                // Note: Need a more reliable check for outstanding buffer IDs
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
            std::sort(drawList.begin(),drawList.end(),DrawListSortStruct(false,false,frameInfo));
            
            // Set up the matrix
            glMatrixMode(GL_PROJECTION);
            glLoadIdentity();
            glOrthof(0, framebufferWidth, framebufferHeight, 0, -1, 1);
            glMatrixMode(GL_MODELVIEW);
            glLoadIdentity();
            // Move things over just a bit to get better sampling
            glTranslatef(0.375, 0.375, 0);
            
            for (unsigned int ii=0;ii<drawList.size();ii++)
            {
                Drawable *drawable = drawList[ii];
                
                if (drawable->isOn(frameInfo))
                {
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
    
    glBindRenderbuffer(GL_RENDERBUFFER, colorRenderbuffer);
    [context presentRenderbuffer:GL_RENDERBUFFER];
    
    // Call the post-frame callback
    if (delegate && [(NSObject *)delegate respondsToSelector:@selector(postFrame:)])
        [delegate postFrame:self];
    
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
