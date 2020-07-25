/*
*  MaplyVectorStyleC.h
*  WhirlyGlobe-MaplyComponent
*
*  Created by Steve Gifford on 4/9/20.
*  Copyright 2011-2020 mousebird consulting
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

#import "VectorObject.h"
#import "MapboxVectorTileParser.h"
#import <string>

namespace WhirlyKit
{

/**
 Settings that control how vector tiles look in relation to their styles.
  
 These are set based on the sort of device we're on, particularly retina vs. non-retina.  They can be manipulated directly as well for your needs.
 
 This is the object backing the ObjC and Android versions.
 */
class VectorStyleSettingsImpl
{
public:
    VectorStyleSettingsImpl(double scale);

    /// Line widths will be scaled by this amount before display.
    float lineScale;
    /// Text sizes will be scaled by this amount before display.
    float textScale;
    /// Markers will be scaled by this amount before display.
    float markerScale;
    /// Importance for markers in the layout engine
    float markerImportance;
    /// Default marker size when none is specified
    float markerSize;
    /// Importance for labels in the layout engine
    float labelImportance;
    /// If set we'll use the zoom levels defined in the style
    bool useZoomLevels;

    /// For symbols we'll try to pull a UUID out of this field to stick in the marker and label uniqueID
    std::string uuidField;

    /// Draw priority calculated as offset from here
    int baseDrawPriority;

    /// Offset between levels
    int drawPriorityPerLevel;

    /**
        The overall map scale calculations will be scaled by this amount.
        
        We use the map scale calculations to figure out what is dispalyed and when.  Not what to load in, mind you, that's a separate, but related calculation.  This controls the scaling of those calculations.  Scale it down to load things in later, up to load them in sooner.
      */
    float mapScaleScale;

    /// Dashed lines will be scaled by this amount before display.
    float dashPatternScale;

    /// Use widened vectors (which do anti-aliasing and such)
    bool useWideVectors;

    /// Where we're using old vectors (e.g. not wide) scale them by this amount
    float oldVecWidthScale;

    /// If we're using widened vectors, only activate them for strokes wider than this.  Defaults to zero.
    float wideVecCuttoff;

    /// If set, this is the shader we'll use on the areal features.
    std::string arealShaderName;

    /// If set, we'll make all the features selectable.  If not, we won't.
    bool selectable;

    /// If set, icons will be loaded from this directory
    std::string iconDirectory;

    /// The default font family for all text
    std::string fontName;
    
    /// If we're using a dfiferent areal shader, set it up here
    SimpleIdentity settingsArealShaderID;
};
typedef std::shared_ptr<VectorStyleSettingsImpl> VectorStyleSettingsImplRef;

class VectorStyleImpl;
typedef std::shared_ptr<VectorStyleImpl> VectorStyleImplRef;

/**
    Base class for styling vectors.  This is set up to manage the styles.
*/
class VectorStyleDelegateImpl
{
public:
    /// Return the styles that apply to the given feature (attributes).
    virtual std::vector<VectorStyleImplRef> stylesForFeature(DictionaryRef attrs,
                                                             const QuadTreeIdentifier &tileID,
                                                             const std::string &layerName) = 0;
    
    /// Return true if the given layer is meant to display for the given tile (zoom level)
    virtual bool layerShouldDisplay(const std::string &name,
                                    const QuadTreeNew::Node &tileID) = 0;

    /// Return the style associated with the given UUID.
    virtual VectorStyleImplRef styleForUUID(long long uuid) = 0;

    // Return a list of all the styles in no particular order.  Needed for categories and indexing
    virtual std::vector<VectorStyleImplRef> allStyles() = 0;
    
    /// Return the background color for a given zoom level
    virtual RGBAColorRef backgroundColor(double zoom) = 0;
};
typedef std::shared_ptr<VectorStyleDelegateImpl> VectorStyleDelegateImplRef;

/**
 Base class for an individual vector style (also called a layer).
 */
class VectorStyleImpl
{
public:
    /// Unique Identifier for this style
    virtual long long getUuid() = 0;
    
    /// Category used for sorting
    virtual std::string getCategory() = 0;
    
    // Note: This no longer really holds
    /// Set if this geometry is additive (e.g. sticks around) rather than replacement
    virtual bool geomAdditive() = 0;

    /// Construct objects related to this style based on the input data.
    virtual void buildObjects(PlatformThreadInfo *inst, std::vector<VectorObjectRef> &vecObjs,VectorTileDataRef tileInfo) = 0;
};

}
