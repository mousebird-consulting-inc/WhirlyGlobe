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
//#import <Eigen/OpenGLSupport>

using namespace WhirlyKit;

namespace WhirlyKit
{
PerformanceTimer::TimeEntry::TimeEntry()
{
    name = "";
    minDur = MAXFLOAT;
    maxDur = 0.0;
    avgDur = 0.0;
    numRuns = 0;
}
    
bool PerformanceTimer::TimeEntry::operator<(const WhirlyKit::PerformanceTimer::TimeEntry &that) const
{
    return name < that.name;
}
    
void PerformanceTimer::TimeEntry::addTime(NSTimeInterval dur)
{
    minDur = std::min(minDur,dur);
    maxDur = std::max(maxDur,dur);
    avgDur += dur;
    numRuns++;
}
    
PerformanceTimer::CountEntry::CountEntry()
{
    name = "";
    minCount = 1<<30;
    maxCount = 0;
    avgCount = 0;
    numRuns = 0;
}
    
bool PerformanceTimer::CountEntry::operator<(const WhirlyKit::PerformanceTimer::CountEntry &that) const
{
    return name < that.name;
}
    
void PerformanceTimer::CountEntry::addCount(int count)
{
    minCount = std::min(minCount,count);
    maxCount = std::max(maxCount,count);
    avgCount += count;
    numRuns++;
}
    
void PerformanceTimer::startTiming(const std::string &what)
{
    actives[what] = CFAbsoluteTimeGetCurrent();
}

void PerformanceTimer::stopTiming(const std::string &what)
{
    std::map<std::string,NSTimeInterval>::iterator it = actives.find(what);
    if (it == actives.end())
        return;
    NSTimeInterval start = it->second;
    actives.erase(it);
    
    std::map<std::string,TimeEntry>::iterator eit = timeEntries.find(what);
    if (eit != timeEntries.end())
        eit->second.addTime(CFAbsoluteTimeGetCurrent()-start);
    else {
        TimeEntry newEntry;
        newEntry.addTime(CFAbsoluteTimeGetCurrent()-start);
        newEntry.name = what;
        timeEntries[what] = newEntry;
    }
}
    
void PerformanceTimer::addCount(const std::string &what,int count)
{
    std::map<std::string,CountEntry>::iterator it = countEntries.find(what);
    if (it != countEntries.end())
        it->second.addCount(count);
    else {
        CountEntry newEntry;
        newEntry.addCount(count);
        newEntry.name = what;
        countEntries[what] = newEntry;
    }
}
    
void PerformanceTimer::clear()
{
    actives.clear();
    timeEntries.clear();
    countEntries.clear();
}

void PerformanceTimer::log()
{
    for (std::map<std::string,TimeEntry>::iterator it = timeEntries.begin();
         it != timeEntries.end(); ++it)
    {
        TimeEntry &entry = it->second;
        if (entry.numRuns > 0)
            NSLog(@"  %s: min, max, avg = (%.2f,%.2f,%.2f) ms",entry.name.c_str(),1000*entry.minDur,1000*entry.maxDur,1000*entry.avgDur / entry.numRuns);
    }
    for (std::map<std::string,CountEntry>::iterator it = countEntries.begin();
         it != countEntries.end(); ++it)
    {
        CountEntry &entry = it->second;
        if (entry.numRuns > 0)
            NSLog(@"  %s: min, max, avg = (%d,%d,%2.f,%d) count",entry.name.c_str(),entry.minCount,entry.maxCount,(float)entry.avgCount / (float)entry.numRuns,entry.avgCount);
    }
}

// Compare two matrices float by float
// The default comparison seems to have an epsilon and the cwise version isn't getting picked up
bool matrixAisSameAsB(Matrix4f &a,Matrix4f &b)
{
    float *floatsA = a.data();
    float *floatsB = b.data();
    
    for (unsigned int ii=0;ii<16;ii++)
        if (floatsA[ii] != floatsB[ii])
            return false;
    
    return true;
}
    
}

@implementation WhirlyKitRendererFrameInfo

@synthesize sceneRenderer;
@synthesize theView;
@synthesize modelTrans;
@synthesize projMat;
@synthesize scene;
@synthesize frameLen;
@synthesize currentTime;
@synthesize eyeVec;

@end

// Alpha stuff goes at the end
// Otherwise sort by draw priority
class drawListSortStruct
{
public:
    // These methods are here to make the compiler shut up
    drawListSortStruct() { }
    ~drawListSortStruct() { }
    drawListSortStruct(const drawListSortStruct &that) {  }
    drawListSortStruct & operator = (const drawListSortStruct &that) { return *this; }
    bool operator()(DrawableRef a,DrawableRef b) 
    {
        if (a->hasAlpha(frameInfo) == b->hasAlpha(frameInfo))
            return a->getDrawPriority() < b->getDrawPriority();

        return !a->hasAlpha(frameInfo);
    }
    
    WhirlyKitRendererFrameInfo *frameInfo;
};

@interface WhirlyKitSceneRendererES1()
- (void)setupView;
@end

@implementation WhirlyKitSceneRendererES1

@synthesize context;
@synthesize scene,theView;
@synthesize zBuffer;
@synthesize doCulling;
@synthesize framebufferWidth,framebufferHeight;
@synthesize scale;
@synthesize framesPerSec;
@synthesize perfInterval;
@synthesize numDrawables;
@synthesize delegate;
@synthesize useViewChanged;

- (id <WhirlyKitESRenderer>) init
{
	if ((self = [super init]))
	{
		frameCount = 0;
		framesPerSec = 0.0;
        numDrawables = 0;
		frameCountStart = nil;
        zBuffer = true;
        doCulling = true;
        clearColor.r = 0.0;  clearColor.g = 0.0;  clearColor.b = 0.0;  clearColor.a = 1.0;
        perfInterval = -1;
        scale = [[UIScreen mainScreen] scale];
		
		context = [[EAGLContext alloc] initWithAPI:kEAGLRenderingAPIOpenGLES1];
        
        if (!context || ![EAGLContext setCurrentContext:context])
		{
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

        // Off by default because animations will get confused
        useViewChanged = false;
	}
	
	return self;
}

- (void) dealloc
{
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
	
	context = nil;
	
}

// We'll take the maximum requested time
- (void)setRenderUntil:(NSTimeInterval)newRenderUntil
{
    renderUntil = std::max(renderUntil,newRenderUntil);
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
	
    glMatrixMode(GL_MODELVIEW);
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

	// Set it back to model view
	glMatrixMode(GL_MODELVIEW);	
	glEnable(GL_BLEND);	
    
	// Set a blending function to use
	glBlendFunc(GL_ONE, GL_ONE_MINUS_SRC_ALPHA);
}

// Make the screen a bit bigger for testing
static const float ScreenOverlap = 0.1;

// Calculate an acceptable MBR from world coords
- (Mbr) calcCurvedMBR:(Point3f *)corners view:(WhirlyGlobeView *)globeView modelTrans:(Eigen::Matrix4f *)modelTrans frameSize:(Point2f)frameSize
{
    Mbr localScreenMbr;
    
    for (unsigned int ii=0;ii<WhirlyKitCullableCorners;ii++)
    {
        CGPoint screenPt = [globeView pointOnScreenFromSphere:corners[ii] transform:modelTrans frameSize:frameSize];
        localScreenMbr.addPoint(Point2f(screenPt.x,screenPt.y));
    }    
    
    return localScreenMbr;
}

- (void) mergeDrawableSet:(const std::set<DrawableRef,IdentifiableRefSorter> &)newDrawables globeView:(WhirlyGlobeView *)globeView frameSize:(Point2f)frameSize modelTrans:(Eigen::Matrix4f *)modelTrans frameInfo:(WhirlyKitRendererFrameInfo *)frameInfo screenMbr:(Mbr)screenMbr toDraw:(std::set<DrawableRef> *) toDraw considered:(int *)drawablesConsidered
{
//    CoordSystem *coordSys = globeView.coordSystem;
    
    // Grab any drawables that live just at this level
    *drawablesConsidered += newDrawables.size();        
    for (std::set<DrawableRef,IdentifiableSorter>::const_iterator it = newDrawables.begin();
         it != newDrawables.end(); ++it)
    {
        DrawableRef draw = *it;
        // Make sure we haven't added it already and it's on
        // Note: We're doing the on check repeatedly
        //       And we're doing the refusal check repeatedly as well, possibly
        if ((toDraw->find(draw) == toDraw->end()) && draw->isOn(frameInfo))
        {
//            Mbr localScreenMbr;
//            std::vector<Point2f> localPts;
//            draw->getLocalMbr().asPoints(localPts);
//            for (unsigned int ii=0;ii<4;ii++)
//            {
//                Point3f worldLoc = coordSys->localToGeocentricish(Point3f(localPts[ii].x(),localPts[ii].y(),0.0));
//                CGPoint screenPt = [globeView pointOnScreenFromSphere:worldLoc transform:modelTrans frameSize:frameSize];
//                localScreenMbr.addPoint(Point2f(screenPt.x,screenPt.y));
//            }

            // If this overlaps, we want to draw it
//            if (!DoingCulling || screenMbr.overlaps(localScreenMbr))
            // Note: Turned this off for now because it doesn't deal well with curvature
            if (true)
                toDraw->insert(draw);
        } 
    }    
}

- (void) findDrawables:(Cullable *)cullable view:(WhirlyGlobeView *)globeView frameSize:(Point2f)frameSize modelTrans:(Eigen::Matrix4f *)modelTrans eyeVec:(Vector3f)eyeVec frameInfo:(WhirlyKitRendererFrameInfo *)frameInfo screenMbr:(Mbr)screenMbr topLevel:(bool)isTopLevel toDraw:(std::set<DrawableRef> *) toDraw considered:(int *)drawablesConsidered
{
    CoordSystem *coordSys = globeView.coordSystem;
    
    // Check the four corners of the cullable to see if they're pointed away
    // But just for the globe case
    bool inView = false;
    if (coordSys->isFlat() || isTopLevel)
    {
        inView = true;
    } else {
        for (unsigned int ii=0;ii<WhirlyKitCullableCorners;ii++)
        {
            Vector3f norm = cullable->cornerNorms[ii];
            if (norm.dot(eyeVec) > 0)
            {
                inView = true;
                break;
            }
        }
    }
    if (doCulling && !inView)
        return;
        
    Mbr localScreenMbr;
    localScreenMbr = [self calcCurvedMBR:&cullable->cornerPoints[0] view:globeView modelTrans:modelTrans frameSize:frameSize];
    
    // If this doesn't overlap what we're viewing, we're done
    if (doCulling && !screenMbr.overlaps(localScreenMbr))
        return;

    // If the footprint of this level on the screen is larger than
    //  the screen area, keep going down (if we can).
    float localScreenArea = localScreenMbr.area();
    float screenArea = screenMbr.area();
    if (isTopLevel || (localScreenArea > screenArea/4 && cullable->hasChildren()))
    {
        // Grab the drawables at this level
        [self mergeDrawableSet:cullable->getDrawables() globeView:globeView frameSize:frameSize modelTrans:modelTrans frameInfo:frameInfo screenMbr:screenMbr toDraw:toDraw considered:drawablesConsidered];
        
        // And recurse downward for the rest
        for (unsigned int ii=0;ii<4;ii++)
        {
            Cullable *child = cullable->getChild(ii);
            if (child)
                [self findDrawables:child view:globeView frameSize:frameSize modelTrans:modelTrans eyeVec:eyeVec frameInfo:frameInfo screenMbr:screenMbr topLevel:false toDraw:toDraw considered:drawablesConsidered];
        }
    } else {
        // If not, then just return what we found here
        [self mergeDrawableSet:cullable->getChildDrawables() globeView:globeView frameSize:frameSize modelTrans:modelTrans frameInfo:frameInfo screenMbr:screenMbr toDraw:toDraw considered:drawablesConsidered];
    }
}

// Check if the view changed from the last frame
- (bool) viewDidChange
{
    if (!useViewChanged)
        return true;
    
    // First time through
    if (lastDraw == 0.0)
        return true;
    
    // Something wants us to draw (probably an animation)
    // We look at the last draw so we can handle jumps in time
    if (lastDraw < renderUntil)
        return true;
        
    Matrix4f newModelMat = [theView calcModelMatrix];
    Matrix4f newViewMat = [theView calcViewMatrix];
    
    // Should be exactly the same
    if (matrixAisSameAsB(newModelMat,modelMat) && matrixAisSameAsB(newViewMat,viewMat))
        return false;
    
    modelMat = newModelMat;
    viewMat = newViewMat;
    return true;
}

- (void)forceDrawNextFrame
{
    lastDraw = 0;
}

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

    // Note: Testing
    Eigen::Matrix4f modelTrans = [theView calcModelMatrix];
    if (globeView)
    {

//        printf("Renderer partialCalc:\n");
//        for (unsigned int iy=0;iy<4;iy++)
//        {
//            for (unsigned int ix=0;ix<4;ix++)
//                printf(" %f",fullMat.data()[iy*4+ix]);
//            printf("\n");
//        }
//        printf("\n");
        Eigen::Matrix4f viewTrans = [theView calcViewMatrix];
        
        glMultMatrixf(viewTrans.data());
        glMultMatrixf(modelTrans.data());
    } else {
        glMultMatrixf(modelTrans.data());
    }
    
    if (zBuffer)
    {
        glDepthMask(GL_TRUE);
        glEnable(GL_DEPTH_TEST);
    } else {
        glDepthMask(GL_FALSE);
        glDisable(GL_DEPTH_TEST);
    }

	glClearColor(clearColor.r / 255.0, clearColor.g / 255.0, clearColor.b / 255.0, clearColor.a / 255.0);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		
	glEnable(GL_CULL_FACE);
//    // Note: Debugging
//    glDisable(GL_CULL_FACE);
        
    // Call the pre-frame callback
    if (delegate && [(NSObject *)delegate respondsToSelector:@selector(preFrame:)])
        [delegate preFrame:self];
    
    if (perfInterval > 0)
        perfTimer.stopTiming("Render Setup");
    
	if (scene)
	{
		numDrawables = 0;
        
        WhirlyKitRendererFrameInfo *frameInfo = [[WhirlyKitRendererFrameInfo alloc] init];
        frameInfo.sceneRenderer = self;
        frameInfo.theView = theView;
        frameInfo.modelTrans = modelTrans;
        frameInfo.scene = scene;
        frameInfo.frameLen = duration;
        frameInfo.currentTime = CFAbsoluteTimeGetCurrent();
        Matrix4f projMat;
        glGetFloatv(GL_PROJECTION_MATRIX,projMat.data());
        frameInfo.projMat = projMat;
		
        if (perfInterval > 0)
            perfTimer.startTiming("Scene processing");

        if (perfInterval > 0)
            perfTimer.addCount("Scene changes", scene->changeRequests.size());
        
		// Merge any outstanding changes into the scenegraph
		// Or skip it if we don't acquire the lock
		// Note: Time this and move it elsewhere
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
		std::vector<DrawableRef> drawList;
		drawList.reserve(toDraw.size());
		for (std::set<DrawableRef>::iterator it = toDraw.begin();
			 it != toDraw.end(); ++it)
			drawList.push_back(*it);

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
        drawList.insert(drawList.end(), generatedDrawables.begin(), generatedDrawables.end());
        drawListSortStruct sortStruct;
        sortStruct.frameInfo = frameInfo;
		std::sort(drawList.begin(),drawList.end(),sortStruct);
        
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
			DrawableRef drawable = drawList[ii];
            // The first time we hit an explicitly alpha drawable
            //  turn off the depth buffer
            if (depthMaskOn && drawable->hasAlpha(frameInfo))
            {
                depthMaskOn = false;
                glDisable(GL_DEPTH_TEST);
            }
            drawable->draw(frameInfo,scene);	
            numDrawables++;
            if (perfInterval > 0)
            {
                BasicDrawable *basicDraw = dynamic_cast<BasicDrawable *>(drawable.get());
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
            drawList.insert(drawList.end(), screenDrawables.begin(), screenDrawables.end());
            drawListSortStruct sortStruct;
            sortStruct.frameInfo = frameInfo;
            std::sort(drawList.begin(),drawList.end(),sortStruct);
            
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
                DrawableRef drawable = drawList[ii];
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
}

@end
