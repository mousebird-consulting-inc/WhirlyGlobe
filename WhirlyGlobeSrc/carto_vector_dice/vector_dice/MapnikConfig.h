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
#include <boost/uuid/uuid.hpp>
#include <boost/uuid/uuid_generators.hpp>
#include <boost/uuid/uuid_io.hpp>

typedef boost::shared_ptr<boost::regex> RegExRef;

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

    class Layer;
    
    class Symbolizer
    {
    public:
        Symbolizer();

        // These are set when we compile the styles.
        // Not actually unique at this level
        boost::uuids::uuid uuid;

        // Generated name
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
        
        // Min and max scale denominators
        float minScaleDenom,maxScaleDenom;
    };
    
    // Collection of all the symbolizers in one place
    class SymbolizerTable
    {
    public:
        // Raw symbolizers
        std::vector<Symbolizer> symbolizers;
    };
    
    class SortedLayer;
    class Filter;
    
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
        RegExRef exp;

        // Single comparison:  x = val, etc..
        class Comparison
        {
        public:
            void setFilter(const std::string &filterText);
            bool isValid();
            
            RegExRef exp;
            // Filters are very simple right now.  Just basic comparison.
            typedef enum {CompareEqual,CompareLess,CompareLessEqual,CompareMore,CompareMoreEqual,CompareNotEqual} ComparisonType;
            ComparisonType compareType;
            // Data types
            typedef enum {CompareString,CompareReal} ComparisonValueType;
            ComparisonValueType compareValueType;
            std::string attrName,attrValStr;
            double attrValReal;
        };

        // All comparisons are AND and... that's it
        // Note: Yes, that's lame
        typedef enum {OperatorAND,OperatorOR} LogicalOperator;
        LogicalOperator logicalOp;
        std::vector<Comparison> comparisons;
    };
    
    // Symbolizers compiled into
    class CompiledSymbolizerTable
    {
    public:
        CompiledSymbolizerTable() : currentDrawPriority(0) { }
        
        class SubSymbolizer
        {
        public:
            SubSymbolizer() : uuid(boost::uuids::random_generator()()) { }
            
            // Convert to a string
            void toString(std::string &str);
            
            // Unique Identifier
            boost::uuids::uuid uuid;
            
            // Name (made up)
            std::string name;
            
            // Min and max scale denominators
            float minScaleDenom,maxScaleDenom;
            
            // Draw priority (just order of the symbolizer)
            int drawPriority;
            
            // XML describing original symbolizer
            tinyxml2::XMLElement *xmlEl;
        };
        
        // Group of related symbolizers
        class SymbolizerGroup
        {
        public:
            SymbolizerGroup() : uuid(boost::uuids::random_generator()()) { }
            
            // Convert to a string
            void toString(std::string &str);
            
            // Everything in this group shares this filter
            Filter filter;
            
            // Unique Identifier
            boost::uuids::uuid uuid;
            
            // The data type this symbolizer is expecting
            SymbolDataType dataType;
            // When displaying we may add the geometry or replace it per level
            Symbolizer::TileGeometryType geomType;
            // Symbolizer type
            SymbolizerType symType;
            
            // Symbolizers that inherit from this group
            std::vector<SubSymbolizer> subSyms;
            
            // Attributes we want preserved in the source data
            std::set<std::string> attrs;
        };
        
        // Turn a sorted style into a symbolizer group
        void addSymbolizerGroup(MapnikConfig *mapnikConfig,void *inStyle,std::vector<SymbolizerGroup> &retGroups);
        
        // Layer names (we write these out for convenience in the styles file)
        std::set<std::string> layerNames;
        
        // Zoom level range
        int minLevel,maxLevel;
        
        std::vector<SymbolizerGroup> symGroups;
        
        // Write the symbolizers out as a JSON style table
        bool writeJSON(std::string &json);
        
        int currentDrawPriority;
    };
    
    // Rules within a style
    class Rule
    {
    public:
        Rule();
        ~Rule();
        
        tinyxml2::XMLElement *xmlEl;
        
        // Return a version of the rule that just applies to the given data type
        // Symbolizers have data types
        Rule ruleForDataType(MapnikConfig *,SymbolDataType dataType) const;
        
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
    
    // Base class for the data sources
    class DataSource
    {
    public:
        virtual ~DataSource() { }
        
        // Return a name for id purposes
        virtual std::string getName() = 0;
        
        // Return the path to a shapefile
        virtual std::string getShapefileName(Layer *layer,const char *outDir) = 0;
    };
    typedef boost::shared_ptr<DataSource> DataSourceRef;
    
    // Shapefile data source
    class ShapefileDataSource : public DataSource
    {
    public:
        ShapefileDataSource(const std::string &fileName,const std::vector<std::string> &paths);
        ~ShapefileDataSource() { }
        
        std::string getName();
        std::string getShapefileName(Layer *layer,const char *outDir);

    protected:
        std::string fileName;
    };
    
    // PostGIS data source
    class PostGISDataSource : public DataSource
    {
    public:
        std::string getName();
        std::string getShapefileName(Layer *layer,const char *outDir);

        std::string dbname;
        std::string table;
        std::string idStr;
    protected:
        std::string fileName;
        std::string shapefileName;
    };
    
    // OGR data source
    class DataSourceOGR : public DataSource
    {
    public:
        DataSourceOGR(const std::string &fileName);
        
        std::string getName();
        std::string getShapefileName(Layer *layer,const char *outDir);
        
    protected:
        std::string fileName;
        std::string shapeFileName;
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
        std::vector<DataSourceRef> dataSources;
    };
    
    // Sorts a layer into style/rules that share filters
    class SortedLayer
    {
    public:
        SortedLayer(MapnikConfig *mapnikConfig,const Layer &layer);
        
        // Style
        class SortedStyle
        {
        public:
            // Filter that they all share
            Filter filter;
            // All rules must apply to this data type
            SymbolDataType dataType;

            // Scale we need this data to appear at
            int maxScale;

            // An instance of the given style that applies here
            class Instance
            {
            public:
                const Style *style;
                std::vector<Rule> rules;
            };
            
            std::vector<Instance> styleInstances;
        };
        
        bool isValid() { return valid; }
        bool valid;
        
        // Styles sorted by level and filter
        std::vector<SortedStyle> sortStyles;
    };

    // Try parsing the XML document.  Return an error if we failed.
    bool parseXML(tinyxml2::XMLDocument *doc,const std::vector<std::string> &paths,std::string &error);

    // Look up a given style by name
    const MapnikConfig::Style *findStyle(const std::string &styleName);
    
    // Write the TileJSON-like format for vector tiles
    bool writeTileJSON(std::string &json,const std::string &webDbName,const std::string &webDbURL);
    
    Symbolizer::TileGeometryType defaultGeomType;
    
    // Parameters (unparsed at present)
    tinyxml2::XMLElement *paramEl;
    
    std::vector<Style> styles;
    std::vector<Layer> layers;
    // Symbolzer
    SymbolizerTable symbolizerTable;
    // This is the form we want to write out
    CompiledSymbolizerTable compiledSymTable;
};

#endif /* defined(__vector_dice__MapnikConfig__) */
