/*
 *  GlobeQuadDisplayLayer.mm
 *  WhirlyGlobeLib
 *
 *  Created by Steve Gifford on 4/17/12.
 *  Copyright 2011 mousebird consulting
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

#import "GlobeQuadDisplayLayer.h"
#import "GlobeMath.h"
#import "GlobeLayerViewWatcher.h"
#import "UIImage+Stuff.h"
#import <boost/math/special_functions/fpclassify.hpp>

using namespace Eigen;
using namespace WhirlyKit;
using namespace WhirlyGlobe;

@interface WhirlyGlobeQuadDisplayLayer()
- (void)buildTile:(Quadtree::NodeInfo *)nodeInfo draw:(BasicDrawable **)draw tex:(Texture **)tex texScale:(Point2f)texScale texOffset:(Point2f)texOffset lines:(bool)buildLines;
@property (nonatomic,assign) GlobeScene *scene;
@end

namespace WhirlyGlobe 
{
    
// Calculate the importance for the given texel
static float calcImportance(WhirlyGlobeViewState *viewState,Point3f eyeVec,Point3f pt,Point2f pixSize,Point2f frameSize,WhirlyKit::CoordSystem *coordSys)
{
    Point3f pts[4];
    pts[0] = pt + Point3f(-pixSize.x()/2.0,-pixSize.y()/2.0,0.0);
    pts[1] = pt + Point3f(pixSize.x()/2.0,-pixSize.y()/2.0,0.0);
    pts[2] = pt + Point3f(pixSize.x()/2.0,pixSize.y()/2.0,0.0);
    pts[3] = pt + Point3f(-pixSize.x()/2.0,pixSize.y()/2.0,0.0);
    
    // Convert to 3-space
    Point3f pts3d[4];
    Point2f screenPts[4];
    bool forwardFacing = false;
    for (unsigned int ii=0;ii<4;ii++)
    {
        pts3d[ii] = coordSys->localToGeocentricish(pts[ii]);
        
        // Check the normal (point in this case) against the eye vec
        if (pts3d[ii].dot(eyeVec) > 0.0)
            forwardFacing = true;
        
        CGPoint screenPt = [viewState pointOnScreenFromSphere:pts3d[ii] transform:&viewState->modelMatrix frameSize:frameSize];
        screenPts[ii] = Point2f(screenPt.x,screenPt.y);
    }
    
    // Look at area on the screen
    float area = 0.0;
    if (forwardFacing)
    {
        Point2f ac = screenPts[2]-screenPts[0];
        Point2f bd = screenPts[3]-screenPts[1];
        area = 0.5 * (ac.x()*bd.y()-bd.x()*ac.y());
    }
    
    if (boost::math::isnan(area))
        area = 0.0;
    
    //        if (area > 10000.0)
    //        {
    //            NSLog(@"Got one");
    //        }
    
    return std::abs(area);
}

    
// Calculate the max pixel size for a tile
float ScreenImportance(WhirlyGlobeViewState *viewState,WhirlyKit::Point2f frameSize,WhirlyKit::Point3f eyeVec,int pixelsSquare,WhirlyKit::CoordSystem *coordSys,Mbr nodeMbr)
{
    Point2f pixSize((nodeMbr.ur().x()-nodeMbr.ll().x())/pixelsSquare,(nodeMbr.ur().y()-nodeMbr.ll().y())/pixelsSquare);
    Point3f testPoints[5];
    testPoints[0] = Point3f(nodeMbr.ll().x(),nodeMbr.ll().y(),0.0);
    testPoints[1] = Point3f(nodeMbr.ur().x(),nodeMbr.ll().y(),0.0);
    testPoints[2] = Point3f(nodeMbr.ur().x(),nodeMbr.ur().y(),0.0);
    testPoints[3] = Point3f(nodeMbr.ll().x(),nodeMbr.ur().y(),0.0);
    testPoints[4] = (testPoints[0]+testPoints[2])/2.0;
    
    // Let's make sure we at least overlap the screen
    Mbr mbrOnScreen;
    for (unsigned int ii=0;ii<4;ii++)
    {
        Point3f pt3d = coordSys->localToGeocentricish(testPoints[ii]);
        
        CGPoint screenPt = [viewState pointOnScreenFromSphere:pt3d transform:&viewState->modelMatrix frameSize:frameSize];
        mbrOnScreen.addPoint(Point2f(screenPt.x,screenPt.y));
    }
    Mbr frameMbr(Point2f(0,0),Point2f(frameSize.x(),frameSize.y()));
    if (!mbrOnScreen.overlaps(frameMbr))
        return 0.0;
    
    float maxImport = 0.0;
    for (unsigned int ii=0;ii<5;ii++)
    {
        float thisImport = calcImportance(viewState,eyeVec,testPoints[ii],pixSize,frameSize,coordSys);
        maxImport = std::max(thisImport,maxImport);
    }
    
    return maxImport;
}

LoadedTile::LoadedTile()
{
    isOn = false;
    drawId = EmptyIdentity;
    texId = EmptyIdentity;
    for (unsigned int ii=0;ii<4;ii++)
    {
        childIsOn[ii] = false;
        childDrawIds[ii] = false;
    }
}

// Add the geometry and texture to the scene for a given tile
void LoadedTile::addToScene(WhirlyGlobeQuadDisplayLayer *layer,GlobeScene *scene)
{
    BasicDrawable *draw = NULL;
    Texture *tex = NULL;
    [layer buildTile:&nodeInfo draw:&draw tex:&tex texScale:Point2f(1.0,1.0) texOffset:Point2f(0.0,0.0) lines:layer.lineMode];
    drawId = draw->getId();
    if (tex)
        texId = tex->getId();
    else
        texId = EmptyIdentity;
    
    // Now for the changes to the scenegraph
    // Texture first, then drawable
    std::vector<ChangeRequest *> changeRequests;
    if (tex)
        changeRequests.push_back(new AddTextureReq(tex));
    changeRequests.push_back(new AddDrawableReq(draw));    
    scene->addChangeRequests(changeRequests);
    
    // Just in case, we don't have any child drawables here
    for (unsigned int ii=0;ii<4;ii++)
    {
        childIsOn[ii] = false;
        childDrawIds[ii] = EmptyIdentity;
    }
    
    isOn = true;
}

// Clean out the geometry and texture associated with the given tile
void LoadedTile::clearContents(WhirlyGlobeQuadDisplayLayer *layer,GlobeScene *scene,std::vector<ChangeRequest *> &changeRequests)
{
    if (drawId != EmptyIdentity)
    {
        changeRequests.push_back(new RemDrawableReq(drawId));
        drawId = EmptyIdentity;
    }
    if (texId != EmptyIdentity)
    {
        changeRequests.push_back(new RemTextureReq(texId));
        texId = EmptyIdentity;
    }
    for (unsigned int ii=0;ii<4;ii++)
        if (childDrawIds[ii] != EmptyIdentity)
        {
            changeRequests.push_back(new RemDrawableReq(childDrawIds[ii]));
        }
}

// Make sure a given tile overlaps the real world
bool isValidTile(Mbr theMbr)
{    
    Mbr realMbr(Point2f(-M_PI,-M_PI/2.0),Point2f(M_PI,M_PI/2.0));
    return theMbr.overlaps(realMbr);
}

// Update based on what children are doing
void LoadedTile::updateContents(Quadtree *tree,WhirlyGlobeQuadDisplayLayer *layer,std::vector<ChangeRequest *> &changeRequests)
{
    std::vector<Quadtree::Identifier> childIdents;
    tree->childrenForNode(nodeInfo.ident,childIdents);
        
    //    NSLog(@"Updating children for node (%d,%d,%d)",nodeInfo.ident.x,nodeInfo.ident.y,nodeInfo.ident.level);
    
    // If there are no children and it's not on, turn it on
    if (childIdents.size() == 0)
    {
        //        if (!isOn)
        {
            changeRequests.push_back(new OnOffChangeRequest(drawId,true));
            isOn = true;
        }
        
        // Also turn off any children that may have been on
        for (unsigned int ii=0;ii<4;ii++)
            //            if (childIsOn[ii])
        {
            changeRequests.push_back(new OnOffChangeRequest(childDrawIds[ii],false));
            childIsOn[ii] = false;
        }
    } else {
        // Turn off our representation, the fake children will do that work
        //        if (isOn)
        {
            changeRequests.push_back(new OnOffChangeRequest(drawId,false));
            isOn = false;
        }
        
        // Work through the possible children
        int whichChild = 0;
        for (unsigned int iy=0;iy<2;iy++)
            for (unsigned int ix=0;ix<2;ix++)
            {
                // Is it here?
                bool isPresent = false;
                Quadtree::Identifier childIdent(2*nodeInfo.ident.x+ix,2*nodeInfo.ident.y+iy,nodeInfo.ident.level+1);
                for (unsigned int jj=0;jj<childIdents.size();jj++)
                {
                    Quadtree::Identifier thisChildIdent = childIdents[jj];
                    if (thisChildIdent.x == childIdent.x &&
                        thisChildIdent.y == childIdent.y)
                    {
                        isPresent = true;
                        break;
                    }
                }
                
                // If it exists, make sure we're not representing it here
                if (isPresent)
                {
                    //                    NSLog(@"  Child present: (%d,%d,%d)",childIdent.x,childIdent.y,childIdent.level);
                    // Turn the child back off
                    if (childIsOn[whichChild])
                    {
                        changeRequests.push_back(new OnOffChangeRequest(childDrawIds[whichChild],false));
                        childIsOn[whichChild] = false;
                    } else {
                        if (childDrawIds[whichChild] != EmptyIdentity)
                        {
                            changeRequests.push_back(new RemDrawableReq(childDrawIds[whichChild]));
                            childDrawIds[whichChild] = EmptyIdentity;
                        }
                    }
                } else {
                    //                    NSLog(@"  Child missing: (%d,%d,%d)",childIdent.x,childIdent.y,childIdent.level);
                    
                    // It's not there, so make sure we're faking it with our texture
                    //                    if (!childIsOn[whichChild])
                    {
                        // May need to build the geometry
                        if (childDrawIds[whichChild] == EmptyIdentity)
                        {
                            Quadtree::NodeInfo childInfo = tree->generateNode(childIdent);
                            if (isValidTile(childInfo.mbr))
                            {
                                BasicDrawable *childDraw = NULL;
                                [layer buildTile:&childInfo draw:&childDraw tex:NULL texScale:Point2f(0.5,0.5) texOffset:Point2f(0.5*ix,0.5*iy) lines:((texId == EmptyIdentity)||layer.lineMode)];
                                childDrawIds[whichChild] = childDraw->getId();
                                if (!layer.lineMode && texId)
                                    childDraw->setTexId(texId);
                                changeRequests.push_back(new AddDrawableReq(childDraw));
                                childIsOn[whichChild] = true;
                            }
                        } else
                            // Just turn it on
                            changeRequests.push_back(new OnOffChangeRequest(childDrawIds[whichChild],true));
                    }
                }
                
                whichChild++;
            }
    }
    
    //    tree->Print();
}


void LoadedTile::Print(Quadtree *tree)
{
    NSLog(@"Node (%d,%d,%d), draw is %@, drawId = %d, texId = %d",nodeInfo.ident.x,nodeInfo.ident.y,nodeInfo.ident.level,(isOn ? @"on" : @"off"),(int)drawId,(int)texId);
    for (unsigned int ii=0;ii<4;ii++)
    {
        NSLog(@" Child %d is %@, drawId = %d",ii,(childIsOn[ii] ? @"on" : @"off"),(int)childDrawIds[ii]);
    }
    std::vector<Quadtree::Identifier> childIdents;
    tree->childrenForNode(nodeInfo.ident,childIdents);
    for (unsigned int ii=0;ii<childIdents.size();ii++)
        NSLog(@" Query child (%d,%d,%d)",childIdents[ii].x,childIdents[ii].y,childIdents[ii].level);
}
    
}

@implementation WhirlyGlobeQuadDisplayLayer

@synthesize coordSys;
@synthesize mbr;
@synthesize maxTiles;
@synthesize minTileArea;
@synthesize lineMode;
@synthesize drawEmpty;
@synthesize debugMode;
@synthesize dataSource;
@synthesize scene;
@synthesize viewUpdatePeriod;

- (id)initWithDataSource:(NSObject<WhirlyGlobeQuadDataSource> *)inDataSource renderer:(WhirlyKitSceneRendererES1 *)inRenderer;
{
    self = [super init];
    if (self)
    {
        dataSource = inDataSource;
        coordSys = [dataSource coordSystem];
        mbr = [dataSource validExtents];
        minZoom = [dataSource minZoom];
        maxZoom = [dataSource maxZoom];
        maxTiles = 100;
        minTileArea = 1.0;
        viewUpdatePeriod = 1.0;
        quadtree = new Quadtree([dataSource totalExtents],minZoom,maxZoom,maxTiles,minTileArea,self);
        renderer = inRenderer;
        lineMode = false;
        drawEmpty = false;
        debugMode = false;
    }
    
    return self;
}

- (void)dealloc
{
    for (LoadedTileSet::iterator it = tileSet.begin();
         it != tileSet.end(); ++it)
        delete *it;
    tileSet.clear();
    
    if (quadtree)
        delete quadtree;
    if (layerThread.viewWatcher)
        [(WhirlyGlobeLayerViewWatcher *)layerThread.viewWatcher removeWatcherTarget:self selector:@selector(viewUpdate:)];
}

- (void)startWithThread:(WhirlyKitLayerThread *)inLayerThread scene:(Scene *)inScene
{
    layerThread = inLayerThread;
	scene = (GlobeScene *)inScene;
        
    // We want view updates, but only 1s in frequency
    if (layerThread.viewWatcher)
        [(WhirlyGlobeLayerViewWatcher *)layerThread.viewWatcher addWatcherTarget:self selector:@selector(viewUpdate:) minTime:viewUpdatePeriod];
}

// Called every so often by the view watcher
// It's here that we evaluate what to load
- (void)viewUpdate:(WhirlyGlobeViewState *)inViewState
{
    viewState = inViewState;
    nodesForEval.clear();
    quadtree->reevaluateNodes();
    
    // Add everything at the minLevel back in
    for (int ix=0;ix<1<<minZoom;ix++)
        for (int iy=0;iy<1<<minZoom;iy++)
        {
            Quadtree::NodeInfo thisNode = quadtree->generateNode(Quadtree::Identifier(ix,iy,minZoom));
            nodesForEval.insert(thisNode);
        }
    
//    NSLog(@"View update called: (eye) = (%f,%f,%f), nodesForEval = %lu",eyeVec3.x(),eyeVec3.y(),eyeVec3.z(),nodesForEval.size());
    [self performSelector:@selector(evalStep:) withObject:nil afterDelay:0.0];
}

// Dump out info about what we've got loaded in
- (void)dumpInfo
{
    NSLog(@"***Loaded Tiles***");
    for (LoadedTileSet::iterator it = tileSet.begin();
         it != tileSet.end(); ++it)
    {
        (*it)->Print(quadtree);
    }
    NSLog(@"******");
    
    quadtree->Print();
}

// Run the evaluation step for outstanding nodes
- (void)evalStep:(id)Sender
{
    if (nodesForEval.empty())
        return;
    
    // We'll gather up all the changes for this step
    // This avoids flashing
    std::vector<ChangeRequest *> changeRequests;
    
    // Grab the first node.
    QuadNodeInfoSet::iterator nodeIt = nodesForEval.end();
    nodeIt--;
    Quadtree::NodeInfo nodeInfo = *nodeIt;
    nodesForEval.erase(nodeIt);
    
    // The quad tree will take this node over an existing one
    bool isLoaded = quadtree->isTileLoaded(nodeInfo.ident);
    if (isLoaded || quadtree->willAcceptTile(nodeInfo))
    {
        if (!isLoaded)
        {
            // Tell the quad tree what we're up to
            std::vector<Quadtree::Identifier> tilesToRemove;
            quadtree->addTile(nodeInfo, tilesToRemove);
            
            // Parents to update after these changes
            std::set<Quadtree::Identifier> parents;
            
            // Build the new tile
            LoadedTile *newTile = new LoadedTile();
            newTile->nodeInfo = nodeInfo;
            newTile->addToScene(self,scene);
            Quadtree::Identifier parentId;
            if (quadtree->hasParent(nodeInfo.ident,parentId))
                parents.insert(parentId);
                tileSet.insert(newTile);
                
                // Remove the old tiles
                if (!tilesToRemove.empty())
                    for (unsigned int ii=0;ii<tilesToRemove.size();ii++)
                    {
                        Quadtree::Identifier &thisIdent = tilesToRemove[ii];
                        //                    NSLog(@"Quad tree removed (%d,%d,%d)",thisIdent.x,thisIdent.y,thisIdent.level);
                        
                        if (quadtree->hasParent(thisIdent,parentId))
                            parents.insert(parentId);
                            
                        // Get rid of the old one
                        LoadedTile dummyTile;
                        dummyTile.nodeInfo.ident = thisIdent;
                        LoadedTileSet::iterator it = tileSet.find(&dummyTile);
                        if (it != tileSet.end())
                        {
                            LoadedTile *theTile = *it;
                            
                            // Note: Debugging check
                            std::vector<Quadtree::Identifier> childIDs;
                            quadtree->childrenForNode(theTile->nodeInfo.ident, childIDs);
                            if (childIDs.size() > 0)
                                NSLog(@" *** Deleting node with children *** ");
                                
                                theTile->clearContents(self,scene,changeRequests);
                                tileSet.erase(it);
                                delete theTile;
                        }
                    }
//            NSLog(@"Quad loaded node (%d,%d,%d) = %.4f",nodeInfo.ident.x,nodeInfo.ident.y,nodeInfo.ident.level,nodeInfo.importance);
            
            // Let the parent nodes know they need to update
#if 0
            for (std::set<Quadtree::Identifier>::iterator it = parents.begin();
                 it != parents.end(); ++it)
            {
                LoadedMBTile dummyTile;
                dummyTile.nodeInfo.ident = *it;
                LoadedMBTileSet::iterator lit = tileSet.find(&dummyTile);
                if (lit != tileSet.end())
                {
                    LoadedMBTile *theTile = *lit;
                    theTile->updateContents(quadtree,self, changeRequests);
                }
            }
#endif
            for (LoadedTileSet::iterator it = tileSet.begin();
                 it != tileSet.end(); ++it)
            {
                LoadedTile *theTile = *it;
                theTile->updateContents(quadtree, self, changeRequests);
            }
        }
        
        if (nodeInfo.ident.level < maxZoom)
        {
            // Now try the children
            std::vector<Quadtree::NodeInfo> childNodes;
            quadtree->generateChildren(nodeInfo.ident, childNodes);
            nodesForEval.insert(childNodes.begin(),childNodes.end());
        }
    } else
    {
        //        NSLog(@"Quad rejecting node (%d,%d,%d) = %.4f",nodeInfo.ident.x,nodeInfo.ident.y,nodeInfo.ident.level,nodeInfo.importance);
    }

    // Flush out all the changes at once for this step
    if (!changeRequests.empty())
    {
        scene->addChangeRequests(changeRequests);
    }
    
    if (debugMode)
        [self dumpInfo];
    
    [self performSelector:@selector(evalStep:) withObject:nil afterDelay:0.0];
}

// Tesselation for each chunk of the sphere
const int SphereTessX = 10, SphereTessY = 10;

- (void)buildTile:(Quadtree::NodeInfo *)nodeInfo draw:(BasicDrawable **)draw tex:(Texture **)tex texScale:(Point2f)texScale texOffset:(Point2f)texOffset lines:(bool)buildLines
{
    Mbr theMbr = nodeInfo->mbr;

    // Make sure this overlaps the area we care about
    if (!theMbr.overlaps(mbr))
    {
        NSLog(@"Building bogus tile: (%d,%d,%d)",nodeInfo->ident.x,nodeInfo->ident.y,nodeInfo->ident.level);
    }
    
    // Snap to the designated area
    if (theMbr.ll().x() < mbr.ll().x())
        theMbr.ll().x() = mbr.ll().x();
    if (theMbr.ur().x() > mbr.ur().x())
        theMbr.ur().x() = mbr.ur().x();
    if (theMbr.ll().y() < mbr.ll().y())
        theMbr.ll().y() = mbr.ll().y();
    if (theMbr.ur().y() > mbr.ur().y())
        theMbr.ur().y() = mbr.ur().y();

    // Number of pieces at this level
    int xDim = 1<<nodeInfo->ident.level;
    int yDim = 1<<nodeInfo->ident.level;
    
    //    NSLog(@"Chunk ll = (%.4f,%.4f)  ur = (%.4f,%.4f)",mbr.ll().x(),mbr.ll().y(),mbr.ur().x(),mbr.ur().y());
    
    // Size of each chunk
    Point2f chunkSize = theMbr.ur() - theMbr.ll();
    
    // Unit size of each tesselation in spherical mercator
    Point2f incr(chunkSize.x()/SphereTessX,chunkSize.y()/SphereTessY);
    
    // Texture increment for each tesselation
    TexCoord texIncr(1.0/(float)SphereTessX,1.0/(float)SphereTessY);
    
	// We're viewing this as a parameterization from ([0->1.0],[0->1.0]) so we'll
	//  break up these coordinates accordingly
    Point2f paramSize(1.0/(xDim*SphereTessX),1.0/(yDim*SphereTessY));
    
    // We need the corners in geographic for the cullable
    Point2f chunkLL = theMbr.ll();
    Point2f chunkUR = theMbr.ur();
    GeoCoord geoLL(coordSys->localToGeographic(Point3f(chunkLL.x(),chunkLL.y(),0.0)));
    GeoCoord geoUR(coordSys->localToGeographic(Point3f(chunkUR.x(),chunkUR.y(),0.0)));
    
    // Get texture (locally)
    if (tex)
    {
        NSData *texData = [dataSource fetchImageForLevel:nodeInfo->ident.level col:nodeInfo->ident.x row:nodeInfo->ident.y];
        if (texData)
        {
            UIImage *texImage = [UIImage imageWithData:texData];
            if (texImage)
            {
                // Create the texture and set it up in OpenGL
                Texture *newTex = new Texture(texImage);
                [EAGLContext setCurrentContext:layerThread.glContext];
                newTex->createInGL();
                *tex = newTex;
            }
        } else
            *tex = NULL;
    }
    
    if (draw)
    {
        // We'll set up and fill in the drawable
        BasicDrawable *chunk = new BasicDrawable((SphereTessX+1)*(SphereTessY+1),2*SphereTessX*SphereTessY);
        //	chunk->setType(GL_POINTS);
        // Note: Put this back
//        chunk->setGeoMbr(GeoMbr(geoLL,geoUR));
        chunk->setGeoMbr(GeoMbr(GeoCoord::CoordFromDegrees(-180, -90),GeoCoord::CoordFromDegrees(180, 90)));
        
        // We're in line mode or the texture didn't load
        if (buildLines || (tex && !(*tex)))
        {
            chunk->setType(GL_LINES);
            
            // Two lines per cell
            for (unsigned int iy=0;iy<SphereTessY;iy++)
                for (unsigned int ix=0;ix<SphereTessX;ix++)
                {
                    Point3f org3D = coordSys->localToGeocentricish(Point3f(chunkLL.x()+ix*incr.x(),chunkLL.y()+iy*incr.y(),0.0));
                    Point3f ptA_3D = coordSys->localToGeocentricish(Point3f(chunkLL.x()+(ix+1)*incr.x(),chunkLL.y()+iy*incr.y(),0.0));
                    Point3f ptB_3D = coordSys->localToGeocentricish(Point3f(chunkLL.x()+ix*incr.x(),chunkLL.y()+(iy+1)*incr.y(),0.0));
                    
                    TexCoord texCoord(ix*texIncr.x()*texScale.x()+texOffset.x(),1.0-(iy*texIncr.y()*texScale.y()+texOffset.y()));
                    
                    chunk->addPoint(org3D);
                    chunk->addNormal(org3D);
                    chunk->addTexCoord(texCoord);
                    chunk->addPoint(ptA_3D);
                    chunk->addNormal(ptA_3D);
                    chunk->addTexCoord(texCoord);
                    
                    chunk->addPoint(org3D);
                    chunk->addNormal(org3D);
                    chunk->addTexCoord(texCoord);
                    chunk->addPoint(ptB_3D);
                    chunk->addNormal(ptB_3D);
                    chunk->addTexCoord(texCoord);
                }
        } else {
            chunk->setType(GL_TRIANGLES);
            // Generate point, texture coords, and normals
            for (unsigned int iy=0;iy<SphereTessY+1;iy++)
                for (unsigned int ix=0;ix<SphereTessX+1;ix++)
                {
                    Point3f loc3D = coordSys->localToGeocentricish(Point3f(chunkLL.x()+ix*incr.x(),chunkLL.y()+iy*incr.y(),0.0));
                    
                    // Do the texture coordinate seperately
                    TexCoord texCoord(ix*texIncr.x()*texScale.x()+texOffset.x(),1.0-(iy*texIncr.y()*texScale.y()+texOffset.y()));
                    
                    chunk->addPoint(loc3D);
                    chunk->addTexCoord(texCoord);
                    chunk->addNormal(loc3D);
                }
            
            // Two triangles per cell
            for (unsigned int iy=0;iy<SphereTessY;iy++)
            {
                for (unsigned int ix=0;ix<SphereTessX;ix++)
                {
                    BasicDrawable::Triangle triA,triB;
                    triA.verts[0] = iy*(SphereTessX+1)+ix;
                    triA.verts[1] = iy*(SphereTessX+1)+(ix+1);
                    triA.verts[2] = (iy+1)*(SphereTessX+1)+(ix+1);
                    triB.verts[0] = triA.verts[0];
                    triB.verts[1] = triA.verts[2];
                    triB.verts[2] = (iy+1)*(SphereTessX+1)+ix;
                    chunk->addTriangle(triA);
                    chunk->addTriangle(triB);
                }
            }
            
            if (tex && *tex)
                chunk->setTexId((*tex)->getId());
        }
        
        *draw = chunk;
    }    
}

#pragma mark - Quad Tree Importance Delegate

- (float)importanceForTile:(WhirlyKit::Quadtree::Identifier)ident mbr:(Mbr)theMbr tree:(WhirlyKit::Quadtree *)tree
{
    return [dataSource importanceForTile:ident mbr:theMbr viewInfo:viewState frameSize:Point2f(renderer.framebufferWidth,renderer.framebufferHeight)];
}

@end

