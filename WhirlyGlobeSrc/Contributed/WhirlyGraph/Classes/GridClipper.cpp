//
//  GridClipper.cpp
//  WhirlyGlobeApp
//
//  Created by Stephen Gifford on 7/16/11.
//  Copyright 2011 mousebird consulting. All rights reserved.
//

#include "GridClipper.h"

using namespace WhirlyGlobe;

// Clip the given loop to the given grid (org and spacing)
// Return true on success and the new polygons in the rets
bool ClipLoopToGrid(const WhirlyGlobe::VectorRing &ring,Point2f org,Point2f spacing,std::vector<WhirlyGlobe::VectorRing> &rets)
{
    Mbr mbr(ring);
}