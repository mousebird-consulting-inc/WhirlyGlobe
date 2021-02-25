/*
 *  QuadTileBuilder.mm
 *  WhirlyGlobeLib
 *
 *  Created by Steve Gifford on 3/29/18.
 *  Copyright 2012-2018 Saildrone Inc
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

#import "QuadTileBuilder.h"
#import "LoadedTileNew.h"
#import "WhirlyKitLog.h"

namespace WhirlyKit
{
    
QuadTileBuilderDelegate::QuadTileBuilderDelegate()
{
}
    
QuadTileBuilderDelegate::~QuadTileBuilderDelegate()
{
}
    
QuadTileBuilder::QuadTileBuilder(CoordSystemRef coordSys,QuadTileBuilderDelegate *delegate)
    : delegate(delegate), debugMode(false)
{
    geomSettings.sampleX = 20;
    geomSettings.sampleY = 20;
    geomSettings.topSampleX = 30;
    geomSettings.topSampleY = 40;
    geomSettings.enableGeom = false;
    geomSettings.singleLevel = false;
    geomManage.coordSys = coordSys;
}
    
QuadTileBuilder::~QuadTileBuilder()
{
}

TileBuilderDelegateInfo QuadTileBuilder::getLoadingState()
{
    TileBuilderDelegateInfo info;
    
    info.loadTiles = geomManage.getAllTiles();
    
    return info;
}
    
void QuadTileBuilder::setBuildGeom(bool newVal)
{
    geomSettings.buildGeom = newVal;
}

bool QuadTileBuilder::getBuildGeom() const
{
    return geomSettings.buildGeom;
}
    
void QuadTileBuilder::setCoverPoles(bool coverPoles)
{
    geomManage.coverPoles = coverPoles;
}

bool QuadTileBuilder::getCoverPoles() const
{
    return geomManage.coverPoles;
}

void QuadTileBuilder::setEdgeMatching(bool edgeMatch)
{
    geomManage.buildSkirts = edgeMatch;
}

bool QuadTileBuilder::getEdgeMatching() const
{
    return geomManage.buildSkirts;
}

void QuadTileBuilder::setBaseDrawPriority(int baseDrawPriority)
{
    geomSettings.baseDrawPriority = baseDrawPriority;
}

int QuadTileBuilder::getBaseDrawPriority() const
{
    return geomSettings.baseDrawPriority;
}

void QuadTileBuilder::setDrawPriorityPerLevel(int drawPriorityPerLevel)
{
    geomSettings.drawPriorityPerLevel = drawPriorityPerLevel;
}

int QuadTileBuilder::getDrawPriorityPerLevel() const
{
    return geomSettings.drawPriorityPerLevel;
}

void QuadTileBuilder::setSingleLevel(bool singleLevel)
{
    geomSettings.singleLevel = singleLevel;
}

bool QuadTileBuilder::getSingleLevel() const
{
    return geomSettings.singleLevel;
}
    
void QuadTileBuilder::setColor(const RGBAColor &color)
{
    geomSettings.color = color;
}

const RGBAColor &QuadTileBuilder::getColor() const
{
    return geomSettings.color;
}
    
void QuadTileBuilder::setShaderID(SimpleIdentity programID)
{
    geomSettings.programID = programID;
}

SimpleIdentity QuadTileBuilder::getProgramID() const
{
    return geomSettings.programID;
}
    
void QuadTileBuilder::setDebugMode(bool newMode)
{
    debugMode = newMode;
}

bool QuadTileBuilder::getDebugMode() const
{
    return debugMode;
}

LoadedTileNewRef QuadTileBuilder::getLoadedTile(const QuadTreeNew::Node &ident)
{
    return geomManage.getTile(ident);
}

void QuadTileBuilder::setController(QuadDisplayControllerNew *inControl)
{
    QuadLoaderNew::setController(inControl);
    
    MbrD mbr = MbrD(control->getDataStructure()->getValidExtents());
    geomManage.setup(inControl->getRenderer(),geomSettings,control, control->getScene()->getCoordAdapter(),geomManage.coordSys,mbr);
    
    delegate->setBuilder(this,control);
}
    
/// Load some tiles, unload others, and the rest had their importance values change
/// Return the nodes we wanted to keep rather than delete
QuadTreeNew::NodeSet QuadTileBuilder::quadLoaderUpdate(PlatformThreadInfo *threadInfo,const WhirlyKit::QuadTreeNew::ImportantNodeSet &loadTiles,const WhirlyKit::QuadTreeNew::NodeSet &unloadTiles,const WhirlyKit::QuadTreeNew::ImportantNodeSet &updateTiles,int targetLevel, ChangeSet &changes)
{
    TileBuilderDelegateInfo info;
    info.unloadTiles = unloadTiles;
    info.changeTiles = updateTiles;
    
    QuadTreeNew::NodeSet toKeep;
    if (!unloadTiles.empty()) {
        toKeep = delegate->builderUnloadCheck(this,loadTiles,unloadTiles,targetLevel);
        // Remove the keep nodes and add them to update with very little importance
        for (const QuadTreeNew::Node &node: toKeep) {
            info.unloadTiles.erase(node);
            info.changeTiles.insert(QuadTreeNew::ImportantNode(node,0.0));
        }
    }
    
    // Have the geometry manager add/remove the tiles and deal with changes
    auto tileChanges = geomManage.addRemoveTiles(loadTiles,info.unloadTiles,changes);
    
    // Tell the delegate what we're up to
    info.targetLevel = targetLevel;
    info.loadTiles = tileChanges.addedTiles;
    info.enableTiles = tileChanges.enabledTiles;
    info.disableTiles = tileChanges.disabledTiles;
    delegate->builderLoad(threadInfo, this, info, changes);
    
    if (debugMode)
    {
        wkLogLevel(Verbose,"----- Tiles to add ------");
        for (auto tile : loadTiles)
            wkLogLevel(Verbose,"  %d: (%d,%d)",tile.level,tile.x,tile.y);
        wkLogLevel(Verbose,"----- Tiles to remove ------");
        for (auto tile : unloadTiles)
            wkLogLevel(Verbose,"  %d: (%d,%d)",tile.level,tile.x,tile.y);
        wkLogLevel(Verbose,"----- Tiles that changed importance ------");
        for (auto tile : updateTiles)
            wkLogLevel(Verbose,"  %d: (%d,%d)",tile.level,tile.x,tile.y);
        wkLogLevel(Verbose,"----- Nodes to enable ------");
        for (auto tile : tileChanges.enabledTiles)
            wkLogLevel(Verbose,"  %d: (%d,%d)",tile->ident.level,tile->ident.x,tile->ident.y);
        wkLogLevel(Verbose,"----- Nodes to disable ------");
        for (auto tile : tileChanges.disabledTiles)
            wkLogLevel(Verbose,"  %d: (%d,%d)",tile->ident.level,tile->ident.x,tile->ident.y);
        wkLogLevel(Verbose,"----- Tiles to keep -----");
        for (auto tile : toKeep)
            wkLogLevel(Verbose,"  %d: (%d,%d)",tile.level,tile.x,tile.y);
        wkLogLevel(Verbose,"----- ------------- ------");
    }
    
    // We need the layer flush to run if we're holding on to nodes
    if (!toKeep.empty() && changes.empty())
        changes.push_back(NULL);
    
    // Caller will flush out any visual changes

    return toKeep;
}

/// Called right before the layer thread flushes its change requests
void QuadTileBuilder::quadLoaderPreSceenFlush(ChangeSet &changes)
{
    delegate->builderPreSceneFlush(this,changes);
}

/// Called when a layer is shutting down (on the layer thread)
void QuadTileBuilder::quadLoaderShutdown(PlatformThreadInfo *threadInfo,ChangeSet &changes)
{
    geomManage.cleanup(changes);
    delegate->builderShutdown(threadInfo,this,changes);

    delegate = NULL;
}

}
