/*
 *  GeometryManager.mm
 *  WhirlyGlobeLib
 *
 *  Created by Steve Gifford on 11/25/15.
 *  Copyright 2012-2015 mousebird consulting
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

#import "GeometryManager.h"
#import "SelectionManager.h"
#import "BaseInfo.h"
#import "NSDictionary+Stuff.h"
#import "UIColor+Stuff.h"
#import "BasicDrawableInstance.h"

using namespace Eigen;
using namespace WhirlyKit;

typedef enum {GeometryBBoxSingle,GeometryBBoxTriangle,GeometryBBoxNone} GeometryBoundingBox;

// Used to pass geometry around internally
@interface WhirlyKitGeomInfo : WhirlyKitBaseInfo
@property (nonatomic) UIColor *color;
@property (nonatomic,assign) int boundingBox;
@property (nonatomic) float pointSize;
@property (nonatomic) bool zBufferRead;
@property (nonatomic) bool zBufferWrite;
- (id)initWithDesc:(NSDictionary *)desc;
@end

@implementation WhirlyKitGeomInfo

- (id)initWithDesc:(NSDictionary *)desc
{
    self = [super initWithDesc:desc];
    if (!self)
        return nil;
    
    [self parseDict:desc];
    
    return self;
}

- (void)parseDict:(NSDictionary *)dict
{
    _color = [dict objectForKey:@"color" checkType:[UIColor class] default:[UIColor whiteColor]];
    _boundingBox = [dict enumForKey:@"boundingbox" values:@[@"single",@"triangle",@"none"] default:GeometryBBoxSingle];
    _pointSize = [dict floatForKey:@"pointSize" default:4.0];
    _zBufferRead = [dict floatForKey:@"zbufferread" default:true];
    _zBufferWrite = [dict floatForKey:@"zbufferwrite" default:true];
}

@end

namespace WhirlyKit
{

void GeomSceneRep::clearContents(SelectionManager *selectManager,ChangeSet &changes)
{
    for (SimpleIDSet::iterator it = drawIDs.begin();
         it != drawIDs.end(); ++it)
        changes.push_back(new RemDrawableReq(*it));
    if (selectManager && !selectIDs.empty())
        selectManager->removeSelectables(selectIDs);
}

void GeomSceneRep::enableContents(SelectionManager *selectManager,bool enable,ChangeSet &changes)
{
    for (SimpleIDSet::iterator it = drawIDs.begin();
         it != drawIDs.end(); ++it)
        changes.push_back(new OnOffChangeRequest(*it, enable));
    if (selectManager && !selectIDs.empty())
        selectManager->enableSelectables(selectIDs, enable);
}
    
GeometryRaw::GeometryRaw()
    : type(WhirlyKitGeometryTriangles), texId(EmptyIdentity)
{
}

GeometryRaw::GeometryRaw(const GeometryRaw &that)
{
    type = that.type;
    pts = that.pts;
    norms = that.norms;
    texCoords = that.texCoords;
    colors = that.colors;
    triangles = that.triangles;
    texId = that.texId;
}
    
bool GeometryRaw::operator == (const GeometryRaw &that) const
{
    return texId == that.texId && type == that.type;
}

bool GeometryRaw::isValid() const
{
    if (type != WhirlyKitGeometryLines && type != WhirlyKitGeometryTriangles)
        return false;
    int numPoints = (int)pts.size();
    if (numPoints == 0)
        return false;
    
    if (!norms.empty() && norms.size() != numPoints)
        return false;
    if (!texCoords.empty() && texCoords.size() != numPoints)
        return false;
    if (!colors.empty() && colors.size() != numPoints)
        return false;
    if (type == WhirlyKitGeometryTriangles && triangles.empty())
        return false;
    if (texId > 0 && texCoords.empty())
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

void GeometryRaw::applyTransform(const Matrix4d &mat)
{
    for (unsigned int ii=0;ii<pts.size();ii++)
    {
        Point3d &pt = pts[ii];
        Vector4d outPt = mat * Eigen::Vector4d(pt.x(),pt.y(),pt.z(),1.0);
        pt = Point3d(outPt.x()/outPt.w(),outPt.y()/outPt.w(),outPt.z()/outPt.w());
    }
    
    for (unsigned int ii=0;ii<norms.size();ii++)
    {
        Point3d &norm = norms[ii];
        Vector4d projNorm = mat * Eigen::Vector4d(norm.x(),norm.y(),norm.z(),0.0);
        norm = Point3d(projNorm.x(),projNorm.y(),projNorm.z()).normalized();
    }
}
    
void GeometryRaw::estimateSize(int &numPts,int &numTris)
{
    numPts = pts.size();
    numTris = triangles.size();
}
    
// Calculate bounding box
void GeometryRaw::calcBounds(Point3d &ll,Point3d &ur)
{
    ll.x() = MAXFLOAT;  ll.y() = MAXFLOAT;  ll.z() = MAXFLOAT;
    ur.x() = -MAXFLOAT;  ur.y() = -MAXFLOAT;  ur.z() = -MAXFLOAT;
    
    for (const auto pt : pts)
    {
        ll.x() = std::min(ll.x(),pt.x());
        ll.y() = std::min(ll.y(),pt.y());
        ll.z() = std::min(ll.z(),pt.z());
        ur.x() = std::max(ur.x(),pt.x());
        ur.y() = std::max(ur.y(),pt.y());
        ur.z() = std::max(ur.z(),pt.z());
    }
}

void GeometryRaw::buildDrawables(std::vector<BasicDrawable *> &draws,const Eigen::Matrix4d &mat,const RGBAColor *colorOverride,WhirlyKitGeomInfo *geomInfo)
{
    if (!isValid())
        return;
    
    BasicDrawable *draw = NULL;
    for (unsigned int ii=0;ii<triangles.size();ii++)
    {
        RawTriangle tri = triangles[ii];
        // See if we need a new drawable
        if (!draw || draw->getNumPoints() + 3 > MaxDrawablePoints || draw->getNumTris() + 1 > MaxDrawableTriangles)
        {
            draw = new BasicDrawable("Raw Geometry");
            if (geomInfo)
                [geomInfo setupBasicDrawable:draw];
            if (colorOverride)
                draw->setColor(*colorOverride);
            draw->setType(GL_TRIANGLES);
            if (texId > 0)
                draw->setTexId(0,texId);
            draws.push_back(draw);
        }
        
        // Add the triangle by copying its vertices (meh)
        int baseVert = draw->getNumPoints();
        for (unsigned int jj=0;jj<3;jj++)
        {
            const Point3d &pt = pts[tri.verts[jj]];
            Vector4d outPt = mat * Eigen::Vector4d(pt.x(),pt.y(),pt.z(),1.0);
            Point3d newPt(outPt.x()/outPt.w(),outPt.y()/outPt.w(),outPt.z()/outPt.w());
            draw->addPoint(newPt);
            if (!norms.empty())
            {
                const Point3d &norm = norms[tri.verts[jj]];
                // Note: Not the right way to transform normals
                Vector4d projNorm = mat * Eigen::Vector4d(norm.x(),norm.y(),norm.z(),0.0);
                Point3d newNorm(projNorm.x(),projNorm.y(),projNorm.z());
                newNorm.normalize();
                draw->addNormal(newNorm);
            }
            if (texId > 0)
                draw->addTexCoord(0,texCoords[tri.verts[jj]]);
            if (!colors.empty() && !colorOverride)
                draw->addColor(colors[tri.verts[jj]]);
        }
        
        draw->addTriangle(BasicDrawable::Triangle(baseVert,baseVert+1,baseVert+2));
    }
}
    
GeometryRawPoints::GeometryRawPoints()
{
}

GeometryRawPoints::~GeometryRawPoints()
{
    for (GeomPointAttrData *attrs : attrData)
        delete attrs;
    attrData.clear();
}
    
void GeometryRawPoints::addValue(int idx,int val)
{
    if (idx >= attrData.size())
        return;

    GeomPointAttrData *attrs = attrData[idx];
    GeomPointAttrDataInt *intAttrs = dynamic_cast<GeomPointAttrDataInt *> (attrs);
    if (intAttrs)
        intAttrs->vals.push_back(val);
}
    
void GeometryRawPoints::addValue(int idx,float val)
{
    if (idx >= attrData.size())
        return;
    
    GeomPointAttrData *attrs = attrData[idx];
    GeomPointAttrDataFloat *fAttrs = dynamic_cast<GeomPointAttrDataFloat *> (attrs);
    if (fAttrs)
        fAttrs->vals.push_back(val);
}
    
void GeometryRawPoints::addPoint(int idx,const Point2f &pt)
{
    if (idx >= attrData.size())
        return;
    
    GeomPointAttrData *attrs = attrData[idx];
    GeomPointAttrDataPoint2f *f2Attrs = dynamic_cast<GeomPointAttrDataPoint2f *> (attrs);
    if (f2Attrs)
        f2Attrs->vals.push_back(pt);
}
    
void GeometryRawPoints::addPoint(int idx,const Point3f &pt)
{
    if (idx >= attrData.size())
        return;
    
    GeomPointAttrData *attrs = attrData[idx];
    GeomPointAttrDataPoint3f *f3Attrs = dynamic_cast<GeomPointAttrDataPoint3f *> (attrs);
    if (f3Attrs)
        f3Attrs->vals.push_back(pt);
}
    
void GeometryRawPoints::addPoint(int idx,const Point3d &pt)
{
    if (idx >= attrData.size())
        return;
    
    GeomPointAttrData *attrs = attrData[idx];
    GeomPointAttrDataPoint3d *d3Attrs = dynamic_cast<GeomPointAttrDataPoint3d *> (attrs);
    if (d3Attrs)
        d3Attrs->vals.push_back(pt);
    else {
        GeomPointAttrDataPoint3f *f3Attrs = dynamic_cast<GeomPointAttrDataPoint3f *>(attrs);
        if (f3Attrs)
            f3Attrs->vals.push_back(Point3f(pt.x(),pt.y(),pt.z()));
    }
}
    
void GeometryRawPoints::addPoint(int idx,const Eigen::Vector4f &pt)
{
    if (idx >= attrData.size())
        return;
    
    GeomPointAttrData *attrs = attrData[idx];
    GeomPointAttrDataPoint4f *f4Attrs = dynamic_cast<GeomPointAttrDataPoint4f *> (attrs);
    if (f4Attrs)
        f4Attrs->vals.push_back(pt);
}

int GeometryRawPoints::addAttribute(const std::string &name,GeomRawDataType dataType)
{
    // Make sure we don't already have it
    for (GeomPointAttrData *data : attrData)
        if (name == data->name)
            return -1;
    
    int idx = -1;
    switch (dataType)
    {
        case GeomRawIntType:
        {
            GeomPointAttrDataInt *attrs = new GeomPointAttrDataInt();
            attrs->name = name;
            idx = attrData.size();
            attrData.push_back(attrs);
        }
            break;
        case GeomRawFloatType:
        {
            GeomPointAttrDataFloat *attrs = new GeomPointAttrDataFloat();
            attrs->name = name;
            idx = attrData.size();
            attrData.push_back(attrs);
        }
            break;
        case GeomRawFloat2Type:
        {
            GeomPointAttrDataPoint2f *attrs = new GeomPointAttrDataPoint2f();
            attrs->name = name;
            idx = attrData.size();
            attrData.push_back(attrs);
        }
            break;
        case GeomRawFloat3Type:
        {
            GeomPointAttrDataPoint3f *attrs = new GeomPointAttrDataPoint3f();
            attrs->name = name;
            idx = attrData.size();
            attrData.push_back(attrs);
        }
            break;
        case GeomRawFloat4Type:
        {
            GeomPointAttrDataPoint4f *attrs = new GeomPointAttrDataPoint4f();
            attrs->name = name;
            idx = attrData.size();
            attrData.push_back(attrs);
        }
            break;
        case GeomRawDouble2Type:
        {
            GeomPointAttrDataPoint2d *attrs = new GeomPointAttrDataPoint2d();
            attrs->name = name;
            idx = attrData.size();
            attrData.push_back(attrs);
        }
            break;
        case GeomRawDouble3Type:
        {
            GeomPointAttrDataPoint3d *attrs = new GeomPointAttrDataPoint3d();
            attrs->name = name;
            idx = attrData.size();
            attrData.push_back(attrs);
        }
            break;
        default:
            return -1;
            break;
    }
    
    return idx;
}
    
int GeometryRawPoints::findAttribute(const std::string &name) const
{
    int which = 0;
    for (auto attr : attrData)
    {
        if (attr->name == name)
            return which;
        which++;
    }
    
    return -1;
}
    
bool GeometryRawPoints::valid() const
{
    int numVals = -1;
    bool hasPosition = false;
    for (auto attrs : attrData)
    {
        if (attrs->name == "a_position")
            hasPosition = true;
        if (numVals == -1)
            numVals = attrs->getNumVals();
        else {
            if (numVals != attrs->getNumVals())
                return false;
        }
    }
    
    return hasPosition;
}
    
void GeometryRawPoints::buildDrawables(std::vector<BasicDrawable *> &draws,const Eigen::Matrix4d &mat,WhirlyKitGeomInfo *geomInfo) const
{
    if (!valid())
        return;
    
    BasicDrawable *draw = NULL;
    
    int posIdx = findAttribute("a_position");
    int colorIdx = findAttribute("a_color");

    int numVals = attrData[posIdx]->getNumVals();
    
    std::vector<int> attrIdxs(attrData.size());
    
    for (unsigned int vert=0;vert<numVals;vert++)
    {
        // See if we need a new drawable
        if (!draw || draw->getNumPoints() + 3 > MaxDrawablePoints)
        {
            draw = new BasicDrawable("Raw Geometry");
            if (geomInfo) {
                [geomInfo setupBasicDrawable:draw];
            }
            if (!mat.isIdentity())
                draw->setMatrix(&mat);
            draw->setType(GL_POINTS);
            draws.push_back(draw);
            
            // Add the various attributes
            int which = 0;
            for (auto attrs : attrData)
            {
                BDAttributeDataType dataType = BDDataTypeMax;
                switch (attrs->dataType)
                {
                    case GeomRawIntType:
                        dataType = BDIntType;
                        break;
                    case GeomRawFloatType:
                        dataType = BDFloatType;
                        break;
                    case GeomRawFloat2Type:
                        dataType = BDFloat2Type;
                        break;
                    case GeomRawFloat3Type:
                        dataType = BDFloat3Type;
                        break;
                    case GeomRawFloat4Type:
                        dataType = BDFloat4Type;
                        break;
                    case GeomRawDouble2Type:
                        dataType = BDFloat2Type;
                        break;
                    case GeomRawDouble3Type:
                        dataType = BDFloat3Type;
                        break;
                    default:
                        break;
                }
                attrIdxs[which] = draw->addAttribute(dataType, attrs->name);
                which++;
            }
        }

        // Note: Should copy these in more directly
        int which = 0;
        for (auto attrs : attrData)
        {
            int attrIdx = attrIdxs[which];
            switch (attrs->dataType)
            {
                case GeomRawIntType:
                {
                    GeomPointAttrDataInt *attrsInt = dynamic_cast<GeomPointAttrDataInt *>(attrs);
                    draw->addAttributeValue(attrIdx, attrsInt->vals[vert]);
                }
                    break;
                case GeomRawFloatType:
                {
                    GeomPointAttrDataFloat *attrsFloat = dynamic_cast<GeomPointAttrDataFloat *>(attrs);
                    draw->addAttributeValue(attrIdx, attrsFloat->vals[vert]);
                }
                    break;
                case GeomRawFloat2Type:
                {
                    GeomPointAttrDataPoint2f *attrsFloat2 = dynamic_cast<GeomPointAttrDataPoint2f *>(attrs);
                    draw->addAttributeValue(attrIdx, attrsFloat2->vals[vert]);
                }
                    break;
                case GeomRawFloat3Type:
                {
                    GeomPointAttrDataPoint3f *attrsFloat3 = dynamic_cast<GeomPointAttrDataPoint3f *>(attrs);
                    const Point3f &pt = attrsFloat3->vals[vert];
                    if (which == posIdx)
                        draw->addPoint(pt);
                    else
                        draw->addAttributeValue(attrIdx, pt);
                }
                    break;
                case GeomRawFloat4Type:
                {
                    GeomPointAttrDataPoint4f *attrsFloat4 = dynamic_cast<GeomPointAttrDataPoint4f *>(attrs);
                    const Vector4f &pt = attrsFloat4->vals[vert];
                    if (which == colorIdx)
                    {
                        RGBAColor color(pt.x()*255,pt.y()*255,pt.z()*255,pt.w()*255);
                        draw->addColor(color);
                    } else
                        draw->addAttributeValue(attrIdx, pt);
                }
                    break;
                case GeomRawDouble2Type:
                {
                    GeomPointAttrDataPoint2d *attrsDouble2 = dynamic_cast<GeomPointAttrDataPoint2d *>(attrs);
                    const Point2d &pt = attrsDouble2->vals[vert];
                    draw->addAttributeValue(attrIdx, Point2f(pt.x(),pt.y()));
                }
                    break;
                case GeomRawDouble3Type:
                {
                    GeomPointAttrDataPoint3d *attrsDouble3 = dynamic_cast<GeomPointAttrDataPoint3d *>(attrs);
                    const Point3d &pt = attrsDouble3->vals[vert];
                    if (which == posIdx)
                        draw->addPoint(pt);
                    else
                        draw->addAttributeValue(attrIdx, Point3f(pt.x(),pt.y(),pt.z()));
                }
                    break;
                default:
                    break;
            }
            which++;
        }
    }
}

    
GeometryManager::GeometryManager()
{
    pthread_mutex_init(&geomLock, NULL);
}
    
GeometryManager::~GeometryManager()
{
    pthread_mutex_destroy(&geomLock);
    for (GeomSceneRepSet::iterator it = sceneReps.begin();
         it != sceneReps.end(); ++it)
        delete *it;
    sceneReps.clear();
}
    
SimpleIdentity GeometryManager::addGeometry(std::vector<GeometryRaw> &geom,const std::vector<GeometryInstance> &instances,NSDictionary *desc,ChangeSet &changes)
{
    SelectionManager *selectManager = (SelectionManager *)scene->getManager(kWKSelectionManager);
    GeomSceneRep *sceneRep = new GeomSceneRep();
    
    WhirlyKitGeomInfo *geomInfo = [[WhirlyKitGeomInfo alloc] initWithDesc:desc];

    // Calculate the bounding box for the whole thing
    Point3d ll,ur;

    // Sort the geometry by type and texture
    std::vector<std::vector<GeometryRaw *>> sortedGeom;
    for (unsigned int ii=0;ii<geom.size();ii++)
    {
        GeometryRaw *raw = &geom[ii];
        
        raw->calcBounds(ll, ur);
        
        bool found = false;
        for (unsigned int jj=0;jj<sortedGeom.size();jj++)
        {
            std::vector<GeometryRaw *> &sg = sortedGeom[jj];
            if (*(sg.at(0)) == *raw)
            {
                found = true;
                sg.push_back(raw);
                break;
            }
        }
        if (!found)
        {
            std::vector<GeometryRaw *> arr;
            arr.push_back(raw);
            sortedGeom.push_back(arr);
        }
    }
    
    // Work through the model instances
    for (unsigned int ii=0;ii<instances.size();ii++)
    {
        const GeometryInstance &inst = instances[ii];
        Vector4d center = inst.mat * Vector4d(0,0,0,1);
        center.x() /= center.w();  center.y() /= center.w();  center.z() /= center.w();
        Eigen::Affine3d transBack(Eigen::Translation3d(-center.x(),-center.y(),-center.z()));
        Matrix4d transBackMat = transBack.matrix();
        Matrix4d instMat = transBackMat * inst.mat;
        
        // Convert the sorted lists of geometry into drawables
        for (unsigned int jj=0;jj<sortedGeom.size();jj++)
        {
            std::vector<GeometryRaw *> &sg = sortedGeom[jj];
            for (unsigned int kk=0;kk<sg.size();kk++)
            {
                std::vector<BasicDrawable *> draws;
                GeometryRaw *raw = sg[kk];
                raw->buildDrawables(draws,instMat,(inst.colorOverride ? &inst.color : NULL),geomInfo);
                
                // Set the various parameters and store the drawables created
                for (unsigned int ll=0;ll<draws.size();ll++)
                {
                    BasicDrawable *draw = draws[ll];
                    draw->setType((raw->type == WhirlyKitGeometryLines ? GL_LINES : GL_TRIANGLES));
                    draw->setOnOff(geomInfo.enable);
//                    draw->setColor([geomInfo.color asRGBAColor]);
                    draw->setVisibleRange(geomInfo.minVis, geomInfo.maxVis);
                    draw->setDrawPriority(geomInfo.drawPriority);
                    draw->setRequestZBuffer(true);
                    draw->setWriteZBuffer(true);
                    Eigen::Affine3d trans(Eigen::Translation3d(center.x(),center.y(),center.z()));
                    Matrix4d transMat = trans.matrix();
                    draw->setMatrix(&transMat);
                    sceneRep->drawIDs.insert(draw->getId());
                    changes.push_back(new AddDrawableReq(draw));
                }
            }
        }
        
        // Note: Not sharing drawables between instances
        
        // Add a selection box for each instance
        if (inst.selectable)
        {
            selectManager->addPolytopeFromBox(inst.getId(), ll, ur, inst.mat, geomInfo.minVis, geomInfo.maxVis, geomInfo.enable);
            sceneRep->selectIDs.insert(inst.getId());
        }
    }
    
    SimpleIdentity geomID = sceneRep->getId();
    
    pthread_mutex_lock(&geomLock);
    sceneReps.insert(sceneRep);
    pthread_mutex_unlock(&geomLock);
    
    return geomID;
}
    
/// Add geometry we're planning to reuse (as a model, for example)
SimpleIdentity GeometryManager::addBaseGeometry(std::vector<GeometryRaw> &geom,ChangeSet &changes)
{
    GeomSceneRep *sceneRep = new GeomSceneRep();
    
    // Sort the geometry by type and texture
    std::vector<std::vector<GeometryRaw *>> sortedGeom;
    for (unsigned int ii=0;ii<geom.size();ii++)
    {
        GeometryRaw *raw = &geom[ii];
        
        raw->calcBounds(sceneRep->ll, sceneRep->ur);
        
        bool found = false;
        for (unsigned int jj=0;jj<sortedGeom.size();jj++)
        {
            std::vector<GeometryRaw *> &sg = sortedGeom[jj];
            if (*(sg.at(0)) == *raw)
            {
                found = true;
                sg.push_back(raw);
                break;
            }
        }
        if (!found)
        {
            std::vector<GeometryRaw *> arr;
            arr.push_back(raw);
            sortedGeom.push_back(arr);
        }
    }

    // Instance the geometry once for now
    Matrix4d instMat = Matrix4d::Identity();
    
    // Convert the sorted lists of geometry into drawables
    for (unsigned int jj=0;jj<sortedGeom.size();jj++)
    {
        std::vector<GeometryRaw *> &sg = sortedGeom[jj];
        for (unsigned int kk=0;kk<sg.size();kk++)
        {
            std::vector<BasicDrawable *> draws;
            GeometryRaw *raw = sg[kk];
            raw->buildDrawables(draws,instMat,NULL,nil);
            
            // Set the various parameters and store the drawables created
            for (unsigned int ll=0;ll<draws.size();ll++)
            {
                BasicDrawable *draw = draws[ll];
                draw->setType((raw->type == WhirlyKitGeometryLines ? GL_LINES : GL_TRIANGLES));
                draw->setOnOff(false);
                draw->setRequestZBuffer(true);
                draw->setWriteZBuffer(true);
                sceneRep->drawIDs.insert(draw->getId());
                changes.push_back(new AddDrawableReq(draw));
            }
        }
    }
    
    SimpleIdentity geomID = sceneRep->getId();
    
    pthread_mutex_lock(&geomLock);
    sceneReps.insert(sceneRep);
    pthread_mutex_unlock(&geomLock);
    
    return geomID;
}

/// Add instances that reuse base geometry
SimpleIdentity GeometryManager::addGeometryInstances(SimpleIdentity baseGeomID,const std::vector<GeometryInstance> &instances,NSDictionary *desc,ChangeSet &changes)
{
    pthread_mutex_lock(&geomLock);
    NSTimeInterval startTime = CFAbsoluteTimeGetCurrent();

    // Look for the scene rep we're basing this on
    GeomSceneRep *baseSceneRep = NULL;
    GeomSceneRep dummyRep(baseGeomID);
    GeomSceneRepSet::iterator it = sceneReps.find(&dummyRep);
    if (it != sceneReps.end())
        baseSceneRep = *it;
    
    if (!baseSceneRep)
    {
        pthread_mutex_unlock(&geomLock);
        return EmptyIdentity;
    }
    
    SelectionManager *selectManager = (SelectionManager *)scene->getManager(kWKSelectionManager);
    GeomSceneRep *sceneRep = new GeomSceneRep();
    WhirlyKitGeomInfo *geomInfo = [[WhirlyKitGeomInfo alloc] initWithDesc:desc];
    
    // Check for moving models
    bool hasMotion = false;
    for (const GeometryInstance &inst : instances)
        if (inst.duration > 0.0)
            hasMotion = true;
    
    // Work through the model instances
    std::vector<BasicDrawableInstance::SingleInstance> singleInsts;
    for (unsigned int ii=0;ii<instances.size();ii++)
    {
        const GeometryInstance &inst = instances[ii];
        BasicDrawableInstance::SingleInstance singleInst;
        if (desc && desc[@"color"])
        {
            singleInst.colorOverride = true;
            singleInst.color = [geomInfo.color asRGBAColor];
        }
        if (inst.colorOverride)
        {
            singleInst.colorOverride = true;
            singleInst.color = inst.color;
        }
        singleInst.center = inst.center;
        singleInst.mat = inst.mat;
        if (hasMotion)
        {
            singleInst.endCenter = inst.endCenter;
            singleInst.duration = inst.duration;
        }
        singleInsts.push_back(singleInst);
        
        // Add a selection box for each instance
        if (inst.selectable)
        {
            if (hasMotion)
                selectManager->addMovingPolytopeFromBox(inst.getId(), baseSceneRep->ll, baseSceneRep->ur, inst.center, inst.endCenter, startTime, inst.duration, inst.mat, geomInfo.minVis, geomInfo.maxVis, geomInfo.enable);
            else
                selectManager->addPolytopeFromBox(inst.getId(), baseSceneRep->ll, baseSceneRep->ur, inst.mat, geomInfo.minVis, geomInfo.maxVis, geomInfo.enable);
            sceneRep->selectIDs.insert(inst.getId());
        }
    }

    // Instance each of the drawables in the base
    for (SimpleIdentity baseDrawID : baseSceneRep->drawIDs)
    {
        BasicDrawableInstance *drawInst = new BasicDrawableInstance("GeometryManager",baseDrawID,BasicDrawableInstance::LocalStyle);
        [geomInfo setupBasicDrawableInstance:drawInst];
        //                    draw->setColor([geomInfo.color asRGBAColor]);
        drawInst->setRequestZBuffer(true);
        drawInst->setWriteZBuffer(true);
        drawInst->addInstances(singleInsts);
        if (geomInfo.programID != EmptyIdentity)
            drawInst->setProgram(geomInfo.programID);
        if (hasMotion)
        {
            drawInst->setStartTime(startTime);
            drawInst->setIsMoving(true);
        }
        
        sceneRep->drawIDs.insert(drawInst->getId());
        changes.push_back(new AddDrawableReq(drawInst));
    }

    SimpleIdentity geomID = sceneRep->getId();
    
    sceneReps.insert(sceneRep);
    pthread_mutex_unlock(&geomLock);
    
    return geomID;
}
    
SimpleIdentity GeometryManager::addGeometryPoints(const GeometryRawPoints &geomPoints,const Eigen::Matrix4d &mat,NSDictionary *desc,ChangeSet &changes)
{
    GeomSceneRep *sceneRep = new GeomSceneRep();
    
    WhirlyKitGeomInfo *geomInfo = [[WhirlyKitGeomInfo alloc] initWithDesc:desc];
    
    // Calculate the bounding box for the whole thing
    Point3d ll,ur;

    std::vector<BasicDrawable *> draws;
    geomPoints.buildDrawables(draws,mat,geomInfo);
    
    // Set the various parameters and store the drawables created
    for (unsigned int ll=0;ll<draws.size();ll++)
    {
        BasicDrawable *draw = draws[ll];
        draw->setType(GL_POINTS);
        draw->setOnOff(geomInfo.enable);
        draw->setColor([geomInfo.color asRGBAColor]);
        draw->setVisibleRange(geomInfo.minVis, geomInfo.maxVis);
        draw->setDrawPriority(geomInfo.drawPriority);
        draw->setRequestZBuffer(geomInfo.zBufferRead);
        draw->setRequestZBuffer(geomInfo.zBufferWrite);
//        Eigen::Affine3d trans(Eigen::Translation3d(center.x(),center.y(),center.z()));
//        Matrix4d transMat = trans.matrix();
//        draw->setMatrix(&transMat);
        sceneRep->drawIDs.insert(draw->getId());
        changes.push_back(new AddDrawableReq(draw));
    }
    
    SimpleIdentity geomID = sceneRep->getId();
    
    pthread_mutex_lock(&geomLock);
    sceneReps.insert(sceneRep);
    pthread_mutex_unlock(&geomLock);
    
    return geomID;
}

void GeometryManager::enableGeometry(SimpleIDSet &geomIDs,bool enable,ChangeSet &changes)
{
    SelectionManager *selectManager = (SelectionManager *)scene->getManager(kWKSelectionManager);
    
    pthread_mutex_lock(&geomLock);
    
    for (SimpleIDSet::iterator git = geomIDs.begin(); git != geomIDs.end(); ++git)
    {
        GeomSceneRep dummyRep(*git);
        GeomSceneRepSet::iterator it = sceneReps.find(&dummyRep);
        if (it != sceneReps.end())
        {
            GeomSceneRep *geomRep = *it;
            geomRep->enableContents(selectManager,enable,changes);
        }
    }
    
    pthread_mutex_unlock(&geomLock);
}

void GeometryManager::removeGeometry(SimpleIDSet &geomIDs,ChangeSet &changes)
{
    SelectionManager *selectManager = (SelectionManager *)scene->getManager(kWKSelectionManager);

    pthread_mutex_lock(&geomLock);
    
    NSTimeInterval curTime = CFAbsoluteTimeGetCurrent();
    for (SimpleIDSet::iterator git = geomIDs.begin(); git != geomIDs.end(); ++git)
    {
        GeomSceneRep dummyRep(*git);
        GeomSceneRepSet::iterator it = sceneReps.find(&dummyRep);
        
        if (it != sceneReps.end())
        {
            GeomSceneRep *sceneRep = *it;
            
            if (sceneRep->fade > 0.0)
            {
                for (SimpleIDSet::iterator it = sceneRep->drawIDs.begin();
                     it != sceneRep->drawIDs.end(); ++it)
                    changes.push_back(new FadeChangeRequest(*it, curTime, curTime+sceneRep->fade));
                
                __block NSObject * __weak thisCanary = canary;

                // Spawn off the deletion for later
                dispatch_after(dispatch_time(DISPATCH_TIME_NOW, sceneRep->fade * NSEC_PER_SEC),
                               scene->getDispatchQueue(),
                               ^{
                                   if (thisCanary)
                                   {
                                       SimpleIDSet theIDs;
                                       theIDs.insert(sceneRep->getId());
                                       ChangeSet delChanges;
                                       removeGeometry(theIDs, delChanges);
                                       scene->addChangeRequests(delChanges);
                                   }
                               }
                               );
                
                sceneRep->fade = 0.0;
            } else {
                sceneRep->clearContents(selectManager,changes);
                sceneReps.erase(it);
                delete sceneRep;
            }
        }
    }

    pthread_mutex_unlock(&geomLock);
}

}
