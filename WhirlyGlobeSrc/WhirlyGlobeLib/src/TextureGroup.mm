/*
 *  TextureGroup.mm
 *  WhirlyGlobeLib
 *
 *  Created by Steve Gifford on 1/3/11.
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

#import <UIKit/UIKit.h>
#import "UIImage+Stuff.h"
#import "TextureGroup.h"

@implementation WhirlyKitTextureGroup

@synthesize baseName,basePath,ext;
@synthesize numX,numY;
@synthesize pixelsSquare,borderPixels;

- (id) initWithInfo:(NSString *)infoName;
{
    // This should be the info plist.  That has everything
    NSDictionary *dict = [NSDictionary dictionaryWithContentsOfFile:infoName];
    if (!dict)
    {
        return nil;
    }

	if ((self = [super init]))
	{
	// If the user specified a real path, as opposed to just
	//  the file, we'll hang on to that
	self.basePath=[infoName stringByDeletingLastPathComponent];
        self.ext = [dict objectForKey:@"format"];
        self.baseName = [dict objectForKey:@"baseName"];
        numX = [[dict objectForKey:@"tilesInX"] intValue];
        numY = [[dict objectForKey:@"tilesInY"] intValue];
        pixelsSquare = [[dict objectForKey:@"pixelsSquare"] intValue];
        borderPixels = [[dict objectForKey:@"borderSize"] intValue];
	}
	
	return self;
}
                    

// Generate a file name for loading a given piece
- (NSString *) generateFileNameX:(unsigned int)x y:(unsigned int)y
{
	if (x >= numX || y >= numY)
		return nil;

	// Construct the name, but take into account the basepath, if set	
	NSString* result = [NSString stringWithFormat:@"%@_%dx%d",baseName,x,y];
	if (self.basePath)
		result = [self.basePath stringByAppendingPathComponent:result];
	
	return result;
}

- (void)calcTexMappingOrg:(WhirlyKit::TexCoord *)org dest:(WhirlyKit::TexCoord *)dest
{
    org->u() = org->v() = (float)borderPixels/(float)pixelsSquare;
    dest->u() = dest->v() = 1.f - org->u();
}

@end
