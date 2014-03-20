/*
 *  ViewPlacementGenerator.mm
 *  WhirlyGlobeLib
 *
 *  Created by Steve Gifford on 7/25/12.
 *  Copyright 2011-2013 mousebird consulting
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

#import "ViewPlacementGenerator.h"
#import "MaplyView.h"

using namespace Eigen;
using namespace WhirlyKit;
using namespace WhirlyGlobe;

namespace WhirlyKit
{
    
ViewPlacementGenerator::ViewPlacementGenerator(const std::string &name)
    : Generator(name)
{
}
    
ViewPlacementGenerator::~ViewPlacementGenerator()
{
    viewInstanceSet.clear();
}
    
void ViewPlacementGenerator::addView(GeoCoord loc,UIView *view,float minVis,float maxVis)
{
    // Make sure it isn't already there
    removeView(view);
    
    ViewInstance viewInst(loc,view);
    viewInst.minVis = minVis;
    viewInst.maxVis = maxVis;
    CGRect frame = view.frame;
    viewInst.offset = Point2f(frame.origin.x,frame.origin.y);
    viewInstanceSet.insert(viewInst);
}

void ViewPlacementGenerator::moveView(GeoCoord loc,UIView *view,float minVis,float maxVis)
{
    // Make sure it isn't already there
    std::set<ViewInstance>::iterator it = viewInstanceSet.find(ViewInstance(view));
    if (it != viewInstanceSet.end())
        viewInstanceSet.erase(it);
    
    ViewInstance viewInst(loc,view);
    viewInst.minVis = minVis;
    viewInst.maxVis = maxVis;
    CGRect frame = view.frame;
    viewInst.offset = Point2f(frame.origin.x,frame.origin.y);
    viewInstanceSet.insert(viewInst);
}
    
void ViewPlacementGenerator::freezeView(UIView *view)
{
    ViewInstance newVI(view);
    std::set<ViewInstance>::iterator it = viewInstanceSet.find(newVI);
    if (it != viewInstanceSet.end())
    {
        newVI = *it;
        viewInstanceSet.erase(it);
        newVI.active = false;
        viewInstanceSet.insert(newVI);
    }
}

void ViewPlacementGenerator::unfreezeView(UIView *view)
{
    ViewInstance newVI(view);
    std::set<ViewInstance>::iterator it = viewInstanceSet.find(newVI);
    if (it != viewInstanceSet.end())
    {
        newVI = *it;
        viewInstanceSet.erase(it);
        newVI.active = true;
        viewInstanceSet.insert(newVI);
    }
}
    
void ViewPlacementGenerator::removeView(UIView *view)
{
    std::set<ViewInstance>::iterator it = viewInstanceSet.find(ViewInstance(view));
    if (it != viewInstanceSet.end())
        viewInstanceSet.erase(it);
}
    
// Work through the list of views, moving things around and/or hiding as needed
void ViewPlacementGenerator::generateDrawables(WhirlyKitRendererFrameInfo *frameInfo,std::vector<DrawableRef> &drawables,std::vector<DrawableRef> &screenDrawables)
{
    CoordSystemDisplayAdapter *coordAdapter = frameInfo.scene->getCoordAdapter();
    
    // Note: Make this work for generic 3D views
    WhirlyGlobeView *globeView = (WhirlyGlobeView *)frameInfo.theView;
    if (![globeView isKindOfClass:[WhirlyGlobeView class]])
        globeView = nil;
    MaplyView *mapView = (MaplyView *)frameInfo.theView;
    if (![mapView isKindOfClass:[MaplyView class]])
        mapView = nil;
    if (!globeView && !mapView)
        return;

    // Overall extents we'll look at.  Everything else is tossed.
    // Note: This is too simple
    Mbr frameMbr;
    float marginX = frameInfo.sceneRenderer.framebufferWidth * 1.1;
    float marginY = frameInfo.sceneRenderer.framebufferHeight * 1.1;
    frameMbr.ll() = Point2f(0 - marginX,0 - marginY);
    frameMbr.ur() = Point2f(frameInfo.sceneRenderer.framebufferWidth + marginX,frameInfo.sceneRenderer.framebufferHeight + marginY);
    
    std::vector<Eigen::Matrix4d> modelAndViewMats; // modelAndViewNormalMats;
    for (unsigned int offi=0;offi<frameInfo.offsetMatrices.size();offi++)
    {
        // Project the world location to the screen
        Eigen::Matrix4d modelAndViewMat = Matrix4fToMatrix4d(frameInfo.viewTrans) * frameInfo.offsetMatrices[offi] * Matrix4fToMatrix4d(frameInfo.modelTrans);
//        Eigen::Matrix4d modelAndViewNormalMat = modelAndViewMat.inverse().transpose();
        modelAndViewMats.push_back(modelAndViewMat);
//        modelAndViewNormalMats.push_back(modelAndViewNormalMat);
    }
        
    for (std::set<ViewInstance>::iterator it = viewInstanceSet.begin();
         it != viewInstanceSet.end(); ++it)
    {
        const ViewInstance &viewInst = *it;
        bool hidden = NO;
        CGPoint screenPt;
        
        if (!it->active)
            continue;
        
        // Height above globe test
        float visVal = [frameInfo.theView heightAboveSurface];
        if (!(viewInst.minVis == DrawVisibleInvalid || viewInst.maxVis == DrawVisibleInvalid ||
              ((viewInst.minVis <= visVal && visVal <= viewInst.maxVis) ||
               (viewInst.maxVis <= visVal && visVal <= viewInst.minVis))))
            hidden = YES;

        if (!hidden)
        {
            // Note: Calculate this ahead of time
            Point3d worldLoc = coordAdapter->localToDisplay(coordAdapter->getCoordSystem()->geographicToLocal3d(viewInst.loc));
     
            // Check that it's not behind the globe
            if (globeView)
            {
                // Make sure this one is facing toward the viewer
                Point3f worldLoc3f(worldLoc.x(),worldLoc.y(),worldLoc.z());
                if (CheckPointAndNormFacing(worldLoc3f,worldLoc3f.normalized(),frameInfo.viewAndModelMat,frameInfo.viewModelNormalMat) < 0.0)
                    hidden = YES;
            }

            if (!hidden)
            {
                bool visible = false;
                for (unsigned int offi=0;offi<modelAndViewMats.size();offi++)
                {
                    // Project the world location to the screen
                    Eigen::Matrix4d &modelTrans = modelAndViewMats[offi];
                    CGPoint thisScreenPt;
                    if (globeView)
                        thisScreenPt = [globeView pointOnScreenFromSphere:worldLoc transform:&modelTrans frameSize:Point2f(frameInfo.sceneRenderer.framebufferWidth,frameInfo.sceneRenderer.framebufferHeight)];
                    else
                        thisScreenPt = [mapView pointOnScreenFromPlane:worldLoc transform:&modelTrans frameSize:Point2f(frameInfo.sceneRenderer.framebufferWidth,frameInfo.sceneRenderer.framebufferHeight)];
                    
                    // Note: This check is too simple
                    if ((thisScreenPt.x >= frameMbr.ll().x() && thisScreenPt.y >= frameMbr.ll().y() &&
                         thisScreenPt.x <= frameMbr.ur().x() && thisScreenPt.y <= frameMbr.ur().y()))
                    {
                        visible = true;
                        screenPt = thisScreenPt;
                    }
                }
                
                if (!visible)
                hidden = true;
            }
        }
        
        if (!hidden)
        {
            CGSize size = viewInst.view.frame.size;
            // Note: We should really be passing this in
            float scale = [UIScreen mainScreen].scale;
            // We can only modify UIViews on the main thread
            if ([NSThread currentThread] != [NSThread mainThread])
            {
                dispatch_async(dispatch_get_main_queue(),
                               ^{
                                   viewInst.view.hidden = false;
                                   viewInst.view.frame = CGRectMake(screenPt.x / scale + viewInst.offset.x(), screenPt.y / scale + viewInst.offset.y(), size.width, size.height);
                               });
            } else {
                viewInst.view.hidden = false;
                viewInst.view.frame = CGRectMake(screenPt.x / scale + viewInst.offset.x(), screenPt.y / scale + viewInst.offset.y(), size.width, size.height);
            }
        } else {
            if ([NSThread currentThread] != [NSThread mainThread])
            {
                dispatch_async(dispatch_get_main_queue(),
                               ^{
                                   viewInst.view.hidden = true;
                               });
            } else
                viewInst.view.hidden = true;
        }
    }
}

void ViewPlacementGenerator::dumpStats()
{
    NSLog(@"ViewPlacement Generator: %ld",viewInstanceSet.size());
}

}
