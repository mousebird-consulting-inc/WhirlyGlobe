//
//  Tesselator.mm
//  WhirlyGlobeApp
//
//  Created by Stephen Gifford on 7/17/11.
//  Copyright 2011-2012 mousebird consulting. All rights reserved.
//

#import <list>
#import "Tesselator.h"

namespace WhirlyKit
{

void TesselateRing(const VectorRing &ring,std::vector<VectorRing> &rets)
{
    int startRet = rets.size();
        
	// Simple cases
	if (ring.size() < 3)
		return;
    
	if (ring.size() == 3)
	{
		rets.push_back(ring);
		return;
	}
    
	// Convert to a linked list
	std::list<Point2f> poly;
	for (unsigned int ii=0;ii<ring.size();ii++)
		poly.push_back(ring[ii]);
    std::reverse(poly.begin(),poly.end());
    
	// Whittle down the polygon until there's 3 left
	while (poly.size() > 3)
	{
		std::list<Point2f>::iterator bestPt = poly.end();
		std::list<Point2f>::iterator prevBestPt = poly.end();
		std::list<Point2f>::iterator nextBestPt = poly.end();
        
		// Look for the best point
		std::list<Point2f>::iterator prevPt = poly.end(); --prevPt;
		std::list<Point2f>::iterator pt = poly.begin();
		std::list<Point2f>::iterator nextPt = poly.begin(); nextPt++;
		while (pt != poly.end())
		{
			bool valid = true;
			// First, see if this is a valid triangle
			// Pt should be on the left of prev->next
			Point2f dir0(pt->x()-prevPt->x(),pt->y()-prevPt->y());
			Point2f dir1(nextPt->x()-prevPt->x(),nextPt->y()-prevPt->y());
			float z = dir0.x()*dir1.y() - dir0.y()*dir1.x();
            
			if (z < 0.0)
				valid = false;
                        
			// Check that none of the other points fall within the proposed triangle
			if (valid)
			{
				VectorRing newTri;
				newTri.push_back(*prevPt);
				newTri.push_back(*pt);
				newTri.push_back(*nextPt);
				for (std::list<Point2f>::iterator it = poly.begin();it!=poly.end();++it)
				{
					// Obviously the three points we're going to use don't count
					if (it == prevPt || it == nextPt || it == pt)
						continue;
                    
					if (PointInPolygon(*it,newTri))
					{
						valid = false;
						break;
					} else {
                    }
				}
			}

            // any valid point will do, we're not going to optimize further
			if (valid)
			{
                bestPt = pt;
                prevBestPt = prevPt;
                nextBestPt = nextPt;
                
                break;
			}
            
			if ((++prevPt) == poly.end())
				prevPt = poly.begin();
			++pt;
			if ((++nextPt) == poly.end())
				nextPt = poly.begin();
		}
        
		// Form the triangle (bestPt-1,bestPt,bestPt+1)
		if (bestPt == poly.end())
		{
//            printf("Tesselate failure for %d input\n",(int)ring.size());
            break;
            
		} else {
			VectorRing newTri;
			newTri.push_back(*prevBestPt);
			newTri.push_back(*bestPt);
			newTri.push_back(*nextBestPt);
			rets.push_back(newTri);
		}
		poly.erase(bestPt);
	}
    
	// What's left should be a single triangle
	VectorRing lastTri;
	for (std::list<Point2f>::iterator it = poly.begin();it != poly.end();++it)
		lastTri.push_back(*it);
	rets.push_back(lastTri);
    
    for (unsigned int ii=startRet;ii<rets.size();ii++)
    {
        VectorRing &retTri = rets[ii];
        std::reverse(retTri.begin(),retTri.end());
    }
}

}
