//
//  MapnikConfig.cpp
//  vector_dice
//
//  Created by Steve Gifford on 1/1/14.
//  Copyright (c) 2014 mousebird consulting. All rights reserved.
//

#import <map>
#include <libgen.h>
#include "ogrsf_frmts.h"
#include "MapnikConfig.h"
#include <boost/lexical_cast.hpp>
#include <boost/algorithm/string.hpp>
#include <boost/filesystem.hpp>

using namespace tinyxml2;

MapnikConfig::Symbolizer::Symbolizer()
    : xmlEl(NULL), geomType(TileGeomReplace)
{
}

void MapnikConfig::CompiledSymbolizerTable::addSymbolizerGroup(MapnikConfig *mapnikConfig,void *inStyle,std::vector<SymbolizerGroup> &retGroups)
{
    SortedLayer::SortedStyle *style = (SortedLayer::SortedStyle *)inStyle;
    std::map<SymbolizerType,SymbolizerGroup *> groups;
    
    // Work through the symbolizers, building up compatible groups as we go
    for (unsigned int ii=0;ii<style->styleInstances.size();ii++)
    {
        SortedLayer::SortedStyle::Instance &inst = style->styleInstances[ii];
        for (unsigned int ri=0;ri<inst.rules.size();ri++)
        {
            Rule &rule = inst.rules[ri];
            for (unsigned int si=0;si<rule.symbolizers.size();si++)
            {
                Symbolizer &sym = mapnikConfig->symbolizerTable.symbolizers[rule.symbolizers[si]];
                // Look for a matching type
                auto it = groups.find(sym.symType);
                SymbolizerGroup *group = NULL;
                if (it == groups.end())
                {
                    group = new SymbolizerGroup();
                    group->symType = sym.symType;
                    group->dataType = style->dataType;
                    group->geomType = Symbolizer::TileGeomAdd;
                    group->filter = rule.filter;
                    group->attrs.insert(rule.attrs.begin(),rule.attrs.end());
                    groups[group->symType] = group;
                } else
                    group = it->second;
                SubSymbolizer subSym;
//                subSym.drawPriority = rule.symbolizers[si];
                subSym.drawPriority = currentDrawPriority++;
                subSym.minScaleDenom = sym.minScaleDenom;
                subSym.maxScaleDenom = sym.maxScaleDenom;
                subSym.xmlEl = sym.xmlEl;
                group->subSyms.push_back(subSym);
            }
        }
    }
    
    // Add the groups to the compiled symbolizer table
    for (auto& it : groups)
    {
        symGroups.push_back(*it.second);
        retGroups.push_back(*it.second);
        // And clean them up
        delete it.second;
    }
}

void MapnikConfig::CompiledSymbolizerTable::SymbolizerGroup::toString(std::string &json)
{
    XMLElement *symEl = subSyms[0].xmlEl;

    json += "\t\t{\n";
    json += (std::string)"\t\t\t\"uuid\": " + "\"" + boost::uuids::to_string(uuid) + "\"" + ",\n";
    json += (std::string)"\t\t\t\"type\": " + "\"" + symEl->Name() + "\"" + ",\n";

    json += (std::string)"\t\t\t\"tilegeom\": " + "\"" + (geomType == Symbolizer::TileGeomAdd ? "add" : "replace") + "\",\n";
    
    json += (std::string)"\t\t\t\"substyles\" : [\n";
    
    for (unsigned int ii=0;ii<subSyms.size();ii++)
    {
        subSyms[ii].toString(json);
        if (ii < subSyms.size()-1)
            json += ",";
        json += "\n";
    }
    json += (std::string)"\t\t\t]\n";
    json += "\t\t}";
}

// Convert a color value from rgba(r,g,b,a) to #rrggbbaa if necessary
bool ConvertColor(const std::string &inVal,std::string &outVal)
{
    std::string floatMatch = "([+-]?(?=[.]?[0-9])[0-9]*(?:[.][0-9]*)?(?:[Ee][+-]?[0-9]+)?)";
    std::string commaMatch = "\\s*,\\s*";
    RegExRef exp(new boost::regex("rgba\\(" + floatMatch + commaMatch + floatMatch + commaMatch + floatMatch + commaMatch + floatMatch + "\\)"));
    boost::smatch what;
    if (boost::regex_match(inVal, what, *exp, boost::match_default))
    {
        if (what.size() != 5)
            return false;

        int r = (int)boost::lexical_cast<double>(what[1]);
        int g = (int)boost::lexical_cast<double>(what[2]);
        int b = (int)boost::lexical_cast<double>(what[3]);
        int a = (int)(boost::lexical_cast<double>(what[4])*255);
        char outStr[1024];
        sprintf(outStr,"#%02x%02x%02x%02x",(unsigned int)(a&0xFF),(unsigned int)(r&0xFF),(unsigned int)(g&0xFF),(unsigned int)(b&0xFF));
        outVal = outStr;
        
        return true;
    } else {
        return false;
    }
}

void MapnikConfig::CompiledSymbolizerTable::SubSymbolizer::toString(std::string &json)
{
    XMLElement *symEl = xmlEl;

    std::string indent = "\t\t\t\t";
    json += indent + "{\n";
    if (minScaleDenom > 0)
        json += indent + "\t\"minscaledenom\": " + std::to_string(minScaleDenom) + ",\n";
    if (maxScaleDenom > 0)
        json += indent + "\t\"maxscaledenom\": " + std::to_string(maxScaleDenom) + ",\n";
    json += indent + "\t\"drawpriority\": " + std::to_string(drawPriority) + ",\n";
    
    const char *bodyStr = symEl->GetText();
    
    // Convert the attributes
    for (const XMLAttribute *attr = symEl->FirstAttribute();attr;attr = attr->Next())
    {
        const char *valStr = attr->Value();
        bool isNumber = true;
        try
        {
            double x = boost::lexical_cast<double>(valStr);
            x = 0.0;
        }
        catch(...)
        {
            isNumber = false;
        }
        
        json += indent + "\t\"" + attr->Name() + "\": ";
        if (isNumber)
            json += attr->Value();
        else {
            std::string attrVal;
            if (!ConvertColor(attr->Value(),attrVal))
                attrVal = attr->Value();
            json += (std::string)"\"" + attrVal + "\"";
        }
        if (attr->Next() || bodyStr)
            json += ",";
        json += "\n";
    }
    
    // Some of these have a body as well
    if (bodyStr)
    {
        json += indent + "\t\"" + "attribute" + "\": " + "\"" + bodyStr + "\"\n";
    }
    
    json += indent + "}";
}

bool MapnikConfig::CompiledSymbolizerTable::writeJSON(std::string &json)
{
    json += "{\n";
    
    json += "\t\"parameters\": {\n";
    json += (std::string)"\t\t\"minLevel\": " + std::to_string(minLevel) + ",\n";
    json += (std::string)"\t\t\"maxLevel\": " + std::to_string(maxLevel) + "\n";
    json += "\t},\n";
    
    json += "\t\"layers\": [";
    for (std::set<std::string>::iterator it = layerNames.begin();it != layerNames.end();++it)
    {
        if (it != layerNames.begin())
            json += ",";
        json += (std::string)"\"" + *it + "\"";
    }
    json += "\t],\n";
    
    json += "\t\"styles\": [\n";
    
    // Work through the individual symbolizers
    for (unsigned int ii=0;ii<symGroups.size();ii++)
    {
        std::string symJson;
        symGroups[ii].toString(symJson);
        json += symJson;
        
        if (ii != symGroups.size()-1)
            json += ",";
        json += "\n";
    }
    json += "\t]\n";
    
    json += "}\n";
    
    return true;
}

bool MapnikConfig::writeTileJSON(std::string &json,const std::string &webDbName,const std::string &webDbURL)
{
    json += "{\n";

    json += "\t\"style\": \"" + webDbURL + webDbName + "_styles.json\",\n";
    
    json += "\t\"tiles\":[\n";
    json += "\t\t\"" + webDbURL + "{z}/{x}/{y}.mvt\"\n";
    json += "\t]\n";
    
    json += "}\n";

    return true;
}

MapnikConfig::Filter::Filter()
{
}

MapnikConfig::Filter::~Filter()
{
}

bool MapnikConfig::Filter::isValid()
{
    return !comparisons.empty();
}

bool MapnikConfig::Filter::isEmpty()
{
    return !isValid();
}

bool MapnikConfig::Filter::Comparison::isValid()
{
    return exp.get() != NULL;
}

void MapnikConfig::Filter::Comparison::setFilter(const std::string &filterText)
{
    exp = RegExRef(new boost::regex("\\(\\[(\\w+)\\]\\s(=|>|<|<=|>=|&gt;|&lt;|&lt;&gt;|!=)\\s(.*?)\\)"));
    boost::smatch what;
    if (boost::regex_match(filterText, what, *exp, boost::match_default))
    {
        if (what.size() != 4)
            throw 1;
        
        attrName = what[1];
        std::string strVal = what[3];
        std::string attrComp = what[2];
        if (!attrComp.compare("="))
        {
            compareType = CompareEqual;
        } else if (!attrComp.compare("&lt;&gt;") || !attrComp.compare("&gt;&lt;") || !attrComp.compare("!="))
        {
            compareType = CompareNotEqual;
        } else if (!attrComp.compare("&lt;") || !attrComp.compare("<"))
        {
            compareType = CompareLess;
        } else if (!attrComp.compare("&gt;") || !attrComp.compare(">"))
        {
            compareType = CompareMore;
        } else if (!attrComp.compare("<="))
        {
            compareType = CompareLessEqual;
        } else if (!attrComp.compare(">="))
        {
            compareType = CompareMoreEqual;
        } else
        {
            // Unknown comparison type
            throw 1;
        }
        
        if (strVal.empty())
        {
            throw 1;
        }
        if (strVal[0] == '\'')
        {
            if (strVal.back() != '\'')
                throw 1;
            attrValStr = strVal.substr(1,strVal.length()-2);
            compareValueType = CompareString;
        } else {
            attrValReal = boost::lexical_cast<double>(strVal);
            compareValueType = CompareReal;
        }
    } else {
        throw 1;
    }
}

void MapnikConfig::Filter::setFilter(const std::string &filterText)
{
    filter = filterText;
    
    try {
        // Let's try two ANDs (yes, this is stupid)
        exp = RegExRef(new boost::regex("(\\(.+\\))\\s(?:and)\\s(\\(.+\\))\\s(?:and)\\s(\\(.+\\))"));
        boost::smatch what;
        if (boost::regex_match(filterText, what, *exp, boost::match_default))
        {
            if (what.size() != 4)
                throw 1;
            
            logicalOp = OperatorAND;
            for (unsigned int ii=0;ii<3;ii++)
            {
                Comparison comp;
                comp.setFilter(what[ii+1]);
                comparisons.push_back(comp);
            }
        } else {
            // Let's try an AND
            exp = RegExRef(new boost::regex("(\\(.+\\))\\s(?:and)\\s(\\(.+\\))"));
            if (boost::regex_match(filterText, what, *exp, boost::match_default))
            {
                if (what.size() != 3)
                    throw 1;

                logicalOp = OperatorAND;
                for (unsigned int ii=0;ii<2;ii++)
                {
                    Comparison comp;
                    comp.setFilter(what[ii+1]);
                    comparisons.push_back(comp);
                }
            } else {
                // Okay, how about a single operation
                Comparison comp;
                comp.setFilter(filterText);
                logicalOp = OperatorAND;
                if (comp.isValid())
                    comparisons.push_back(comp);
            }
        }
    }
    catch (...)
    {
        exp.reset();
        throw (std::string)"Could not create filter.  May be too complex:\n" + filterText.c_str();
    }
}

MapnikConfig::Rule::Rule() :
    xmlEl(NULL), minScale(-1), maxScale(-1)
{
}

MapnikConfig::Rule MapnikConfig::Rule::ruleForDataType(MapnikConfig *mapnikConfig, SymbolDataType dataType) const
{
    Rule rule = *this;
    rule.symbolizers.clear();
    
    for (unsigned int ii=0;ii<symbolizers.size();ii++)
    {
        Symbolizer &sym = mapnikConfig->symbolizerTable.symbolizers[symbolizers[ii]];
        if (sym.dataType == dataType)
            rule.symbolizers.push_back(symbolizers[ii]);
    }
    
    return rule;
}

MapnikConfig::Rule::~Rule()
{
}

MapnikConfig::Style::Style() :
    xmlEl(NULL)
{
    
}

MapnikConfig::Style::~Style()
{
    
}

MapnikConfig::ShapefileDataSource::ShapefileDataSource(const std::string &fileName,const std::vector<std::string> &paths)
: fileName(fileName)
{
    // Look for the file in one of the paths first
    std::string baseName = boost::filesystem::basename(fileName) + boost::filesystem::extension(fileName);
    if (!baseName.empty())
    {
        for (unsigned int ii=0;ii<paths.size();ii++)
        {
            std::string fullName = paths[ii] + "/" + baseName;
            if (boost::filesystem::exists(fullName))
            {
                this->fileName = fullName;
                break;
            }
        }
    }
}

std::string MapnikConfig::ShapefileDataSource::getName()
{
    return fileName;
}

std::string MapnikConfig::ShapefileDataSource::getShapefileName(Layer *layer,const char *outDir)
{
    return fileName;
}

MapnikConfig::DataSourceOGR::DataSourceOGR(const std::string &fileName)
: fileName(fileName)
{
}

std::string MapnikConfig::DataSourceOGR::getName()
{
    return fileName;
}

std::string MapnikConfig::DataSourceOGR::getShapefileName(Layer *layer, const char *outdir)
{
    if (shapeFileName.empty())
    {
        // Look for the file in one of the paths first
        std::string baseName = boost::filesystem::basename(fileName) + boost::filesystem::extension(fileName);
        if (baseName.empty())
        return "";

        shapeFileName = (std::string)outdir + "/" + baseName + ".shp";
        
        // Run ogr2ogr to generate a shapefile.  This is sort of stupid.
        std::string execStr = ((std::string)"/usr/local/bin/ogr2ogr " + "\"" + shapeFileName + "\" " + "\"" + fileName + "\"");
        if (system(execStr.c_str()))
        {
            fprintf(stderr, "Failed to execute postgis request:\n%s\n",execStr.c_str());
            throw execStr;
        }
    }
    
    return shapeFileName;
}

std::string MapnikConfig::PostGISDataSource::getName()
{
    return idStr;
}

std::string MapnikConfig::PostGISDataSource::getShapefileName(Layer *layer,const char *outDir)
{
    if (shapefileName.empty())
    {
        shapefileName = (std::string)outDir + "/" + layer->name + ".shp";
        
        // Let's see if it's there already
        FILE *fp = fopen(shapefileName.c_str(),"r");
        if (fp)
        {
            fclose(fp);
            return shapefileName;
        }
        
        // Run pgsql2shp to generate a shapefile.
        // This way we don't have to have GDAL installed with postgis.  I know, I know.
        std::string query = "SELECT * FROM " + table;
        boost::replace_all(query, "\"", "\\\"");
        boost::replace_all(query, "\n", " ");
        std::string execStr = ((std::string)"/usr/local/bin/pgsql2shp -f " + "\"" + shapefileName + "\"" + " " + dbname + " " + "\"" + query + "\"");
        if (system(execStr.c_str()))
        {
            fprintf(stderr, "Failed to execute postgis request:\n%s\n",execStr.c_str());
            throw execStr;
        }
    }
    
    return shapefileName;
}

MapnikConfig::Layer::Layer() :
    xmlEl(NULL)
{
    
}

MapnikConfig::Layer::~Layer()
{
}

MapnikConfig::SortedLayer::SortedLayer(MapnikConfig *mapnikConfig,const Layer &layer)
{
    valid = false;
    
    // Work through the styles that apply to this layer
    for (unsigned si=0;si<layer.styleNames.size();si++)
    {
        const MapnikConfig::Style *style = mapnikConfig->findStyle(layer.styleNames[si]);
        if (!style)
        {
            fprintf(stderr, "Dangling style name %s in layer %s",layer.styleNames[si].c_str(),layer.name.c_str());
            return;
        }

        // Look through the style's rules
        for (unsigned int ri=0;ri<style->rules.size();ri++)
        {
            const MapnikConfig::Rule &inRule = style->rules[ri];
            
            // Work through the data types
            for (unsigned int di=0;di<MapnikConfig::SymbolDataUnknown;di++)
            {
                MapnikConfig::Rule rule = inRule.ruleForDataType(mapnikConfig,(SymbolDataType)di);
                if (rule.symbolizers.empty())
                    continue;
                int maxScale = 0;
                if (rule.maxScale > -1)
                    maxScale = rule.maxScale;
                
                // Look for a style that matches this filter
                SortedStyle *thisSStyle = NULL;
                for (unsigned ssi=0;ssi<sortStyles.size();ssi++)
                {
                    if (!sortStyles[ssi].filter.filter.compare(rule.filter.filter) && sortStyles[ssi].dataType == di)
                    {
                        thisSStyle = &sortStyles[ssi];
                        break;
                    }
                }
                // Or add one
                if (!thisSStyle)
                {
                    sortStyles.resize(sortStyles.size()+1);
                    thisSStyle = &sortStyles.back();
                    thisSStyle->maxScale = maxScale;
                    thisSStyle->filter = rule.filter;
                    thisSStyle->dataType = (SymbolDataType)di;
                }
                // Might need this at a lower loading level
                if (maxScale > thisSStyle->maxScale)
                    thisSStyle->maxScale = maxScale;
                
                // Now look for a style entry here
                SortedStyle::Instance *inst = NULL;
                for (unsigned int si=0;si<thisSStyle->styleInstances.size();si++)
                    if (thisSStyle->styleInstances[si].style == style)
                    {
                        inst = &thisSStyle->styleInstances[si];
                        break;
                    }
                if (!inst)
                {
                    thisSStyle->styleInstances.resize(thisSStyle->styleInstances.size()+1);
                    inst = &thisSStyle->styleInstances.back();
                    inst->style = style;
                }
                
                // And move the rule over
                inst->rules.push_back(rule);
            }
        }
    }
    
    valid = true;
}

MapnikConfig::MapnikConfig() :
    paramEl(NULL), defaultGeomType(Symbolizer::TileGeomAdd)
{
}

MapnikConfig::~MapnikConfig()
{
}

bool MapnikConfig::parseXML(XMLDocument *doc,const std::vector<std::string> &paths,std::string &error)
{
    try
    {
        // Top level Map element
        XMLElement *mapEl = doc->FirstChildElement("Map");
        if (!mapEl)
        {
            error = "Missing Map node";
            return false;
        }
        
        // Parameters, which we're not parsing at the moment
        paramEl = mapEl->FirstChildElement("Parameters");
        if (!paramEl)
        {
            error = "Expecting Parameters node";
            return false;
        }
        
        // Now for the styles
        for (XMLElement *styleEl = mapEl->FirstChildElement("Style");
             styleEl; styleEl = styleEl->NextSiblingElement("Style"))
        {
            Style style;

            // Name
            style.name = styleEl->Attribute("name");
            if (style.name.length() == 0)
            {
                error = "Expecting name for style";
                return false;
            }

            // Filter mode
            const char* filter = styleEl->Attribute("filter-mode");
            if (!filter)
            {
                error = "Expecting filter-mode for style " + style.name;
                return false;
            }
            if (!strcmp(filter,"first"))
                style.filterMode = FilterFirst;
            else if (!strcmp(filter,"all"))
                style.filterMode = FilterAll;
            else {
                error = "Expecting 'all' or 'first' for filter-mode in style " + style.name;
                return false;
            }
            
            // Now for the rules
            int ruleCount = 0;
            for (XMLElement *ruleEl = styleEl->FirstChildElement("Rule");
                 ruleEl; ruleEl = ruleEl->NextSiblingElement("Rule"))
            {
                Rule rule;
                
                // Min and max scale (optional)
                XMLElement *minScale = ruleEl->FirstChildElement("MinScaleDenominator");
                if (minScale)
                    rule.minScale = std::stoi(minScale->GetText());
                XMLElement *maxScale = ruleEl->FirstChildElement("MaxScaleDenominator");
                if (maxScale)
                    rule.maxScale = std::stoi(maxScale->GetText());
                
                // Filter (optional)
                XMLElement *filterEl = ruleEl->FirstChildElement("Filter");
                if (filterEl)
                {
                    std::string filterText = filterEl->GetText();
                    rule.filter.setFilter(filterText);
                    // Note: debugging
    //                if (!rule.filter.isValid())
    //                {
    //                    error = "Could not parse filter (" + filterText + ") in rule in style " + style.name;
    //                    return false;
    //                }
                }

                // Work through the symbolizers
                int symCount = 0;
                for (XMLElement *symEl = ruleEl->FirstChildElement();
                     symEl; symEl = symEl->NextSiblingElement())
                {
                    std::string name = symEl->Name();
                    SymbolDataType dataType = SymbolDataUnknown;
                    SymbolizerType symType = UnknownSymbolizer;
                    if (!name.compare("MarkersSymbolizer"))
                    {
                        dataType = SymbolDataPoint;
                        symType = MarkersSymbolizer;
                    } else if (!name.compare("LineSymbolizer"))
                    {
                        dataType = SymbolDataLinear;
                        symType = LineSymbolizer;
                    } else if (!name.compare("TextSymbolizer"))
                    {
                        dataType = SymbolDataPoint;
                        symType = TextSymbolizer;
                    } else if (!name.compare("PolygonSymbolizer"))
                    {
                        dataType = SymbolDataAreal;
                        symType = PolygonSymbolizer;
                    }
                    
                    if (symType != UnknownSymbolizer)
                    {
                        int whichSym = (int)symbolizerTable.symbolizers.size();
                        // Set up the symbolizer
                        Symbolizer sym;
                        sym.dataType = dataType;
                        sym.symType = symType;
                        sym.name = style.name + "_rule" + std::to_string(ruleCount) + "_" + std::to_string(symCount);
                        sym.xmlEl = symEl;
                        sym.geomType = Symbolizer::TileGeomAdd;
                        sym.minScaleDenom = rule.minScale;
                        sym.maxScaleDenom = rule.maxScale;
    //                    // If the min scale isn't set for this rule, then the geometry sticks around
    //                    if (rule.minScale == -1)
    //                        sym.geomType = Symbolizer::TileGeomAdd;
    //                    else
    //                        sym.geomType = Symbolizer::TileGeomReplace;
                        symbolizerTable.symbolizers.push_back(sym);
                        rule.symbolizers.push_back(whichSym);
                        
                        // Look for attributes we'll want to keep
                        const char *bodyStr = symEl->GetText();
                        if (bodyStr)
                        {
                            boost::regex regex("\\[(\\w+)\\]");
                            boost::smatch what;
                            if (boost::regex_match((std::string)bodyStr, what, regex, boost::match_default))
                            {
                                if (what.size() == 2)
                                {
                                    std::string attrName = what[1];
                                    rule.attrs.insert(attrName);
                                } else {
                                    error = (std::string)"Confusing format in body for symbolizer in style " + style.name;
                                    return false;
                                }
                            } else {
                                error = (std::string)"Could not parse body (" + bodyStr + ") of symbolizer in style " + style.name;
                                return false;
                            }
                        }
                        symCount++;
                    } else if (strstr(name.c_str(),"Symbolizer"))
                    {
                        error = "Unknown symbolizer type " + name + " in rule in style " + style.name;
                        return false;
                    }
                }
                
                ruleCount++;
                style.rules.push_back(rule);
            }
            
            styles.push_back(style);
        }
        
        // And the layers
        for (XMLElement *layerEl = mapEl->FirstChildElement("Layer");
             layerEl; layerEl = layerEl->NextSiblingElement("Layer"))
        {
            Layer layer;
            
            // Name
            layer.name = layerEl->Attribute("name");
            if (layer.name.length() == 0)
            {
                error = "Expecting name for layer";
                return false;
            }
            
            // SRS
            layer.srs = layerEl->Attribute("srs");
            
            // References to styles
            for (XMLElement *styleRefEl = layerEl->FirstChildElement("StyleName");
                 styleRefEl; styleRefEl = styleRefEl->NextSiblingElement("StyleName"))
                layer.styleNames.push_back(styleRefEl->GetText());
            
            // Data sources
            for (XMLElement *dataSourceEl = layerEl->FirstChildElement("Datasource");
                 dataSourceEl; dataSourceEl = dataSourceEl->NextSiblingElement("Datasource"))
            {
                // Two parameters, one with the file name and one with the data type
                std::string dataType,fileName,dbname,table,idStr;
                for (XMLElement *paramEl = dataSourceEl->FirstChildElement("Parameter");
                     paramEl; paramEl = paramEl->NextSiblingElement("Parameter"))
                {
                    std::string name = paramEl->Attribute("name");
                    if (!name.compare("file"))
                    {
                        fileName = paramEl->GetText();
                    } else if (!name.compare("type"))
                    {
                        dataType = paramEl->GetText();
                    } else if (!name.compare("dbname"))
                    {
                        dbname = paramEl->GetText();
                    } else if (!name.compare("table"))
                    {
                        table = paramEl->GetText();
                    } else if (!name.compare("id"))
                    {
                        idStr = paramEl->GetText();
                    } else
                    {
                        // Note: Tolerant of fields we don't know here
    //                    error = "Unknown name " + name + " for data source paramater in layer " + layer.name;
    //                    return false;
                    }
                }
                
                DataSource *dataSource = NULL;
                if (!dataType.compare("shape"))
                {
                    ShapefileDataSource *shapeDataSource = new ShapefileDataSource(fileName,paths);
                    dataSource = shapeDataSource;
                } else if (!dataType.compare("postgis"))
                {
                    PostGISDataSource *pgDataSource = new PostGISDataSource();
                    pgDataSource->dbname = dbname;
                    pgDataSource->table = table;
                    pgDataSource->idStr = idStr;
                    dataSource = pgDataSource;
                } else if (!dataType.compare("ogr"))
                {
                    DataSourceOGR *ogrDataSource = new DataSourceOGR(fileName);
                    dataSource = ogrDataSource;
                } else
                {
                    error = "Data type: " + dataType + " is unsupported for data sources in layer " + layer.name;
                    return false;
                }
                
                layer.dataSources.push_back(DataSourceRef(dataSource));
            }
            
            if (layer.dataSources.size() != 1)
            {
                error = "Expecting exactly one data source per layer: " + layer.name;
                return false;
            }
            
            layers.push_back(layer);
        }
    }
    catch (const std::string &errStr)
    {
        error = errStr;
        return false;
    }
    catch (...)
    {
        error = "Unknown problem";
        return false;
    }
    
    return true;
}

const MapnikConfig::Style *MapnikConfig::findStyle(const std::string &styleName)
{
    for (unsigned int ii=0;ii<styles.size();ii++)
        if (!styleName.compare(styles[ii].name))
            return &styles[ii];
    
    return NULL;
}
