/*
*  MapboxVectorStyleSetC.h
*  WhirlyGlobeLib
*
*  Created by Steve Gifford on 4/8/20.
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

#import "MapboxVectorStyleSetC.h"
#import "MapboxVectorStyleLayer.h"
#import "SharedAttributes.h"
#import "WhirlyKitLog.h"
#import "MapboxVectorStyleBackground.h"
#import <regex>

namespace WhirlyKit
{

bool MapboxRegexField::parse(const std::string &textField)
{
    // Parse out the {} groups in the text
    // TODO: We're missing a boatload of stuff in the spec
    std::regex regex{R"([{}]+)"};
    std::sregex_token_iterator it{textField.begin(), textField.end(), regex, -1};
    std::vector<std::string> regexChunks{it, {}};
    bool isJustText = textField[0] != '{';
    for (auto regexChunk : regexChunks) {
        if (regexChunk.empty())
            continue;
        MapboxTextChunk textChunk;
        if (isJustText) {
            textChunk.str = regexChunk;
        } else {
            textChunk.keys.push_back(regexChunk);
            // For some reason name:en is sometimes name_en
            std::string textVariant = regexChunk;
            textVariant = std::regex_replace(textVariant, std::regex(":"), "_");
            textChunk.keys.push_back(textVariant);
        }
        chunks.push_back(textChunk);
        isJustText = !isJustText;
    }

    valid = true;
    
    return true;
}

bool MapboxRegexField::parse(const std::string &fieldName,
                                                 MapboxVectorStyleSetImpl *styleSet,
                                                 DictionaryRef styleEntry)
{
    std::string textField = styleSet->stringValue(fieldName, styleEntry, "");
    if (!textField.empty())
        parse(textField);
    
    return true;
}

std::string MapboxRegexField::build(DictionaryRef attrs)
{
    bool found = false;
    bool didLookup = false;
    std::string text = "";
    
    for (auto chunk : chunks) {
        if (!chunk.str.empty())
            text += chunk.str;
        else {
            for (auto key : chunk.keys) {
                didLookup = true;
                if (attrs->hasField(key)) {
                    found = true;
                    std::string keyVal = attrs->getString(key);
                    if (!keyVal.empty()) {
                        text += keyVal;
                        break;
                    }
                }
            }
        }
    }

    if (didLookup && !found)
        return "";
    
    if (!text.empty() && text[text.size()-1] == '\n')
        text.resize(text.size()-1);

    return text;
}

MaplyVectorFunctionStop::MaplyVectorFunctionStop()
: zoom(-1.0), val(0.0)
{
}

bool MaplyVectorFunctionStops::parse(DictionaryRef entry,MapboxVectorStyleSetImpl *styleSet,bool isText)
{
    base = entry->getDouble("base",1.0);
    
    std::vector<DictionaryEntryRef> dataArray = entry->getArray("stops");
    if (dataArray.size() < 2)
    {
        wkLogLevel(Warn, "Expecting at least two arguments for function stops.");
        return false;
    }
    for (auto stop : dataArray) {
        if (stop->getType() == DictTypeArray) {
            std::vector<DictionaryEntryRef> stopEntries = stop->getArray();
            if (stopEntries.size() != 2) {
                wkLogLevel(Warn,"Expecting two arguments in each entry for a function stop.");
                return false;
            }
            
            MaplyVectorFunctionStop fStop;
            fStop.zoom = stopEntries[0]->getDouble();
            if (stopEntries[1]->getType() == DictTypeDouble || stopEntries[1]->getType() == DictTypeInt) {
                fStop.val = stopEntries[1]->getDouble();
            } else {
                switch (stopEntries[1]->getType())
                {
                    case DictTypeString:
                        if (isText)
                            fStop.textField.parse(stopEntries[1]->getString());
                        else
                            fStop.color = styleSet->colorValue("", stopEntries[1], NULL, NULL, false);
                        break;
                    case DictTypeObject:
                        fStop.color = RGBAColorRef(new RGBAColor(stopEntries[1]->getColor()));
                        break;
                    default:
                        wkLogLevel(Warn, "Expecting color compatible object in function stop.");
                        return false;
                        break;
                }
            }
            
            stops.push_back(fStop);
        } else {
            wkLogLevel(Warn, "Expecting arrays in the function stops.");
            return false;
        }
    }

    return true;
}

double MaplyVectorFunctionStops::valueForZoom(double zoom)
{
    const MaplyVectorFunctionStop *a = &stops[0];
    const MaplyVectorFunctionStop *b = nullptr;
    if (zoom <= a->zoom)
        return a->val;
    for (int which = 1;which < stops.size(); which++)
    {
        b = &stops[which];
        if (a->zoom <= zoom && zoom < b->zoom)
        {
            double ratio = 1.0;
            if (base == 1.0) {
                ratio = (zoom-a->zoom)/(b->zoom-a->zoom);
            } else {
                double soFar = zoom-a->zoom;
                ratio = (pow(base, soFar) - 1.0) / (pow(base,b->zoom-a->zoom) - 1.0);
            }
            return ratio * (b->val-a->val) + a->val;
        }
        a = b;
    }

    return b ? b->val : 0;
}

RGBAColorRef MaplyVectorFunctionStops::colorForZoom(double zoom)
{
    const MaplyVectorFunctionStop *a = &stops[0];
    const MaplyVectorFunctionStop *b = nullptr;
    if (zoom <= a->zoom)
        return a->color;
    for (int which = 1;which < stops.size(); which++)
    {
        b = &stops[which];
        if (a->zoom <= zoom && zoom < b->zoom)
        {
            double ratio = 1.0;
            if (base == 1.0) {
                ratio = (zoom-a->zoom)/(b->zoom-a->zoom);
            } else {
                double soFar = zoom-a->zoom;
                ratio = (pow(base, soFar) - 1.0) / (pow(base,b->zoom-a->zoom) - 1.0);
            }
            float ac[4],bc[4];
            a->color->asUnitFloats(ac);
            b->color->asUnitFloats(bc);
            float res[4];
            for (unsigned int ii=0;ii<4;ii++)
                res[ii] = ratio * (bc[ii]-ac[ii]) + ac[ii];
            return RGBAColorRef(new RGBAColor(RGBAColor::FromUnitFloats(res)));
        }
        a = b;
    }

    return b ? b->color : RGBAColorRef();
}

MapboxRegexField MaplyVectorFunctionStops::textForZoom(double zoom)
{
    const MaplyVectorFunctionStop *a = &stops[0];
    const MaplyVectorFunctionStop *b = nullptr;
    if (zoom <= a->zoom)
        return a->textField;
    for (int which = 1;which < stops.size(); which++)
    {
        b = &stops[which];
        if (a->zoom <= zoom && zoom < b->zoom)
            return a->textField;
        a = b;
    }

    return b ? b->textField : MapboxRegexField();
}

double MaplyVectorFunctionStops::minValue()
{
    double val = MAXFLOAT;

    for (auto stop : stops)
        val = std::min(val,stop.val);

    return val;
}

double MaplyVectorFunctionStops::maxValue()
{
    double val = -MAXFLOAT;

    for (auto stop : stops) {
        val = std::max(val,stop.val);
    }

    return val;
}

MapboxTransDouble::MapboxTransDouble(double value)
{
    val = value;
}

MapboxTransDouble::MapboxTransDouble(MaplyVectorFunctionStopsRef inStops)
{
    val = 0.0;
    stops = inStops;
}

double MapboxTransDouble::valForZoom(double zoom)
{
    return stops ? stops->valueForZoom(zoom) : val;
}

bool MapboxTransDouble::isExpression()
{
    return stops.get() != nullptr;
}

FloatExpressionInfoRef MapboxTransDouble::expression()
{
    if (!stops)
        return FloatExpressionInfoRef();
    
    FloatExpressionInfoRef floatExp(new FloatExpressionInfo());
    floatExp->type = ExpressionExponential;
    floatExp->base = stops->base;
    floatExp->stopInputs.resize(stops->stops.size());
    floatExp->stopOutputs.resize(stops->stops.size());
    for (unsigned int ii=0;ii<stops->stops.size();ii++) {
        floatExp->stopInputs[ii] = stops->stops[ii].zoom;
        floatExp->stopOutputs[ii] = stops->stops[ii].val;
    }
    
    return floatExp;
}


double MapboxTransDouble::minVal()
{
    if (stops) {
        return stops->minValue();
    } else
        return val;
}

double MapboxTransDouble::maxVal()
{
    if (stops) {
        return stops->maxValue();
    } else
        return val;
}

MapboxTransColor::MapboxTransColor(RGBAColorRef color)
: color(color), useAlphaOverride(false), alpha(1.0)
{
}

MapboxTransColor::MapboxTransColor(MaplyVectorFunctionStopsRef stops)
: useAlphaOverride(false), alpha(1.0), stops(stops)
{
}

void MapboxTransColor::setAlphaOverride(double alphaOverride)
{
    useAlphaOverride = true;
    alpha = alphaOverride;
}

RGBAColor MapboxTransColor::colorForZoom(double zoom)
{
    RGBAColor theColor = *(stops ? stops->colorForZoom(zoom) : color);

    if (useAlphaOverride) {
        theColor.a = alpha * 255.0;
    }
    
    return theColor;
}

ColorExpressionInfoRef MapboxTransColor::expression()
{
    if (!stops)
        return ColorExpressionInfoRef();
    
    ColorExpressionInfoRef colorExp(new ColorExpressionInfo());
    colorExp->type = ExpressionExponential;
    colorExp->base = stops->base;
    colorExp->stopInputs.resize(stops->stops.size());
    colorExp->stopOutputs.resize(stops->stops.size());
    for (unsigned int ii=0;ii<stops->stops.size();ii++) {
        colorExp->stopInputs[ii] = stops->stops[ii].zoom;
        if (stops->stops[ii].color)
            colorExp->stopOutputs[ii] = *(stops->stops[ii].color);
    }
    
    return colorExp;
}

MapboxTransText::MapboxTransText(const std::string &inText)
{
    textField.parse(inText);
}

MapboxTransText::MapboxTransText(MaplyVectorFunctionStopsRef stops)
: stops(stops)
{
}

MapboxRegexField MapboxTransText::textForZoom(double zoom)
{
    if (stops) {
        return stops->textForZoom(zoom);
    }
    
    return textField;
}

MapboxVectorStyleSetImpl::MapboxVectorStyleSetImpl(Scene *inScene,CoordSystem *coordSys,VectorStyleSettingsImplRef settings)
: scene(inScene), currentID(0), tileStyleSettings(settings), coordSys(coordSys), zoomSlot(-1)
{
    vecManage = (VectorManager *)scene->getManager(kWKVectorManager);
    wideVecManage = (WideVectorManager *)scene->getManager(kWKWideVectorManager);
    markerManage = (MarkerManager *)scene->getManager(kWKMarkerManager);
    labelManage = (LabelManager *)scene->getManager(kWKLabelManager);
    compManage = (ComponentManager *)scene->getManager(kWKComponentManager);

    // We'll look for the versions that do expressions first and
    //  then fall back to the simpler ones
    Program *prog = scene->findProgramByName(MaplyScreenSpaceExpShader);
    if (!prog)
        prog = scene->findProgramByName(MaplyScreenSpaceDefaultShader);
    if (prog)
        screenMarkerProgramID = prog->getId();

    prog = scene->findProgramByName(MaplyTriangleExpShader);
    if (!prog)
        prog = scene->findProgramByName(MaplyDefaultTriangleShader);
    if (prog)
        vectorArealProgramID = prog->getId();

    prog = scene->findProgramByName(MaplyNoLightTriangleExpShader);
    if (!prog)
        prog = scene->findProgramByName(MaplyNoLightTriangleShader);
    if (prog)
        vectorLinearProgramID = prog->getId();

    prog = scene->findProgramByName(MaplyWideVectorExpShader);
    if (!prog)
        prog = scene->findProgramByName(MaplyDefaultWideVectorShader);
    if (prog)
        wideVectorProgramID = prog->getId();
}

MapboxVectorStyleSetImpl::~MapboxVectorStyleSetImpl()
{
}

bool MapboxVectorStyleSetImpl::parse(PlatformThreadInfo *inst,DictionaryRef styleDict)
{
    name = styleDict->getString("name");
    version = styleDict->getInt("version");
    
    // Layers are where the action is
    std::vector<DictionaryEntryRef> layerStyles = styleDict->getArray("layers");
    int which = 0;
    for (auto layerStyle : layerStyles) {
        if (layerStyle->getType() == DictTypeDictionary) {
            MapboxVectorStyleLayerRef layer(MapboxVectorStyleLayer::VectorStyleLayer(inst,this,layerStyle->getDict(),(1*which + tileStyleSettings->baseDrawPriority)));
            if (!layer) {
                continue;
            } else {
                // Sort into various buckets for quick lookup
                layersByName[layer->ident] = layer;
                layersByUUID[layer->getUuid(inst)] = layer;
                if (!layer->sourceLayer.empty()) {
                    auto it = layersBySource.find(layer->sourceLayer);
                    if (it != layersBySource.end()) {
                        it->second.push_back(layer);
                    } else {
                        std::vector<MapboxVectorStyleLayerRef> layers;
                        layers.push_back(layer);
                        layersBySource[layer->sourceLayer] = layers;
                    }
                }
                layers.push_back(layer);
            }
        }
        which++;
    }
    
    return true;
}

long long MapboxVectorStyleSetImpl::generateID()
{
    return currentID++;
}

int MapboxVectorStyleSetImpl::intValue(const std::string &name,DictionaryRef dict,int defVal)
{
    DictionaryEntryRef thing = dict->getEntry(name);
    if (!thing)
        return defVal;
        
    if (thing->getType() == DictTypeDouble || thing->getType() == DictTypeInt || thing->getType() == DictTypeIdentity)
        return thing->getInt();

    wkLogLevel(Warn, "Expected integer for %s but got something else",name.c_str());
    return defVal;
}

double MapboxVectorStyleSetImpl::doubleValue(DictionaryEntryRef thing,double defVal)
{
    if (!thing)
        return defVal;
    
    if (thing->getType() == DictTypeDouble || thing->getType() == DictTypeInt || thing->getType() == DictTypeIdentity)
        return thing->getDouble();

    wkLogLevel(Warn, "Expected double for %s but got something else",name.c_str());
    return defVal;
}

double MapboxVectorStyleSetImpl::doubleValue(const std::string &name,DictionaryRef dict,double defVal)
{
    if (!dict)
        return defVal;

    DictionaryEntryRef thing = dict->getEntry(name);
    if (!thing)
        return defVal;
    
    if (thing->getType() == DictTypeDouble || thing->getType() == DictTypeInt || thing->getType() == DictTypeIdentity)
        return thing->getDouble();
    
    wkLogLevel(Warn, "Expected double for %s but got something else",name.c_str());
    return defVal;
}

bool MapboxVectorStyleSetImpl::boolValue(const std::string &name,DictionaryRef dict,const std::string &onString,bool defVal)
{
    if (!dict)
        return defVal;
    
    DictionaryEntryRef thing = dict->getEntry(name);
    if (!thing)
        return defVal;
    
    if (thing->getType() == DictTypeString)
        return thing->getString() == onString;
    else if (thing->getType() == DictTypeInt)
        return thing->getInt();
    else if (thing->getType() == DictTypeDouble)
        return thing->getInt();
    else
        return defVal;
}

std::string MapboxVectorStyleSetImpl::stringValue(const std::string &name,DictionaryRef dict,const std::string &defVal)
{
    if (!dict)
        return defVal;
    
    DictionaryEntryRef thing = dict->getEntry(name);
    if (!thing)
        return defVal;

    if (thing->getType() == DictTypeString)
        return thing->getString();

    wkLogLevel(Warn, "Expected string for %s but got something else",name.c_str());
    return defVal;
}

std::vector<DictionaryEntryRef> MapboxVectorStyleSetImpl::arrayValue(const std::string &name,DictionaryRef dict)
{
    std::vector<DictionaryEntryRef> ret;

    if (!dict)
        return ret;
    
    DictionaryEntryRef thing = dict->getEntry(name);
    if (!thing)
        return ret;
    
    if (thing->getType() == DictTypeArray)
        return thing->getArray();
    
    wkLogLevel(Warn, "Expected string for %s but got something else",name.c_str());
    return ret;
}

RGBAColorRef MapboxVectorStyleSetImpl::colorValue(const std::string &name,DictionaryEntryRef val,DictionaryRef dict,RGBAColorRef defVal,bool multiplyAlpha)
{
    DictionaryEntryRef thing;
    if (dict)
        thing = dict->getEntry(name);
    else
        thing = val;
    if (!thing)
        return defVal;

    if (thing->getType() != DictTypeString) {
        wkLogLevel(Warn,"Expecting a string for color (%s)",name.c_str());
        return defVal;
    }

    std::string str = thing->getString();
    if (str.empty())
    {
        wkLogLevel(Warn,"Expecting non-empty string for color (%s)",name.c_str());
        return defVal;
    }
    // Hex string
    if (str[0] == '#')
    {
        // Hex string
        unsigned int iVal = 0;
        try {
            iVal = std::stoul(str.substr(1), nullptr, 16);
        }
        catch (...) {
            wkLogLevel(Warn,"Invalid hex value (%s) in color (%s)",str.c_str(),name.c_str());
            return defVal;
        }

        int red,green,blue;
        int alpha = 255;
        if (str.size() == 4)
        {
            red = (iVal >> 8) & 0xf;  red |= red << 4;
            green = (iVal >> 4) & 0xf;  green |= green << 4;
            blue = iVal & 0xf;  blue |= blue << 4;
        } else if (str.size() > 7) {
            red = (iVal >> 24) & 0xff;
            green = (iVal >> 16) & 0xff;
            blue = (iVal >> 8) & 0xff;
            alpha = iVal & 0xff;
        } else {
            red = (iVal >> 16) & 0xff;
            green = (iVal >> 8) & 0xff;
            blue = iVal & 0xff;
        }
        return RGBAColorRef(new RGBAColor(red,green,blue,alpha));
    } else if (str.find("rgb(") == 0) {
        std::regex reg("[(),]");
        std::sregex_token_iterator iter(str.begin()+4, str.end(), reg, -1);
        std::sregex_token_iterator end;
        std::vector<std::string> toks(iter, end);

        if (toks.size() != 3) {
            wkLogLevel(Warn, "Unrecognized format in color %s",name.c_str());
            return defVal;
        }
        int red = std::stoi(toks[0]);
        int green = std::stoi(toks[1]);
        int blue = std::stoi(toks[2]);

        return RGBAColorRef(new RGBAColor(red,green,blue,1.0));
    } else if (str.find("rgba(") == 0) {
        std::regex reg("[(),]");
        std::sregex_token_iterator iter(str.begin()+5, str.end(), reg, -1);
        std::sregex_token_iterator end;
        std::vector<std::string> toks(iter, end);

        if (toks.size() != 4) {
            wkLogLevel(Warn, "Unrecognized format in color %s",name.c_str());
            return defVal;
        }
        int red = std::stoi(toks[0]);
        int green = std::stoi(toks[1]);
        int blue = std::stoi(toks[2]);
        double alpha = std::stod(toks[3]);
        
        if (multiplyAlpha)
            return RGBAColorRef(new RGBAColor(red * alpha,green * alpha,blue * alpha,255.0*alpha));
        else
            return RGBAColorRef(new RGBAColor(red,green,blue,255.0*alpha));
    } else if (str.find("hsl(") == 0) {
        std::regex reg("[(),]");
        std::sregex_token_iterator iter(str.begin()+4, str.end(), reg, -1);
        std::sregex_token_iterator end;
        std::vector<std::string> toks(iter, end);

        if (toks.size() != 3) {
            wkLogLevel(Warn, "Unrecognized format in color %s",name.c_str());
            return defVal;
        }
        int hue = std::stoi(toks[0]);
        int sat = std::stoi(toks[1]);
        int light = std::stoi(toks[2]);
        float newLight = light / 100.0;
        float newSat = sat / 100.0;

        return RGBAColorRef(new RGBAColor(RGBAColor::FromHSL(hue, newSat, newLight)));
    } else if (str.find("hsla(") == 0) {
        std::regex reg("[(),]");
        std::sregex_token_iterator iter(str.begin()+5, str.end(), reg, -1);
        std::sregex_token_iterator end;
        std::vector<std::string> toks(iter, end);

        if (toks.size() != 4) {
            wkLogLevel(Warn, "Unrecognized format in color %s",name.c_str());
            return defVal;
        }
        int hue = std::stoi(toks[0]);
        int sat = std::stoi(toks[1]);
        int light = std::stoi(toks[2]);
        float alpha = std::stod(toks[3]);
        float newLight = light / 100.0;
        float newSat = sat / 100.0;

        RGBAColorRef color(new RGBAColor(RGBAColor::FromHSL(hue, newSat, newLight)));
        color->a = alpha * 255.0;
        
        return color;
    }

    wkLogLevel(Warn,"Didn't recognize format of color (%s)",name.c_str());
    return defVal;
}

RGBAColorRef MapboxVectorStyleSetImpl::colorValue(const std::string &name,DictionaryEntryRef val,DictionaryRef dict,RGBAColor defVal,bool multiplyAlpha)
{
    RGBAColorRef color(new RGBAColor(defVal));
    
    return colorValue(name, val, dict, color, multiplyAlpha);
}

int MapboxVectorStyleSetImpl::enumValue(DictionaryEntryRef entry,const char * const options[],int defVal)
{
    if (!entry || entry->getType() != DictTypeString)
        return defVal;

    std::string name = entry->getString();
    
    for (int which = 0; options[which]; which++) {
        const char * const val = options[which];
        if (!strcmp(val, name.c_str()))
            return which;
    }

    wkLogLevel(Warn,"Found unexpected value (%s) in enumerated type",name.c_str());
    return defVal;
}

MapboxTransDoubleRef MapboxVectorStyleSetImpl::transDouble(DictionaryEntryRef theEntry, double defVal)
{
    if (!theEntry)
        return std::make_shared<MapboxTransDouble>(defVal);
    
    // This is probably stops
    if (theEntry->getType() == DictTypeDictionary) {
        auto stops = std::make_shared<MaplyVectorFunctionStops>();
        stops->parse(theEntry->getDict(), this, false);
        if (stops) {
            return MapboxTransDoubleRef(new MapboxTransDouble(stops));
        } else {
            wkLogLevel(Warn, "Expecting key word 'stops' in entry %s",name.c_str());
        }
    } else if (theEntry->getType() == DictTypeDouble || theEntry->getType() == DictTypeInt) {
        return std::make_shared<MapboxTransDouble>(theEntry->getDouble());
    } else {
        wkLogLevel(Warn,"Unexpected type found in entry %s. Was expecting a double.",name.c_str());
    }

    return MapboxTransDoubleRef();
}


MapboxTransDoubleRef MapboxVectorStyleSetImpl::transDouble(const std::string &name,DictionaryRef entry,double defVal)
{
    return transDouble(entry ? entry->getEntry(name) : DictionaryEntryRef(), defVal);
}

MapboxTransColorRef MapboxVectorStyleSetImpl::transColor(const std::string &name,DictionaryRef entry,const RGBAColor *defVal)
{
    RGBAColorRef defValRef;
    if (defVal)
        defValRef = RGBAColorRef(new RGBAColor(*defVal));

    if (!entry) {
        if (defVal)
            return MapboxTransColorRef(new MapboxTransColor(RGBAColorRef(new RGBAColor(*defVal))));
        return MapboxTransColorRef();
    }

    // They pass in the whole dictionary and let us look the field up
    DictionaryEntryRef theEntry = entry->getEntry(name);
    if (!theEntry) {
        if (defVal)
            return MapboxTransColorRef(new MapboxTransColor(RGBAColorRef(new RGBAColor(*defVal))));
        return MapboxTransColorRef();
    }

    // This is probably stops
    if (theEntry->getType() == DictTypeDictionary) {
        MaplyVectorFunctionStopsRef stops(new MaplyVectorFunctionStops());
        stops->parse(theEntry->getDict(), this, false);
        if (stops) {
            return MapboxTransColorRef(new MapboxTransColor(stops));;
        } else {
            wkLogLevel(Warn, "Expecting key word 'stops' in entry %s",name.c_str());
        }
    } else if (theEntry->getType() == DictTypeString) {
        RGBAColorRef color = colorValue(name, theEntry, DictionaryRef(), defValRef, false);
        if (color)
            return MapboxTransColorRef(new MapboxTransColor(color));
        else {
            wkLogLevel(Warn,"Unexpected type found in entry %s. Was expecting a color.",name.c_str());
        }
    } else {
        wkLogLevel(Warn,"Unexpected type found in entry %s. Was expecting a color.",name.c_str());
    }

    return MapboxTransColorRef();
}

MapboxTransColorRef MapboxVectorStyleSetImpl::transColor(const std::string &name,DictionaryRef entry,const RGBAColor &inColor)
{
    RGBAColor color = inColor;
    return transColor(name, entry, &color);
}

MapboxTransTextRef MapboxVectorStyleSetImpl::transText(const std::string &name,DictionaryRef entry,const std::string &str)
{
    if (!entry) {
        if (!str.empty())
            return MapboxTransTextRef(new MapboxTransText(str));
        return MapboxTransTextRef();
    }
    
    // They pass in the whole dictionary and let us look the field up
    DictionaryEntryRef theEntry = entry->getEntry(name);
    if (!theEntry) {
        if (!str.empty())
            return MapboxTransTextRef(new MapboxTransText(str));
        return MapboxTransTextRef();
    }

    // This is probably stops
    if (theEntry->getType() == DictTypeDictionary) {
        MaplyVectorFunctionStopsRef stops(new MaplyVectorFunctionStops());
        stops->parse(theEntry->getDict(), this, true);
        if (stops) {
            return MapboxTransTextRef(new MapboxTransText(stops));;
        } else {
            wkLogLevel(Warn, "Expecting key word 'stops' in entry %s",name.c_str());
        }
    } else if (theEntry->getType() == DictTypeString) {
        std::string text = theEntry->getString();
        return MapboxTransTextRef(new MapboxTransText(text));
    } else {
        wkLogLevel(Warn,"Unexpected type found in entry %s. Was expecting a string.",name.c_str());
    }

    return MapboxTransTextRef();
}

void MapboxVectorStyleSetImpl::unsupportedCheck(const char *field,const char *what,DictionaryRef styleEntry)
{
    if (styleEntry && styleEntry->hasField(field)) {
#if DEBUG
        wkLogLevel(Warn,"Found unsupported field (%s) for (%s)",field,what);
#endif
    }
}

RGBAColorRef MapboxVectorStyleSetImpl::resolveColor(MapboxTransColorRef color,MapboxTransDoubleRef opacity,double zoom,MBResolveColorType resolveMode)
{
    // No color means no color
    if (!color)
        return RGBAColorRef();

    const RGBAColor thisColor = color->colorForZoom(zoom);

    // No opacity means full opacity
    if (!opacity || color->hasAlphaOverride())
        return std::make_shared<RGBAColor>(thisColor);

    double const thisOpacity = opacity->valForZoom(zoom) * 255;

    float vals[4];
    thisColor.asUnitFloats(vals);
    switch (resolveMode)
    {
        case MBResolveColorOpacityMultiply:
            return std::make_shared<RGBAColor>(vals[0]*thisOpacity,vals[1]*thisOpacity,vals[2]*thisOpacity,vals[3]*thisOpacity);
        case MBResolveColorOpacityReplaceAlpha:
            return std::make_shared<RGBAColor>(vals[0]*255,vals[1]*255,vals[2]*255,thisOpacity);
        case MBResolveColorOpacityComposeAlpha:
            return std::make_shared<RGBAColor>(vals[0]*255,vals[1]*255,vals[2]*255,vals[3]*thisOpacity);
        default:
            assert(!"Invalid color resolve type");
            return RGBAColorRef();
    }
}

RGBAColor MapboxVectorStyleSetImpl::color(RGBAColor color,double opacity)
{
    float vals[4];
    color.asUnitFloats(vals);

    return RGBAColor(vals[0]*opacity*255,vals[1]*opacity*255,vals[2]*opacity*255,vals[3]*opacity*255);
}

MapboxVectorStyleLayerRef MapboxVectorStyleSetImpl::getLayer(const std::string &name)
{
    auto it = layersByName.find(name);
    if (it == layersByName.end())
        return MapboxVectorStyleLayerRef();
    
    return it->second;
}

void MapboxVectorStyleSetImpl::setZoomSlot(int inZoomSlot)
{
    zoomSlot = inZoomSlot;
}

VectorStyleImplRef MapboxVectorStyleSetImpl::backgroundStyle(PlatformThreadInfo *inst) const
{
    const auto it = layersByName.find("background");
    if (it != layersByName.end()) {
        if (auto backLayer = std::dynamic_pointer_cast<MapboxVectorLayerBackground>(it->second)) {
            return backLayer;
        }
    }
    return VectorStyleImplRef();
}

RGBAColorRef MapboxVectorStyleSetImpl::backgroundColor(PlatformThreadInfo *inst,double zoom)
{
    auto it = layersByName.find("background");
    if (it != layersByName.end()) {
        MapboxVectorLayerBackgroundRef backLayer = std::dynamic_pointer_cast<MapboxVectorLayerBackground>(it->second);
        if (backLayer) {
            return RGBAColorRef(new RGBAColor(backLayer->paint.color->colorForZoom(zoom)));
        }
    }
    
    return RGBAColorRef();
}


std::vector<VectorStyleImplRef> MapboxVectorStyleSetImpl::stylesForFeature(PlatformThreadInfo *inst,
                                                                           const Dictionary &attrs,
                                                                           const QuadTreeIdentifier &tileID,
                                                                           const std::string &layerName)
{
    std::vector<VectorStyleImplRef> styles;
    
    const auto it = layersBySource.find(layerName);
    if (it != layersBySource.end()) {
        for (const auto &layer : it->second) {
            if (!layer->filter || layer->filter->testFeature(attrs, tileID)) {
                styles.reserve(it->second.size());
                styles.push_back(layer);
            }
        }
    }
    
    return styles;
}

/// Return true if the given layer is meant to display for the given tile (zoom level)
bool MapboxVectorStyleSetImpl::layerShouldDisplay(PlatformThreadInfo *inst,
                                                  const std::string &layerName,
                                                  const QuadTreeNew::Node &tileID)
{
    const auto it = layersBySource.find(layerName);
    return it != layersBySource.end();
}

/// Return the style associated with the given UUID.
VectorStyleImplRef MapboxVectorStyleSetImpl::styleForUUID(PlatformThreadInfo *inst,long long uuid)
{
    const auto it = layersByUUID.find(uuid);
    return (it == layersByUUID.end()) ? nullptr : it->second;
}

// Return a list of all the styles in no particular order.  Needed for categories and indexing
std::vector<VectorStyleImplRef> MapboxVectorStyleSetImpl::allStyles(PlatformThreadInfo *inst)
{
    return std::vector<VectorStyleImplRef>(layers.begin(), layers.end());
}

void MapboxVectorStyleSetImpl::addSprites(MapboxVectorStyleSpritesRef newSprites)
{
    sprites = newSprites;
}

}
