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

static const std::string strUnderbar("_");
static const std::string strBase("base");
static const std::string strStops("stops");
static const std::string strName("name");
static const std::string strVersion("version");
static const std::string strLayers("layers");
static const std::string strBackground("background");
static const std::regex colorSeparatorPattern("[(),]");
static const std::regex fieldSeparatorPattern(R"([{}]+)");
static const std::regex colonPattern(":");

bool MapboxRegexField::parse(const std::string &textField)
{
    // Parse out the {} groups in the text
    // TODO: We're missing a boatload of stuff in the spec
    const auto &regex = fieldSeparatorPattern;
    std::sregex_token_iterator it{textField.begin(), textField.end(), regex, -1};
    bool isJustText = textField[0] != '{';
    for (; it != std::sregex_token_iterator(); ++it) {
        if (it->length() == 0)
        {
            continue;
        }
        const auto regexChunk = std::string(*it);
        MapboxTextChunk textChunk;
        if (isJustText) {
            textChunk.str = regexChunk;
        } else {
            textChunk.keys.push_back(regexChunk);
            // For some reason name:en is sometimes name_en
            textChunk.keys.push_back(std::regex_replace(regexChunk, colonPattern, strUnderbar));
        }
        chunks.push_back(textChunk);
        isJustText = !isJustText;
    }

    valid = true;

    return true;
}

bool MapboxRegexField::parse(const std::string &fieldName,
                             MapboxVectorStyleSetImpl *styleSet,
                             const DictionaryRef &styleEntry)
{
    const std::string textField = styleSet->stringValue(fieldName, styleEntry, std::string());
    return textField.empty() || parse(textField);
}

std::string MapboxRegexField::build(const DictionaryRef &attrs)
{
    bool found = false;
    bool didLookup = false;

    std::string text;
    text.reserve(chunks.size() * 20);

    for (const auto &chunk : chunks) {
        if (!chunk.str.empty())
            text += chunk.str;
        else {
            for (const auto &key : chunk.keys) {
                didLookup = true;
                if (attrs->hasField(key)) {
                    found = true;
                    const std::string keyVal = attrs->getString(key);
                    if (!keyVal.empty()) {
                        text += keyVal;
                        break;
                    }
                }
            }
        }
    }

    if (didLookup && !found)
        return std::string();

    if (!text.empty() && text[text.size()-1] == '\n')
        text.resize(text.size()-1);

    return text;
}

MaplyVectorFunctionStop::MaplyVectorFunctionStop()
: zoom(-1.0), val(0.0)
{
}

bool MaplyVectorFunctionStops::parse(const DictionaryRef &entry,MapboxVectorStyleSetImpl *styleSet,bool isText)
{
    base = entry->getDouble(strBase,1.0);
    
    std::vector<DictionaryEntryRef> dataArray = entry->getArray(strStops);
    if (dataArray.size() < 2)
    {
        wkLogLevel(Warn, "Expecting at least two arguments for function stops.");
        return false;
    }
    for (const auto &stop : dataArray) {
        if (stop->getType() == DictTypeArray) {
            const std::vector<DictionaryEntryRef> stopEntries = stop->getArray();
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
                            fStop.color = styleSet->colorValue(std::string(), stopEntries[1], nullptr, nullptr, false);
                        break;
                    case DictTypeObject:
                        fStop.color = std::make_shared<RGBAColor>(stopEntries[1]->getColor());
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
            return std::make_shared<RGBAColor>(RGBAColor::FromUnitFloats(res));
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

    for (const auto &stop : stops)
    {
        val = std::min(val,stop.val);
    }

    return val;
}

double MaplyVectorFunctionStops::maxValue()
{
    double val = -MAXFLOAT;

    for (const auto &stop : stops)
    {
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
    stops = std::move(inStops);
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
    
    auto floatExp = std::make_shared<FloatExpressionInfo>();
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
    return stops ? stops->minValue() : val;
}

double MapboxTransDouble::maxVal()
{
    return stops ? stops->maxValue() : val;
}

MapboxTransColor::MapboxTransColor(RGBAColorRef color) :
    color(std::move(color)),
    useAlphaOverride(false),
    alpha(1.0)
{
}

MapboxTransColor::MapboxTransColor(MaplyVectorFunctionStopsRef stops) :
    useAlphaOverride(false),
    alpha(1.0),
    stops(std::move(stops))
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

bool MapboxTransColor::isExpression() const
{
    return stops.get() != nullptr;
}

ColorExpressionInfoRef MapboxTransColor::expression()
{
    if (!stops)
        return ColorExpressionInfoRef();
    
    auto colorExp = std::make_shared<ColorExpressionInfo>();
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

MapboxTransText::MapboxTransText(MaplyVectorFunctionStopsRef stops) :
    stops(std::move(stops))
{
}

MapboxRegexField MapboxTransText::textForZoom(double zoom)
{
    return stops ? stops->textForZoom(zoom) : textField;
}

static constexpr size_t TypicalLayerCount = 500;

MapboxVectorStyleSetImpl::MapboxVectorStyleSetImpl(Scene *inScene,
                                                   CoordSystem *coordSys,
                                                   VectorStyleSettingsImplRef settings) :
    scene(inScene),
    version(-1),
    currentID(0),
    tileStyleSettings(std::move(settings)),
    coordSys(coordSys),
    zoomSlot(-1),
    layersByName(TypicalLayerCount),
    layersByUUID(TypicalLayerCount),
    layersBySource(TypicalLayerCount)
{
    layers.reserve(TypicalLayerCount);

    vecManage = scene->getManager<VectorManager>(kWKVectorManager);
    wideVecManage = scene->getManager<WideVectorManager>(kWKWideVectorManager);
    markerManage = scene->getManager<MarkerManager>(kWKMarkerManager);
    labelManage = scene->getManager<LabelManager>(kWKLabelManager);
    compManage = scene->getManager<ComponentManager>(kWKComponentManager);

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

bool MapboxVectorStyleSetImpl::parse(PlatformThreadInfo *inst,const DictionaryRef &styleDict)
{
    name = styleDict->getString(strName);
    version = styleDict->getInt(strVersion);

    // Layers are where the action is
    const std::vector<DictionaryEntryRef> layerStyles = styleDict->getArray(strLayers);
    int which = 0;
    for (const auto &layerStyle : layerStyles) {
        if (layerStyle->getType() == DictTypeDictionary) {
            auto layer = MapboxVectorStyleLayer::VectorStyleLayer(inst,this,layerStyle->getDict(),(1*which + tileStyleSettings->baseDrawPriority));
            if (!layer)
            {
                continue;
            }

            // Sort into various buckets for quick lookup
            layersByName[layer->ident] = layer;
            layersByUUID[layer->getUuid(inst)] = layer;
            if (!layer->sourceLayer.empty())
            {
                layersBySource.insert(std::make_pair(layer->sourceLayer, layer));
            }
            layers.push_back(layer);
        }
        which++;
    }
    
    return true;
}

long long MapboxVectorStyleSetImpl::generateID()
{
    return currentID++;
}

int MapboxVectorStyleSetImpl::intValue(const std::string &inName,const DictionaryRef &dict,int defVal)
{
    const auto thing = dict->getEntry(inName);
    switch (thing ? thing->getType() : DictTypeNone)
    {
        case DictTypeDouble:
        case DictTypeInt:
        case DictTypeInt64:
        case DictTypeIdentity:
            return thing->getInt();
        default:
            if (thing)
            {
                wkLogLevel(Warn,"Expected integer for %s but got type %d",inName.c_str(),thing->getType());
            }
            return defVal;
    }
}

double MapboxVectorStyleSetImpl::doubleValue(const DictionaryEntryRef &thing,double defVal)
{
    if (!thing)
        return defVal;
    
    if (thing->getType() == DictTypeDouble || thing->getType() == DictTypeInt || thing->getType() == DictTypeIdentity)
        return thing->getDouble();

    wkLogLevel(Warn, "Expected double for %s but got something else",name.c_str());
    return defVal;
}

double MapboxVectorStyleSetImpl::doubleValue(const std::string &name,const DictionaryRef &dict,double defVal)
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

bool MapboxVectorStyleSetImpl::boolValue(const std::string &name,const DictionaryRef &dict,const std::string &onString,bool defVal)
{
    if (!dict)
        return defVal;

    const auto thing = dict->getEntry(name);
    switch (thing ? thing->getType() : DictTypeNone)
    {
        case DictTypeString: return thing->getString() == onString;
        case DictTypeInt:
        case DictTypeInt64:
        case DictTypeIdentity:
        case DictTypeDouble: return thing->getInt() != 0;
        default:             return defVal;
    }
}

std::string MapboxVectorStyleSetImpl::stringValue(const std::string &inName,const DictionaryRef &dict,const std::string &defVal)
{
    if (!dict)
        return defVal;
    
    DictionaryEntryRef thing = dict->getEntry(inName);
    if (!thing)
        return defVal;

    if (thing->getType() == DictTypeString)
        return thing->getString();

    wkLogLevel(Warn, "Expected string for %s but got something else",inName.c_str());
    return defVal;
}

std::vector<DictionaryEntryRef> MapboxVectorStyleSetImpl::arrayValue(const std::string &inName,const DictionaryRef &dict)
{
    std::vector<DictionaryEntryRef> ret;

    if (!dict)
        return ret;
    
    DictionaryEntryRef thing = dict->getEntry(inName);
    if (!thing)
        return ret;
    
    if (thing->getType() == DictTypeArray)
        return thing->getArray();
    
    wkLogLevel(Warn, "Expected array for %s but got something else",inName.c_str());
    return ret;
}

RGBAColorRef MapboxVectorStyleSetImpl::colorValue(const std::string &inName, const DictionaryEntryRef &val, const DictionaryRef &dict, const RGBAColorRef &defVal, bool multiplyAlpha)
{
    DictionaryEntryRef thing;
    if (dict)
        thing = dict->getEntry(inName);
    else
        thing = val;
    if (!thing)
        return defVal;

    if (thing->getType() != DictTypeString) {
        wkLogLevel(Warn, "Expecting a string for color (%s)", inName.c_str());
        return defVal;
    }

    std::string str = thing->getString();
    if (str.empty())
    {
        wkLogLevel(Warn, "Expecting non-empty string for color (%s)", inName.c_str());
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
            wkLogLevel(Warn, "Invalid hex value (%s) in color (%s)", str.c_str(), inName.c_str());
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
        return std::make_shared<RGBAColor>(red,green,blue,alpha);
    } else if (str.find("rgb(") == 0) {
        const auto &reg = colorSeparatorPattern;
        const std::sregex_token_iterator iter(str.begin()+4, str.end(), reg, -1);
        const std::vector<std::string> toks(iter, std::sregex_token_iterator());

        if (toks.size() != 3) {
            wkLogLevel(Warn, "Unrecognized format in color %s", inName.c_str());
            return defVal;
        }
        const int red = std::stoi(toks[0]);
        const int green = std::stoi(toks[1]);
        const int blue = std::stoi(toks[2]);

        return std::make_shared<RGBAColor>(red,green,blue,1.0);
    } else if (str.find("rgba(") == 0) {
        const auto &reg = colorSeparatorPattern;
        const std::sregex_token_iterator iter(str.begin()+5, str.end(), reg, -1);
        std::vector<std::string> toks(iter, std::sregex_token_iterator());

        if (toks.size() != 4) {
            wkLogLevel(Warn, "Unrecognized format in color %s", inName.c_str());
            return defVal;
        }
        const int red = std::stoi(toks[0]);
        const int green = std::stoi(toks[1]);
        const int blue = std::stoi(toks[2]);
        const double alpha = std::stod(toks[3]);
        
        if (multiplyAlpha)
            return std::make_shared<RGBAColor>(red * alpha,green * alpha,blue * alpha,255.0*alpha);
        else
            return std::make_shared<RGBAColor>(red,green,blue,255.0*alpha);
    } else if (str.find("hsl(") == 0) {
        const auto &reg = colorSeparatorPattern;
        const std::sregex_token_iterator iter(str.begin()+4, str.end(), reg, -1);
        const std::vector<std::string> toks(iter, std::sregex_token_iterator());

        if (toks.size() != 3) {
            wkLogLevel(Warn, "Unrecognized format in color %s", inName.c_str());
            return defVal;
        }
        const int hue = std::stoi(toks[0]);
        const int sat = std::stoi(toks[1]);
        const int light = std::stoi(toks[2]);
        const float newLight = light / 100.0;
        const float newSat = sat / 100.0;

        return std::make_shared<RGBAColor>(RGBAColor::FromHSL(hue, newSat, newLight));
    } else if (str.find("hsla(") == 0) {
        const auto &reg = colorSeparatorPattern;
        const std::sregex_token_iterator iter(str.begin()+5, str.end(), reg, -1);
        const std::vector<std::string> toks(iter, std::sregex_token_iterator());

        if (toks.size() != 4) {
            wkLogLevel(Warn, "Unrecognized format in color %s", inName.c_str());
            return defVal;
        }
        const int hue = std::stoi(toks[0]);
        const int sat = std::stoi(toks[1]);
        const int light = std::stoi(toks[2]);
        const auto alpha = (float)std::stod(toks[3]);
        const auto newLight = light / 100.0f;
        const auto newSat = sat / 100.0f;

        auto color = std::make_shared<RGBAColor>(RGBAColor::FromHSL(hue, newSat, newLight));
        color->a = (uint8_t)(alpha * 255.0);

        return color;
    }

    wkLogLevel(Warn, "Didn't recognize format of color (%s)", inName.c_str());
    return defVal;
}

RGBAColorRef MapboxVectorStyleSetImpl::colorValue(const std::string &inName,const DictionaryEntryRef &val,const DictionaryRef &dict,const RGBAColor &defVal,bool multiplyAlpha)
{
    return colorValue(inName, val, dict, std::make_shared<RGBAColor>(defVal), multiplyAlpha);
}

int MapboxVectorStyleSetImpl::enumValue(const DictionaryEntryRef &entry,const char * const options[],int defVal)
{
    if (!entry || entry->getType() != DictTypeString)
        return defVal;

    const std::string localName = entry->getString();
    
    for (int which = 0; options[which]; which++)
    {
        const char * const val = options[which];
        if (!strcmp(val, localName.c_str()))
        {
            return which;
        }
    }

    wkLogLevel(Warn, "Found unexpected value (%s) in enumerated type", localName.c_str());
    return defVal;
}

MapboxTransDoubleRef MapboxVectorStyleSetImpl::transDouble(const DictionaryEntryRef &theEntry, double defVal)
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


MapboxTransDoubleRef MapboxVectorStyleSetImpl::transDouble(const std::string &name,const DictionaryRef &entry,double defVal)
{
    return transDouble(entry ? entry->getEntry(name) : DictionaryEntryRef(), defVal);
}

MapboxTransColorRef MapboxVectorStyleSetImpl::transColor(const std::string &name,const DictionaryRef &entry,const RGBAColor *defVal)
{
    RGBAColorRef defValRef;
    if (defVal)
        defValRef = std::make_shared<RGBAColor>(*defVal);

    if (!entry) {
        if (defVal)
            return std::make_shared<MapboxTransColor>(std::make_shared<RGBAColor>(*defVal));
        return MapboxTransColorRef();
    }

    // They pass in the whole dictionary and let us look the field up
    DictionaryEntryRef theEntry = entry->getEntry(name);
    if (!theEntry) {
        if (defVal)
            return std::make_shared<MapboxTransColor>(std::make_shared<RGBAColor>(*defVal));
        return MapboxTransColorRef();
    }

    // This is probably stops
    if (theEntry->getType() == DictTypeDictionary) {
        auto stops = std::make_shared<MaplyVectorFunctionStops>();
        if (stops->parse(theEntry->getDict(), this, false)) {
            return std::make_shared<MapboxTransColor>(stops);
        } else {
            wkLogLevel(Warn, "Expecting key word 'stops' in entry %s",name.c_str());
        }
    } else if (theEntry->getType() == DictTypeString) {
        RGBAColorRef color = colorValue(name, theEntry, DictionaryRef(), defValRef, false);
        if (color)
            return std::make_shared<MapboxTransColor>(color);
        else {
            wkLogLevel(Warn,"Unexpected type found in entry %s. Was expecting a color.",name.c_str());
        }
    } else {
        wkLogLevel(Warn,"Unexpected type found in entry %s. Was expecting a color.",name.c_str());
    }

    return MapboxTransColorRef();
}

MapboxTransColorRef MapboxVectorStyleSetImpl::transColor(const std::string &inName, const DictionaryRef &entry, const RGBAColor &inColor)
{
    RGBAColor color = inColor;
    return transColor(inName, entry, &color);
}

MapboxTransTextRef MapboxVectorStyleSetImpl::transText(const std::string &inName, const DictionaryRef &entry, const std::string &str)
{
    if (!entry) {
        return str.empty() ? MapboxTransTextRef() : std::make_shared<MapboxTransText>(str);
    }
    
    // They pass in the whole dictionary and let us look the field up
    DictionaryEntryRef theEntry = entry->getEntry(inName);
    if (!theEntry) {
        return str.empty() ? MapboxTransTextRef() : std::make_shared<MapboxTransText>(str);
    }

    // This is probably stops
    if (theEntry->getType() == DictTypeDictionary) {
        auto stops = std::make_shared<MaplyVectorFunctionStops>();
        if (stops->parse(theEntry->getDict(), this, true)) {
            return std::make_shared<MapboxTransText>(stops);
        } else {
            wkLogLevel(Warn, "Expecting key word 'stops' in entry %s", inName.c_str());
        }
    } else if (theEntry->getType() == DictTypeString) {
        return std::make_shared<MapboxTransText>(theEntry->getString());
    } else {
        wkLogLevel(Warn, "Unexpected type found in entry %s. Was expecting a string.", inName.c_str());
    }

    return MapboxTransTextRef();
}

void MapboxVectorStyleSetImpl::unsupportedCheck(const char *field,const char *what,const DictionaryRef &styleEntry)
{
    if (styleEntry && styleEntry->hasField(field)) {
#if DEBUG
        wkLogLevel(Warn,"Found unsupported field (%s) for (%s)",field,what);
#endif
    }
}

RGBAColorRef MapboxVectorStyleSetImpl::resolveColor(const MapboxTransColorRef &color,const MapboxTransDoubleRef &opacity,double zoom,MBResolveColorType resolveMode)
{
    // No color means no color
    if (!color)
        return RGBAColorRef();

    const RGBAColor thisColor = color->colorForZoom(zoom);

    // No opacity means full opacity
    if (!opacity || color->hasAlphaOverride())
        return std::make_shared<RGBAColor>(thisColor);

    const double thisOpacity = opacity->valForZoom(zoom) * 255;

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
    return {
        (uint8_t)(vals[0]*opacity*255),
        (uint8_t)(vals[1]*opacity*255),
        (uint8_t)(vals[2]*opacity*255),
        (uint8_t)(vals[3]*opacity*255)
    };
}

MapboxVectorStyleLayerRef MapboxVectorStyleSetImpl::getLayer(const std::string &inName)
{
    const auto it = layersByName.find(inName);
    return (it == layersByName.end()) ? MapboxVectorStyleLayerRef() : it->second;
}

VectorStyleImplRef MapboxVectorStyleSetImpl::backgroundStyle(PlatformThreadInfo *inst) const
{
    const auto it = layersByName.find(strBackground);
    if (it != layersByName.end()) {
        if (auto backLayer = std::dynamic_pointer_cast<MapboxVectorLayerBackground>(it->second)) {
            return backLayer;
        }
    }
    return VectorStyleImplRef();
}

RGBAColorRef MapboxVectorStyleSetImpl::backgroundColor(PlatformThreadInfo *inst,double zoom)
{
    const auto it = layersByName.find(strBackground);
    if (it != layersByName.end()) {
        if (const auto backLayer = std::dynamic_pointer_cast<MapboxVectorLayerBackground>(it->second)) {
            return std::make_shared<RGBAColor>(backLayer->paint.color->colorForZoom(zoom));
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

    const auto range = layersBySource.equal_range(layerName);
    for (auto i = range.first; i != range.second; ++i)
    {
        auto &layer = i->second;
        if (!layer->filter || layer->filter->testFeature(attrs, tileID))
        {
            if (styles.empty())
            {
                styles.reserve(std::distance(range.first, range.second));
            }
            styles.push_back(layer);
        }
    }
    
    return styles;
}

/// Return true if the given layer is meant to display for the given tile (zoom level)
bool MapboxVectorStyleSetImpl::layerShouldDisplay(PlatformThreadInfo *inst,
                                                  const std::string &layerName,
                                                  const QuadTreeNew::Node &tileID)
{
    const auto range = layersBySource.equal_range(layerName);
    for (auto i = range.first; i != range.second; ++i)
    {
        if (i->second->visible || !i->second->representation.empty())
        {
            return true;
        }
    }
    return false;
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
    sprites = std::move(newSprites);
}

}
