/*
 *  GlobeAnimateHeight.h
 *  WhirlyGlobeLib
 *
 *  Created by Steve Gifford on 2/7/14.
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

#import "WhirlyTypes.h"
#import "WhirlyVector.h"
#import "WhirlyGeometry.h"
#import "GlobeView.h"

namespace WhirlyGlobe
{

/** Protocol for a delegate that handles tilt calculation.
 */
class TiltCalculator
{
public:
    TiltCalculator();

    /// If this is called, the pan delegate will vary the tilt between the given values for the
    ///  given height range.
    virtual void setContraints(double minTilt,double maxTilt,double minHeight,double maxHeight);

    /// Returns true if the tilt zoom mode is set and the appropriate values
    virtual bool getConstraints(double &retMinTilt,double &retMaxTilt,double &retMinHeight,double &retMaxHeight);

    /// Return a calculated tilt
    virtual double tiltFromHeight(double height) = 0;
    
    /// Return the maximum allowable tilt
    virtual double getMaxTilt() { return maxTilt; }

    /// Called by an actual tilt gesture.  We're setting the tilt as given
    virtual void setTilt(double newTilt) = 0;
    
protected:
    bool active;
    double tilt;
    double minTilt,maxTilt,minHeight,maxHeight;
};
    
typedef std::shared_ptr<TiltCalculator> TiltCalculatorRef;
    
/** A simple tilt delegate that varies tilt by height for a given range.
 */
class StandardTiltDelegate : public TiltCalculator
{
public:
    StandardTiltDelegate(GlobeView *globeView);

    /// Return a calculated tilt
    virtual double tiltFromHeight(double height);

    /// Return the maximum allowable tilt
    virtual double getMaxTilt();

    /// Called by an actual tilt gesture.  We're setting the tilt as given
    virtual void setTilt(double newTilt);

protected:
    GlobeView *globeView;
    double outsideTilt;
};
    
typedef std::shared_ptr<StandardTiltDelegate> StandardTiltDelegateRef;

/// Animate height for a globe view over time
class AnimateViewHeight : public GlobeViewAnimationDelegate
{
public:
    /// Start interpolating height immediately for the given time period
    AnimateViewHeight(GlobeView *globeView,double toHeight,WhirlyKit::TimeInterval howLong);
    
    /// Update the globe view
    virtual void updateView(GlobeView *globeView);
    
    /// If set, we're constraining the tilt based on height
    void setTiltDelegate(TiltCalculatorRef newDelegate);
    
protected:
    GlobeView *globeView;
    WhirlyKit::TimeInterval startDate;
    WhirlyKit::TimeInterval endDate;
    double startHeight,endHeight;
    TiltCalculatorRef tiltDelegate;
};

}
