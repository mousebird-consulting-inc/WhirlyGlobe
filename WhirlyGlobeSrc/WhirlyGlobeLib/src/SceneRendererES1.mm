/*
 *  SceneRendererES1.mm
 *  WhirlyGlobeLib
 *
 *  Created by Steve Gifford on 1/13/11.
 *  Copyright 2011 mousebird consulting
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

using namespace WhirlyGlobe;

@implementation RendererFrameInfo

@synthesize sceneRenderer;
@synthesize globeView;
@synthesize scene;
@synthesize frameLen;
@synthesize currentTime;
@synthesize eyeVec;

@end

// Alpha stuff goes at the end
// Otherwise sort by draw priority
struct drawListSortStruct
{
    bool operator()(const Drawable *a,const Drawable *b) 
    {
        if (a->hasAlpha(frameInfo) == b->hasAlpha(frameInfo))
            return a->getDrawPriority() < b->getDrawPriority();

        return !a->hasAlpha(frameInfo);
    }
    
    RendererFrameInfo *frameInfo;
};

@interface SceneRendererES1()
- (void)setupView;
@property (nonatomic,retain) NSDate *frameCountStart;
@end

@implementation SceneRendererES1

@synthesize scene,view;
@synthesize framebufferWidth,framebufferHeight;
@synthesize frameCountStart;
@synthesize framesPerSec;
@synthesize numDrawables;
@synthesize delegate;

- (id <ESRenderer>) init
{
	if ((self = [super init]))
	{
		frameCount = 0;
		framesPerSec = 0.0;
        numDrawables = 0;
		frameCountStart = nil;
        clearColor.r = 0.0;  clearColor.g = 0.0;  clearColor.b = 0.0;  clearColor.a = 1.0;
		
		context = [[EAGLContext alloc] initWithAPI:kEAGLRenderingAPIOpenGLES1];
        
        if (!context || ![EAGLContext setCurrentContext:context])
		{
            [self release];
            return nil;
        }

        // Create default framebuffer object.
        glGenFramebuffers(1, &defaultFramebuffer);
        glBindFramebuffer(GL_FRAMEBUFFER, defaultFramebuffer);
        
        // Create color render buffer and allocate backing store.
        glGenRenderbuffers(1, &colorRenderbuffer);
        glBindRenderbuffer(GL_RENDERBUFFER, colorRenderbuffer);
        glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_RENDERBUFFER, colorRenderbuffer);

		// Allocate depth buffer
		glGenRenderbuffers(1, &depthRenderbuffer);
		glBindRenderbuffer(GL_RENDERBUFFER, depthRenderbuffer);
	}
	
	return self;
}

- (void) dealloc
{
	self.frameCountStart = nil;
	[EAGLContext setCurrentContext:context];
	
	if (defaultFramebuffer)
	{
		glDeleteFramebuffers(1, &defaultFramebuffer);
		defaultFramebuffer = 0;
	}
	
	if (colorRenderbuffer)
	{
		glDeleteRenderbuffers(1, &colorRenderbuffer);
		colorRenderbuffer = 0;
	}
	
	if (depthRenderbuffer)
	{
		glDeleteRenderbuffers(1, &depthRenderbuffer	);
		depthRenderbuffer = 0;
	}
	
	[context release];
	context = nil;
	
	[super dealloc];
}

- (void)useContext
{
	if (context)
		[EAGLContext setCurrentContext:context];
}

- (BOOL) resizeFromLayer:(CAEAGLLayer *)layer
{	
    [EAGLContext setCurrentContext:context];

	glBindRenderbuffer(GL_RENDERBUFFER, colorRenderbuffer);
	[context renderbufferStorage:GL_RENDERBUFFER fromDrawable:(CAEAGLLayer *)layer];
	glGetRenderbufferParameteriv(GL_RENDERBUFFER, GL_RENDERBUFFER_WIDTH, &framebufferWidth);
	glGetRenderbufferParameteriv(GL_RENDERBUFFER, GL_RENDERBUFFER_HEIGHT, &framebufferHeight);	

	// For this sample, we also need a depth buffer, so we'll create and attach one via another renderbuffer.
	glBindRenderbuffer(GL_RENDERBUFFER, depthRenderbuffer);
	glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT16, framebufferWidth, framebufferHeight);
	glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, depthRenderbuffer);
	
	if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
	{
		NSLog(@"Failed to make complete framebuffer object %x", glCheckFramebufferStatus(GL_FRAMEBUFFER));		
		return NO;
	}
	
	[self setupView];
	
	return YES;
}

- (void) setClearColor:(UIColor *)color
{
    clearColor = [color asRGBAColor];
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

	glEnable(GL_DEPTH_TEST);

	// Set it back to model view
	glMatrixMode(GL_MODELVIEW);	
	glEnable(GL_BLEND);	
}

- (void) render:(CFTimeInterval)duration
{  
	if (!self.frameCountStart)
		self.frameCountStart = [NSDate date];
	
	[view animate];
	
    [EAGLContext setCurrentContext:context];
    
    glBindFramebuffer(GL_FRAMEBUFFER, defaultFramebuffer);
    glViewport(0, 0, framebufferWidth, framebufferHeight);

    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
	Point2f frustLL,frustUR;
	GLfloat near=0,far=0;
	[view calcFrustumWidth:framebufferWidth height:framebufferHeight ll:frustLL ur:frustUR near:near far:far];
	glFrustumf(frustLL.x(),frustUR.x(),frustLL.y(),frustUR.y(),near,far);
	
	glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
	Eigen::Affine3f modelTrans = [view calcModelMatrix];
	glLoadMatrixf(modelTrans.data());

	glClearColor(clearColor.r, clearColor.g, clearColor.b, clearColor.a);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		
	glEnable(GL_CULL_FACE);
    
    glDepthMask(GL_TRUE);
    
    // Call the pre-frame callback
    if (delegate && [(NSObject *)delegate respondsToSelector:@selector(preFrame:)])
        [delegate preFrame:self];
    
	if (scene)
	{
		numDrawables = 0;
        
        RendererFrameInfo *frameInfo = [[[RendererFrameInfo alloc] init] autorelease];
        frameInfo.sceneRenderer = self;
        frameInfo.globeView = view;
        frameInfo.scene = scene;
        frameInfo.frameLen = duration;
        frameInfo.currentTime = [NSDate timeIntervalSinceReferenceDate];
		
		// Merge any outstanding changes into the scenegraph
		// Or skip it if we don't acquire the lock
		// Note: Time this and move it elsewhere
		scene->processChanges(view);
		
		// We need a reverse of the eye vector in model space
		// We'll use this to determine what's pointed away
		Eigen::Matrix4f modelTransInv = modelTrans.inverse().matrix();
		Vector4f eyeVec4 = modelTransInv * Vector4f(0,0,1,0);
		Vector3f eyeVec3(eyeVec4.x(),eyeVec4.y(),eyeVec4.z());
        frameInfo.eyeVec = eyeVec3;
		
		// Snag the projection matrix so we can use it later
		Eigen::Matrix4f projMat;
		glGetFloatv(GL_PROJECTION_MATRIX,projMat.data());
		WhirlyGlobe::Mbr viewMbr(Point2f(-1,-1),Point2f(1,1));
		
		Vector4f test1(frustLL.x(),frustLL.y(),near,1.0);
		Vector4f test2(frustUR.x(),frustUR.y(),near,1.0);
		Vector4f projA = projMat * test1;
		Vector4f projB = projMat * test2;
		Vector3f projA_3(projA.x()/projA.w(),projA.y()/projA.w(),projA.z()/projA.w());
		Vector3f projB_3(projB.x()/projB.w(),projB.y()/projB.w(),projB.z()/projB.w());
		
		// Look through the cullables to assemble the set of drawables
		// We may encounter the same drawable multiple times, hence the std::set
		std::set<const WhirlyGlobe::Drawable *> toDraw;
		unsigned int numX,numY;
		scene->getCullableSize(numX,numY);
		const WhirlyGlobe::Cullable *cullables = scene->getCullables();
		for (unsigned int ci=0;ci<numX*numY;ci++)
		{
			// Check the four corners of the cullable to see if they're pointed away
			const WhirlyGlobe::Cullable *theCullable = &cullables[ci];
			bool inView = false;
			for (unsigned int ii=0;ii<4;ii++)
			{
				Vector3f norm = theCullable->cornerNorms[ii];
				if (norm.dot(eyeVec3) > 0)
				{
					inView = true;
					break;
				}
			}
			
			// Now project the corners onto the viewing plane and see if we overlap
			// This lets us catch things around the edges
			if (inView)
			{
				WhirlyGlobe::Mbr cullMbr;
				
				for (unsigned int ii=0;ii<4;ii++)
				{
					// Build up the MBR on the view plane
					Vector3f pt = theCullable->cornerPoints[ii];
					Vector4f projPt = projMat * (modelTrans * Vector4f(pt.x(),pt.y(),pt.z(),1.0));
					Vector3f projPt3(projPt.x()/projPt.w(),projPt.y()/projPt.w(),projPt.z()/projPt.w());
					cullMbr.addPoint(Point2f(projPt3.x(),projPt3.y()));
				}
				
				if (!cullMbr.overlaps(viewMbr))
				{
					inView = false;
				}
			}
			
			if (inView)
			{
				const std::set<WhirlyGlobe::Drawable *> &theseDrawables = theCullable->getDrawables();
				toDraw.insert(theseDrawables.begin(),theseDrawables.end());
			}
		}

        // Turn these drawables in to a vector
		std::vector<const WhirlyGlobe::Drawable *> drawList;
		drawList.reserve(toDraw.size());
		for (std::set<const WhirlyGlobe::Drawable *>::iterator it = toDraw.begin();
			 it != toDraw.end(); ++it)
			drawList.push_back(*it);
        
        // Now ask our generators to make their drawables
        // Note: Not doing any culling here
        //       And we should reuse these Drawables
        std::vector<WhirlyGlobe::Drawable *> generatedDrawables;
        const GeneratorSet *generators = scene->getGenerators();
        for (GeneratorSet::iterator it = generators->begin();
             it != generators->end(); ++it)
            (*it)->generateDrawables(frameInfo, generatedDrawables);
        
        // Add the generated drawables and sort them all together
        drawList.insert(drawList.end(), generatedDrawables.begin(), generatedDrawables.end());
        drawListSortStruct sortStruct;
        sortStruct.frameInfo = frameInfo;
		std::sort(drawList.begin(),drawList.end(),sortStruct);
		
        bool depthMaskOn = true;
		for (unsigned int ii=0;ii<drawList.size();ii++)
		{
			const WhirlyGlobe::Drawable *drawable = drawList[ii];
			if (drawable->isOn(frameInfo))
			{
                // The first time we hit an explicitly alpha drawable
                //  turn off the depth buffer
                if (drawable->hasAlpha(frameInfo) && depthMaskOn)
                {
                    depthMaskOn = false;
                    glDepthMask(GL_FALSE);
                }
				drawable->draw(frameInfo,scene);	
				numDrawables++;
			}
		}
        
        // Anything generated needs to be cleaned up
        // Note: Should have the generators keep them
        for (unsigned int ig=0;ig<generatedDrawables.size();ig++)
        {
            delete generatedDrawables[ig];
        }
        generatedDrawables.clear();
	}
    
    glDepthMask(GL_TRUE);
    glBindRenderbuffer(GL_RENDERBUFFER, colorRenderbuffer);
    [context presentRenderbuffer:GL_RENDERBUFFER];

	// Update the frames per sec
	if (frameCount++ > RenderFrameCount)
	{
		NSTimeInterval howLong = [self.frameCountStart timeIntervalSinceNow];
		framesPerSec = frameCount / (-howLong);
		self.frameCountStart = [NSDate date];
		frameCount = 0;
	}
    
    // Call the pre-frame callback
    if (delegate && [(NSObject *)delegate respondsToSelector:@selector(postFrame:)])
        [delegate postFrame:self]; 
}

@end
