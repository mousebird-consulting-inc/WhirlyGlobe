/*
 *  ShapeDrawableBuilder.h
 *  WhirlyGlobeLib
 *
 *  Created by Steve Gifford on 9/28/11.
 *  Copyright 2011-2015 mousebird consulting. All rights reserved.
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

#import "Identifiable.h"
#import "WhirlyVector.h"
#import "BaseInfo.h"
#import "BasicDrawable.h"


namespace WhirlyKit
{

/// Used to pass shape info between the shape layer and the drawable builder
///  and within the threads of the shape layer
class WhirlyKitShapeInfo : public BaseInfo
{
public:
    WhirlyKitShapeInfo();
    ~WhirlyKitShapeInfo();

    void setColor(RGBAColor value) { color = value; }
    RGBAColor getColor() { return color; }

    void setLineWidth(float value) { lineWidth = value; }
    float getLineWidth() { return lineWidth; }

    void setShapeId(WhirlyKit::SimpleIdentity value) { shapeId = value; }
    WhirlyKit::SimpleIdentity getShapeId() { return shapeId; }

    void setInsideOut(bool value) { insideOut = value; }
    bool getInsideOut() { return insideOut; }

    void setZBufferRead(bool value) { zBufferRead = value; }
    bool getZBufferRead() { return zBufferRead; }

    void setZBufferWrite(bool value) { zBufferWrite = value; }
    bool getZBufferWrite() { return zBufferWrite; }

    void setHasCenter(bool value) { hasCenter = value; }
    bool getHasCenter() { return hasCenter; }

    void setCenter(Point3d value) { center = value; }
    Point3d getCenter() { return center; }

private:
    RGBAColor color;
    float lineWidth;
    WhirlyKit::SimpleIdentity shapeId;
    bool insideOut;
    bool zBufferRead;
    bool zBufferWrite;
    bool hasCenter;
    WhirlyKit::Point3d center;
};



/** This drawable builder is associated with the shape layer.  It's
    exposed so it can be used by the active model version as well.
  */
class ShapeDrawableBuilder
{
public:
    /// Construct the builder with the fade value for each created drawable and
    ///  whether we're doing lines or points
    ShapeDrawableBuilder(WhirlyKit::CoordSystemDisplayAdapter *coordAdapter,WhirlyKitShapeInfo *shapeInfo,bool linesOrPoints,const Point3d &center);
    virtual ~ShapeDrawableBuilder();

    /// A group of points (in display space) all at once
    void addPoints(std::vector<Point3f> &pts,RGBAColor color,Mbr mbr,float lineWidth,bool closed);

    /// Flush out the current drawable (if there is one) to the list of finished ones
    void flush();

    /// Retrieve the scene changes and the list of drawable IDs for later
    void getChanges(WhirlyKit::ChangeSet &changeRequests,SimpleIDSet &drawIDs);

    WhirlyKitShapeInfo *getShapeInfo() { return shapeInfo; }

public:
    CoordSystemDisplayAdapter *coordAdapter;
    GLenum primType;
    WhirlyKitShapeInfo *shapeInfo;
    Mbr drawMbr;
    BasicDrawable *drawable;
    std::vector<BasicDrawable *> drawables;
    Point3d center;
};

/** Drawable Builder (Triangle version) is used to build up shapes made out of triangles.
    It's intended for use by the shape layer and the active version of that.
 */
class ShapeDrawableBuilderTri
{
public:
    /// Construct with the visual description
    ShapeDrawableBuilderTri(WhirlyKit::CoordSystemDisplayAdapter *coordAdapter,WhirlyKitShapeInfo *shapeInfo,const Point3d &center);
    virtual ~ShapeDrawableBuilderTri();

    // Add a triangle with normals
    void addTriangle(Point3f p0,Point3f n0,RGBAColor c0,Point3f p1,Point3f n1,RGBAColor c1,Point3f p2,Point3f n2,RGBAColor c2,Mbr shapeMbr);

    // Add a triangle with normals
    void addTriangle(Point3d p0,Point3d n0,RGBAColor c0,Point3d p1,Point3d n1,RGBAColor c1,Point3d p2,Point3d n2,RGBAColor c2,Mbr shapeMbr);

    // Add a group of pre-build triangles
    void addTriangles(std::vector<Point3f> &pts,std::vector<Point3f> &norms,std::vector<RGBAColor> &colors,std::vector<BasicDrawable::Triangle> &tris);

    // Add a convex outline, triangulated
    void addConvexOutline(std::vector<Point3f> &pts,Point3f norm,RGBAColor color,Mbr shapeMbr);

    // Add a convex outline, triangulated
    void addConvexOutline(std::vector<Point3d> &pts,Point3d norm,RGBAColor color,Mbr shapeMbr);

    // Add a complex outline, triangulated
    void addComplexOutline(std::vector<Point3d> &pts,Point3d norm,RGBAColor color,Mbr shapeMbr);

    /// Flush out the current drawable (if there is one) to the list of finished ones
    void flush();

    /// Retrieve the scene changes and the list of drawable IDs for later
    void getChanges(ChangeSet &changeRequests,SimpleIDSet &drawIDs);

    WhirlyKitShapeInfo *getShapeInfo() { return shapeInfo; }

public:
    // Creates a new local drawable with all the appropriate settings
    void setupNewDrawable();

    CoordSystemDisplayAdapter *coordAdapter;    
    Mbr drawMbr;
    WhirlyKitShapeInfo *shapeInfo;
    BasicDrawable *drawable;
    std::vector<BasicDrawable *> drawables;
    Point3d center;
};

}
