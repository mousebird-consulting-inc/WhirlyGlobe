/*
 *  LabelLayer.mm
 *  WhirlyGlobeLib
 *
 *  Created by Steve Gifford on 2/7/11.
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

#import "LabelLayer.h"
#import "LabelRenderer.h"
#import "WhirlyGeometry.h"
#import "GlobeMath.h"
#import "NSString+Stuff.h"
#import "NSDictionary+Stuff.h"
#import "ScreenSpaceGenerator.h"
#import "FontTextureManager.h"

using namespace Eigen;
using namespace WhirlyKit;

@implementation WhirlyKitSingleLabel
@synthesize text;
@synthesize loc;
@synthesize rotation;
@synthesize desc;
@synthesize iconTexture;
@synthesize isSelectable;
@synthesize selectID;
@synthesize iconSize;
@synthesize screenOffset;

// Generate a key string to uniquely identify this label for reuse
- (std::string)keyString
{
    std::string theStr = [text asStdString];
    
    if (desc)
        theStr += [[desc description] asStdString];
    
    return theStr;
}

- (bool)calcWidth:(float *)width height:(float *)height defaultFont:(UIFont *)font
{
    CGSize textSize = [text sizeWithFont:font];
    if (textSize.width == 0 || textSize.height == 0)
        return false;
    
    if (*width != 0.0)
        *height = *width * textSize.height / ((float)textSize.width);
    else
        *width = *height * textSize.width / ((float)textSize.height);
    
    return true;
}

// Calculate the corners in this order:  (ll,lr,ur,ul)
- (void)calcExtents2:(float)width2 height2:(float)height2 iconSize:(Point2f)theIconSize justify:(WhirlyKitLabelJustify)justify corners:(Point3f *)pts norm:(Point3f *)norm iconCorners:(Point3f *)iconPts coordAdapter:(WhirlyKit::CoordSystemDisplayAdapter *)coordAdapter
{
    Point3f center = coordAdapter->localToDisplay(coordAdapter->getCoordSystem()->geographicToLocal(loc));
    Point3f up(0,0,1);
    Point3f horiz,vert;
    if (coordAdapter->isFlat())
    {
        *norm = up;
        horiz = Point3f(1,0,0);
        vert = Point3f(0,1,0);
    } else {
        *norm = center;
        horiz = up.cross(*norm).normalized();
        vert = norm->cross(horiz).normalized();;
    }
    Point3f ll;
    
    
    switch (justify)
    {
        case WhirlyKitLabelLeft:
            ll = center + theIconSize.x() * horiz - height2 * vert;
            break;
        case WhirlyKitLabelMiddle:
            ll = center - (width2 + theIconSize.x()/2) * horiz - height2 * vert;
            break;
        case WhirlyKitLabelRight:
            ll = center - 2*width2 * horiz - height2 * vert;
            break;
    }
    pts[0] = ll;
    pts[1] = ll + 2*width2 * horiz;
    pts[2] = ll + 2*width2 * horiz + 2 * height2 * vert;
    pts[3] = ll + 2 * height2 * vert;

    // Now add the quad for the icon
    switch (justify)
    {
        case WhirlyKitLabelLeft:
            ll = center - height2*vert;
            break;
        case WhirlyKitLabelMiddle:
            ll = center - (width2 + theIconSize.x()) * horiz - height2*vert;
            break;
        case WhirlyKitLabelRight:
            ll = center - (2*width2 + theIconSize.x()) * horiz - height2*vert;
            break;
    }
    iconPts[0] = ll;
    iconPts[1] = ll + theIconSize.x()*horiz;
    iconPts[2] = ll + theIconSize.x()*horiz + theIconSize.y()*vert;
    iconPts[3] = ll + theIconSize.y()*vert;
}

// This version calculates extents for a screen space label
- (void)calcScreenExtents2:(float)width2 height2:(float)height2 iconSize:(Point2f)theIconSize justify:(WhirlyKitLabelJustify)justify corners:(Point3f *)pts iconCorners:(Point3f *)iconPts useIconOffset:(bool)useIconOffset
{
    Point3f center(0,0,0);
    Point3f ll;
    Point3f horiz = Point3f(1,0,0);
    Point3f vert = Point3f(0,1,0);
    
    Point2f iconSizeForLabel = (useIconOffset ? theIconSize : Point2f(0,0));
    switch (justify)
    {
        case WhirlyKitLabelLeft:
            ll = center + iconSizeForLabel.x() * horiz - height2 * vert;
            break;
        case WhirlyKitLabelMiddle:
            ll = center - (width2 + iconSizeForLabel.x()/2) * horiz - height2 * vert;
            break;
        case WhirlyKitLabelRight:
            ll = center - 2*width2 * horiz - height2 * vert;
            break;
    }
    pts[0] = ll;
    pts[1] = ll + 2*width2 * horiz;
    pts[2] = ll + 2*width2 * horiz + 2 * height2 * vert;
    pts[3] = ll + 2 * height2 * vert;
    
    // Now add the quad for the icon
    switch (justify)
    {
        case WhirlyKitLabelLeft:
            ll = center - height2*vert;
            break;
        case WhirlyKitLabelMiddle:
            ll = center - (width2 + iconSizeForLabel.x()) * horiz - height2*vert;
            break;
        case WhirlyKitLabelRight:
            ll = center - (2*width2 + iconSizeForLabel.x()) * horiz - height2*vert;
            break;
    }
    iconPts[0] = ll;
    iconPts[1] = ll + iconSizeForLabel.x()*horiz;
    iconPts[2] = ll + iconSizeForLabel.x()*horiz + iconSizeForLabel.y()*vert;
    iconPts[3] = ll + iconSizeForLabel.y()*vert;
}

- (void)calcExtents:(NSDictionary *)topDesc corners:(Point3f *)pts norm:(Point3f *)norm coordAdapter:(CoordSystemDisplayAdapter *)coordAdapter
{
    WhirlyKitLabelInfo *labelInfo = [[WhirlyKitLabelInfo alloc] initWithStrs:[NSArray arrayWithObject:self.text] desc:topDesc];
    
    // Width and height can be overriden per label
    float theWidth = labelInfo.width;
    float theHeight = labelInfo.height;
    if (desc)
    {
        theWidth = [desc floatForKey:@"width" default:theWidth];
        theHeight = [desc floatForKey:@"height" default:theHeight];
    }
    
    CGSize textSize = [text sizeWithFont:labelInfo.font];
    
    float width2,height2;
    if (theWidth != 0.0)
    {
        height2 = theWidth * textSize.height / ((float)2.0 * textSize.width);
        width2 = theWidth/2.0;
    } else {
        width2 = theHeight * textSize.width / ((float)2.0 * textSize.height);
        height2 = theHeight/2.0;
    }
    
    // If there's an icon, we need to offset the label
    Point2f theIconSize = (iconTexture==EmptyIdentity ? Point2f(0,0) : Point2f(2*height2,2*height2));

    Point3f corners[4],iconCorners[4];
    [self calcExtents2:width2 height2:height2 iconSize:theIconSize justify:labelInfo.justify corners:corners norm:norm iconCorners:iconCorners coordAdapter:coordAdapter];
    
    // If we have an icon, we need slightly different corners
    if (iconTexture)
    {
        pts[0] = iconCorners[0];
        pts[1] = corners[1];
        pts[2] = corners[2];
        pts[3] = iconCorners[3];
    } else {
        pts[0] = corners[0];
        pts[1] = corners[1];
        pts[2] = corners[2];
        pts[3] = corners[3];
    }
}

@end

@implementation WhirlyKitLabelLayer
{
    WhirlyKitFontTextureManager *fontTexManager;
}
        
@synthesize layoutLayer;

- (id)init
{
    if ((self = [super init]))
    {
        textureAtlasSize = LabelTextureAtlasSizeDefault;
        _useFontManager = true;
    }
    
    return self;
}

- (id)initWithTexAtlasSize:(unsigned int)inTextureAtlasSize
{
    if ((self = [super init]))
    {
        textureAtlasSize = inTextureAtlasSize;
    }
    
    return self;
}

- (void)clear
{
    layerThread = nil;
    for (LabelSceneRepMap::iterator it=labelReps.begin();
         it!=labelReps.end(); ++it)
        delete it->second;
    labelReps.clear();
    
    scene = NULL;
}

- (void)dealloc
{
    [self clear];
}

// We only do things when called on, so nothing much to do here
- (void)startWithThread:(WhirlyKitLayerThread *)inLayerThread scene:(WhirlyKit::Scene *)inScene;
{
    layerThread = inLayerThread;
    scene = inScene;
    screenGenId = scene->getScreenSpaceGeneratorID();
}

// Clean out our textures and drawables
- (void)shutdown
{
    [NSObject cancelPreviousPerformRequestsWithTarget:self];
    std::vector<ChangeRequest *> changeRequests;
    SelectionManager *selectManager = scene->getSelectionManager();
    
    for (LabelSceneRepMap::iterator it=labelReps.begin();
         it!=labelReps.end(); ++it)
    {
        LabelSceneRep *labelRep = it->second;
        for (SimpleIDSet::iterator idIt = labelRep->drawIDs.begin();
             idIt != labelRep->drawIDs.end(); ++idIt)
            changeRequests.push_back(new RemDrawableReq(*idIt));
        for (SimpleIDSet::iterator idIt = labelRep->texIDs.begin();
             idIt != labelRep->texIDs.end(); ++idIt)        
            changeRequests.push_back(new RemTextureReq(*idIt));
        for (SimpleIDSet::iterator idIt = labelRep->screenIDs.begin();
             idIt != labelRep->screenIDs.end(); ++idIt)
            [layerThread addChangeRequest:(new ScreenSpaceGeneratorRemRequest(screenGenId, *idIt))];
        for (SimpleIDSet::iterator idIt = labelRep->drawStrIDs.begin();
             idIt != labelRep->drawStrIDs.end(); ++idIt)
            [fontTexManager removeString:*idIt changes:changeRequests];

        if (labelRep->selectID != EmptyIdentity && selectManager)
            selectManager->removeSelectable(labelRep->selectID);
        
        if (layoutLayer && !labelRep->screenIDs.empty())
            [layoutLayer removeLayoutObjects:labelRep->screenIDs];
    }
    
    if (fontTexManager)
    {
        [fontTexManager clear:changeRequests];
        fontTexManager = nil;
    }
    
    [layerThread addChangeRequests:changeRequests];
    
    [self clear];
}

// Create the label and keep track of it
// We're in the layer thread here
// Note: Badly optimized for single label case
- (void)runAddLabels:(WhirlyKitLabelInfo *)labelInfo
{
    // Set up the representation (but then hand it off)
    LabelSceneRep *labelRep = new LabelSceneRep();
    labelRep->fade = labelInfo.fade;
    labelRep->setId(labelInfo.labelId);

    if (_useFontManager && !fontTexManager)
        fontTexManager = scene->getFontTextureManager();
        
    // Set up the label renderer
    WhirlyKitLabelRenderer *labelRenderer = [[WhirlyKitLabelRenderer alloc] init];
    labelRenderer->labelInfo = labelInfo;
    labelRenderer->textureAtlasSize = textureAtlasSize;
    labelRenderer->coordAdapter = scene->getCoordAdapter();
    labelRenderer->labelRep = labelRep;
    labelRenderer->scene = scene;
    labelRenderer->screenGenId = screenGenId;
    labelRenderer->fontTexManager = (labelInfo.screenObject ? fontTexManager : nil);

    // Can't use fancy strings on ios5 and we can't use dynamic texture atlases in a block
    bool oldiOS = [[[UIDevice currentDevice] systemVersion] floatValue] < 6.0;
    if (!oldiOS && !labelRenderer->fontTexManager)
    {
        // Do the render somewhere else and merge in the results back on our thread
        dispatch_async(dispatch_get_global_queue(DISPATCH_QUEUE_PRIORITY_DEFAULT, 0), ^
                       {
                           [labelRenderer render];
                           [self performSelector:@selector(mergeRenderedLabels:) onThread:layerThread withObject:labelRenderer waitUntilDone:NO];
                       });
    } else {
        // For old iOS versions and for font texture rendering, we'll do the work on this thread.
        // The former can't handle it and the latter is fast enough to not need it
        labelRenderer->useAttributedString = !oldiOS;
        [labelRenderer render];
        [self mergeRenderedLabels:labelRenderer];        
    }

    // Note: This means we can't delete the labels while they're rendering.  Bug.
//    labelReps[labelRep->getId()] = labelRep;
}

- (void)mergeRenderedLabels:(WhirlyKitLabelRenderer *)labelRenderer
{    
    SelectionManager *selectManager = scene->getSelectionManager();
    
    // Flush out the changes
    [layerThread addChangeRequests:labelRenderer->changeRequests];
    
    // And any layout constraints to the layout engine
    if (layoutLayer && ([labelRenderer->layoutObjects count] > 0))
        [layoutLayer addLayoutObjects:labelRenderer->layoutObjects];
    
    // And set up the selectables
    //                [selectLayer addSelectableScreenRect:label.selectID rect:pts2d minVis:labelInfo.minVis maxVis:labelInfo.maxVis];
    //                [selectLayer addSelectableRect:label.selectID rect:pts];
    if (selectManager)
    {
        for (unsigned int ii=0;ii<labelRenderer->selectables2D.size();ii++)
        {
            RectSelectable2D &sel = labelRenderer->selectables2D[ii];
            selectManager->addSelectableScreenRect(sel.selectID,sel.pts,sel.minVis,sel.maxVis);
        }
        for (unsigned int ii=0;ii<labelRenderer->selectables3D.size();ii++)
        {
            RectSelectable3D &sel = labelRenderer->selectables3D[ii];
            selectManager->addSelectableRect(sel.selectID,sel.pts,sel.minVis,sel.maxVis);
        }
    }
    
    labelReps[labelRenderer->labelRep->getId()] = labelRenderer->labelRep;
}

// Remove the given label
- (void)runRemoveLabel:(NSNumber *)num
{
    SimpleIdentity labelId = [num unsignedIntValue];
    SelectionManager *selectManager = scene->getSelectionManager();
    
    std::vector<ChangeRequest *> changeRequests;
    
    LabelSceneRepMap::iterator it = labelReps.find(labelId);
    if (it != labelReps.end())
    {
        LabelSceneRep *labelRep = it->second;
        
        // We need to fade them out, then delete
        if (labelRep->fade > 0.0)
        {
            NSTimeInterval curTime = CFAbsoluteTimeGetCurrent();
            for (SimpleIDSet::iterator idIt = labelRep->drawIDs.begin();
                 idIt != labelRep->drawIDs.end(); ++idIt)
                changeRequests.push_back(new FadeChangeRequest(*idIt,curTime,curTime+labelRep->fade));
            
            for (SimpleIDSet::iterator idIt = labelRep->screenIDs.begin();
                 idIt != labelRep->screenIDs.end(); ++idIt)
                changeRequests.push_back(new ScreenSpaceGeneratorFadeRequest(screenGenId, *idIt, curTime, curTime+labelRep->fade));
            
            // Reset the fade and try to delete again later
            [self performSelector:@selector(runRemoveLabel:) withObject:num afterDelay:labelRep->fade];
            labelRep->fade = 0.0;
        } else {
            for (SimpleIDSet::iterator idIt = labelRep->drawIDs.begin();
                 idIt != labelRep->drawIDs.end(); ++idIt)
                changeRequests.push_back(new RemDrawableReq(*idIt));
            for (SimpleIDSet::iterator idIt = labelRep->texIDs.begin();
                 idIt != labelRep->texIDs.end(); ++idIt)        
                changeRequests.push_back(new RemTextureReq(*idIt));
            for (SimpleIDSet::iterator idIt = labelRep->screenIDs.begin();
                 idIt != labelRep->screenIDs.end(); ++idIt)
                changeRequests.push_back(new ScreenSpaceGeneratorRemRequest(screenGenId, *idIt));
            for (SimpleIDSet::iterator idIt = labelRep->drawStrIDs.begin();
                 idIt != labelRep->drawStrIDs.end(); ++idIt)
                [fontTexManager removeString:*idIt changes:changeRequests];
            
            if (labelRep->selectID != EmptyIdentity && selectManager)
                selectManager->removeSelectable(labelRep->selectID);
            
            if (layoutLayer && !labelRep->screenIDs.empty())
                [layoutLayer removeLayoutObjects:labelRep->screenIDs];
            
            labelReps.erase(it);
            delete labelRep;
        }
    }
    
    [layerThread addChangeRequests:changeRequests];
}

// Pass off label creation to a routine in our own thread
- (SimpleIdentity) addLabel:(NSString *)str loc:(WhirlyKit::GeoCoord)loc desc:(NSDictionary *)desc
{
    WhirlyKitSingleLabel *theLabel = [[WhirlyKitSingleLabel alloc] init];
    theLabel.text = str;
    [theLabel setLoc:loc];
    WhirlyKitLabelInfo *labelInfo = [[WhirlyKitLabelInfo alloc] initWithStrs:[NSArray arrayWithObject:theLabel] desc:desc];
    
    if (!layerThread || ([NSThread currentThread] == layerThread))
        [self runAddLabels:labelInfo];
    else
        [self performSelector:@selector(runAddLabels:) onThread:layerThread withObject:labelInfo waitUntilDone:NO];
    
    return labelInfo.labelId;
}

- (SimpleIdentity) addLabel:(WhirlyKitSingleLabel *)label
{
    return [self addLabels:[NSMutableArray arrayWithObject:label] desc:label.desc];
}

/// Add a group of labels
- (SimpleIdentity) addLabels:(NSArray *)labels desc:(NSDictionary *)desc
{
    if (!layerThread || !scene)
    {
        NSLog(@"WhirlyGlobe Label has not been initialized, yet you're calling addLabel.  Dropping data on floor.");
        return EmptyIdentity;
    }

    WhirlyKitLabelInfo *labelInfo = [[WhirlyKitLabelInfo alloc] initWithStrs:labels desc:desc];
    
    if (!layerThread || ([NSThread currentThread] == layerThread))
        [self runAddLabels:labelInfo];
    else
        [self performSelector:@selector(runAddLabels:) onThread:layerThread withObject:labelInfo waitUntilDone:NO];
    
    return labelInfo.labelId;        
}

// Change visual representation for a group of labels
// Only doing min/max vis for now
- (void) runChangeLabel:(WhirlyKitLabelInfo *)labelInfo
{
    LabelSceneRepMap::iterator it = labelReps.find(labelInfo.labelId);
    
    if (it != labelReps.end())
    {    
        LabelSceneRep *sceneRep = it->second;
        
        for (SimpleIDSet::iterator idIt = sceneRep->drawIDs.begin();
             idIt != sceneRep->drawIDs.end(); ++idIt)
        {
            // Changed visibility
            [layerThread addChangeRequest:(new VisibilityChangeRequest(*idIt, labelInfo.minVis, labelInfo.maxVis))];
        }
    }    
}

// Change how the label is displayed
- (void)changeLabel:(WhirlyKit::SimpleIdentity)labelID desc:(NSDictionary *)dict
{
    WhirlyKitLabelInfo *labelInfo = [[WhirlyKitLabelInfo alloc] initWithSceneRepId:labelID desc:dict];
    
    if (!layerThread || ([NSThread currentThread] == layerThread))
        [self runChangeLabel:labelInfo];
    else
        [self performSelector:@selector(runChangeLabel:) onThread:layerThread withObject:labelInfo waitUntilDone:NO];
}

// Set up the label to be removed in the layer thread
- (void) removeLabel:(WhirlyKit::SimpleIdentity)labelId
{
    NSNumber *num = [NSNumber numberWithUnsignedInt:labelId];
    if (!layerThread || ([NSThread currentThread] == layerThread))
        [self runRemoveLabel:num];
    else
        [self performSelector:@selector(runRemoveLabel:) onThread:layerThread withObject:num waitUntilDone:NO];
}

@end
