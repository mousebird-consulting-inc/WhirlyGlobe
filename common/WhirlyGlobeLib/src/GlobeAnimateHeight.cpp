/*
 *  GlobeAnimateHeight.mm
 *  WhirlyGlobeLib
 *
 *  Created by Steve Gifford on 5/23/11.
 *  Copyright 2011-2019 mousebird consulting
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

#import "GlobeAnimateHeight.h"
#import "Platform.h"

using namespace WhirlyKit;

namespace WhirlyGlobe
{
    
TiltCalculator::TiltCalculator()
    : minTilt(0.0), maxTilt(0.0), minHeight(0.0), maxHeight(0.0)
{
}
    
void TiltCalculator::setContraints(double inMinTilt,double inMaxTilt,double inMinHeight,double inMaxHeight)
{
    active = true;
    minTilt = inMinTilt;
    maxTilt = inMaxTilt;
    minHeight = inMinHeight;
    maxHeight = inMaxHeight;
}

bool TiltCalculator::getConstraints(double &retMinTilt,double &retMaxTilt,double &retMinHeight,double &retMaxHeight)
{
    retMinTilt = minTilt;
    retMaxTilt = maxTilt;
    retMinHeight = minHeight;
    retMaxHeight = maxHeight;
    
    return active;
}
    
StandardTiltDelegate::StandardTiltDelegate(GlobeView *inGlobeView)
{
    globeView = inGlobeView;
}

double StandardTiltDelegate::tiltFromHeight(double height)
{
    double maxValidTilt = getMaxTilt();
    if (!active)
    {
        return std::min(outsideTilt,maxValidTilt);
    }
    
    double newTilt = 0.0;
    
    // Now the tilt, if we're in that mode
    double newHeight = height;
    if (newHeight <= minHeight)
        newTilt = minTilt;
        else if (newHeight >= maxHeight)
            newTilt = maxTilt;
            else {
                float t = (newHeight-minHeight)/(maxHeight - minHeight);
                if (t != 0.0)
                    newTilt = t * (maxTilt - minTilt) + minTilt;
                    }
    
    return std::min(newTilt,maxValidTilt);
}

/// Return the maximum allowable tilt
double StandardTiltDelegate::getMaxTilt()
{
    return asin(1.0/(1.0+globeView->getHeightAboveGlobe()));
}

/// Called by an actual tilt gesture.  We're setting the tilt as given
void StandardTiltDelegate::setTilt(double newTilt)
{
    active = false;
    outsideTilt = newTilt;
}

AnimateViewHeight::AnimateViewHeight(GlobeView *inGlobeView,double toHeight,TimeInterval howLong)
{
    globeView = inGlobeView;
    startDate = TimeGetCurrent();
    endDate = startDate+howLong;
    startHeight = globeView->getHeightAboveGlobe();
    endHeight = toHeight;
}
    
void AnimateViewHeight::setTiltDelegate(TiltCalculatorRef newDelegate)
{
    tiltDelegate = newDelegate;
}

// Called by the view when it's time to update
void AnimateViewHeight::updateView(GlobeView *globeView)
{
    if (startDate == 0.0)
        return;
	
	TimeInterval now = TimeGetCurrent();
    double span = endDate-startDate;
    double remain = endDate - now;
    
	// All done.  Snap to the end
	if (remain < 0)
	{
        globeView->setHeightAboveGlobe(endHeight,false);
        startDate = 0;
        endDate = 0;
        globeView->cancelAnimation();
	} else {
		// Interpolate somewhere along the path
		double t = (span-remain)/span;
        globeView->setHeightAboveGlobe(startHeight + (endHeight-startHeight)*t,true);

        if (tiltDelegate)
        {
            double newTilt = tiltDelegate->tiltFromHeight(globeView->getHeightAboveGlobe());
            globeView->setTilt(newTilt);
        }
    }
}

}
