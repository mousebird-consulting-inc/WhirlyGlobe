/*
 *  GeometryLayer.mm
 *  WhirlyGlobeLib
 *
 *  Created by Steve Gifford on 6/19/12.
 *  Copyright 2012 mousebird consulting
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

#import "GeometryLayer.h"
#import "NSDictionary+Stuff.h"
#import "UIColor+Stuff.h"

using namespace Eigen;
using namespace WhirlyKit;
using namespace WhirlyGlobe;

void GeomSceneRep::removeFromScene(std::vector<WhirlyKit::ChangeRequest *> &changeRequests)
{
    for (SimpleIDSet::iterator it = drawIDs.begin();
         it != drawIDs.end(); ++it)
        changeRequests.push_back(new RemDrawableReq(*it));
}

void GeomSceneRep::fadeOutScene(std::vector<WhirlyKit::ChangeRequest *> &changeRequests)
{
    NSTimeInterval curTime = CFAbsoluteTimeGetCurrent();
    for (SimpleIDSet::iterator it = drawIDs.begin();
         it != drawIDs.end(); ++it)
        changeRequests.push_back(new FadeChangeRequest(*it, curTime, curTime+fade));
}

// Used to pass geometry around internally
@interface GeomInfo : NSObject
{
@public
    SimpleIdentity  sceneRepId;
    NSArray         *geom;
    BOOL            enable;
    UIColor         *color;
    float           fade;
    int             drawPriority;
    SimpleIdentity  replaceId;
}

- (void)parseDict:(NSDictionary *)dict;
@end

@implementation GeomInfo

- (id)initWithGeometry:(NSArray *)inGeom desc:(NSDictionary *)dict
{
    self = [super init];
    if (self)
    {
        geom = inGeom;
        [self parseDict:dict];
    }
    
    return self;
}

- (id)initWithSceneRepId:(SimpleIdentity)inId desc:(NSDictionary *)dict
{
    if ((self = [super init]))
    {
        sceneRepId = inId;
        [self parseDict:dict];
    }
    
    return self;
}

- (void)parseDict:(NSDictionary *)dict
{
    enable = [dict boolForKey:@"enable" default:YES];
    color = [dict objectForKey:@"color" checkType:[UIColor class] default:[UIColor whiteColor]];
    fade = [dict floatForKey:@"fade" default:0.0];
    drawPriority = [dict intForKey:@"drawPriority" default:0];
}

@end

@interface WhirlyGlobeGeometry()
- (BasicDrawable *)buildDrawable;
@end

@implementation WhirlyGlobeGeometry
// Base version does nothing
- (BasicDrawable *)buildDrawable
{
    return nil;
}
@end

@implementation WhirlyGlobeGeometryRaw

@synthesize type;
@synthesize pts;
@synthesize norms;
@synthesize texCoords;
@synthesize colors;
@synthesize triangles;
@synthesize texId;

+ (WhirlyGlobeGeometryRaw *)geometryWithGeometry:(WhirlyGlobeGeometryRaw *)inGeom
{
    WhirlyGlobeGeometryRaw *rawGeom = [[WhirlyGlobeGeometryRaw alloc] init];
    rawGeom.type = inGeom.type;
    rawGeom.pts = inGeom.pts;
    rawGeom.norms = inGeom.norms;
    rawGeom.texCoords = inGeom.texCoords;
    rawGeom.colors = inGeom.colors;
    rawGeom.triangles = inGeom.triangles;
    rawGeom.texId = inGeom.texId;
    
    return rawGeom;
}

- (bool)isValid
{
    if (type != WhirlyGlobeGeometryLines && type != WhirlyGlobeGeometryTriangles)
        return false;
    int numPoints = pts.size();
    if (numPoints == 0)
        return false;
    
    if (!norms.empty() && norms.size() != numPoints)
        return false;
    if (!texCoords.empty() && texCoords.size() != numPoints)
        return false;
    if (!colors.empty() && colors.size() != numPoints)
        return false;
    if (type == WhirlyGlobeGeometryTriangles && triangles.empty())
        return false;
    if (texId != EmptyIdentity && texCoords.empty())
        return false;
    for (unsigned int ii=0;ii<triangles.size();ii++)
    {
        RawTriangle tri = triangles[ii];
        for (unsigned int jj=0;jj<3;jj++)
            if (tri.verts[jj] >= pts.size() || tri.verts[jj] < 0)
                return false;
    }
    
    return true;
}

- (void)applyTransform:(Matrix4f &)mat
{
    for (unsigned int ii=0;ii<pts.size();ii++)
    {
        Point3f &pt = pts[ii];
        Vector4f outPt = mat * Eigen::Vector4f(pt.x(),pt.y(),pt.z(),1.0);
        pt = Point3f(outPt.x()/outPt.w(),outPt.y()/outPt.w(),outPt.z()/outPt.w());
    }
    
    for (unsigned int ii=0;ii<norms.size();ii++)
    {
        Point3f &norm = norms[ii];
        Vector4f projNorm = mat * Eigen::Vector4f(norm.x(),norm.y(),norm.z(),0.0);
        norm = Point3f(projNorm.x(),projNorm.y(),projNorm.z()).normalized();
    }
}

+ (Eigen::Matrix4f)makePosition:(WhirlyKit::Point3f)pos up:(WhirlyKit::Point3f)up forward:(WhirlyKit::Point3f)forward heading:(float)ang
{
    Point3f yaxis = forward.normalized();
    Point3f xaxis = yaxis.cross(up).normalized();
    Point3f zaxis = xaxis.cross(yaxis);
    
    Matrix4f mat;
    mat(0,0) = xaxis.x();  mat(1,0) = xaxis.y();  mat(2,0) = xaxis.z();  mat(3,0) = 0.0;
    mat(0,1) = yaxis.x();  mat(1,1) = yaxis.y();  mat(2,1) = yaxis.z();  mat(3,1) = 0.0;
    mat(0,2) = zaxis.x();  mat(1,2) = zaxis.y();  mat(2,2) = zaxis.z();  mat(3,2) = 0.0;
    mat(0,3) = pos.x();  mat(1,3) = pos.y();  mat(2,3) = pos.z();  mat(3,3) = 1.0;
    
    Eigen::AngleAxisf rot(-ang,up);
    Matrix4f resMat = ((Affine3f)rot).matrix() * mat;
    
    return resMat;
}

- (void)applyPosition:(WhirlyKit::Point3f)pos up:(WhirlyKit::Point3f)up forward:(WhirlyKit::Point3f)forward heading:(float)ang;
{
    Matrix4f theMat = [WhirlyGlobeGeometryRaw makePosition:pos up:up forward:forward heading:ang];
    [self applyTransform:theMat];
}

- (BasicDrawable *)buildDrawable
{
    if (![self isValid])
        return nil;
    
    BasicDrawable *draw = new BasicDrawable();
    switch (type)
    {
        case WhirlyGlobeGeometryLines:
            draw->setType(GL_LINES);
            break;
        case WhirlyGlobeGeometryTriangles:
            draw->setType(GL_TRIANGLES);
            break;
        default:
            break;
    }
    draw->setTexId(texId);
    for (unsigned int ii=0;ii<pts.size();ii++)
    {
        draw->addPoint(pts[ii]);
        if (!norms.empty())
            draw->addNormal(norms[ii]);
        if (texId != EmptyIdentity)
            draw->addTexCoord(texCoords[ii]);
        if (!colors.empty())
            draw->addColor(colors[ii]);
    }
    for (unsigned int ii=0;ii<triangles.size();ii++)
    {
        RawTriangle tri = triangles[ii];
        draw->addTriangle(BasicDrawable::Triangle(tri.verts[0],tri.verts[1],tri.verts[2]));
    }
    
    return draw;
}

- (void)makeDrawables:(std::vector<WhirlyKit::Drawable *> &)drawables
{
    BasicDrawable *theDraw = [self buildDrawable];
    if (theDraw)
        drawables.push_back(theDraw);
}

@end

@implementation WhirlyGlobeGeometrySet

@synthesize textures;
@synthesize geom;

static unsigned short CacheFileVersion = 1;

- (id)initWithFile:(NSString *)fullPath
{
    self = [super init];
    if (!self)
        return nil;
    
    FILE *fp = fopen([fullPath cStringUsingEncoding:NSASCIIStringEncoding],"r");
    if (!fp)
        return nil;
    
    try
    {
        unsigned short fileVersion;
        if (fread(&fileVersion, sizeof(unsigned short), 1, fp) != 1)
            throw 1;
        if (fileVersion != CacheFileVersion)
            throw 1;
        
        // Textures
        unsigned int numTex;
        if (fread(&numTex, sizeof(unsigned int), 1, fp) != 1)
            throw 1;
        for (unsigned int ii=0;ii<numTex;ii++)
        {
//            Texture *tex = new Texture(fp);
//            textures.push_back(tex);
        }
        
        // Raw geometry
        unsigned int numGeom;
        if (fread(&numGeom, sizeof(unsigned int), 1, fp) != 1)
            throw 1;
        geom = [NSMutableArray array];
        for (unsigned int ii=0;ii<numGeom;ii++)
        {
            WhirlyGlobeGeometryRaw *rawGeom = [[WhirlyGlobeGeometryRaw alloc] init];
            
            unsigned int type;
            unsigned int texId;
            std::vector<WhirlyKit::Point3f> &pts = rawGeom.pts;
            std::vector<WhirlyKit::Point3f> &norms = rawGeom.norms;
            std::vector<WhirlyKit::TexCoord> &texCoords = rawGeom.texCoords;
            std::vector<WhirlyKit::RGBAColor> &colors = rawGeom.colors;
            std::vector<WhirlyGlobe::RawTriangle> &triangles = rawGeom.triangles;
            
            if (fread(&type, sizeof(unsigned int), 1, fp) != 1 ||
                fread(&texId, sizeof(unsigned int), 1, fp) != 1)
                throw 1;
            
            unsigned int numPts;
            if (fread(&numPts, sizeof(unsigned int), 1, fp) != 1)
                throw 1;
            pts.resize(numPts);
            for (unsigned int ii=0;ii<pts.size();ii++)
            {
                float coords[3];
                if (fread(coords,sizeof(float),3,fp) != 3)
                    throw 1;
                Point3f &pt = pts[ii];
                pt.x() = coords[0];  pt.y() = coords[1];  pt.z() = coords[2];
            }
            
            unsigned int numNorms;
            if (fread(&numNorms, sizeof(unsigned int), 1, fp) != 1)
                throw 1;
            norms.resize(numNorms);
            for (unsigned int ii=0;ii<norms.size();ii++)
            {
                float coords[3];
                if (fread(coords,sizeof(float),3,fp) != 3)
                    throw 1;
                Point3f &nm = norms[ii];
                nm.x() = coords[0];  nm.y() = coords[1];  nm.z() = coords[2];
            }
            
            unsigned int numTexCoord;
            if (fread(&numTexCoord, sizeof(unsigned int), 1, fp) != 1)
                throw 1;
            texCoords.resize(numTexCoord);
            for (unsigned int ii=0;ii<texCoords.size();ii++)
            {
                float coords[2];
                if (fread(coords,sizeof(float),2,fp) != 3)
                    throw 1;
                TexCoord &tc = texCoords[ii];
                tc.u() = coords[0];  tc.v() = coords[1];
            }
            
            unsigned int numColors = colors.size();
            if (fread(&numColors, sizeof(unsigned int), 1, fp) != 1)
                throw 1;
            colors.resize(numColors);
            for (unsigned int ii=0;ii<colors.size();ii++)
            {
                unsigned char vals[4];
                if (fread(vals,sizeof(unsigned char),4,fp) != 4)
                    throw 1;
                RGBAColor &c = colors[ii];
                c.r = vals[0];  c.g = vals[1];  c.b = vals[2];  c.a = vals[3];
            }
            
            unsigned int numTri = triangles.size();
            if (fread(&numTri, sizeof(unsigned int ), 1, fp) != 1)
                throw 1;
            triangles.resize(numTri);
            for (unsigned int ii=0;ii<triangles.size();ii++)
            {
                RawTriangle &tri = triangles[ii];
                if (fread(tri.verts,sizeof(unsigned int),3,fp) != 3)
                    throw 1;
            }            
            
            [geom addObject:rawGeom];
        }
    }
    catch (...)
    {
        fclose(fp);
        return nil;
    }
    
    return self;
}

- (bool)writeToFile:(NSString *)fullPath
{
    FILE *fp = fopen([fullPath cStringUsingEncoding:NSASCIIStringEncoding],"w+");
    if (!fp)
        return false;
    
    try {
        // File version number
        unsigned short fileVersion = CacheFileVersion;
        if (fwrite(&fileVersion, sizeof(unsigned short), 1, fp) != 1)
            throw 1;
        
        // Textures first
        unsigned int numTex = textures.size();
        if (fwrite(&numTex, sizeof(unsigned int), 1, fp) != 1)
            throw 1;
        for (unsigned int ii=0;ii<textures.size();ii++)
        {
//            Texture *tex = textures[ii];
//            if (!tex->writeToFile(fp))
//                throw 1;
        }
        
        // Raw geometry
        unsigned int numGeom = [geom count];
        if (fwrite(&numGeom, sizeof(unsigned int), 1, fp) != 1)
            throw 1;
        for (WhirlyGlobeGeometryRaw *rawGeom in geom)
        {
            unsigned int type = rawGeom.type;
            unsigned int texId = rawGeom.texId;
            std::vector<WhirlyKit::Point3f> &pts = rawGeom.pts;
            std::vector<WhirlyKit::Point3f> &norms = rawGeom.norms;
            std::vector<WhirlyKit::TexCoord> &texCoords = rawGeom.texCoords;
            std::vector<WhirlyKit::RGBAColor> &colors = rawGeom.colors;
            std::vector<WhirlyGlobe::RawTriangle> &triangles = rawGeom.triangles;
            
            if (fwrite(&type, sizeof(unsigned int), 1, fp) != 1 ||
                fwrite(&texId, sizeof(unsigned int), 1, fp) != 1)
                throw 1;
            
            unsigned int numPts = pts.size();
            if (fwrite(&numPts, sizeof(unsigned int), 1, fp) != 1)
                throw 1;
            for (unsigned int ii=0;ii<pts.size();ii++)
            {
                Point3f &pt = pts[ii];
                float coords[3];
                coords[0] = pt.x();  coords[1] = pt.y();  coords[2] = pt.z();
                if (fwrite(coords,sizeof(float),3,fp) != 3)
                    throw 1;
            }
            
            unsigned int numNorms = norms.size();
            if (fwrite(&numNorms, sizeof(unsigned int), 1, fp) != 1)
                throw 1;
            for (unsigned int ii=0;ii<norms.size();ii++)
            {
                Point3f &nm = norms[ii];
                float coords[3];
                coords[0] = nm.x();  coords[1] = nm.y();  coords[2] = nm.z();
                if (fwrite(coords,sizeof(float),3,fp) != 3)
                    throw 1;
            }

            unsigned int numTexCoord = texCoords.size();
            if (fwrite(&numTexCoord, sizeof(unsigned int), 1, fp) != 1)
                throw 1;
            for (unsigned int ii=0;ii<texCoords.size();ii++)
            {
                TexCoord &tc = texCoords[ii];
                float coords[2];
                coords[0] = tc.u();  coords[1] = tc.v();
                if (fwrite(coords,sizeof(float),2,fp) != 3)
                    throw 1;
            }

            unsigned int numColors = colors.size();
            if (fwrite(&numColors, sizeof(unsigned int), 1, fp) != 1)
                throw 1;
            for (unsigned int ii=0;ii<colors.size();ii++)
            {
                RGBAColor &c = colors[ii];
                unsigned char vals[4];
                vals[0] = c.r;  vals[1] = c.g;  vals[2] = c.b;  vals[3] = c.a;
                if (fwrite(vals,sizeof(unsigned char),4,fp) != 4)
                    throw 1;
            }
            
            unsigned int numTri = triangles.size();
            if (fwrite(&numTri, sizeof(unsigned int ), 1, fp) != 1)
                throw 1;
            for (unsigned int ii=0;ii<triangles.size();ii++)
            {
                RawTriangle &tri = triangles[ii];
                if (fwrite(tri.verts,sizeof(unsigned int),3,fp) != 3)
                    throw 1;
            }
        }

    } catch (...)
    {
        fclose(fp);
        return false;
    }
    
    return true;
}

@end

@implementation WhirlyGlobeGeometryLayer

- (void)clear
{
    geomReps.clear();    
    scene = NULL;
}

- (void)dealloc
{
    [self clear];
}

- (void)startWithThread:(WhirlyKitLayerThread *)inLayerThread scene:(WhirlyKit::Scene *)inScene
{
	scene = inScene;
    layerThread = inLayerThread;
}

- (void)shutdown
{
    [NSObject cancelPreviousPerformRequestsWithTarget:self];
    std::vector<ChangeRequest *> changeRequests;
    
    for (GeomSceneRepSet::iterator it = geomReps.begin();
         it != geomReps.end(); ++it)
        (*it)->removeFromScene(changeRequests);
        
    scene->addChangeRequests(changeRequests);
    
    [self clear];
}

// Actually create the geometry
// In the layer thread here
- (void)runAddGeometry:(GeomInfo *)geomInfo
{
    std::vector<ChangeRequest *> changeRequests;
    GeomSceneRepRef geomRep(new GeomSceneRep());
    geomRep->setId(geomInfo->sceneRepId);
    
    if (geomInfo->replaceId != EmptyIdentity)
    {
        GeomSceneRepRef dummyRep(new GeomSceneRep());
        dummyRep->setId(geomInfo->replaceId);
        GeomSceneRepSet::iterator it = geomReps.find(dummyRep);
        if (it != geomReps.end())
        {
            geomRep->removeFromScene(changeRequests);
            geomReps.erase(geomRep);
        }
    }
    
    // Work through the array of geometry, building as we go
    for (WhirlyGlobeGeometry *geom in geomInfo->geom)
    {
        BasicDrawable *draw = [geom buildDrawable];
        if (draw)
        {
            draw->setDrawPriority(geomInfo->drawPriority);
//            draw->setAlpha(true);
            changeRequests.push_back(new AddDrawableReq(draw));
            geomRep->drawIDs.insert(draw->getId());
        }
    }
    
    scene->addChangeRequests(changeRequests);
    
    geomReps.insert(geomRep);
}

// Remove the representation
// In the layer thread here
- (void)runRemoveGeometry:(NSNumber *)num
{
    GeomSceneRepRef dummyRep(new GeomSceneRep());
    dummyRep->setId([num intValue]);
    GeomSceneRepSet::iterator it = geomReps.find(dummyRep);
    if (it != geomReps.end())
    {
        GeomSceneRepRef geomRep = *it;
        std::vector<WhirlyKit::ChangeRequest *> changeRequests;
        if (geomRep->fade > 0.0)
        {
            geomRep->fadeOutScene(changeRequests);
            
            // Reset the fade and remove it later
            [self performSelector:@selector(runRemoveGeometry:) withObject:num afterDelay:geomRep->fade];
            geomRep->fade = 0.0;
        } else {
            // Just remove it
            geomRep->removeFromScene(changeRequests);
            geomReps.erase(geomRep);
        }
        
        scene->addChangeRequests(changeRequests);
    }
}

/// Add a sphere at the given location
- (WhirlyKit::SimpleIdentity)addGeometry:(WhirlyGlobeGeometry *)geom desc:(NSDictionary *)desc
{
    NSArray *array = [NSArray arrayWithObject:geom];
    return [self addGeometryArray:array desc:desc];
}

/// Add a group of geometry together
- (WhirlyKit::SimpleIdentity)addGeometryArray:(NSArray *)geom desc:(NSDictionary *)desc
{
    if (!layerThread || !scene)
    {
        NSLog(@"WhirlyGlobe Geometry layer has not been initialized, yet you're calling addGeometry.  Dropping data on floor.");
        return EmptyIdentity;
    }

    GeomInfo *geomInfo = [[GeomInfo alloc] initWithGeometry:geom desc:desc];
    geomInfo->sceneRepId = Identifiable::genId();
    
    if (!layerThread || ([NSThread currentThread] == layerThread))
        [self runAddGeometry:geomInfo];
    else
        [self performSelector:@selector(runAddGeometry:) onThread:layerThread withObject:geomInfo waitUntilDone:NO];
    
    return geomInfo->sceneRepId;
}

/// Replace one group of geometry with another
- (WhirlyKit::SimpleIdentity)replaceGeometry:(WhirlyKit::SimpleIdentity)geomID withGeometry:(WhirlyGlobeGeometry *)geom desc:(NSDictionary *)desc
{
    NSArray *array = [NSArray arrayWithObject:geom];
    return [self replaceGeometry:geomID withGeometryArray:array desc:desc];
}

/// Replace one group of geometry with a whole array
- (WhirlyKit::SimpleIdentity)replaceGeometry:(WhirlyKit::SimpleIdentity)geomID withGeometryArray:(NSArray *)geom desc:(NSDictionary *)desc
{
    if (!layerThread || !scene)
    {
        NSLog(@"WhirlyGlobe Geometry layer has not been initialized, yet you're calling addGeometry.  Dropping data on floor.");
        return EmptyIdentity;
    }
    
    GeomInfo *geomInfo = [[GeomInfo alloc] initWithGeometry:geom desc:desc];
    geomInfo->replaceId = geomID;
    geomInfo->sceneRepId = Identifiable::genId();
    
    if (!layerThread || ([NSThread currentThread] == layerThread))
        [self runAddGeometry:geomInfo];
    else
        [self performSelector:@selector(runAddGeometry:) onThread:layerThread withObject:geomInfo waitUntilDone:NO];
    
    return geomInfo->sceneRepId;
}

/// Remove an entire set of geometry at once by its ID
- (void)removeGeometry:(WhirlyKit::SimpleIdentity)geomID
{
    if (!layerThread || ([NSThread currentThread] == layerThread))
        [self runRemoveGeometry:[NSNumber numberWithInt:geomID]];
    else
        [self performSelector:@selector(runRemoveGeometry:) onThread:layerThread withObject:[NSNumber numberWithInt:geomID] waitUntilDone:NO];
}

@end
