/*
 *  MarkerGenerator.h
 *  WhirlyGlobeLib
 *
 *  Created by Steve Gifford on 11/21/11.
 *  Copyright 2011-2013 mousebird consulting. All rights reserved.
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

#import <math.h>
#import <set>
#import <map>
#import "Identifiable.h"
#import "Drawable.h"
#import "Generator.h"

namespace WhirlyKit
{
    
/** The Marker Generator produces geometry for individual markers on the
    rendering thread.  In general, you won't be interacting with this.
 */
class MarkerGenerator : public Generator 
{
public:
    MarkerGenerator();
    virtual ~MarkerGenerator();

    /// Generate the drawables for the given frame
    void generateDrawables(WhirlyKit::RendererFrameInfo *frameInfo,std::vector<DrawableRef> &drawables,std::vector<DrawableRef> &screenDrawables);
    
    typedef std::map<SimpleIdentity,BasicDrawable *> DrawableMap;

    /** An individual marker contains the geometry we want to put on top
        of the globe at any given time.  These are only used when the marker
        will iterate through multiple textures over a given period.
      */
    class Marker : public Identifiable
    {
    public:
        /// Called by the marker generator build the geometry
        void addToDrawables(WhirlyKit::RendererFrameInfo *frameInfo,DrawableMap &drawables,float minZres);
        
        bool enable;
        RGBAColor color;
        Point2f loc;
        Eigen::Vector3f norm;
        Point3f pts[4];
        std::vector<std::vector<TexCoord> > texCoords;
        std::vector<SimpleIdentity> texIDs;
        NSTimeInterval start;
        NSTimeInterval period;
        NSTimeInterval fadeUp,fadeDown;
        int drawOffset;
        int drawPriority;
        float minVis,maxVis;
    };

    /// Called by the renderer to add a marker from a layer
    void addMarker(Marker *marker);
    /// Called to add a whole group of markers
    void addMarkers(std::vector<Marker *> markers);
    
    /// Enable/disable markers
    void enableMarkers(std::vector<SimpleIdentity> &markerIDs,bool enable);
    
    /// Called by the renderer to remove a marker
    void removeMarker(SimpleIdentity markerId);
    /// Called to remove a whole group of markers
    void removeMarkers(std::vector<SimpleIdentity> &markerIDs);
    
    /// Return a marker.  Only used by the change request objects.
    Marker *getMarker(SimpleIdentity markerId);
    
    /// Dump out stats for debugging
    void dumpStats();
    
protected:
    typedef std::set<Marker *,IdentifiableSorter> MarkerSet;
    MarkerSet markers;
};
    
/** The Marker Generator Add Request comes from the MarkerLayer and is
    routed through the renderer to the MarkerGenerator which keeps track
    of it.  Markers are then referred to by ID.
    Markers passed in are deleted by the request object.
  */
class MarkerGeneratorAddRequest : public GeneratorChangeRequest
{
public:
    /// Construct with the marker generator's ID and the marker
    MarkerGeneratorAddRequest(SimpleIdentity genID,MarkerGenerator::Marker *marker);
    /// Construct with a vector of markers to add
    MarkerGeneratorAddRequest(SimpleIdentity genID,const std::vector<MarkerGenerator::Marker *> &markers);
    ~MarkerGeneratorAddRequest();
    
    virtual void execute2(Scene *scene,WhirlyKitSceneRendererES *renderer,Generator *gen);
    
protected:
    std::vector<MarkerGenerator::Marker *> markers;
};
    
/** A Marker Generator Remove Request comes form the MarkerLayer, is routed
    through the renderer and handed off to the MarkerGenerator.
  */
class MarkerGeneratorRemRequest : public GeneratorChangeRequest
{
public:
    /// Construct with the marker generator's ID and the marker ID to remove
    MarkerGeneratorRemRequest(SimpleIdentity genID,SimpleIdentity markerID);
    /// Construct with the generator ID and a list of marker IDs to remove
    MarkerGeneratorRemRequest(SimpleIdentity genID,const std::vector<SimpleIdentity> &markerIDs);
    ~MarkerGeneratorRemRequest();
    
    virtual void execute2(Scene *scene,WhirlyKitSceneRendererES *renderer,Generator *gen);
    
protected:
    std::vector<SimpleIdentity> markerIDs;
};
    
/** Enable/disable a grup of markers.
  */
class MarkerGeneratorEnableRequest : public GeneratorChangeRequest
{
public:
    /// Construct with the marker generator's ID and the marker IDs to modify
    MarkerGeneratorEnableRequest(SimpleIdentity genID,const std::vector<SimpleIdentity> &markerIDs,bool enable);
    ~MarkerGeneratorEnableRequest();
    
    virtual void execute2(Scene *scene,WhirlyKitSceneRendererES *renderer,Generator *gen);
protected:
    bool enable;
    std::vector<SimpleIdentity> markerIDs;
};
    
/** Change the fade values on one or more markers.  This would be in
    preparation for deleting it, usually.
 */
class MarkerGeneratorFadeRequest : public GeneratorChangeRequest
{
public:
    /// Construct with the IDs for the generator, the marker, and the fade up/down absolute times
    MarkerGeneratorFadeRequest(SimpleIdentity genID,SimpleIdentity markerID,NSTimeInterval fadeUp,NSTimeInterval fadeDown);
    /// Construct with the ID for the generator, a list of IDs for markers and the fade up/down absolute times
    MarkerGeneratorFadeRequest(SimpleIdentity genID,const std::vector<SimpleIdentity> markerIDs,NSTimeInterval fadeUp,NSTimeInterval fadeDown);
    
    virtual void execute2(Scene *scene,WhirlyKitSceneRendererES *renderer,Generator *gen);

protected:
    NSTimeInterval fadeUp,fadeDown;
    std::vector<SimpleIdentity> markerIDs;
};
    
}
