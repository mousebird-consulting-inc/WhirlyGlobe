/*  ShapeDrawableBuilder.h
 *  WhirlyGlobeLib
 *
 *  Created by Steve Gifford on 9/28/11.
 *  Copyright 2011-2022 mousebird consulting.
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
 */

#import "Identifiable.h"
#import "WhirlyVector.h"
#import "BaseInfo.h"
#import "BasicDrawable.h"
#import "BasicDrawableBuilder.h"
#import "SceneRenderer.h"

namespace WhirlyKit
{

/// Used to pass shape info between the shape layer and the drawable builder
///  and within the threads of the shape layer
struct ShapeInfo : public BaseInfo
{
    ShapeInfo();
    ShapeInfo(const Dictionary &);
    ShapeInfo(const ShapeInfo &that);
    virtual ~ShapeInfo() = default;

    // Convert contents to a string for debugging
    virtual std::string toString() const { return BaseInfo::toString() + " +ShapeInfo..."; }

    RGBAColor color = RGBAColor::white();
    float lineWidth = 1.0f;
    bool insideOut = false;
    bool hasCenter = false;
    WhirlyKit::Point3d center = { 0, 0, 0 };
};
typedef std::shared_ptr<ShapeInfo> ShapeInfoRef;

/** This drawable builder is associated with the shape layer.  It's
    exposed so it can be used by the active model version as well.
  */
struct ShapeDrawableBuilder
{
    EIGEN_MAKE_ALIGNED_OPERATOR_NEW;
    
    /// Construct the builder with the fade value for each created drawable and
    ///  whether we're doing lines or points
    ShapeDrawableBuilder(const CoordSystemDisplayAdapter *,
                         SceneRenderer *,
                         const ShapeInfo &,
                         bool linesOrPoints,
                         const Point3d &center);
    virtual ~ShapeDrawableBuilder() = default;

    void setClipCoords(bool newClipCoords);

    /// A group of points (in display space) all at once
    void addPoints(const Point3dVector &,RGBAColor,const Mbr &,float lineWidth,bool closed);

    /// Flush out the current drawable (if there is one) to the list of finished ones
    void flush();

    /// Retrieve the scene changes and the list of drawable IDs for later
    void getChanges(ChangeSet &,SimpleIDSet &drawIDs);

    const ShapeInfo *getShapeInfo() { return &shapeInfo; }

    const CoordSystemDisplayAdapter *coordAdapter;
    SceneRenderer *sceneRender;

protected:
    GeometryType primType;
    const ShapeInfo &shapeInfo;
    Mbr drawMbr;
    BasicDrawableBuilderRef drawable;
    std::vector<BasicDrawableBuilderRef> drawables;
    Point3d center;
    // Some special shapes are already in OpenGL clip space
    bool clipCoords = false;
};

/** Drawable Builder (Triangle version) is used to build up shapes made out of triangles.
    It's intended for use by the shape layer and the active version of that.
 */
struct ShapeDrawableBuilderTri
{
    EIGEN_MAKE_ALIGNED_OPERATOR_NEW;
    
    /// Construct with the visual description
    ShapeDrawableBuilderTri(const CoordSystemDisplayAdapter *,
                            SceneRenderer *,
                            const ShapeInfo &,
                            const Point3d &center);
    virtual ~ShapeDrawableBuilderTri() = default;

    // If set the geometry is already in OpenGL clip coordinates, so we don't transform it
    void setClipCoords(bool newClipCoords);
    
    // If set, we'll apply the given texture
    void setTexIDs(const std::vector<SimpleIdentity> &texIDs);

    // Add a triangle with normals
    void addTriangle(Point3f p0,Point3f n0,RGBAColor c0,Point3f p1,Point3f n1,RGBAColor c1,Point3f p2,Point3f n2,RGBAColor c2,Mbr shapeMbr);

    // Add a triangle with normals
    void addTriangle(Point3d p0,Point3d n0,RGBAColor c0,Point3d p1,Point3d n1,RGBAColor c1,Point3d p2,Point3d n2,RGBAColor c2,Mbr shapeMbr);

    // Add a triangle with normals and texture coords
    void addTriangle(const Point3d &p0,const Point3d &n0,RGBAColor c0,const TexCoord &tx0,const Point3d &p1,const Point3d &n1,RGBAColor c1,const TexCoord &tx1,const Point3d &p2,const Point3d &n2,RGBAColor c2,const TexCoord &tx2,Mbr shapeMbr);
    
    // Add a group of pre-build triangles
    void addTriangles(Point3dVector &pts,Point3dVector &norms,std::vector<RGBAColor> &colors,std::vector<BasicDrawable::Triangle> &tris);

    // Add a convex outline, triangulated
    void addConvexOutline(Point3fVector &pts,Point3f norm,RGBAColor color,Mbr shapeMbr);

    // Add a convex outline, triangulated
    void addConvexOutline(Point3dVector &pts,Point3d norm,RGBAColor color,Mbr shapeMbr);

    // Add a convex outline, triangulated with texture coords
    void addConvexOutline(Point3dVector &pts,std::vector<TexCoord> &texCoords,Point3d norm,RGBAColor color,Mbr shapeMbr);

    // Add a complex outline, triangulated
    void addComplexOutline(Point3dVector &pts,Point3d norm,RGBAColor color,Mbr shapeMbr);

    /// Flush out the current drawable (if there is one) to the list of finished ones
    void flush();

    /// Retrieve the scene changes and the list of drawable IDs for later
    void getChanges(ChangeSet &changeRequests,SimpleIDSet &drawIDs);

    const ShapeInfo *getShapeInfo() { return &shapeInfo; }

    const std::vector<BasicDrawableBuilderRef> &getDrawables() const { return drawables; }

    const CoordSystemDisplayAdapter *coordAdapter;
    SceneRenderer *sceneRender;

    /// The name set on generated drawables, for debugging
    void setDrawableName(const char *name) { drawableName = name ? name : ""; }
    void setDrawableName(std::string name) { drawableName = std::move(name); }

protected:
    // Creates a new local drawable with all the appropriate settings
    void setupNewDrawable();

    Mbr drawMbr;
    const ShapeInfo &shapeInfo;
    BasicDrawableBuilderRef drawable;
    std::vector<BasicDrawableBuilderRef> drawables;
    std::vector<SimpleIdentity> texIDs;
    Point3d center;
    std::string drawableName;
    bool clipCoords = false;
};

}
