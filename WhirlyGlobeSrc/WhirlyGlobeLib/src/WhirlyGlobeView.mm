//
//  WhirlyGlobeView.mm
//  WhirlyGlobeLib
//
//  Created by Stephen Gifford on 1/3/11.
//  Copyright 2011 mousebird consulting. All rights reserved.
//

#import "WhirlyGlobeView.h"

@interface WhirlyGlobeView()
@property (nonatomic,retain) CADisplayLink *displayLink;
-(void)setupView;
@end

@implementation WhirlyGlobeView

@synthesize scene;
@synthesize frameInterval;
@synthesize animating;
@synthesize displayLink;


+ (WhirlyGlobeView *)whirlyGlobeWithScene:(WhirlyGlobe::GlobeScene *)scene frameInterval:(unsigned int)frameInterval
{
	WhirlyGlobeView *globeView = [[[WhirlyGlobeView alloc] init] autorelease];
	globeView.scene = scene;
	globeView.frameInterval = frameInterval;
	
	return globeView;
}


- (id) init
{
	if (self = [super init])
	{
		EAGLContext *aContext = [[EAGLContext alloc] initWithAPI:kEAGLRenderingAPIOpenGLES1];
				
		if (!aContext)
			NSLog(@"Failed to create ES context");
		else if (![EAGLContext setCurrentContext:aContext])
			NSLog(@"Failed to set ES context current");
		
		self.context = aContext;
		[aContext release];
		
		animating = FALSE;
		frameInterval = 1;
		self.displayLink = nil;
		
		[self setupView];
	}
	
	return self;
}

- (void)dealloc
{
    // Tear down context.
    if ([EAGLContext currentContext] == context)
        [EAGLContext setCurrentContext:nil];
    self.context = nil;
    
    [super dealloc];
}

// Set up the various view parameters
- (void)setupView
{
	const GLfloat			lightAmbient[] = {0.2, 0.2, 0.2, 1.0};
	const GLfloat			lightDiffuse[] = {0.6, 0.6, 0.6, 1.0};
	const GLfloat			matAmbient[] = {0.6, 0.6, 0.6, 1.0};
	const GLfloat			matDiffuse[] = {1.0, 1.0, 1.0, 1.0};	
	const GLfloat			matSpecular[] = {1.0, 1.0, 1.0, 1.0};
	const GLfloat			lightPosition[] = {0.0, 0.0, 1.0, 0.0}; 
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
	glEnable(GL_DEPTH_TEST);
	glEnable(GL_COLOR_MATERIAL);

	// Simple perspective setup
	GLfloat fieldOfView = 60.0 / 360.0 * 2 * (float)M_PI;
	GLfloat					size;
	
	//Set the OpenGL projection matrix
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	size = 0.01 * tanf(fieldOfView / 2.0);
	CGRect rect = self.bounds;
	glFrustumf(-size, size, -size / (rect.size.width / rect.size.height), size / (rect.size.width / rect.size.height), 0.01, 10.0);
	glViewport(0, 0, rect.size.width, rect.size.height);
	
	// Set it back to model view
	glMatrixMode(GL_MODELVIEW);
	
	
	// Configure textures
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	
	//Configure OpenGL arrays
	glEnableClientState(GL_VERTEX_ARRAY);
	glEnableClientState(GL_NORMAL_ARRAY);
	glEnableClientState(GL_TEXTURE_COORD_ARRAY);
	glEnable(GL_NORMALIZE);

	//Make the OpenGL modelview matrix the default
	glMatrixMode(GL_MODELVIEW);
}

- (void)startAnimation
{
    if (!animating)
    {
        CADisplayLink *aDisplayLink = [CADisplayLink displayLinkWithTarget:self selector:@selector(drawFrame)];
        [aDisplayLink setFrameInterval:frameInterval];
        [aDisplayLink addToRunLoop:[NSRunLoop currentRunLoop] forMode:NSDefaultRunLoopMode];
        self.displayLink = aDisplayLink;
        
        animating = TRUE;
    }
}

- (void)stopAnimation
{
    if (animating)
    {
        [self.displayLink invalidate];
        self.displayLink = nil;
        animating = FALSE;
    }
}

- (void)drawFrame
{
	[self setFramebuffer];
    
    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);

//	glMatrixMode(GL_PROJECTION);
//	glLoadIdentity();
//	glMatrixMode(GL_MODELVIEW);
//	glLoadIdentity();
//	glTranslatef(0.0f, (GLfloat)(sinf(transY)/2.0f), 0.0f);
//	transY += 0.075f;
	
//	glVertexPointer(2, GL_FLOAT, 0, squareVertices);
//	glEnableClientState(GL_VERTEX_ARRAY);
//	glColorPointer(4, GL_UNSIGNED_BYTE, 0, squareColors);
//	glEnableClientState(GL_COLOR_ARRAY);

//   glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
	
/*	glEnableClientState(GL_VERTEX_ARRAY);
	glEnableClientState(GL_NORMAL_ARRAY);
	glEnableClientState(GL_TEXTURE_COORD_ARRAY);
*/
	
	// Draw the drawables
	if (scene)
	{
		for (std::set<WhirlyGlobe::Drawable *>::iterator it = scene->drawables.begin();
			 it != scene->drawables.end(); ++it)
		{
			WhirlyGlobe::Drawable &drawable = *(*it);
			
			glVertexPointer(3, GL_FLOAT, 0, &drawable.points[0]);
			glNormalPointer(GL_FLOAT, 0, &drawable.norms[0]);
			glTexCoordPointer(2, GL_FLOAT, 0, &drawable.texCoords[0]);
			glBindTexture(GL_TEXTURE_2D, drawable.textureId);
			glDrawElements(GL_TRIANGLES, drawable.tris.size()*3, GL_UNSIGNED_SHORT, (unsigned short *)&drawable.tris[0]);
		}
	}
    
    [self presentFramebuffer];
}

@end
