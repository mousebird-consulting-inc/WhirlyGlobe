//
//  MapnikConfig.cpp
//  vector_dice
//
//  Created by Steve Gifford on 1/1/14.
//  Copyright (c) 2014 mousebird consulting. All rights reserved.
//

#include "MapnikConfig.h"
#include <boost/lexical_cast.hpp>

using namespace tinyxml2;

bool MapnikConfig::SymbolizerTable::writeJSON(std::string &json)
{
    json += "{\n";
    
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
    for (unsigned int ii=0;ii<symbolizerElements.size();ii++)
    {
        XMLElement *symEl = symbolizerElements[ii];
        
        json += "\t\t{\n";
        json += (std::string)"\t\t\t\"type\": " + "\"" + symEl->Name() + "\"";
        if (symEl->FirstAttribute())
            json += ",";
        json += "\n";
        
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
            
            json += (std::string)"\t\t\t\"" + attr->Name() + "\": ";
            if (isNumber)
                json += attr->Value();
            else
                json += (std::string)"\"" + attr->Value() + "\"";
            if (attr->Next() || bodyStr)
                json += ",";
            json += "\n";
        }
        
        // Some of these have a body as well
        if (bodyStr)
        {
            json += (std::string)"\t\t\t\"" + "attribute" + "\": " + "\"" + bodyStr + "\"";
        }
        
        json += "\t\t}";
        if (ii != symbolizerElements.size()-1)
            json += ",";
        json += "\n";
    }
    json += "\t]\n";
    
    
    json += "}\n";
    
    return true;
}

MapnikConfig::Filter::Filter()
: exp(NULL)
{
}

MapnikConfig::Filter::~Filter()
{
}

bool MapnikConfig::Filter::isValid()
{
    return (exp.get() != NULL);
}

bool MapnikConfig::Filter::isEmpty()
{
    return !isValid();
}

void MapnikConfig::Filter::setFilter(const std::string &filterText)
{
    filter = filterText;
    
    try {
        exp = RegExRef(new boost::regex("\\(\\[(\\w+)\\]\\s(=|&gt|&lt|&lt&gt)\\s\'(\\w+)\'\\)"));
        boost::smatch what;
        if (boost::regex_match(filterText, what, *exp, boost::match_default))
        {
            if (what.size() != 4)
                throw 1;

            attrName = what[1];
            attrVal = what[3];
            std::string attrComp = what[2];
            if (!attrComp.compare("="))
            {
                compareType = CompareEqual;
            } else if (!attrComp.compare("&lt&gt") || !attrComp.compare("&gt&lt"))
            {
                compareType = CompareNotEqual;
            } else if (!attrComp.compare("&lt"))
            {
                compareType = CompareLess;
            } else if (!attrComp.compare("&gt"))
            {
                compareType = CompareMore;
            } else {
                // Unknown comparison type
                throw 1;
            }
        } else {
            throw 1;
        }
    }
    catch (...)
    {
        exp.reset();
    }
}

MapnikConfig::Rule::Rule() :
    xmlEl(NULL), minScale(-1), maxScale(-1)
{
    
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

MapnikConfig::Layer::Layer() :
    xmlEl(NULL)
{
    
}

MapnikConfig::Layer::~Layer()
{
    
}

MapnikConfig::MapnikConfig() :
    paramEl(NULL)
{
    
}

MapnikConfig::~MapnikConfig()
{
    
}

bool MapnikConfig::parseXML(XMLDocument *doc,std::string &error)
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
                if (!rule.filter.isValid())
                {
                    error = "Could not parse filter (" + filterText + ") in rule in style " + style.name;
                    return false;
                }
            }

            // Work through the symbolizers
            for (XMLElement *symEl = ruleEl->FirstChildElement();
                 symEl; symEl = symEl->NextSiblingElement())
            {
                std::string name = symEl->Name();
                if (!name.compare("MarkersSymbolizer") || !name.compare("LineSymbolizer") ||
                    !name.compare("TextSymbolizer") || !name.compare("PolygonSymbolizer"))
                {
                    int whichSym = (int)symbolizerTable.symbolizerElements.size();
                    symbolizerTable.symbolizerElements.push_back(symEl);
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
                } else if (strstr(name.c_str(),"Symbolizer"))
                {
                    error = "Unknown symbolizer type " + name + " in rule in style " + style.name;
                    return false;
                }
            }
            
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
            std::string dataType,fileName;
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
                } else {
                    error = "Unknown name " + name + " for data source paramater in layer " + layer.name;
                    return false;
                }
            }
            if (dataType.compare("shape"))
            {
                error = "Data type: " + dataType + " is unsupported for data sources in layer " + layer.name;
                return false;
            }
            layer.dataSources.push_back(fileName);
        }
        
        if (layer.dataSources.size() != 1)
        {
            error = "Expecting exactly one data source per layer: " + layer.name;
            return false;
        }
        
        layers.push_back(layer);
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
