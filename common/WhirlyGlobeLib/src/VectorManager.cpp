/*
 *  VectorManager.mm
 *  WhirlyGlobeLib
 *
 *  Created by Steve Gifford on 1/26/11.
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
    
VectorInfo::VectorInfo()
: filled(false), sample(false), texId(EmptyIdentity), texScale(1.0,1.0),
subdivEps(0.0), gridSubdiv(false), texProj(TextureProjectionNone),
color(RGBAColor(255,255,255,255)), lineWidth(1.0),
centered(true), vecCenterSet(false), vecCenter(0.0,0.0)
{
}

VectorInfo::VectorInfo(const Dictionary &dict)
: BaseInfo(dict)
{
    filled = dict.getBool(MaplyFilled,false);
    sample = dict.getBool("sample",false);
    texId = dict.getInt(MaplyVecTexture,EmptyIdentity);
    texScale.x() = dict.getDouble(MaplyVecTexScaleX,1.0);
    texScale.y() = dict.getDouble(MaplyVecTexScaleY,1.0);
    subdivEps = dict.getDouble(MaplySubdivEpsilon,0.0);
    std::string subdivType = dict.getString(MaplySubdivType);
    gridSubdiv = !subdivType.compare(MaplySubdivGrid);
    texProj = TextureProjectionNone;
    std::string texProjStr = dict.getString(MaplyVecTextureProjection,"");
    if (!texProjStr.compare(MaplyVecProjectionTangentPlane))
        texProj = TextureProjectionTanPlane;
    else if (!texProjStr.compare(MaplyVecProjectionScreen))
        texProj = TextureProjectionScreen;
    color = dict.getColor(MaplyColor,RGBAColor(255,255,255,255));
    lineWidth = dict.getDouble(MaplyVecWidth,1.0);
    centered = dict.getBool(MaplyVecCentered,true);
    vecCenterSet = false;
    if (dict.hasField(MaplyVecCenterX) && dict.hasField(MaplyVecCenterY))
    {
        vecCenterSet = true;
        vecCenter.x() = dict.getDouble(MaplyVecCenterX);
        vecCenter.x() = dict.getDouble(MaplyVecCenterY);
    }
}
    
// Really Android?  Really?
template <typename T>
std::string to_string(T value)
{
    std::ostringstream os ;
    os << value ;
    return os.str() ;
}

std::string VectorInfo::toString()
{
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
    for (SimpleIDSet::iterator it = drawIDs.begin(); it != drawIDs.end(); ++it)
        changes.push_back(new RemDrawableReq(*it));
}

/* Drawable Builder
 Used to construct drawables with multiple shapes in them.
 Eventually, we'll move this out to be a more generic object.
 */
class VectorDrawableBuilder
{
public:
    VectorDrawableBuilder(Scene *scene,SceneRenderer *sceneRender,ChangeSet &changeRequests,VectorSceneRep *sceneRep,
                          const VectorInfo *vecInfo,bool linesOrPoints,bool doColor)
    : changeRequests(changeRequests), scene(scene), sceneRender(sceneRender), sceneRep(sceneRep), vecInfo(vecInfo), drawable(NULL), centerValid(false), center(0,0,0), geoCenter(0,0), doColor(doColor)
    {
        primType = (linesOrPoints ? Lines : Points);
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
    
    void addPoints(VectorRing3d &inPts,bool closed,MutableDictionaryRef attrs)
    {
        VectorRing pts;
        pts.reserve(pts.size());
        for (const auto &pt : inPts)
            pts.push_back(Point2f(pt.x(),pt.y()));
        
        addPoints(pts,closed,attrs);
    }

    void addPoints(VectorRing &pts,bool closed,MutableDictionaryRef attrs)
    {
        CoordSystemDisplayAdapter *coordAdapter = scene->getCoordAdapter();
        RGBAColor ringColor = attrs->getColor(MaplyColor, vecInfo->color);
        
        // Decide if we'll appending to an existing drawable or
        //  create a new one
        int ptCount = (int)(2*(pts.size()+1));
        if (!drawable || (drawable->getNumPoints()+ptCount > MaxDrawablePoints))
        {
            // We're done with it, toss it to the scene
            if (drawable)
                flush();
            
            drawable = sceneRender->makeBasicDrawableBuilder("Vector Layer");
            drawMbr.reset();
            drawable->setType(primType);
            vecInfo->setupBasicDrawable(drawable);
            // Adjust according to the vector info
            drawable->setColor(ringColor);
            drawable->setLineWidth(vecInfo->lineWidth);
        }
        drawMbr.addPoints(pts);
        
        Point3f prevPt,prevNorm,firstPt,firstNorm;
        for (unsigned int jj=0;jj<pts.size();jj++)
        {
            // Convert to real world coordinates and offset from the globe
            Point2f &geoPt = pts[jj];
            Point2d geoCoordD(geoPt.x()+geoCenter.x(),geoPt.y()+geoCenter.y());
            Point3d localPt = coordAdapter->getCoordSystem()->geographicToLocal(geoCoordD);
            Point3d norm3d = coordAdapter->normalForLocal(localPt);
            Point3f norm(norm3d.x(),norm3d.y(),norm3d.z());
            Point3d pt3d = coordAdapter->localToDisplay(localPt) - center;
            Point3f pt(pt3d.x(),pt3d.y(),pt3d.z());
            
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
                    Eigen::Affine3d trans(Eigen::Translation3d(center.x(),center.y(),center.z()));
                    Matrix4d transMat = trans.matrix();
                    drawable->setMatrix(&transMat);
                }
                
                if (vecInfo->fade > 0.0)
                {
                    TimeInterval curTime = scene->getCurrentTime();
                    drawable->setFade(curTime,curTime+vecInfo->fade);
                }
                changeRequests.push_back(new AddDrawableReq(drawable->getDrawable()));
            }
            drawable = NULL;
        }
    }
    
protected:
    bool doColor;
    Scene *scene;
    SceneRenderer *sceneRender;
    ChangeSet &changeRequests;
    VectorSceneRep *sceneRep;
    Mbr drawMbr;
    BasicDrawableBuilderRef drawable;
    const VectorInfo *vecInfo;
    Point3d center;
    Point2d geoCenter;
    bool centerValid;
    GeometryType primType;
};

/* Drawable Builder (Triangle version)
 Used to construct drawables with multiple shapes in them.
 Eventually, we'll move this out to be a more generic object.
 */
class VectorDrawableBuilderTri
{
public:
    VectorDrawableBuilderTri(Scene *scene,SceneRenderer *sceneRender,ChangeSet &changeRequests,VectorSceneRep *sceneRep,
                             const VectorInfo *vecInfo,bool doColor)
    : changeRequests(changeRequests), scene(scene), sceneRender(sceneRender), sceneRep(sceneRep), vecInfo(vecInfo), drawable(NULL), centerValid(false), center(0,0,0), doColor(doColor), geoCenter(0,0)
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
    
    // This version converts a ring into a mesh (chopping, tesselating, etc...)
    void addPoints(VectorRing &ring,MutableDictionaryRef attrs)
    {
        // Grid subdivision is done here
        std::vector<VectorRing> inRings;
        if (vecInfo->subdivEps > 0.0 && vecInfo->gridSubdiv)
            ClipLoopToGrid(ring, Point2f(0.0,0.0), Point2f(vecInfo->subdivEps,vecInfo->subdivEps), inRings);
        else
            inRings.push_back(ring);
        VectorTrianglesRef mesh(VectorTriangles::createTriangles());
        for (unsigned int ii=0;ii<inRings.size();ii++)
            TesselateRing(inRings[ii],mesh);
        
        addPoints(mesh, attrs);
    }

    // This version converts a ring into a mesh (chopping, tesselating, etc...)
    void addPoints(VectorRing3d &inRing,MutableDictionaryRef attrs)
    {
        VectorRing ring;
        ring.reserve(inRing.size());
        for (const auto &pt : inRing)
            ring.push_back(Point2f(pt.x(),pt.y()));
        
        // Grid subdivision is done here
        std::vector<VectorRing> inRings;
        if (vecInfo->subdivEps > 0.0 && vecInfo->gridSubdiv)
            ClipLoopToGrid(ring, Point2f(0.0,0.0), Point2f(vecInfo->subdivEps,vecInfo->subdivEps), inRings);
        else
            inRings.push_back(ring);
        VectorTrianglesRef mesh(VectorTriangles::createTriangles());
        for (unsigned int ii=0;ii<inRings.size();ii++)
            TesselateRing(inRings[ii],mesh);
        
        addPoints(mesh, attrs);
    }

    // This version converts a ring into a mesh (chopping, tesselating, etc...)
    void addPoints(std::vector<VectorRing> &rings,MutableDictionaryRef attrs)
    {
        // Grid subdivision is done here
        std::vector<VectorRing> inRings;
        if (vecInfo->subdivEps > 0.0 && vecInfo->gridSubdiv)
            for (unsigned int ii=0;ii<rings.size();ii++)
                ClipLoopToGrid(rings[ii], Point2f(0.0,0.0), Point2f(vecInfo->subdivEps,vecInfo->subdivEps), inRings);
        else
            inRings = rings;
        VectorTrianglesRef mesh(VectorTriangles::createTriangles());
        TesselateLoops(inRings, mesh);
        
        addPoints(mesh, attrs);
    }

    // If it's a mesh, we're assuming it's been fully processed (triangulated, chopped, and so on)
    void addPoints(VectorTrianglesRef mesh,MutableDictionaryRef attrs)
    {
        RGBAColor ringColor = attrs->getColor(MaplyColor, vecInfo->color);

        CoordSystemDisplayAdapter *coordAdapter = scene->getCoordAdapter();
        Point2f centroid(0,0);
        if (attrs->hasField(MaplyVecCenterX) && attrs->hasField(MaplyVecCenterY))
        {
            centroid.x() = attrs->getDouble(MaplyVecCenterX);
            centroid.y() = attrs->getDouble(MaplyVecCenterY);
        }
        
        for (unsigned int ir=0;ir<mesh->tris.size();ir++)
        {
            VectorRing pts;
            mesh->getTriangle(ir, pts);
            // Decide if we'll appending to an existing drawable or
            //  create a new one
            int ptCount = (int)pts.size();
            int triCount = (int)(pts.size()-2);
            if (!drawable ||
                (drawable->getNumPoints()+ptCount > MaxDrawablePoints) ||
                (drawable->getNumTris()+triCount > MaxDrawableTriangles))
            {
                // We're done with it, toss it to the scene
                if (drawable)
                    flush();
                
                drawable = sceneRender->makeBasicDrawableBuilder("Vector Layer");
                drawMbr.reset();
                drawable->setType(Triangles);
                vecInfo->setupBasicDrawable(drawable);
                drawable->setColor(ringColor);
                if (vecInfo->texId != EmptyIdentity)
                    drawable->setTexId(0, vecInfo->texId);
            }
            int baseVert = drawable->getNumPoints();
            drawMbr.addPoints(pts);
            
            bool doTexCoords = vecInfo->texId != EmptyIdentity;
            
            // Need an origin for this type of texture coordinate projection
            Point3d planeOrg(0,0,0),planeUp(0,0,1),planeX(1,0,0),planeY(0,1,0);
            if (vecInfo->texProj == TextureProjectionTanPlane)
            {
                Point3d localPt = coordAdapter->getCoordSystem()->geographicToLocal3d(GeoCoord(centroid.x(),centroid.y()));
                planeOrg = coordAdapter->localToDisplay(localPt);
                planeUp = coordAdapter->normalForLocal(localPt);
                planeX = Point3d(0,0,1).cross(planeUp);
                planeY = planeUp.cross(planeX);
                planeX.normalize();
                planeY.normalize();
            } else if (vecInfo->texProj == TextureProjectionScreen)
            {
                // Don't need actual tex coordinates for screen space
                doTexCoords = false;
            }
            
            // Generate the textures coordinates
            std::vector<TexCoord> texCoords;
            if (doTexCoords)
            {
                texCoords.reserve(pts.size());
                TexCoord minCoord(MAXFLOAT,MAXFLOAT);
                for (unsigned int jj=0;jj<pts.size();jj++)
                {
                    Point2f &geoPt = pts[jj];
                    Point2d geoCoordD(geoPt.x()+geoCenter.x(),geoPt.y()+geoCenter.y());
                    
                    TexCoord texCoord;
                    switch (vecInfo->texProj)
                    {
                        case TextureProjectionTanPlane:
                        {
                            Point3d dispPt = coordAdapter->localToDisplay(coordAdapter->getCoordSystem()->geographicToLocal(geoCoordD))-center;
                            Point3d dir = dispPt - planeOrg;
                            Point3d comp(dir.dot(planeX),dir.dot(planeY),dir.dot(planeUp));
                            texCoord.x() = comp.x() * vecInfo->texScale.x();
                            texCoord.y() = comp.y() * vecInfo->texScale.y();
                        }
                            break;
                        case TextureProjectionNone:
                        default:
                            texCoord = TexCoord((geoPt.x()-centroid.x())*vecInfo->texScale.x(),(geoPt.y()-centroid.y())*vecInfo->texScale.y());
                            break;
                    }

                    texCoords.push_back(texCoord);
                    minCoord.x() = std::min(minCoord.x(),texCoord.x());
                    minCoord.y() = std::min(minCoord.y(),texCoord.y());
                }
                // Essentially do a mod, since texture coordinates repeat
                int minS = 0;
                int minT = 0;
                if (minCoord.x() != MAXFLOAT) {
                    minS = floorf(minCoord.x());
                    minT = floorf(minCoord.y());
                }
                for (unsigned int jj=0;jj<pts.size();jj++)
                {
                    TexCoord &texCoord = texCoords[jj];
                    texCoord.x() -= minS;
                    texCoord.y() -= minT;
                }
            }
            
            // Add the points
            for (unsigned int jj=0;jj<pts.size();jj++)
            {
                // Convert to real world coordinates and offset from the globe
                Point2f &geoPt = pts[jj];
                Point2d geoCoordD(geoPt.x()+geoCenter.x(),geoPt.y()+geoCenter.y());
                Point3d localPt = coordAdapter->getCoordSystem()->geographicToLocal(geoCoordD);
                Point3d norm3d = coordAdapter->normalForLocal(localPt);
                Point3f norm(norm3d.x(),norm3d.y(),norm3d.z());
                Point3d pt3d = coordAdapter->localToDisplay(localPt) - center;
                Point3f pt(pt3d.x(),pt3d.y(),pt3d.z());
                
                drawable->addPoint(pt);
                if (doColor)
                    drawable->addColor(ringColor);
                drawable->addNormal(norm);
                if (doTexCoords)
                {
                    drawable->addTexCoord(0, texCoords[jj]);
                }
            }
            
            // Add the triangles
            // Note: Should be reusing vertex indices
            if (pts.size() == 3)
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
                    Point2f midPt = drawMbr.mid();
                    Point3d centerPt = scene->getCoordAdapter()->localToDisplay(Point3d(midPt.x(),midPt.y(),0.0));
                    Point2d texScale(vecInfo->texScale.x(),vecInfo->texScale.y());
                    BasicDrawableScreenTexTweaker *texTweaker = new BasicDrawableScreenTexTweaker(centerPt,texScale);
                    drawable->addTweaker(DrawableTweakerRef(texTweaker));
                }

                drawable->setLocalMbr(drawMbr);
                if (centerValid)
                {
                    Eigen::Affine3d trans(Eigen::Translation3d(center.x(),center.y(),center.z()));
                    Matrix4d transMat = trans.matrix();
                    drawable->setMatrix(&transMat);
                }
                sceneRep->drawIDs.insert(drawable->getDrawableID());
                
                if (vecInfo->fade > 0.0)
                {
                    TimeInterval curTime = scene->getCurrentTime();
                    drawable->setFade(curTime,curTime+vecInfo->fade);
                }
                
                changeRequests.push_back(new AddDrawableReq(drawable->getDrawable()));
            }
            drawable = NULL;
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

VectorManager::VectorManager()
{
}

VectorManager::~VectorManager()
{
    for (VectorSceneRepSet::iterator it = vectorReps.begin();
         it != vectorReps.end(); ++it)
        delete *it;
    vectorReps.clear();
}

SimpleIdentity VectorManager::addVectors(ShapeSet *shapes, const VectorInfo &vecInfo, ChangeSet &changes)
{
    if (shapes->empty())
        return EmptyIdentity;
    
    VectorSceneRep *sceneRep = new VectorSceneRep();
    sceneRep->fade = vecInfo.fade;

    // No longer do anything with points in here
//    VectorPointsRef thePoints = std::dynamic_pointer_cast<VectorPoints>(*first);
//    bool linesOrPoints = (thePoints.get() ? false : true);
    
    // Look for per vector colors
    bool doColors = false;
    for (ShapeSet::iterator it = shapes->begin();it != shapes->end(); ++it)
    {
        if ((*it)->getAttrDict()->hasField("color"))
        {
            doColors = true;
            break;
        }
    }

    // Look for a geometry center.  We'll offset everything if there is one
    CoordSystemDisplayAdapter *coordAdapter = scene->getCoordAdapter();
    CoordSystem *coordSys = coordAdapter->getCoordSystem();
    Point3d center(0,0,0);
    bool centerValid = false;
    Point2d geoCenter(0,0);
    // Note: Should work for the globe, but doesn't
    if (vecInfo.centered && coordAdapter->isFlat())
    {
        // We might pass in a center
        if (vecInfo.vecCenterSet)
        {
            geoCenter.x() = vecInfo.vecCenter.x();
            geoCenter.y() = vecInfo.vecCenter.y();
            Point3d dispPt = coordAdapter->localToDisplay(coordSys->geographicToLocal(geoCenter));
            center = dispPt;
            centerValid = true;
        } else {
          // Calculate the center
          GeoMbr geoMbr;
          for (ShapeSet::iterator it = shapes->begin();it != shapes->end(); ++it)
              geoMbr.expand((*it)->calcGeoMbr());
          if (geoMbr.valid())
          {
              Point3d p0 = coordAdapter->localToDisplay(coordSys->geographicToLocal3d(geoMbr.ll()));
              Point3d p1 = coordAdapter->localToDisplay(coordSys->geographicToLocal3d(geoMbr.ur()));
              center = (p0+p1)/2.0;
              centerValid = true;
          }
        }
    }
    
    // Used to toss out drawables as we go
    // Its destructor will flush out the last drawable
    VectorDrawableBuilder drawBuild(scene,renderer,changes,sceneRep,&vecInfo,true,doColors);
    if (centerValid)
        drawBuild.setCenter(center,geoCenter);
    VectorDrawableBuilderTri drawBuildTri(scene,renderer,changes,sceneRep,&vecInfo,doColors);
    if (centerValid)
        drawBuildTri.setCenter(center,geoCenter);
        
    for (ShapeSet::iterator it = shapes->begin();
         it != shapes->end(); ++it)
    {
        VectorArealRef theAreal = std::dynamic_pointer_cast<VectorAreal>(*it);
        if (theAreal.get())
        {
//            std::string tileID = (*it)->getAttrDict()->getString("tile");
//            GeoMbr mbr = theAreal->calcGeoMbr();

            if (vecInfo.filled)
            {
                // Trianglate outside and loops
                drawBuildTri.addPoints(theAreal->loops,theAreal->getAttrDict());
            } else {
                // Work through the loops
                for (unsigned int ri=0;ri<theAreal->loops.size();ri++)
                {
                    VectorRing &ring = theAreal->loops[ri];
                    
                    // Break the edges around the globe (presumably)
                    if (vecInfo.sample > 0.0)
                    {
                        VectorRing newPts;
                        SubdivideEdges(ring, newPts, false, vecInfo.sample);
                        drawBuild.addPoints(newPts,true,theAreal->getAttrDict());
                    } else
                        drawBuild.addPoints(ring,true,theAreal->getAttrDict());
                }
            }
        } else {
            VectorLinearRef theLinear = std::dynamic_pointer_cast<VectorLinear>(*it);
            if (theLinear.get())
            {
                if (vecInfo.filled)
                {
                    // Triangulate the outside
                    drawBuildTri.addPoints(theLinear->pts,theLinear->getAttrDict());
                } else {
                    if (vecInfo.sample > 0.0)
                    {
                        VectorRing newPts;
                        SubdivideEdges(theLinear->pts, newPts, false, vecInfo.sample);
                        drawBuild.addPoints(newPts,false,theLinear->getAttrDict());
                    } else
                        drawBuild.addPoints(theLinear->pts,false,theLinear->getAttrDict());
                }
            } else {
                VectorLinear3dRef theLinear3d = std::dynamic_pointer_cast<VectorLinear3d>(*it);
                if (theLinear3d.get())
                {
                    if (vecInfo.filled)
                    {
                        // Triangulate the outside
                        drawBuildTri.addPoints(theLinear3d->pts,theLinear3d->getAttrDict());
                    } else {
                        if (vecInfo.sample > 0.0)
                        {
                            VectorRing3d newPts;
                            SubdivideEdges(theLinear3d->pts, newPts, false, vecInfo.sample);
                            drawBuild.addPoints(newPts,false,theLinear3d->getAttrDict());
                        } else
                            drawBuild.addPoints(theLinear3d->pts,false,theLinear3d->getAttrDict());
                    }
                } else {
                    VectorTrianglesRef theMesh = std::dynamic_pointer_cast<VectorTriangles>(*it);
                    if (theMesh.get())
                    {
                        if (vecInfo.filled)
                            drawBuildTri.addPoints(theMesh,theMesh->getAttrDict());
                        else {
                            for (unsigned int ti=0;ti<theMesh->tris.size();ti++)
                            {
                                VectorRing ring;
                                theMesh->getTriangle(ti, ring);
                                drawBuild.addPoints(ring,true,theMesh->getAttrDict());
                            }
                        }
                    } else {
                        // Points are... pointless for display
                    }
                }
            }
        }
    }
    
    drawBuild.flush();
    drawBuildTri.flush();
    
    SimpleIdentity vecID = sceneRep->getId();
    {
        std::lock_guard<std::mutex> guardLock(vectorLock);
        vectorReps.insert(sceneRep);
    }
    
    return vecID;
}

SimpleIdentity VectorManager::instanceVectors(SimpleIdentity vecID,const VectorInfo &vecInfo,ChangeSet &changes)
{
    SimpleIdentity newId = EmptyIdentity;
    
    std::lock_guard<std::mutex> guardLock(vectorLock);

    // Look for the representation
    VectorSceneRep dummyRep(vecID);
    VectorSceneRepSet::iterator it = vectorReps.find(&dummyRep);
    if (it != vectorReps.end())
    {
        VectorSceneRep *sceneRep = *it;
        VectorSceneRep *newSceneRep = new VectorSceneRep();
        for (SimpleIDSet::iterator idIt = sceneRep->drawIDs.begin();
             idIt != sceneRep->drawIDs.end(); ++idIt)
        {
            // Make up a BasicDrawableInstance
            BasicDrawableInstanceBuilderRef drawInst = renderer->makeBasicDrawableInstanceBuilder("VectorManager");
            drawInst->setMasterID(*idIt,BasicDrawableInstance::ReuseStyle);
            
            // Changed color
            drawInst->setColor(vecInfo.color);

            // Changed visibility
            drawInst->setVisibleRange(vecInfo.minVis, vecInfo.maxVis);
            
            // Changed line width
            drawInst->setLineWidth(vecInfo.lineWidth);
            
            // Changed draw priority
            drawInst->setDrawPriority(vecInfo.drawPriority);

            newSceneRep->instIDs.insert(drawInst->getDrawableID());
            changes.push_back(new AddDrawableReq(drawInst->getDrawable()));
        }
        
        vectorReps.insert(newSceneRep);
        newId = newSceneRep->getId();
    }
    
    return newId;
}

void VectorManager::changeVectors(SimpleIdentity vecID,const VectorInfo &vecInfo,ChangeSet &changes)
{
    std::lock_guard<std::mutex> guardLock(vectorLock);

    VectorSceneRep dummyRep(vecID);
    VectorSceneRepSet::iterator it = vectorReps.find(&dummyRep);
    
    if (it != vectorReps.end())
    {
        VectorSceneRep *sceneRep = *it;
        // Make sure we change both drawables and instances
        SimpleIDSet allIDs = sceneRep->drawIDs;
        allIDs.insert(sceneRep->instIDs.begin(),sceneRep->instIDs.end());

        for (SimpleIDSet::iterator idIt = allIDs.begin();idIt != allIDs.end(); ++idIt)
        {
            // Changed color
            changes.push_back(new ColorChangeRequest(*idIt, vecInfo.color));
            
            // Changed visibility
            changes.push_back(new VisibilityChangeRequest(*idIt, vecInfo.minVis, vecInfo.maxVis));
            
            // Changed line width
            changes.push_back(new LineWidthChangeRequest(*idIt, vecInfo.lineWidth));
            
            // Changed draw priority
            changes.push_back(new DrawPriorityChangeRequest(*idIt, vecInfo.drawPriority));
        }
    }
}

void VectorManager::removeVectors(SimpleIDSet &vecIDs,ChangeSet &changes)
{
    std::lock_guard<std::mutex> guardLock(vectorLock);

    TimeInterval curTime = scene->getCurrentTime();
    for (SimpleIDSet::iterator vit = vecIDs.begin(); vit != vecIDs.end(); ++vit)
    {
        VectorSceneRep dummyRep(*vit);
        VectorSceneRepSet::iterator it = vectorReps.find(&dummyRep);
        
        if (it != vectorReps.end())
        {
            VectorSceneRep *sceneRep = *it;
            
            SimpleIDSet allIDs = sceneRep->drawIDs;
            allIDs.insert(sceneRep->instIDs.begin(),sceneRep->instIDs.end());

            TimeInterval removeTime = 0.0;
            if (sceneRep->fade > 0.0)
            {
                for (SimpleIDSet::iterator idIt = allIDs.begin();
                     idIt != allIDs.end(); ++idIt)
                    changes.push_back(new FadeChangeRequest(*idIt, curTime, curTime+sceneRep->fade));
                
                removeTime = curTime + sceneRep->fade;
            }
            
            for (SimpleIDSet::iterator idIt = allIDs.begin();
                 idIt != allIDs.end(); ++idIt)
                changes.push_back(new RemDrawableReq(*idIt));
            vectorReps.erase(it);
            
            delete sceneRep;
        }
    }
}
    
void VectorManager::enableVectors(SimpleIDSet &vecIDs,bool enable,ChangeSet &changes)
{
    std::lock_guard<std::mutex> guardLock(vectorLock);

    for (SimpleIDSet::iterator vIt = vecIDs.begin();vIt != vecIDs.end();++vIt)
    {
        VectorSceneRep dummyRep(*vIt);
        VectorSceneRepSet::iterator it = vectorReps.find(&dummyRep);
        if (it != vectorReps.end())
        {
            VectorSceneRep *sceneRep = *it;
            
            SimpleIDSet allIDs = sceneRep->drawIDs;
            allIDs.insert(sceneRep->instIDs.begin(),sceneRep->instIDs.end());
            for (SimpleIDSet::iterator idIt = allIDs.begin(); idIt != allIDs.end(); ++idIt)
                changes.push_back(new OnOffChangeRequest(*idIt,enable));
        }
    }    
}

}
