/*
 *  ScreenSpaceManager.mm
 *  WhirlyGlobeLib
 *
 *  Created by Steve Gifford on 2/21/14.
 *  Copyright 2011-2014 mousebird consulting. All rights reserved.
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

#import "ScreenSpaceBuilder.h"
#import "ScreenSpaceDrawable.h"

namespace WhirlyKit
{

ScreenSpaceBuilder::DrawableState::DrawableState()
    : texID(EmptyIdentity), progID(EmptyIdentity), fadeUp(0.0), fadeDown(0.0),
    drawPriority(0), minVis(DrawVisibleInvalid), maxVis(DrawVisibleInvalid)
{
}
    
bool ScreenSpaceBuilder::DrawableState::operator < (const DrawableState &that) const
{
    if (texID != that.texID)
        return texID < that.texID;
    if (progID != that.progID)
        return progID < that.progID;
    if (drawPriority != that.drawPriority)
        return drawPriority < that.drawPriority;
    if (minVis != that.minVis)
        return  minVis < that.minVis;
    if (maxVis != that.maxVis)
        return  maxVis < that.maxVis;
    if (fadeUp != that.fadeUp)
        return fadeUp < that.fadeUp;
    if (fadeDown != that.fadeDown)
        return fadeDown < that.fadeDown;
    
    return false;
}
    
ScreenSpaceBuilder::ScreenSpaceBuilder()
{
}

ScreenSpaceBuilder::~ScreenSpaceBuilder()
{
    for (DrawableWrapSet::iterator it = drawables.begin(); it != drawables.end(); ++it)
        delete *it;
}
    
void ScreenSpaceBuilder::setTexID(SimpleIdentity texID)
{
    curState.texID = texID;
}

void ScreenSpaceBuilder::setProgramID(SimpleIdentity progID)
{
    curState.progID = progID;
}

void ScreenSpaceBuilder::setFade(NSTimeInterval fadeUp,NSTimeInterval fadeDown)
{
    curState.fadeUp = fadeUp;
    curState.fadeDown = fadeDown;
}

void ScreenSpaceBuilder::setDrawPriority(int drawPriority)
{
    curState.drawPriority = drawPriority;
}

void ScreenSpaceBuilder::setVisibility(float minVis,float maxVis)
{
    curState.minVis = minVis;
    curState.maxVis = maxVis;
}
    
void ScreenSpaceBuilder::addRectangle(const Point3d &worldLoc,const Point3d *coords,const TexCoord *texCoords)
{
    
}

void ScreenSpaceBuilder::buildDrawables(std::vector<ScreenSpaceDrawable *> &draws)
{
    for (DrawableWrapSet::iterator it = drawables.begin(); it != drawables.end(); ++it)
        draws.push_back((*it)->draw);
    
    drawables.clear();
}
    
/// Used to group meshes that we can consolidate together
class MeshList
{
public:
    MeshList() {}
    MeshList(const ScreenSpaceBuilder::Mesh *mesh) { meshes.push_back(mesh); }
    
    // Comparison operator
    bool operator < (const MeshList &that) const
    {
        const ScreenSpaceBuilder::Mesh *mesh = meshes[0];
        const ScreenSpaceBuilder::Mesh *thatMesh = that.meshes[0];
        
        if (mesh->texID != thatMesh->texID)
            return mesh->texID < thatMesh->texID;
        if (mesh->programID != thatMesh->programID)
            return mesh->programID < thatMesh->programID;
        
        return false;
    }
    
    // Construct a drawable with all the meshes
    ScreenSpaceDrawable *createDrawable(const ScreenSpaceBuilder::ScreenSpaceObject &screenShape)
    {
        const ScreenSpaceBuilder::Mesh &firstMesh = screenShape.meshes[0];
        int numVerts = 0;
        int numTris = 0;
        
        for (unsigned int ii=0;ii<screenShape.meshes.size();ii++)
        {
            const ScreenSpaceBuilder::Mesh &mesh = screenShape.meshes[ii];
            numVerts += mesh.coords.size();
            numTris += mesh.tris.size();
        }
        
        if (numVerts == 0 || numTris == 0)
            return NULL;
        
        // Set up the drawable
        ScreenSpaceDrawable *draw = new ScreenSpaceDrawable();
        draw->reserveNumPoints(numVerts);
        draw->reserveNumTexCoords(numVerts, 1);
        draw->reserveNumColors(numVerts);
        draw->reserveNumTris(numTris);
        draw->setTexId(0, firstMesh.texID);
        draw->setProgram(firstMesh.programID);
        draw->setDrawPriority(screenShape.drawPriority);
        draw->setFade(screenShape.fadeDown, screenShape.fadeUp);
        draw->setVisibleRange(screenShape.minVis, screenShape.maxVis);

        // Note: Still need useRotation, keepUpright, rotation
        
        for (unsigned int ii=0;ii<screenShape.meshes.size();ii++)
        {
            const ScreenSpaceBuilder::Mesh &mesh = screenShape.meshes[ii];
            
            int baseVert = draw->getNumPoints();
            for (unsigned int jj=0;jj<mesh.coords.size();jj++)
            {
                draw->addPoint(mesh.worldLoc);
                const Point2d &coord = mesh.coords[jj];
                draw->addOffset(Point2f(coord.x(),coord.y()));
                draw->addColor(mesh.colors[jj]);
                draw->addTexCoord(0, mesh.texCoords[jj]);
                // Note: Normal?
            }
            
            for (unsigned int jj=0;jj<mesh.tris.size();jj++)
            {
                BasicDrawable::Triangle tri = mesh.tris[jj];
                for (unsigned int kk=0;kk<3;kk++)
                    tri.verts[kk] += baseVert;
                draw->addTriangle(tri);
            }
        }
        
        return draw;
    }
    
    std::vector<const ScreenSpaceBuilder::Mesh *> meshes;
};

struct MeshListComparitor
{
    bool operator()(const MeshList *a,const MeshList *b)
    {
        return *a < *b;
    }
};
    
typedef std::set<MeshList *,MeshListComparitor> MeshListSet;
    
ScreenSpaceBuilder::Mesh::Mesh()
    : texID(EmptyIdentity), programID(EmptyIdentity), worldLoc(0,0,0), rotation(0.0)
{
}
    
ScreenSpaceBuilder::ScreenSpaceObject::ScreenSpaceObject()
    : useRotation(false), keepUpright(false), fadeUp(0), fadeDown(0), drawPriority(0),
    minVis(DrawVisibleInvalid), maxVis(DrawVisibleInvalid)
{
}
    
SimpleIdentity ScreenSpaceBuilder::addShapes(const ScreenSpaceObject &screenShape,ChangeSet &changes)
{
    MeshListSet meshes;

    // Sort the meshes we can put together
    for (unsigned int ii=0;ii<screenShape.meshes.size();ii++)
    {
        const Mesh *mesh = &screenShape.meshes[ii];
        MeshList dummy(mesh);
        
        MeshList *meshList = NULL;
        MeshListSet::iterator it = meshes.find(&dummy);
        if (it == meshes.end())
            meshList = new MeshList();
        else
            meshList = *it;
        
        meshList->meshes.push_back(mesh);
    }

    ScreenSpaceRep *sceneRep = new ScreenSpaceRep();
    SimpleIdentity sceneId = sceneRep->getId();
    for (MeshListSet::iterator it = meshes.begin(); it != meshes.end(); ++it)
    {
        MeshList *meshList = *it;
        
        ScreenSpaceDrawable *screenDraw = meshList->createDrawable(screenShape);
        if (screenDraw)
        {
            sceneRep->drawIDs.insert(screenDraw->getId());
            changes.push_back(new AddDrawableReq(screenDraw));
        }
    }
    
    for (MeshListSet::iterator it = meshes.begin();it!=meshes.end();++it)
        delete *it;
    
    pthread_mutex_lock(&shapeLock);
    shapeReps.insert(sceneRep);
    pthread_mutex_unlock(&shapeLock);

    return sceneId;
}
        
}
