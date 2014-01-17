//
//  MapnikConfig.h
//  vector_dice
//
//  Created by Steve Gifford on 1/1/14.
//  Copyright (c) 2014 mousebird consulting. All rights reserved.
//

#ifndef __vector_dice__MapnikConfig__
#define __vector_dice__MapnikConfig__

#include <iostream>
#include <vector>
#include "tinyxml2.h"
#include <boost/regex.hpp>
#include <boost/shared_ptr.hpp>

/// A Mapnik config file we've parse into a form we can use
class MapnikConfig
{
public:
    MapnikConfig();
    ~MapnikConfig();
    
    // Data type expected by a given symbolizer
    typedef enum {SymbolDataPoint=0,SymbolDataLinear,SymbolDataAreal,SymbolDataUnknown} SymbolDataType;
    
    // The symbolizer type itself
    typedef enum {MarkersSymbolizer,LineSymbolizer,TextSymbolizer,PolygonSymbolizer,UnknownSymbolizer} SymbolizerType;

    class Symbolizer
    {
    public:
        Symbolizer();
        
        // Convert to a string
        void toString(std::string &str);
        
        // Name (made up)
        std::string name;
        
        // XML for the symbolizers
        tinyxml2::XMLElement *xmlEl;
        // The data type this symbolizer is expecting
        SymbolDataType dataType;
        // When displaying we may add the geometry or replace it per level
        typedef enum {TileGeomAdd,TileGeomReplace} TileGeometryType;
        TileGeometryType geomType;
        // Symbolizer type
        SymbolizerType symType;
    };
    
    // Collection of all the symbolizers in one place
    class SymbolizerTable
    {
    public:
        std::vector<Symbolizer> symbolizers;
        
        // Layer names (we write these out for convenience in the styles file)
        std::set<std::string> layerNames;
        
        // Zoom level range
        int minLevel,maxLevel;
        
        // Write the symbolizers out as a JSON style table
        bool writeJSON(std::string &json);
    };
    
    // Used to filter vector data.
    class Filter
    {
    public:
        Filter();
        ~Filter();
        
        // Check if it's a valid filter an one we can do
        bool isValid();
        
        // Check if the filter is empty (e.g. there is no filter)
        bool isEmpty();
        
        // We'll parse the filter at this point
        void setFilter(const std::string &filterText);
        
        std::string filter;
        typedef boost::shared_ptr<boost::regex> RegExRef;
        RegExRef exp;
        // Filters are very simple right now.  Just basic comparison.
        typedef enum {CompareEqual,CompareLess,CompareMore,CompareNotEqual} ComparisonType;
        ComparisonType compareType;
        // Data types
        typedef enum {CompareString,CompareReal} ComparisonValueType;
        ComparisonValueType compareValueType;
        std::string attrName,attrValStr;
        double attrValReal;
    };
    
    // Rules within a style
    class Rule
    {
    public:
        Rule();
        ~Rule();
        
        tinyxml2::XMLElement *xmlEl;
        
        // Scales in Mapnik's format
        int minScale,maxScale;
        // An optional filter for matching
        Filter filter;
        // Symbolizers applied to this rule
        std::vector<unsigned int> symbolizers;
        // Attributes we want preserved in the source data
        std::set<std::string> attrs;
    };
    
    typedef enum {FilterFirst,FilterAll} FilterMode;
    
    // Mapnik Style
    class Style
    {
    public:
        Style();
        ~Style();
        
        tinyxml2::XMLElement *xmlEl;
        
        // Style name
        std::string name;
        // How we match data in the style (first match or all matches)
        FilterMode filterMode;
        
        std::vector<Rule> rules;
    };
    
    // Mapnik layer definition.  This is where we link up data
    class Layer
    {
    public:
        Layer();
        ~Layer();
        
        tinyxml2::XMLElement *xmlEl;
        
        // Layer name
        std::string name;
        // Spatial reference system
        std::string srs;
        // Styles we apply to this layer
        std::vector<std::string> styleNames;
        // Data sources we'll read in (only shape files)
        std::vector<std::string> dataSources;
    };

    // Try parsing the XML document.  Return an error if we failed.
    bool parseXML(tinyxml2::XMLDocument *doc,std::string &error);

    // Look up a given style by name
    const MapnikConfig::Style *findStyle(const std::string &styleName);
    
    // Parameters (unparsed at present)
    tinyxml2::XMLElement *paramEl;
    
    std::vector<Style> styles;
    std::vector<Layer> layers;
    SymbolizerTable symbolizerTable;
};

#endif /* defined(__vector_dice__MapnikConfig__) */
