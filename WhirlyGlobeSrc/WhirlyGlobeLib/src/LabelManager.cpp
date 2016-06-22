/*
 *  LabelManager.mm
 *  WhirlyGlobeLib
 *
 *  Created by Steve Gifford on 2/7/11.
 *  Copyright 2011-2016 mousebird consulting
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

#import "LabelRenderer.h"
#import "WhirlyGeometry.h"
#import "GlobeMath.h"
#import "ScreenSpaceBuilder.h"
#import "FontTextureManager.h"

#import "LabelManager.h"

using namespace Eigen;

namespace WhirlyKit
{

SingleLabel::SingleLabel()
    : isSelectable(true), selectID(EmptyIdentity), loc(0,0), rotation(0), iconTexture(EmptyIdentity),
    iconSize(0,0), screenOffset(0,0), layoutImportance(0.0)
{
}

//- (bool)calcWidth:(float *)width height:(float *)height defaultFont:(UIFont *)font
//{
//    CGSize textSize = [_text sizeWithFont:font];
//    if (textSize.width == 0 || textSize.height == 0)
//        return false;
//    
//    if (*width != 0.0)
//        *height = *width * textSize.height / ((float)textSize.width);
//    else
//        *width = *height * textSize.width / ((float)textSize.height);
//    
//    return true;
//}
//
//// Calculate the corners in this order:  (ll,lr,ur,ul)
//- (void)calcExtents2:(float)width2 height2:(float)height2 iconSize:(Point2f)theIconSize justify:(WhirlyKitLabelJustify)justify corners:(Point3f *)pts norm:(Point3f *)norm iconCorners:(Point3f *)iconPts coordAdapter:(WhirlyKit::CoordSystemDisplayAdapter *)coordAdapter
//{
//    Point3f center = coordAdapter->localToDisplay(coordAdapter->getCoordSystem()->geographicToLocal(_loc));
//    Point3f up(0,0,1);
//    Point3f horiz,vert;
//    if (coordAdapter->isFlat())
//    {
//        *norm = up;
//        horiz = Point3f(1,0,0);
//        vert = Point3f(0,1,0);
//    } else {
//        *norm = center;
//        horiz = up.cross(*norm).normalized();
//        vert = norm->cross(horiz).normalized();;
//    }
//    Point3f ll;
//    
//    
//    switch (justify)
//    {
//        case WhirlyKitLabelLeft:
//            ll = center + theIconSize.x() * horiz - height2 * vert;
//            break;
//        case WhirlyKitLabelMiddle:
//            ll = center - (width2 + theIconSize.x()/2) * horiz - height2 * vert;
//            break;
//        case WhirlyKitLabelRight:
//            ll = center - 2*width2 * horiz - height2 * vert;
//            break;
//    }
//    pts[0] = ll;
//    pts[1] = ll + 2*width2 * horiz;
//    pts[2] = ll + 2*width2 * horiz + 2 * height2 * vert;
//    pts[3] = ll + 2 * height2 * vert;
//    
//    // Now add the quad for the icon
//    switch (justify)
//    {
//        case WhirlyKitLabelLeft:
//            ll = center - height2*vert;
//            break;
//        case WhirlyKitLabelMiddle:
//            ll = center - (width2 + theIconSize.x()) * horiz - height2*vert;
//            break;
//        case WhirlyKitLabelRight:
//            ll = center - (2*width2 + theIconSize.x()) * horiz - height2*vert;
//            break;
//    }
//    iconPts[0] = ll;
//    iconPts[1] = ll + theIconSize.x()*horiz;
//    iconPts[2] = ll + theIconSize.x()*horiz + theIconSize.y()*vert;
//    iconPts[3] = ll + theIconSize.y()*vert;
//}
//
//// This version calculates extents for a screen space label
//- (void)calcScreenExtents2:(float)width2 height2:(float)height2 iconSize:(Point2f)theIconSize justify:(WhirlyKitLabelJustify)justify corners:(Point3f *)pts iconCorners:(Point3f *)iconPts useIconOffset:(bool)useIconOffset
//{
//    Point3f center(0,0,0);
//    Point3f ll;
//    Point3f horiz = Point3f(1,0,0);
//    Point3f vert = Point3f(0,1,0);
//    
//    Point2f iconSizeForLabel = (useIconOffset ? theIconSize : Point2f(0,0));
//    switch (justify)
//    {
//        case WhirlyKitLabelLeft:
//            ll = center + iconSizeForLabel.x() * horiz - height2 * vert;
//            break;
//        case WhirlyKitLabelMiddle:
//            ll = center - (width2 + iconSizeForLabel.x()/2) * horiz - height2 * vert;
//            break;
//        case WhirlyKitLabelRight:
//            ll = center - 2*width2 * horiz - height2 * vert;
//            break;
//    }
//    pts[0] = ll;
//    pts[1] = ll + 2*width2 * horiz;
//    pts[2] = ll + 2*width2 * horiz + 2 * height2 * vert;
//    pts[3] = ll + 2 * height2 * vert;
//    
//    // Now add the quad for the icon
//    switch (justify)
//    {
//        case WhirlyKitLabelLeft:
//            ll = center - height2*vert;
//            break;
//        case WhirlyKitLabelMiddle:
//            ll = center - (width2 + iconSizeForLabel.x()) * horiz - height2*vert;
//            break;
//        case WhirlyKitLabelRight:
//            ll = center - (2*width2 + iconSizeForLabel.x()) * horiz - height2*vert;
//            break;
//    }
//    iconPts[0] = ll;
//    iconPts[1] = ll + iconSizeForLabel.x()*horiz;
//    iconPts[2] = ll + iconSizeForLabel.x()*horiz + iconSizeForLabel.y()*vert;
//    iconPts[3] = ll + iconSizeForLabel.y()*vert;
//}
//
//- (void)calcExtents:(NSDictionary *)topDesc corners:(Point3f *)pts norm:(Point3f *)norm coordAdapter:(CoordSystemDisplayAdapter *)coordAdapter
//{
//    WhirlyKitLabelInfo *labelInfo = [[WhirlyKitLabelInfo alloc] initWithStrs:[NSArray arrayWithObject:self.text] desc:topDesc];
//    
//    // Width and height can be overriden per label
//    float theWidth = labelInfo.width;
//    float theHeight = labelInfo.height;
//    if (_desc)
//    {
//        theWidth = [_desc floatForKey:@"width" default:theWidth];
//        theHeight = [_desc floatForKey:@"height" default:theHeight];
//    }
//    
//    CGSize textSize = [_text sizeWithFont:labelInfo.font];
//    
//    float width2,height2;
//    if (theWidth != 0.0)
//    {
//        height2 = theWidth * textSize.height / ((float)2.0 * textSize.width);
//        width2 = theWidth/2.0;
//    } else {
//        width2 = theHeight * textSize.width / ((float)2.0 * textSize.height);
//        height2 = theHeight/2.0;
//    }
//    
//    // If there's an icon, we need to offset the label
//    Point2f theIconSize = (_iconTexture==EmptyIdentity ? Point2f(0,0) : Point2f(2*height2,2*height2));
//    
//    Point3f corners[4],iconCorners[4];
//    [self calcExtents2:width2 height2:height2 iconSize:theIconSize justify:labelInfo.justify corners:corners norm:norm iconCorners:iconCorners coordAdapter:coordAdapter];
//    
//    // If we have an icon, we need slightly different corners
//    if (_iconTexture)
//    {
//        pts[0] = iconCorners[0];
//        pts[1] = corners[1];
//        pts[2] = corners[2];
//        pts[3] = iconCorners[3];
//    } else {
//        pts[0] = corners[0];
//        pts[1] = corners[1];
//        pts[2] = corners[2];
//        pts[3] = corners[3];
//    }
//}
    
LabelManager::LabelManager()
    : textureAtlasSize(LabelTextureAtlasSizeDefault)
{
    pthread_mutex_init(&labelLock, NULL);
}
    
LabelManager::~LabelManager()
{
    pthread_mutex_destroy(&labelLock);
}
    
SimpleIdentity LabelManager::addLabels(std::vector<SingleLabel *> &labels,const LabelInfo &labelInfo,ChangeSet &changes)
{
    CoordSystemDisplayAdapter *coordAdapter = scene->getCoordAdapter();

    // Set up the representation (but then hand it off)
    LabelSceneRep *labelRep = new LabelSceneRep();
    if (labelInfo.fadeOut > 0.0 && labelInfo.fadeOutTime != 0.0)
        labelRep->fadeOut = labelInfo.fadeOut;
    else
        labelRep->fadeOut = 0.0;

    FontTextureManager *fontTexManager = scene->getFontTextureManager();
    
    // Set up the label renderer
    LabelRenderer labelRenderer(scene,fontTexManager,&labelInfo);
    labelRenderer.textureAtlasSize = textureAtlasSize;
    labelRenderer.coordAdapter = scene->getCoordAdapter();
    labelRenderer.labelRep = labelRep;
    labelRenderer.scene = scene;
    labelRenderer.fontTexManager = (labelInfo.screenObject ? fontTexManager : NULL);
    labelRenderer.scale = renderer->getScale();
   
    labelRenderer.render(labels, changes);
    
    changes.insert(changes.end(),labelRenderer.changeRequests.begin(), labelRenderer.changeRequests.end());

    SelectionManager *selectManager = (SelectionManager *)scene->getManager(kWKSelectionManager);
    LayoutManager *layoutManager = (LayoutManager *)scene->getManager(kWKLayoutManager);
    
    // Create screen shapes
    if (!labelRenderer.screenObjects.empty())
    {
        ScreenSpaceBuilder ssBuild(coordAdapter,renderer->getScale());
        for (unsigned int ii=0;ii<labelRenderer.screenObjects.size();ii++)
            ssBuild.addScreenObject(labelRenderer.screenObjects[ii]);
        ssBuild.flushChanges(changes, labelRep->drawIDs);
    }
    
    // Hand over some to the layout manager
    if (layoutManager && !labelRenderer.layoutObjects.empty())
    {
        for (unsigned int ii=0;ii<labelRenderer.layoutObjects.size();ii++)
            labelRep->layoutIDs.insert(labelRenderer.layoutObjects[ii].getId());
        layoutManager->addLayoutObjects(labelRenderer.layoutObjects);
    }
    
    // Pass on selection data
    if (selectManager)
    {
        for (unsigned int ii=0;ii<labelRenderer.selectables2D.size();ii++)
        {
            std::vector<WhirlyKit::RectSelectable2D> &selectables2D = labelRenderer.selectables2D;
            RectSelectable2D &sel = selectables2D[ii];
            selectManager->addSelectableScreenRect(sel.selectID,sel.center,sel.pts,sel.minVis,sel.maxVis,sel.enable);
            labelRep->selectIDs.insert(sel.selectID);
        }
        for (unsigned int ii=0;ii<labelRenderer.movingSelectables2D.size();ii++)
        {
            std::vector<WhirlyKit::MovingRectSelectable2D> &movingSelectables2D = labelRenderer.movingSelectables2D;
            MovingRectSelectable2D &sel = movingSelectables2D[ii];
            selectManager->addSelectableMovingScreenRect(sel.selectID,sel.center,sel.endCenter,sel.startTime,sel.endTime,sel.pts,sel.minVis,sel.maxVis,sel.enable);
            labelRep->selectIDs.insert(sel.selectID);
        }
        for (unsigned int ii=0;ii<labelRenderer.selectables3D.size();ii++)
        {
            std::vector<WhirlyKit::RectSelectable3D> &selectables3D = labelRenderer.selectables3D;
            RectSelectable3D &sel = selectables3D[ii];
            selectManager->addSelectableRect(sel.selectID,sel.pts,sel.minVis,sel.maxVis,sel.enable);
            labelRep->selectIDs.insert(sel.selectID);
        }
    }

    SimpleIdentity labelID = labelRep->getId();
    pthread_mutex_lock(&labelLock);
    labelReps.insert(labelRep);
    pthread_mutex_unlock(&labelLock);
    
    return labelID;
}

//void LabelManager::changeLabel(SimpleIdentity labelID,NSDictionary *desc,ChangeSet &changes)
//{
//    WhirlyKitLabelInfo *labelInfo = [[WhirlyKitLabelInfo alloc] initWithStrs:nil desc:desc];
//    
//    pthread_mutex_lock(&labelLock);
//    
//    LabelSceneRep dummyRep(labelID);
//    LabelSceneRepSet::iterator it = labelReps.find(&dummyRep);
//    
//    if (it != labelReps.end())
//    {
//        LabelSceneRep *sceneRep = *it;
//        
//        for (SimpleIDSet::iterator idIt = sceneRep->drawIDs.begin();
//             idIt != sceneRep->drawIDs.end(); ++idIt)
//        {
//            // Changed visibility
//            changes.push_back(new VisibilityChangeRequest(*idIt, labelInfo.minVis, labelInfo.maxVis));
//        }
//    }
//
//    pthread_mutex_unlock(&labelLock);
//}
    
void LabelManager::enableLabels(SimpleIDSet labelIDs,bool enable,ChangeSet &changes)
{
    SelectionManager *selectManager = (SelectionManager *)scene->getManager(kWKSelectionManager);
    LayoutManager *layoutManager = (LayoutManager *)scene->getManager(kWKLayoutManager);
    
    pthread_mutex_lock(&labelLock);

    for (SimpleIDSet::iterator lit = labelIDs.begin(); lit != labelIDs.end(); ++lit)
    {
        LabelSceneRep dummyRep(*lit);
        LabelSceneRepSet::iterator it = labelReps.find(&dummyRep);
        if (it != labelReps.end())
        {
            LabelSceneRep *sceneRep = *it;
            for (SimpleIDSet::iterator idIt = sceneRep->drawIDs.begin();
                 idIt != sceneRep->drawIDs.end(); ++idIt)
                changes.push_back(new OnOffChangeRequest(*idIt,enable));
            if (!sceneRep->selectIDs.empty() && selectManager)
                selectManager->enableSelectables(sceneRep->selectIDs, enable);
            if (!sceneRep->layoutIDs.empty() && layoutManager)
                layoutManager->enableLayoutObjects(sceneRep->layoutIDs,enable);
        }
    }
    
    pthread_mutex_unlock(&labelLock);
}


void LabelManager::removeLabels(SimpleIDSet &labelIDs,ChangeSet &changes)
{
    SelectionManager *selectManager = (SelectionManager *)scene->getManager(kWKSelectionManager);
    LayoutManager *layoutManager = (LayoutManager *)scene->getManager(kWKLayoutManager);
    FontTextureManager *fontTexManager = scene->getFontTextureManager();
    
    pthread_mutex_lock(&labelLock);
    
    for (SimpleIDSet::iterator lit = labelIDs.begin(); lit != labelIDs.end(); ++lit)
    {
        LabelSceneRep dummyRep(*lit);
        LabelSceneRepSet::iterator it = labelReps.find(&dummyRep);
        if (it != labelReps.end())
        {
            LabelSceneRep *labelRep = *it;
            
            // Note: Porting
            // We need to fade them out, then delete
//            if (labelRep->fade > 0.0)
//            {
//                TimeInterval curTime = TimeGetCurrent();
//                for (SimpleIDSet::iterator idIt = labelRep->drawIDs.begin();
//                     idIt != labelRep->drawIDs.end(); ++idIt)
//                    changes.push_back(new FadeChangeRequest(*idIt,curTime,curTime+labelRep->fade));
//                
//                for (SimpleIDSet::iterator idIt = labelRep->screenIDs.begin();
//                     idIt != labelRep->screenIDs.end(); ++idIt)
//                    changes.push_back(new ScreenSpaceGeneratorFadeRequest(screenGenId, *idIt, curTime, curTime+labelRep->fade));
//                
//                dispatch_after(dispatch_time(DISPATCH_TIME_NOW, labelRep->fade * NSEC_PER_SEC),
//                               scene->getDispatchQueue(),
//                               ^{
//                                   SimpleIDSet theIDs;
//                                   theIDs.insert(labelRep->getId());
//                                   ChangeSet delChanges;
//                                   removeLabels(theIDs, delChanges);
//                                   scene->addChangeRequests(delChanges);
//                               }
//                               );
//
//                labelRep->fade = 0.0;
//            } else {
                for (SimpleIDSet::iterator idIt = labelRep->drawIDs.begin();
                     idIt != labelRep->drawIDs.end(); ++idIt)
                    changes.push_back(new RemDrawableReq(*idIt));
                for (SimpleIDSet::iterator idIt = labelRep->texIDs.begin();
                     idIt != labelRep->texIDs.end(); ++idIt)
                    changes.push_back(new RemTextureReq(*idIt));
                for (SimpleIDSet::iterator idIt = labelRep->drawStrIDs.begin();
                     idIt != labelRep->drawStrIDs.end(); ++idIt)
                {
                    if (fontTexManager)
                        fontTexManager->removeString(*idIt, changes);
                }
                
                if (selectManager && !labelRep->selectIDs.empty())
                    selectManager->removeSelectables(labelRep->selectIDs);

                // Note: Screenspace  Doesn't handle fade
                if (layoutManager && !labelRep->layoutIDs.empty())
                    layoutManager->removeLayoutObjects(labelRep->layoutIDs);
                
                labelReps.erase(it);
                delete labelRep;
//            }
        }
    }

    pthread_mutex_unlock(&labelLock);
}

}
