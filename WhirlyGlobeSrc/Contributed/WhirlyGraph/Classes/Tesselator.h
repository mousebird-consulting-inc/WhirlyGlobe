//
//  Tesselator.h
//  WhirlyGlobeApp
//
//  Created by Stephen Gifford on 7/17/11.
//  Copyright 2011 mousebird consulting. All rights reserved.
//

#import <vector>
#import <WhirlyGlobe/WhirlyGlobe.h>

void TesselateRing(const WhirlyGlobe::VectorRing &ring,std::vector<WhirlyGlobe::VectorRing> &rets);
