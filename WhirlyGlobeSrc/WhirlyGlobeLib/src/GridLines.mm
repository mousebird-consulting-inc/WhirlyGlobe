/*
 *  GridLines.cpp
 *  WhirlyGlobeLib
 *
 *  Created by Steve Gifford on 1/25/11.
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

#import "GridLines.h"
#import "GlobeMath.h"

@interface WhirlyKitGridLayer()
@end

using namespace WhirlyKit;
using namespace WhirlyGlobe;

@implementation WhirlyKitGridLayer

- (id)initWithX:(unsigned int)inNumX Y:(unsigned int)inNumY
{
	if ((self = [super init]))
	{
		numX = inNumX;
		numY = inNumY;
	}
	
	return self;
}

- (void)startWithThread:(WhirlyKitLayerThread *)layerThread scene:(Scene *)inScene
{
	chunkX = 0;  chunkY = 0;
	scene = inScene;
	[self performSelector:@selector(process:) withObject:nil afterDelay:0.0];
}

// Generate grid lines covering the earth model
- (void)process:(id)sender
{
    CoordSystem *coordSys = scene->getCoordSystem();
	std::vector<ChangeRequest> changeRequests;

	GeoCoord geoIncr(2*M_PI/numX,M_PI/numY);
	GeoCoord geoLL(-M_PI + chunkX*geoIncr.x(),-M_PI/2.0 + chunkY*geoIncr.y());
	GeoMbr geoMbr(geoLL,geoLL+geoIncr);
		
	// Drawable containing just lines
	// Note: Not deeply efficient here
	BasicDrawable *drawable = new BasicDrawable();
	drawable->setType(GL_LINES);
	
	int startX = std::ceil(geoMbr.ll().x()/GridCellSize);
	int endX = std::floor(geoMbr.ur().x()/GridCellSize);
	int startY = std::ceil(geoMbr.ll().y()/GridCellSize);
	int endY = std::floor(geoMbr.ur().y()/GridCellSize);
	
	for (int x = startX;x <= endX; x++)
		for (int y = startY;y <= endY; y++)
		{
			// Start out with the points in 3-space
			// Note: Duplicating work
			Point3f norms[4],pts[4];
			norms[0] = coordSys->pointFromGeo(GeoCoord(x*GridCellSize,y*GridCellSize));
			norms[1] = coordSys->pointFromGeo(GeoCoord((x+1)*GridCellSize,y*GridCellSize));
			norms[2] = coordSys->pointFromGeo(GeoCoord((x+1)*GridCellSize,GridCellSize*(y+1)));
			norms[3] = coordSys->pointFromGeo(GeoCoord(GridCellSize*x,GridCellSize*(y+1)));
			
			// Nudge them out a little bit
			for (unsigned int ii=0;ii<4;ii++)
				pts[ii] = norms[ii] * (1.0 + GlobeLineOffset);
			
			// Add to drawable
			drawable->addPoint(pts[0]);
			drawable->addNormal(norms[0]);
			drawable->addPoint(pts[1]);
			drawable->addNormal(norms[1]);
			drawable->addPoint(pts[0]);
			drawable->addNormal(norms[0]);
			drawable->addPoint(pts[3]);
			drawable->addNormal(norms[3]);
			
		}
	
	scene->addChangeRequest(new AddDrawableReq(drawable));
	
	// Move on to the next chunk
	if (++chunkX >= numX)
	{
		chunkX = 0;
		chunkY++;
	}
	
	// Schedule the next chunk
	if (chunkY < numY)
		[self performSelector:@selector(process:) withObject:nil afterDelay:0.0];	
}

@end
