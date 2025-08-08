/*
 *  MaplyOverlayView.h
 *  WhirlyGlobeLib
 *
 *  Created by Tim Sylvester on 8/22/2023
 *  Copyright 2023-2023 mousebird consulting
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

#import "MaplyView.h"

namespace Maply
{

/** A view based on an externally-provided MVP transformation.
 */
class MapOverlayView : public MapView
{
public:
    MapOverlayView(WhirlyKit::CoordSystemDisplayAdapter *);
    MapOverlayView(const MapOverlayView &);

    // Called to set the matrix, which we'll decompose
    virtual void assignMatrix(const Eigen::Matrix4d &);

    /// Generate the model view matrix for use by OpenGL.
    Eigen::Matrix4d calcModelMatrix() const override;
    
    /// Generate the view matrix for use by OpenGL
    Eigen::Matrix4d calcViewMatrix() const override;

    Eigen::Matrix4d calcProjectionMatrix(WhirlyKit::Point2f frameBufferSize, float margin) const override;

    /// Put together one or more offset matrices to express wrapping
    void getOffsetMatrices(WhirlyKit::Matrix4dVector &offsetMatrices,
                           const WhirlyKit::Point2f &frameBufferSize,
                           float bufferX) const override;

    /// Set the location we're looking from.  Always runs updates
    void setLoc(const WhirlyKit::Point3d &, bool runUpdates) override;

    /// Set the rotation angle
    void setRotAngle(double newRotAngle,bool viewUpdates) override;

    /** Given a location on the screen and the screen size, figure out where we touched
        the plane.  Returns true if we hit and where.
        Returns false if we didn't, which can only happened if we're turned away.
     */
    virtual bool pointOnPlaneFromScreen(WhirlyKit::Point2f pt,const Eigen::Matrix4d *transform,const WhirlyKit::Point2f &frameSize,WhirlyKit::Point3d *hit,bool clip) override;

    /** From a world location in 3D, figure the projection to the screen.
        Returns a point within the frame.
      */
    // virtual WhirlyKit::Point2f pointOnScreenFromPlane(const WhirlyKit::Point3d &worldLoc,const Eigen::Matrix4d *transform,const WhirlyKit::Point2f &frameSize);

protected:
    // Model/view/projection matrix all in one
    // Note: This will only work for 2D mode
    Eigen::Matrix4d mvp;
};
    
typedef std::shared_ptr<MapOverlayView> MapOverlayViewRef;

}
