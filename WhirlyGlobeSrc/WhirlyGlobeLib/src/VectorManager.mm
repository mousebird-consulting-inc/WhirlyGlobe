/*
 *  VectorManager.mm
 *  WhirlyGlobeLib
 *
 *  Created by Steve Gifford on 1/26/11.
 *  Copyright 2011-2015 mousebird consulting
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

#import "VectorManager.h"
#import "WhirlyGeometry.h"
#import "NSDictionary+Stuff.h"
#import "UIColor+Stuff.h"
#import "Tesselator.h"
#import "GridClipper.h"
#import "BaseInfo.h"

using namespace Eigen;
using namespace WhirlyKit;

typedef enum {TextureProjectionNone,TextureProjectionTanPlane,TextureProjectionScreen} TextureProjections;

// Used to describe the drawable we'll construct for a given vector
@interface WhirlyKitVectorInfo : WhirlyKitBaseInfo
{
@public
    // For creation request, the shapes
    ShapeSet                    shapes;
    float                       drawOffset;
    BOOL                        filled;
    float                       sample;
    SimpleIdentity              texId;
    Point2f                     texScale;
    float                       subdivEps;
    BOOL                        gridSubdiv;
    TextureProjections          texProj;
}

@property (nonatomic) UIColor *color;
@property (nonatomic,assign) float lineWidth;

- (void)parseDict:(NSDictionary *)dict;

@end

@implementation WhirlyKitVectorInfo

- (id)initWithShapes:(ShapeSet *)inShapes desc:(NSDictionary *)desc
{
    if ((self = [super initWithDesc:desc]))
    {
        if (inShapes)
            shapes = *inShapes;
        [self parseDict:desc];
    }
    
    return self;
}

- (id)initWithDesc:(NSDictionary *)dict
{
    if ((self = [super initWithDesc:dict]))
    {
        [self parseDict:dict];
    }
    
    return self;
}

- (void)parseDict:(NSDictionary *)dict
{
    drawOffset = [dict floatForKey:@"drawOffset" default:0];
    self.color = [dict objectForKey:@"color" checkType:[UIColor class] default:[UIColor whiteColor]];
    _lineWidth = [dict floatForKey:@"width" default:1.0];
    filled = [dict boolForKey:@"filled" default:false];
    sample = [dict floatForKey:@"sample" default:false];
    texId = [dict intForKey:@"texture" default:EmptyIdentity];
    texScale.x() = [dict floatForKey:@"texscalex" default:1.0];
    texScale.y() = [dict floatForKey:@"texscaley" default:1.0];
    subdivEps = [dict floatForKey:@"subdivisionepsilon" default:0.0];
    NSString *subdivType = [dict stringForKey:@"subdivisiontype" default:nil];
    gridSubdiv = [subdivType isEqualToString:@"grid"];
    NSString *texProjStr = [dict stringForKey:@"texprojection" default:nil];
    texProj = TextureProjectionNone;
    if ([texProjStr isEqualToString:@"texprojectiontanplane"])
        texProj = TextureProjectionTanPlane;
    else if ([texProjStr isEqualToString:@"texprojectionscreen"])
        texProj = TextureProjectionScreen;
}

@end

namespace WhirlyKit
{
    
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
    VectorDrawableBuilder(Scene *scene,ChangeSet &changeRequests,VectorSceneRep *sceneRep,
                          WhirlyKitVectorInfo *vecInfo,bool linesOrPoints,bool doColor)
    : changeRequests(changeRequests), scene(scene), sceneRep(sceneRep), vecInfo(vecInfo), drawable(NULL), doColor(doColor), centerValid(false), center(0,0,0), geoCenter(0,0)
    {
        primType = (linesOrPoints ? GL_LINES : GL_POINTS);
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
    
    void addPoints(VectorRing &pts,bool closed,NSDictionary *attrs)
    {
        CoordSystemDisplayAdapter *coordAdapter = scene->getCoordAdapter();
        RGBAColor baseColor = [vecInfo.color asRGBAColor];
        UIColor *ringColor = attrs[@"color"];
        if ([ringColor isKindOfClass:[UIColor class]])
            baseColor = [ringColor asRGBAColor];
        
        // Decide if we'll appending to an existing drawable or
        //  create a new one
        int ptCount = (int)(2*(pts.size()+1));
        if (!drawable || (drawable->getNumPoints()+ptCount > MaxDrawablePoints))
        {
            // We're done with it, toss it to the scene
            if (drawable)
                flush();
            
            drawable = new BasicDrawable("Vector Layer");
            drawMbr.reset();
            drawable->setType(primType);
            // Adjust according to the vector info
            [vecInfo setupBasicDrawable:drawable];
            drawable->setColor(baseColor);
            drawable->setLineWidth(vecInfo.lineWidth);
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
            if (primType == GL_POINTS)
            {
                drawable->addPoint(pt);
                if (doColor)
                    drawable->addColor(baseColor);
                drawable->addNormal(norm);
            } else {
                if (jj > 0)
                {
                    drawable->addPoint(prevPt);
                    drawable->addPoint(pt);
                    if (doColor)
                    {
                        drawable->addColor(baseColor);
                        drawable->addColor(baseColor);
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
        if (closed && primType == GL_LINES)
        {
            drawable->addPoint(prevPt);
            drawable->addPoint(firstPt);
            if (doColor)
            {
                drawable->addColor(baseColor);
                drawable->addColor(baseColor);
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
                sceneRep->drawIDs.insert(drawable->getId());
                if (centerValid)
                {
                    Eigen::Affine3d trans(Eigen::Translation3d(center.x(),center.y(),center.z()));
                    Matrix4d transMat = trans.matrix();
                    drawable->setMatrix(&transMat);
                }
                
                if (vecInfo.fade > 0.0)
                {
                    NSTimeInterval curTime = CFAbsoluteTimeGetCurrent();
                    drawable->setFade(curTime,curTime+vecInfo.fade);
                }
                changeRequests.push_back(new AddDrawableReq(drawable));
            } else
                delete drawable;
            drawable = NULL;
        }
    }
    
protected:
    bool doColor;
    Scene *scene;
    ChangeSet &changeRequests;
    VectorSceneRep *sceneRep;
    Mbr drawMbr;
    BasicDrawable *drawable;
    WhirlyKitVectorInfo *vecInfo;
    Point3d center;
    Point2d geoCenter;
    bool applyCenter;
    bool centerValid;
    GLenum primType;
};

/* Drawable Builder (Triangle version)
 Used to construct drawables with multiple shapes in them.
 Eventually, we'll move this out to be a more generic object.
 */
class VectorDrawableBuilderTri
{
public:
    VectorDrawableBuilderTri(Scene *scene,ChangeSet &changeRequests,VectorSceneRep *sceneRep,
                             WhirlyKitVectorInfo *vecInfo,bool doColor)
    : changeRequests(changeRequests), scene(scene), sceneRep(sceneRep), vecInfo(vecInfo), drawable(NULL), doColor(doColor), centerValid(false), center(0,0,0), geoCenter(0,0)
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
    void addPoints(VectorRing &ring,NSDictionary *attrs)
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
    
    // If it's a mesh, we're assuming it's been fully processed (triangulated, chopped, and so on)
    void addPoints(VectorTrianglesRef mesh,NSDictionary *attrs)
    {
        RGBAColor baseColor = [vecInfo.color asRGBAColor];
        UIColor *ringColor = attrs[@"color"];
        if ([ringColor isKindOfClass:[UIColor class]])
            baseColor = [ringColor asRGBAColor];

        CoordSystemDisplayAdapter *coordAdapter = scene->getCoordAdapter();
        Point2f centroid(0,0);
        if (attrs[@"veccenterx"] && attrs[@"veccentery"])
        {
            centroid.x() = [attrs[@"veccenterx"] floatValue];
            centroid.y() = [attrs[@"veccentery"] floatValue];
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
                
                drawable = new BasicDrawable("Vector Layer");
                drawMbr.reset();
                drawable->setType(GL_TRIANGLES);
                // Adjust according to the vector info
                [vecInfo setupBasicDrawable:drawable];
                drawable->setDrawOffset(vecInfo->drawOffset);
                drawable->setColor([vecInfo.color asRGBAColor]);
                if (vecInfo->texId != EmptyIdentity)
                    drawable->setTexId(0, vecInfo->texId);
                if (vecInfo.programID != EmptyIdentity)
                    drawable->setProgram(vecInfo.programID);
                //                drawable->setForceZBufferOn(true);
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
                            texCoord.x() = comp.x();
                            texCoord.y() = comp.y();
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
                // Note: Should make sure that's true here
                int minS = floorf(minCoord.x());
                int minT = floorf(minCoord.y());
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
                    drawable->addColor(baseColor);
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
                sceneRep->drawIDs.insert(drawable->getId());
                
                if (vecInfo.fade > 0.0)
                {
                    NSTimeInterval curTime = CFAbsoluteTimeGetCurrent();
                    drawable->setFade(curTime,curTime+vecInfo.fade);
                }
                
                changeRequests.push_back(new AddDrawableReq(drawable));
            } else
                delete drawable;
            drawable = NULL;
        }
    }
    
protected:
    bool doColor;
    Scene *scene;
    ChangeSet &changeRequests;
    VectorSceneRep *sceneRep;
    Mbr drawMbr;
    Point3d center;
    Point2d geoCenter;
    bool centerValid;
    BasicDrawable *drawable;
    WhirlyKitVectorInfo *vecInfo;
};

VectorManager::VectorManager()
{
    canary = [[NSObject alloc] init];
    pthread_mutex_init(&vectorLock, NULL);
}

VectorManager::~VectorManager()
{
    for (VectorSceneRepSet::iterator it = vectorReps.begin();
         it != vectorReps.end(); ++it)
        delete *it;
    vectorReps.clear();

    pthread_mutex_destroy(&vectorLock);
}

SimpleIdentity VectorManager::addVectors(ShapeSet *shapes, NSDictionary *desc, ChangeSet &changes)
{
    WhirlyKitVectorInfo *vecInfo = [[WhirlyKitVectorInfo alloc] initWithShapes:shapes desc:desc];

    // All the shape types should be the same
    ShapeSet::iterator first = vecInfo->shapes.begin();
    if (first == vecInfo->shapes.end())
        return EmptyIdentity;
    
    VectorSceneRep *sceneRep = new VectorSceneRep();
    sceneRep->fade = vecInfo.fade;

    // No longer do anything with points in here
//    VectorPointsRef thePoints = boost::dynamic_pointer_cast<VectorPoints>(*first);
//    bool linesOrPoints = (thePoints.get() ? false : true);
    
    // Look for per vector colors
    bool doColors = false;
    for (ShapeSet::iterator it = vecInfo->shapes.begin();
         it != vecInfo->shapes.end(); ++it)
    {
        if (((*it)->getAttrDict())[@"color"])
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
    if (desc[@"centered"] && [desc[@"centered"] boolValue])
    {
        // We might pass in a center
        if (desc[@"veccenterx"] && desc[@"veccentery"])
        {
            geoCenter.x() = [desc[@"veccenterx"] doubleValue];
            geoCenter.y() = [desc[@"veccentery"] doubleValue];
            Point3d dispPt = coordAdapter->localToDisplay(coordSys->geographicToLocal(geoCenter));
            center = dispPt;
            centerValid = true;
        } else {
            // Calculate the center
            GeoMbr geoMbr;
            for (ShapeSet::iterator it = vecInfo->shapes.begin();
                 it != vecInfo->shapes.end(); ++it)
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
    VectorDrawableBuilder drawBuild(scene,changes,sceneRep,vecInfo,true,doColors);
    if (centerValid)
        drawBuild.setCenter(center,geoCenter);
    VectorDrawableBuilderTri drawBuildTri(scene,changes,sceneRep,vecInfo,doColors);
    if (centerValid)
        drawBuildTri.setCenter(center,geoCenter);
    
    for (ShapeSet::iterator it = vecInfo->shapes.begin();
         it != vecInfo->shapes.end(); ++it)
    {
        VectorArealRef theAreal = boost::dynamic_pointer_cast<VectorAreal>(*it);
        if (theAreal.get())
        {
            if (vecInfo->filled)
            {
                // Triangulate the outside
                drawBuildTri.addPoints(theAreal->loops[0],theAreal->getAttrDict());
            } else {
                // Work through the loops
                for (unsigned int ri=0;ri<theAreal->loops.size();ri++)
                {
                    VectorRing &ring = theAreal->loops[ri];
                    
                    // Break the edges around the globe (presumably)
                    if (vecInfo->sample > 0.0)
                    {
                        VectorRing newPts;
                        SubdivideEdges(ring, newPts, false, vecInfo->sample);
                        drawBuild.addPoints(newPts,true,theAreal->getAttrDict());
                    } else
                        drawBuild.addPoints(ring,true,theAreal->getAttrDict());
                }
            }
        } else {
            VectorLinearRef theLinear = boost::dynamic_pointer_cast<VectorLinear>(*it);
            if (theLinear.get())
            {
                if (vecInfo->filled)
                {
                    // Triangulate the outside
                    drawBuildTri.addPoints(theLinear->pts,theLinear->getAttrDict());
                } else {
                    if (vecInfo->sample > 0.0)
                    {
                        VectorRing newPts;
                        SubdivideEdges(theLinear->pts, newPts, false, vecInfo->sample);
                        drawBuild.addPoints(newPts,false,theLinear->getAttrDict());
                    } else
                        drawBuild.addPoints(theLinear->pts,false,theLinear->getAttrDict());
                }
            } else {
                VectorTrianglesRef theMesh = boost::dynamic_pointer_cast<VectorTriangles>(*it);
                if (theMesh.get())
                {
                    if (vecInfo->filled)
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
                    // Note: Points are.. pointless
                    //                    VectorPointsRef thePoints = boost::dynamic_pointer_cast<VectorPoints>(*it);
                    //                    if (thePoints.get())
                    //                    {
                    //                        drawBuild.addPoints(thePoints->pts,false);
                    //                    }
                }
            }
        }
    }
    
    drawBuild.flush();
    drawBuildTri.flush();
    
    SimpleIdentity vecID = sceneRep->getId();
    pthread_mutex_lock(&vectorLock);
    vectorReps.insert(sceneRep);
    pthread_mutex_unlock(&vectorLock);
    
    return vecID;
}
    
SimpleIdentity VectorManager::instanceVectors(SimpleIdentity vecID,NSDictionary *desc,ChangeSet &changes)
{
    SimpleIdentity newId = EmptyIdentity;
    
    pthread_mutex_lock(&vectorLock);
    
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
            BasicDrawableInstance *drawInst = new BasicDrawableInstance("VectorManager",*idIt,BasicDrawableInstance::ReuseStyle);
            
            // Changed color
            if ([desc objectForKey:@"color"]) {
                RGBAColor newColor = [[desc objectForKey:@"color" checkType:[UIColor class] default:[UIColor whiteColor]] asRGBAColor];
                drawInst->setColor(newColor);
            }
            
            // Changed visibility
            if ([desc objectForKey:@"minVis"] || [desc objectForKey:@"maxVis"]) {
                float minVis = [desc floatForKey:@"minVis" default:DrawVisibleInvalid];
                float maxVis = [desc floatForKey:@"maxVis" default:DrawVisibleInvalid];
                drawInst->setVisibleRange(minVis, maxVis);
            }
            
            // Changed line width
            if ([desc objectForKey:@"width"]) {
                float lineWidth = [desc floatForKey:@"width" default:1.0];
                drawInst->setLineWidth(lineWidth);
            }
            
            // Changed draw priority
            if ([desc objectForKey:@"drawPriority"] || [desc objectForKey:@"priority"]) {
                int priority = [desc intForKey:@"drawPriority" default:0];
                // This looks like an old bug
                priority = [desc intForKey:@"priority" default:priority];
                drawInst->setDrawPriority(priority);
            }

            // Note: Should set fade
            newSceneRep->instIDs.insert(drawInst->getId());
            changes.push_back(new AddDrawableReq(drawInst));
        }
        
        vectorReps.insert(newSceneRep);
        newId = newSceneRep->getId();
    }

    pthread_mutex_unlock(&vectorLock);
    
    return newId;
}

void VectorManager::changeVectors(SimpleIdentity vecID,NSDictionary *desc,ChangeSet &changes)
{
    pthread_mutex_lock(&vectorLock);
    
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
            // Turned it on or off
            if ([desc objectForKey:@"enable"])
                changes.push_back(new OnOffChangeRequest(*idIt, [desc boolForKey:@"enable" default:YES]));
            
            // Changed color
            if ([desc objectForKey:@"color"]) {
                RGBAColor newColor = [[desc objectForKey:@"color" checkType:[UIColor class] default:[UIColor whiteColor]] asRGBAColor];
                changes.push_back(new ColorChangeRequest(*idIt, newColor));
            }
            
            // Changed visibility
            if ([desc objectForKey:@"minVis"] || [desc objectForKey:@"maxVis"]) {
                float minVis = [desc floatForKey:@"minVis" default:DrawVisibleInvalid];
                float maxVis = [desc floatForKey:@"maxVis" default:DrawVisibleInvalid];
                changes.push_back(new VisibilityChangeRequest(*idIt, minVis, maxVis));
            }
            
            // Changed line width
            if ([desc objectForKey:@"width"]) {
                float lineWidth = [desc floatForKey:@"width" default:1.0];
                changes.push_back(new LineWidthChangeRequest(*idIt, lineWidth));
            }
            
            // Changed draw priority
            if ([desc objectForKey:@"drawPriority"] || [desc objectForKey:@"priority"]) {
                int priority = [desc intForKey:@"drawPriority" default:0];
                // This looks like an old bug
                priority = [desc intForKey:@"priority" default:priority];
                changes.push_back(new DrawPriorityChangeRequest(*idIt, priority));
            }
        }
    }
    
    pthread_mutex_unlock(&vectorLock);
}

void VectorManager::removeVectors(SimpleIDSet &vecIDs,ChangeSet &changes)
{
    pthread_mutex_lock(&vectorLock);
    
    for (SimpleIDSet::iterator vit = vecIDs.begin(); vit != vecIDs.end(); ++vit)
    {
        VectorSceneRep dummyRep(*vit);
        VectorSceneRepSet::iterator it = vectorReps.find(&dummyRep);
        
        NSTimeInterval curTime = CFAbsoluteTimeGetCurrent();
        if (it != vectorReps.end())
        {
            VectorSceneRep *sceneRep = *it;
            
            SimpleIDSet allIDs = sceneRep->drawIDs;
            allIDs.insert(sceneRep->instIDs.begin(),sceneRep->instIDs.end());

            if (sceneRep->fade > 0.0)
            {
                for (SimpleIDSet::iterator idIt = allIDs.begin();
                     idIt != allIDs.end(); ++idIt)
                    changes.push_back(new FadeChangeRequest(*idIt, curTime, curTime+sceneRep->fade));
                
                __block NSObject * __weak thisCanary = canary;
                
                dispatch_after(dispatch_time(DISPATCH_TIME_NOW, sceneRep->fade * NSEC_PER_SEC),
                               scene->getDispatchQueue(),
                               ^{
                                   if (thisCanary)
                                   {
                                       SimpleIDSet theIDs;
                                       theIDs.insert(sceneRep->getId());
                                       ChangeSet delChanges;
                                       removeVectors(theIDs, delChanges);
                                       scene->addChangeRequests(delChanges);
                                   }
                               }
                               );
                sceneRep->fade = 0.0;
            } else {
                for (SimpleIDSet::iterator idIt = allIDs.begin();
                     idIt != allIDs.end(); ++idIt)
                    changes.push_back(new RemDrawableReq(*idIt));
                vectorReps.erase(it);
                
                delete sceneRep;
            }
        }
    }
    
    pthread_mutex_unlock(&vectorLock);
}
    
void VectorManager::enableVectors(SimpleIDSet &vecIDs,bool enable,ChangeSet &changes)
{
    pthread_mutex_lock(&vectorLock);
    
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
    
    pthread_mutex_unlock(&vectorLock);    
}

}
