/*  VectorManager.cpp
 *  WhirlyGlobeLib
 *
 *  Created by Steve Gifford on 1/26/11.
 *  Copyright 2011-2023 mousebird consulting
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

#import "WhirlyKitLog.h"
#import "VectorManager.h"
#import "WhirlyGeometry.h"
#import "Tesselator.h"
#import "GridClipper.h"
#import "SharedAttributes.h"
#import "Platform.h"

using namespace Eigen;
using namespace WhirlyKit;

namespace WhirlyKit
{

static const std::string vecBuilderName("Vector Layer"); // NOLINT(cert-err58-cpp)
static const std::string vecManagerName("VectorManager"); // NOLINT(cert-err58-cpp)
static const std::string colorStr("color"); // NOLINT(cert-err58-cpp)   constructor can throw

VectorInfo::VectorInfo(const Dictionary &dict)
    : BaseInfo(dict)
{
    filled = dict.getBool(MaplyFilled,filled);
    texId = dict.getInt(MaplyVecTexture,texId);
    texScale = { dict.getDouble(MaplyVecTexScaleX,texScale.x()),
                 dict.getDouble(MaplyVecTexScaleY,texScale.y()) };
    subdivEps = (float)dict.getDouble(MaplySubdivEpsilon,subdivEps);
    color = dict.getColor(MaplyColor,color);
    lineWidth = (float)dict.getDouble(MaplyVecWidth,lineWidth);
    centered = dict.getBool(MaplyVecCentered,centered);
    closeAreals = dict.getBool(MaplyVecCloseAreals, closeAreals);

    const auto sampleVal = (float)dict.getDouble("sample", 0.0);
    sample = (sampleVal > 0) ? sampleVal : (dict.getBool("sample",sample) ? 0.1f : 0.0f);

    const std::string subdivType = dict.getString(MaplySubdivType);
    gridSubdiv = subdivType == MaplySubdivGrid;

    const std::string texProjStr = dict.getString(MaplyVecTextureProjection,"");
    if (texProjStr == MaplyVecProjectionTangentPlane)
    {
        texProj = TextureProjectionTanPlane;
    }
    else if (texProjStr == MaplyVecProjectionScreen)
    {
        texProj = TextureProjectionScreen;
    }

    if (dict.hasField(MaplyVecCenterX) && dict.hasField(MaplyVecCenterY))
    {
        vecCenterSet = true;
        vecCenter = { dict.getDouble(MaplyVecCenterX), dict.getDouble(MaplyVecCenterY) };
    }
}

std::string VectorInfo::toString() const
{
    using std::to_string;
    std::string outStr = BaseInfo::toString();
    
    outStr +=
    (std::string)"filled = " + (filled ? "yes" : "no") + ";" +
    " sample = " + to_string(sample) + ";" +
    " texId = " + to_string(texId) + ";" +
    " texScale = (" + to_string(texScale.x()) + "," + to_string(texScale.x()) + ");" +
    " subdivEps = " + to_string(subdivEps) + ";" +
    " gridSubdiv = " + (gridSubdiv ? "yes" : "no") + ";" +
    " texProj = " + to_string(texProj) + ";" +
    " color = (" + to_string((int)color.r) + "," + to_string((int)color.g) + "," + to_string((int)color.b) + "," + to_string((int)color.a) + ");" +
    " lineWidth = " + to_string(lineWidth) + ";" +
    " centered = " + (centered ? "yes" : "no") + ";" +
    " vecCenterSet = " + (vecCenterSet ? "yes" : "no") + ";" +
    " vecCenter = (" + to_string(vecCenter.x()) + "," + to_string(vecCenter.y()) + ");";
    
    return outStr;
}

void VectorSceneRep::clear(ChangeSet &changes)
{
    for (auto it : drawIDs)
    {
        changes.push_back(new RemDrawableReq(it));
    }
    drawIDs.clear();
}

/* Drawable Builder
 Used to construct drawables with multiple shapes in them.
 Eventually, we'll move this out to be a more generic object.
 */
class VectorDrawableBuilder
{
public:
    VectorDrawableBuilder(Scene *scene,SceneRenderer *sceneRender,ChangeSet &changeRequests,VectorSceneRep *sceneRep,
                          const VectorInfo *vecInfo,bool linesOrPoints,bool doColor) :
        doColor(doColor),
        scene(scene),
        sceneRender(sceneRender),
        changeRequests(changeRequests),
        sceneRep(sceneRep),
        vecInfo(vecInfo),
        primType(linesOrPoints ? Lines : Points)
    {
    }
    
    ~VectorDrawableBuilder()
    {
        flush();
    }
    
    void setCenter(const Point3d &newCenter,const Point2d &inGeoCenter)
    {
        centerValid = true;
        center = newCenter;
        geoCenter = inGeoCenter;
    }
    
    void addPoints(const VectorRing3d &inPts, bool closed,
                   const MutableDictionaryRef &attrs, bool localCoords)
    {
        // todo: vector copy would be better done as an iterator adaptor
        addPoints(Slice<double,float>(inPts),closed,attrs,localCoords);
    }

    void addPoints(const VectorRing &pts,bool closed,const MutableDictionaryRef &attrs, bool localCoords)
    {
        const CoordSystemDisplayAdapter *coordAdapter = scene->getCoordAdapter();
        const CoordSystem *coordSys = coordAdapter->getCoordSystem();
        const RGBAColor ringColor = attrs->getColor(MaplyColor, vecInfo->color);
        
        // Decide if we'll appending to an existing drawable or create a new one
        const int ptCount = (int)(2*(pts.size()+1));
        if (!drawable || (drawable->getNumPoints()+ptCount > MaxDrawablePoints))
        {
            // We're done with it, toss it to the scene
            if (drawable)
                flush();

            const auto &name = vecInfo->drawableName.empty() ? vecBuilderName : vecInfo->drawableName;
            drawable = sceneRender->makeBasicDrawableBuilder(name);
            drawMbr.reset();
            drawable->setType(primType);
            vecInfo->setupBasicDrawable(drawable);
            // Adjust according to the vector info
            drawable->setColor(ringColor);
            drawable->setLineWidth(vecInfo->lineWidth);
            drawable->setColorExpression(vecInfo->colorExp);
            drawable->setOpacityExpression(vecInfo->opacityExp);
        }
        drawMbr.addPoints(pts);
        
        Point3f prevPt,prevNorm,firstPt,firstNorm;
        for (unsigned int jj=0;jj<pts.size();jj++)
        {
            // Convert to real world coordinates and offset from the globe
            const Point2f &geoPt = pts[jj];
            const Point2d geoCoordD(geoPt.x()+geoCenter.x(),geoPt.y()+geoCenter.y());
            const Point3d localPt = localCoords ? Pad(geoCoordD) : coordSys->geographicToLocal(geoCoordD);
            const Point3d norm3d = coordAdapter->normalForLocal(localPt);
            const Point3f norm = norm3d.cast<float>();
            const Point3d pt3d = coordAdapter->localToDisplay(localPt) - center;
            const Point3f pt = pt3d.cast<float>();
            
            // Add to drawable
            // Depending on the type, we do this differently
            if (primType == Points)
            {
                drawable->addPoint(pt);
                if (doColor)
                   drawable->addColor(ringColor);
                drawable->addNormal(norm);
            } else {
                if (jj > 0)
                {
                    drawable->addPoint(prevPt);
                    drawable->addPoint(pt);
                    if (doColor)
                    {
                        drawable->addColor(ringColor);
                        drawable->addColor(ringColor);
                    }
                    drawable->addNormal(prevNorm);
                    drawable->addNormal(norm);
                } else {
                    firstPt = pt;
                    firstNorm = norm;
                }
                prevPt = pt;
                prevNorm = norm;
            }
        }
        
        // Close the loop
        if (closed && primType == Lines)
        {
            drawable->addPoint(prevPt);
            drawable->addPoint(firstPt);
            if (doColor)
            {
                drawable->addColor(ringColor);
                drawable->addColor(ringColor);
            }
            drawable->addNormal(prevNorm);
            drawable->addNormal(firstNorm);
        }
    }
    
    void flush()
    {
        if (drawable)
        {
            if (drawable->getNumPoints() > 0)
            {
                drawable->setLocalMbr(drawMbr);
                sceneRep->drawIDs.insert(drawable->getDrawableID());
                if (centerValid)
                {
                    const Eigen::Affine3d trans(Eigen::Translation3d(center.x(),center.y(),center.z()));
                    Matrix4d transMat = trans.matrix();
                    drawable->setMatrix(&transMat);
                }
                
                if (vecInfo->fadeIn > 0.0)
                {
                    // fadeDown < fadeUp : fading in
                    const TimeInterval curTime = scene->getCurrentTime();
                    drawable->setFade(curTime,curTime+vecInfo->fadeIn);
                }
                else if (vecInfo->fadeOut > 0.0 && vecInfo->fadeOutTime > 0.0)
                {
                    // fadeUp < fadeDown : fading out
                    drawable->setFade(/*down=*/vecInfo->fadeOutTime+vecInfo->fadeOut, /*up=*/vecInfo->fadeOutTime);
                }

                changeRequests.push_back(new AddDrawableReq(drawable->getDrawable()));
            }
            drawable = nullptr;
        }
    }

protected:
    bool doColor;
    Scene *scene = nullptr;
    SceneRenderer *sceneRender = nullptr;
    ChangeSet &changeRequests;
    VectorSceneRep *sceneRep = nullptr;
    Mbr drawMbr;
    BasicDrawableBuilderRef drawable;
    const VectorInfo *vecInfo;
    Point3d center = { 0, 0, 0 };
    Point2d geoCenter = { 0, 0 };
    bool centerValid = false;
    const GeometryType primType;
};

/* Drawable Builder (Triangle version)
 Used to construct drawables with multiple shapes in them.
 Eventually, we'll move this out to be a more generic object.
 */
class VectorDrawableBuilderTri
{
public:
    VectorDrawableBuilderTri(Scene *scene,SceneRenderer *sceneRender,ChangeSet &changeRequests,VectorSceneRep *sceneRep,
                             const VectorInfo *vecInfo,bool doColor) :
        doColor(doColor),
        scene(scene),
        sceneRender(sceneRender),
        changeRequests(changeRequests),
        sceneRep(sceneRep),
        center(0,0,0),
        geoCenter(0,0),
        centerValid(false),
        drawable(nullptr),
        vecInfo(vecInfo)
    {
    }
    
    ~VectorDrawableBuilderTri()
    {
        flush();
    }
    
    void setCenter(const Point3d &newCenter,const Point2d &newGeoCenter)
    {
        centerValid = true;
        center = newCenter;
        geoCenter = newGeoCenter;
    }
    
    // This version converts a ring into a mesh (chopping, tessellating, etc...)
    void addPoints(const VectorRing &ring,const MutableDictionaryRef &attrs, bool localCoords)
    {
        // Grid subdivision is done here
        std::vector<VectorRing> inRings;
        if (vecInfo->gridSubdiv && vecInfo->subdivEps > 0.0)
        {
            const Point2f origin(0.0f, 0.0f);
            const Point2f spacing(vecInfo->subdivEps,vecInfo->subdivEps);
            ClipLoopToGrid(ring, origin, spacing, inRings);
        }
        else
        {
            inRings.push_back(ring);
        }
        
        VectorTrianglesRef mesh(VectorTriangles::createTriangles());
        for (auto &inRing : inRings)
        {
            TesselateRing(inRing,mesh);
        }
        
        addPoints(mesh, attrs, localCoords);
    }

    // This version converts a ring into a mesh (chopping, tessellating, etc...)
    void addPoints(const VectorRing3d &inRing,const MutableDictionaryRef &attrs, bool localCoords)
    {
        VectorRing ring = Slice<double,float>(inRing);

        // Grid subdivision is done here
        std::vector<VectorRing> inRings;
        if (vecInfo->subdivEps > 0.0 && vecInfo->gridSubdiv)
        {
            const Point2f origin(0.0f, 0.0f);
            const Point2f spacing(vecInfo->subdivEps,vecInfo->subdivEps);
            ClipLoopToGrid(ring, origin, spacing, inRings);
        }
        else
        {
            inRings.push_back(ring);
        }
        
        VectorTrianglesRef mesh(VectorTriangles::createTriangles());
        for (auto &ir : inRings)
        {
            TesselateRing(ir,mesh);
        }
        
        addPoints(mesh, attrs, localCoords);
    }

    // This version converts a ring into a mesh (chopping, tessellating, etc...)
    void addPoints(const std::vector<VectorRing> &rings,const MutableDictionaryRef &attrs, bool localCoords)
    {
        // Grid subdivision is done here
        std::vector<VectorRing> inRings;
        if (vecInfo->subdivEps > 0.0 && vecInfo->gridSubdiv)
        {
            for (const auto & ring : rings)
            {
                const Point2f origin(0.0f, 0.0f);
                const Point2f spacing(vecInfo->subdivEps,vecInfo->subdivEps);
                ClipLoopToGrid(ring, origin, spacing, inRings);
            }
        }
        else
        {
            inRings = rings;
        }

        VectorTrianglesRef mesh(VectorTriangles::createTriangles());
        TesselateLoops(inRings, mesh);
        
        addPoints(mesh, attrs, localCoords);
    }

    void addPoints(const VectorTrianglesRef &mesh, const MutableDictionaryRef &attrs, bool localCoords)
    {
        addPoints(*mesh, attrs, localCoords);
    }

    // If it's a mesh, we're assuming it's been fully processed (triangulated, chopped, and so on)
    void addPoints(const VectorTriangles &mesh, const MutableDictionaryRef &attrs, bool localCoords)
    {
        const RGBAColor ringColor = attrs->getColor(MaplyColor, vecInfo->color);

        const CoordSystemDisplayAdapter *coordAdapter = scene->getCoordAdapter();
        const CoordSystem *coordSys = coordAdapter->getCoordSystem();

        Point2d centroid(0,0);
        if (attrs->hasField(MaplyVecCenterX) && attrs->hasField(MaplyVecCenterY))
        {
            centroid.x() = attrs->getDouble(MaplyVecCenterX);
            centroid.y() = attrs->getDouble(MaplyVecCenterY);

            if (localCoords)
            {
                centroid = Slice(coordSys->geographicToLocal(centroid));
            }
        }
        
        for (size_t ir=0;ir<mesh.tris.size();ir++)
        {
            constexpr int triCount = 1;
            constexpr int ptCount = 3;
            Point2f pts[ptCount];
            if (!mesh.getTriangle(ir, pts))
            {
                continue;
            }

            // Decide if we'll appending to an existing drawable or create a new one
            if (!drawable ||
                (drawable->getNumPoints()+ptCount > MaxDrawablePoints) ||
                (drawable->getNumTris()+triCount > MaxDrawableTriangles))
            {
                // We're done with it, toss it to the scene
                if (drawable)
                    flush();
                
                drawable = sceneRender->makeBasicDrawableBuilder(vecBuilderName);
                drawMbr.reset();
                drawable->setType(Triangles);
                vecInfo->setupBasicDrawable(drawable);
                drawable->setColorExpression(vecInfo->colorExp);
                drawable->setOpacityExpression(vecInfo->opacityExp);
                drawable->setColor(ringColor);
                if (vecInfo->texId != EmptyIdentity)
                    drawable->setTexId(0, vecInfo->texId);
            }
            const unsigned baseVert = drawable->getNumPoints();
            drawMbr.addPoints(pts);
            
            bool doTexCoords = vecInfo->texId != EmptyIdentity;
            
            // Need an origin for this type of texture coordinate projection
            Point3d planeOrg(0,0,0);
            Point3d planeUp(0,0,1);
            Point3d planeX(1,0,0);
            Point3d planeY(0,1,0);
            if (vecInfo->texProj == TextureProjectionTanPlane)
            {
                const Point3d localPt = coordSys->geographicToLocal(centroid);
                planeOrg = coordAdapter->localToDisplay(localPt);
                planeUp = coordAdapter->normalForLocal(localPt);
                planeX = Point3d(0,0,1).cross(planeUp);
                planeY = planeUp.cross(planeX);
                planeX.normalize();
                planeY.normalize();
            }
            else if (vecInfo->texProj == TextureProjectionScreen)
            {
                // Don't need actual tex coordinates for screen space
                doTexCoords = false;
            }
            
            // Generate the textures coordinates
            TexCoord texCoords[ptCount];
            if (doTexCoords)
            {
                TexCoord minCoord(MAXFLOAT,MAXFLOAT);
                int i = 0;
                for (const auto &geoPt : pts)
                {
                    const Point2d geoCoordD = geoPt.cast<double>() + geoCenter;

                    auto &texCoord = texCoords[i++];
                    switch (vecInfo->texProj)
                    {
                        case TextureProjectionTanPlane:
                        {
                            const Point3d locPt = localCoords ? Pad(geoCoordD) : coordSys->geographicToLocal(geoCoordD);
                            const Point3d displayPt = coordAdapter->localToDisplay(locPt) - center;
                            const Point3d dir = displayPt - planeOrg;
                            const Point3d comp(dir.dot(planeX),dir.dot(planeY),dir.dot(planeUp));
                            texCoord = Slice(comp).cast<float>().cwiseProduct(vecInfo->texScale);
                            break;
                        }
                        case TextureProjectionNone:
                        default:
                            texCoord = (geoPt - centroid.cast<float>()).cwiseProduct(vecInfo->texScale);
                            break;
                    }

                    minCoord.x() = std::min(minCoord.x(),texCoord.x());
                    minCoord.y() = std::min(minCoord.y(),texCoord.y());
                }
                // Essentially do a mod, since texture coordinates repeat
                int minS = 0;
                int minT = 0;
                if (minCoord.x() != MAXFLOAT)
                {
                    minS = (int)std::floor(minCoord.x());
                    minT = (int)std::floor(minCoord.y());
                }
                for (auto &texCoord : texCoords)
                {
                    texCoord.x() -= (float)minS;
                    texCoord.y() -= (float)minT;
                }
            }
            
            // Add the points
            for (unsigned int jj=0;jj<ptCount;jj++)
            {
                // Convert to real world coordinates and offset from the globe
                const Point2f &geoPt = pts[jj];
                const Point2d geoCoordD = geoPt.cast<double>() + geoCenter;
                const Point3d localPt = localCoords ? Pad(geoCoordD) : coordSys->geographicToLocal(geoCoordD);
                const Point3d norm3d = coordAdapter->normalForLocal(localPt);
                const Point3f norm(norm3d.x(),norm3d.y(),norm3d.z());
                const Point3d pt3d = coordAdapter->localToDisplay(localPt) - center;
                const Point3f pt = pt3d.cast<float>();
                
                drawable->addPoint(pt);
                if (doColor)
                {
                    drawable->addColor(ringColor);
                }
                drawable->addNormal(norm);
                if (doTexCoords)
                {
                    drawable->addTexCoord(0, texCoords[jj]);
                }
            }
            
            // Add the triangles
            // Note: Should be reusing vertex indices
            drawable->addTriangle(BasicDrawable::Triangle(0+baseVert,2+baseVert,1+baseVert));
        }
    }
    
    void flush()
    {
        if (drawable)
        {
            if (drawable->getNumPoints() > 0)
            {
                // If we're doing screen coordinates, attach the tweaker
                if (vecInfo->texProj == TextureProjectionScreen)
                {
                    const Point2f midPt = drawMbr.mid();
                    const Point3d centerPt = scene->getCoordAdapter()->localToDisplay(Point3d(midPt.x(),midPt.y(),0.0));
                    const Point2d texScale(vecInfo->texScale.x(),vecInfo->texScale.y());
                    drawable->addTweaker(std::make_shared<BasicDrawableScreenTexTweaker>(centerPt,texScale));
                }

                drawable->setLocalMbr(drawMbr);
                if (centerValid)
                {
                    const Eigen::Affine3d trans(Eigen::Translation3d(center.x(),center.y(),center.z()));
                    drawable->setMatrix(trans.matrix());
                }
                sceneRep->drawIDs.insert(drawable->getDrawableID());

                if (vecInfo->fadeIn > 0.0)
                {
                    // fadeDown < fadeUp = fading in
                    const TimeInterval curTime = scene->getCurrentTime();
                    drawable->setFade(curTime,curTime+vecInfo->fadeIn);
                }
                else if (vecInfo->fadeOutTime > 0.0)
                {
                    // fadeUp < fadeDown = fading out
                    drawable->setFade(/*down=*/vecInfo->fadeOutTime+vecInfo->fadeOut, /*up=*/vecInfo->fadeOutTime);
                }

                changeRequests.push_back(new AddDrawableReq(drawable->getDrawable()));
            }
            drawable = nullptr;
        }
    }
    
protected:   
    bool doColor;
    Scene *scene;
    SceneRenderer *sceneRender;
    ChangeSet &changeRequests;
    VectorSceneRep *sceneRep;
    Mbr drawMbr;
    Point3d center;
    Point2d geoCenter;
    bool centerValid;
    BasicDrawableBuilderRef drawable;
    const VectorInfo *vecInfo;
};

VectorManager::~VectorManager()
{
    try
    {
        std::lock_guard<std::mutex> guardLock(lock);
        
        for (auto it : vectorReps)
            delete it;
        vectorReps.clear();
    }
    WK_STD_DTOR_CATCH()
}

// TODO: Get rid of this version
SimpleIdentity VectorManager::addVectors(const ShapeSet *shapes, const VectorInfo &vecInfo, ChangeSet &changes)
{
    if (shapes->empty())
        return EmptyIdentity;
    
    auto *sceneRep = new VectorSceneRep();
    sceneRep->fadeOut = (float)vecInfo.fadeOut;

    // Look for per vector colors
    bool doColors = false;
    for (const auto &it : *shapes)
    {
        if (it->getAttrDict()->hasField(colorStr))
        {
            doColors = true;
            break;
        }
    }

    // Look for a geometry center.  We'll offset everything if there is one
    const CoordSystemDisplayAdapter *coordAdapter = scene->getCoordAdapter();
    const CoordSystem *coordSys = coordAdapter->getCoordSystem();
    Point3d center(0,0,0);
    bool centerValid = false;
    Point2d geoCenter(0,0);
    // Note: Should work for the globe, but doesn't
    if (vecInfo.centered && coordAdapter->isFlat())
    {
        // We might pass in a center
        if (vecInfo.vecCenterSet)
        {
            geoCenter = vecInfo.vecCenter.cast<double>();
            const Point3d localPt = coordSys->geographicToLocal(geoCenter);
            center = coordAdapter->localToDisplay(localPt);
            centerValid = true;
        }
        else
        {
          // Calculate the center
          GeoMbr geoMbr;
          for (auto &shape : *shapes)
          {
              geoMbr.expand(shape->calcGeoMbr());
          }
          if (geoMbr.valid())
          {
              const Point3d p0 = coordAdapter->localToDisplay(coordSys->geographicToLocal3d(geoMbr.ll()));
              const Point3d p1 = coordAdapter->localToDisplay(coordSys->geographicToLocal3d(geoMbr.ur()));
              center = (p0+p1)/2.0;
              centerValid = true;
          }
        }
    }
    
    // Used to toss out drawables as we go
    // Its destructor will flush out the last drawable
    VectorDrawableBuilder drawBuild(scene,renderer,changes,sceneRep,&vecInfo,true,doColors);
    if (centerValid)
    {
        drawBuild.setCenter(center,geoCenter);
    }

    VectorDrawableBuilderTri drawBuildTri(scene,renderer,changes,sceneRep,&vecInfo,doColors);
    if (centerValid)
    {
        drawBuildTri.setCenter(center,geoCenter);
    }

    VectorRing tempRing, tempRing2;
    VectorRing3d tempRing3d;
    constexpr auto localCoords = false;

    for (auto const &it : *shapes)
    {
        if (const auto theAreal = dynamic_cast<VectorAreal*>(it.get()))
        {
            if (vecInfo.filled)
            {
                // Triangulate outside and loops
                drawBuildTri.addPoints(theAreal->loops,theAreal->getAttrDictRef(), localCoords);
                continue;
            }

            // Work through the loops
            for (const auto& ring : theAreal->loops)
            {
                if (ring.size() < 3)
                {
                    // no can do... should we draw it as a line instead?
                    continue;
                }

                const auto *theRing = &ring;
                if (vecInfo.closeAreals && ring.front() != ring.back())
                {
                    // Make a copy, duplicating the first point at the end
                    tempRing.clear();
                    tempRing.reserve(ring.size() + 1);
                    tempRing.assign(ring.begin(), ring.end());
                    tempRing.push_back(ring.front());
                    theRing = &tempRing;
                }

                const auto isClosed = (theRing->front() == theRing->back());

                // Break the edges around the globe (presumably)
                if (vecInfo.sample > 0.0)
                {
                    tempRing2.clear();
                    SubdivideEdges(*theRing, tempRing2, isClosed, vecInfo.sample);
                    theRing = &tempRing2;
                }

                drawBuild.addPoints(*theRing, isClosed, theAreal->getAttrDictRef(), localCoords);
            }
        }
        else if (const auto theLinear = dynamic_cast<VectorLinear*>(it.get()))
        {
            if (vecInfo.filled)
            {
                // Triangulate the outside
                drawBuildTri.addPoints(theLinear->pts,theLinear->getAttrDictRef(), localCoords);
            }
            else
            {
                if (vecInfo.sample > 0.0)
                {
                    tempRing.clear();
                    SubdivideEdges(theLinear->pts, tempRing, false, vecInfo.sample);
                    drawBuild.addPoints(tempRing,false,theLinear->getAttrDictRef(),localCoords);
                }
                else
                {
                    drawBuild.addPoints(theLinear->pts,false,theLinear->getAttrDictRef(),localCoords);
                }
            }
        }
        else if (const auto theLinear3d = dynamic_cast<VectorLinear3d*>(it.get()))
        {
            if (vecInfo.filled)
            {
                // Triangulate the outside
                drawBuildTri.addPoints(theLinear3d->pts,theLinear3d->getAttrDictRef(), localCoords);
            }
            else
            {
                if (vecInfo.sample > 0.0)
                {
                    tempRing3d.clear();
                    SubdivideEdges(theLinear3d->pts, tempRing3d, false, vecInfo.sample);
                    drawBuild.addPoints(tempRing3d,false,theLinear3d->getAttrDictRef(),localCoords);
                }
                else
                {
                    drawBuild.addPoints(theLinear3d->pts,false,theLinear3d->getAttrDictRef(),localCoords);
                }
            }
        }
        else if (const auto theMesh = dynamic_cast<VectorTriangles*>(it.get()))
        {
            if (vecInfo.filled)
            {
                drawBuildTri.addPoints(*theMesh, theMesh->getAttrDictRef(), theMesh->localCoords);
            }
            else
            {
                for (size_t ti=0;ti<theMesh->tris.size();ti++)
                {
                    tempRing.clear();
                    theMesh->getTriangle(ti, tempRing);
                    drawBuild.addPoints(tempRing, true, theMesh->getAttrDictRef(), theMesh->localCoords);
                }
            }
        }
    }
    
    drawBuild.flush();
    drawBuildTri.flush();
    
    SimpleIdentity vecID = sceneRep->getId();
    {
        std::lock_guard<std::mutex> guardLock(lock);
        vectorReps.insert(sceneRep);
    }
    
    return vecID;
}

SimpleIdentity VectorManager::addVectors(const std::vector<VectorShapeRef> *shapes,
                                         const VectorInfo &vecInfo, ChangeSet &changes)
{
    return shapes ? addVectors(*shapes,vecInfo,changes) : EmptyIdentity;
}

SimpleIdentity VectorManager::addVectors(const std::vector<VectorShapeRef> &shapes,
                                         const VectorInfo &vecInfo, ChangeSet &changes)
{
    if (shapes.empty())
    {
        return EmptyIdentity;
    }

    auto sceneRep = std::make_unique<VectorSceneRep>();
    sceneRep->fadeOut = (float)vecInfo.fadeOut;

    // Look for per vector colors
    bool doColors = (vecInfo.colorExp || vecInfo.opacityExp);
    if (!doColors)
    {
        for (const auto &shape : shapes)
        {
            if (shape->getAttrDict()->hasField(colorStr))
            {
                doColors = true;
                break;
            }
        }
    }

    // Look for a geometry center.  We'll offset everything if there is one
    CoordSystemDisplayAdapter *coordAdapter = scene->getCoordAdapter();
    const CoordSystem *coordSys = coordAdapter->getCoordSystem();
    Point3d center(0,0,0);
    bool centerValid = false;
    Point2d geoCenter(0,0);
    // Note: Should work for the globe, but doesn't
    if (vecInfo.centered && coordAdapter->isFlat())
    {
        // We might pass in a center
        if (vecInfo.vecCenterSet)
        {
            geoCenter = vecInfo.vecCenter.cast<double>();
            const Point3d localPt = coordSys->geographicToLocal(geoCenter);
            center = coordAdapter->localToDisplay(localPt);
            centerValid = true;
        }
        else
        {
          // Calculate the center
          GeoMbr geoMbr;
          for (auto &shape : shapes)
          {
              geoMbr.expand(shape->calcGeoMbr());
          }
          if (geoMbr.valid())
          {
              const Point3d p0 = coordAdapter->localToDisplay(coordSys->geographicToLocal3d(geoMbr.ll()));
              const Point3d p1 = coordAdapter->localToDisplay(coordSys->geographicToLocal3d(geoMbr.ur()));
              center = (p0+p1)/2.0;
              centerValid = true;
          }
        }
    }
    
    // Used to toss out drawables as we go
    // Its destructor will flush out the last drawable
    VectorDrawableBuilder drawBuild(scene,renderer,changes,sceneRep.get(),&vecInfo,true,doColors);
    if (centerValid)
    {
        drawBuild.setCenter(center,geoCenter);
    }

    VectorDrawableBuilderTri drawBuildTri(scene,renderer,changes,sceneRep.get(),&vecInfo,doColors);
    if (centerValid)
    {
        drawBuildTri.setCenter(center,geoCenter);
    }

    VectorRing newPts;
    VectorRing3d newPts3;

    for (auto const &it : shapes)
    {
        if (const auto theAreal = dynamic_cast<const VectorAreal*>(it.get()))
        {
            if (vecInfo.filled)
            {
                // Triangulate outside and loops
                drawBuildTri.addPoints(theAreal->loops,theAreal->getAttrDictRef(),false);
            }
            else
            {
                // Work through the loops
                for (unsigned int ri=0;ri<theAreal->loops.size();ri++)
                {
                    const VectorRing &ring = theAreal->loops[ri];
                    
                    // Break the edges around the globe (presumably)
                    if (vecInfo.sample > 0.0)
                    {
                        newPts.clear();
                        SubdivideEdges(ring, newPts, false, vecInfo.sample);
                        drawBuild.addPoints(newPts,true,theAreal->getAttrDictRef(),false);
                    }
                    else
                    {
                        drawBuild.addPoints(ring,true,theAreal->getAttrDictRef(),false);
                    }
                }
            }
        }
        else if (const auto theLinear = dynamic_cast<const VectorLinear*>(it.get()))
        {
            if (vecInfo.filled)
            {
                // Triangulate the outside
                drawBuildTri.addPoints(theLinear->pts,theLinear->getAttrDictRef(),false);
            }
            else if (vecInfo.sample > 0.0)
            {
                newPts.clear();
                SubdivideEdges(theLinear->pts, newPts, false, vecInfo.sample);
                drawBuild.addPoints(newPts,false,theLinear->getAttrDictRef(),false);
            }
            else
            {
                drawBuild.addPoints(theLinear->pts,false,theLinear->getAttrDictRef(),false);
            }
        }
        else if (const auto theLinear3d = dynamic_cast<const VectorLinear3d*>(it.get()))
        {
            if (vecInfo.filled)
            {
                // Triangulate the outside
                drawBuildTri.addPoints(theLinear3d->pts,theLinear3d->getAttrDictRef(),false);
            }
            else if (vecInfo.sample > 0.0)
            {
                newPts3.clear();
                SubdivideEdges(theLinear3d->pts, newPts3, false, vecInfo.sample);
                drawBuild.addPoints(newPts3,false,theLinear3d->getAttrDictRef(),false);
            }
            else
            {
                drawBuild.addPoints(theLinear3d->pts,false,theLinear3d->getAttrDictRef(),false);
            }
        }
        else if (const auto theMesh = dynamic_cast<VectorTriangles*>(it.get()))
        {
            if (vecInfo.filled)
            {
                drawBuildTri.addPoints(*theMesh,theMesh->getAttrDictRef(),theMesh->localCoords);
            }
            else
            {
                for (size_t ti=0;ti<theMesh->tris.size();ti++)
                {
                    newPts.clear();
                    theMesh->getTriangle(ti, newPts);
                    drawBuild.addPoints(newPts,true,theMesh->getAttrDictRef(),theMesh->localCoords);
                }
            }
        }
    }
    
    drawBuild.flush();
    drawBuildTri.flush();
    
    const SimpleIdentity vecID = sceneRep->getId();
    {
        std::lock_guard<std::mutex> guardLock(lock);
        vectorReps.insert(sceneRep.release());
    }
    
    return vecID;
}

SimpleIdentity VectorManager::instanceVectors(SimpleIdentity vecID,const VectorInfo &vecInfo,ChangeSet &changes)
{
    SimpleIdentity newId = EmptyIdentity;
    
    std::lock_guard<std::mutex> guardLock(lock);

    // Look for the representation
    VectorSceneRep dummyRep(vecID);
    const auto it = vectorReps.find(&dummyRep);
    if (it != vectorReps.end())
    {
        const VectorSceneRep *sceneRep = *it;
        auto newSceneRep = std::make_unique<VectorSceneRep>();
        for (const auto &idIt : sceneRep->drawIDs)
        {
            // Make up a BasicDrawableInstance
            const auto drawInst = renderer->makeBasicDrawableInstanceBuilder(vecManagerName);
            drawInst->setMasterID(idIt,BasicDrawableInstance::ReuseStyle);
            
            // Changed color
            drawInst->setColor(vecInfo.color);

            // Changed visibility
            drawInst->setVisibleRange((float)vecInfo.minVis, (float)vecInfo.maxVis);
            
            // Changed line width
            drawInst->setLineWidth(vecInfo.lineWidth);
            
            // Changed draw order
            drawInst->setDrawOrder(vecInfo.drawOrder);
            
            // Changed draw priority
            drawInst->setDrawPriority(vecInfo.drawPriority);
            
            // Changed draw order (is that possible?)
            drawInst->setDrawOrder(vecInfo.drawOrder);

            newSceneRep->instIDs.insert(drawInst->getDrawableID());
            changes.push_back(new AddDrawableReq(drawInst->getDrawable()));
        }
        
        vectorReps.insert(newSceneRep.release());
        newId = newSceneRep->getId();
    }
    
    return newId;
}

static std::unordered_set<SimpleIdentity> &AllIDs(const VectorSceneRep &rep, std::unordered_set<SimpleIdentity> &set)
{
    set.reserve(rep.drawIDs.size() + rep.instIDs.size());
    set.insert(rep.drawIDs.begin(), rep.drawIDs.end());
    set.insert(rep.instIDs.begin(), rep.instIDs.end());
    return set;
}

void VectorManager::changeVectors(SimpleIdentity vecID,const VectorInfo &vecInfo,ChangeSet &changes)
{
    std::unordered_set<SimpleIdentity> allIDs;

    std::lock_guard<std::mutex> guardLock(lock);

    VectorSceneRep dummyRep(vecID);
    const auto it = vectorReps.find(&dummyRep);
    if (it != vectorReps.end())
    {
        const VectorSceneRep *sceneRep = *it;

        // Make sure we change both drawables and instances
        allIDs.clear();
        for (const auto id : AllIDs(*sceneRep,allIDs))
        {
            // Changed color
            changes.push_back(new ColorChangeRequest(id, vecInfo.color));
            
            // Changed visibility
            if (vecInfo.minVis != DrawVisibleInvalid || vecInfo.maxVis != DrawVisibleInvalid)
            {
                changes.push_back(new VisibilityChangeRequest(id, (float)vecInfo.minVis, (float)vecInfo.maxVis));
            }
            
            // Changed line width
            changes.push_back(new LineWidthChangeRequest(id, vecInfo.lineWidth));
            
            // Changed draw priority
            changes.push_back(new DrawPriorityChangeRequest(id, vecInfo.drawPriority));
            
            // Changed draw order
            changes.push_back(new DrawOrderChangeRequest(id, vecInfo.drawOrder));
        }
    }
}

void VectorManager::removeVectors(SimpleIDSet &vecIDs,ChangeSet &changes)
{
    std::unordered_set<SimpleIdentity> allIDs;

    std::lock_guard<std::mutex> guardLock(lock);

    const TimeInterval curTime = scene->getCurrentTime();
    for (const auto id : vecIDs)
    {
        VectorSceneRep dummyRep(id);
        const auto it = vectorReps.find(&dummyRep);
        if (it == vectorReps.end())
        {
            continue;
        }

        // Take ownership and automatically clean up this object
        std::unique_ptr<VectorSceneRep> sceneRep(*it);
        vectorReps.erase(it);

        const bool fade = (sceneRep->fadeOut > 0.0);
        const auto fadeT = fade ? (curTime + sceneRep->fadeOut) : 0.0;

        // Make a copy and merge the IDs into it
        allIDs.clear();
        for (const auto id2 : AllIDs(*sceneRep, allIDs))
        {
            if (fade)
            {
                changes.push_back(new FadeChangeRequest(id2, curTime, fadeT));
            }
            changes.push_back(new RemDrawableReq(id2, fadeT));
        }
    }
}

void VectorManager::enableVectors(SimpleIDSet &vecIDs,bool enable,ChangeSet &changes)
{
    std::unordered_set<SimpleIdentity> allIDs;

    std::lock_guard<std::mutex> guardLock(lock);

    for (unsigned long long vecID : vecIDs)
    {
        VectorSceneRep dummyRep(vecID);
        const auto it = vectorReps.find(&dummyRep);
        if (it != vectorReps.end())
        {
            const VectorSceneRep *sceneRep = *it;
            AllIDs(*sceneRep, allIDs);
        }
    }

    for (const auto id : allIDs)
    {
        changes.push_back(new OnOffChangeRequest(id,enable));
    }
}

}
