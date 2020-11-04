//
//  VectorTilePBFParser.h
//  WhirlyGlobeMaplyComponent
//
//  Created by Tim Sylvester on 10/30/20.
//  Copyright © 2020 mousebird consulting. All rights reserved.
//

#ifndef VectorTilePBFParser_h
#define VectorTilePBFParser_h

#include <Identifiable.h>
#include <VectorData.h>
#include <WhirlyVector.h>

#include <functional>
#include <map>
#include <memory>
#include <set>
#include <string>
#include <vector>

typedef struct pb_istream_s pb_istream_t;
typedef struct pb_field_iter_s pb_field_iter_t;
typedef struct _vector_tile_Tile vector_tile_Tile;
typedef struct _vector_tile_Tile_Feature vector_tile_Tile_Feature;
typedef struct _vector_tile_Tile_Layer vector_tile_Tile_Layer;
typedef struct _vector_tile_Tile_Value vector_tile_Tile_Value;

namespace WhirlyKit
{

class MutableDictionaryC;
class PlatformThreadInfo;
class VectorTileData;
class VectorStyleDelegateImpl;
class VectorObject;

typedef std::shared_ptr<MutableDictionaryC> MutableDictionaryCRef;
typedef std::shared_ptr<VectorObject> VectorObjectRef;

class VectorTilePBFParser
{
public:
    VectorTilePBFParser(
        VectorTileData *tileData,
        VectorStyleDelegateImpl* styleData,
        PlatformThreadInfo *styleInst,
        const std::string &uuidName,
        const std::set<std::string> &uuidValues,
        std::map<SimpleIdentity, std::vector<VectorObjectRef>*>& vecObjByStyle,
        bool localCoords = false,
        bool parseAll = false,
        std::vector<VectorObjectRef>* keepVectors = nullptr,
        std::function<bool()> isCancelled = [](){return false;});

    bool parse(const uint8_t* data, size_t length);

    unsigned getLayerCount() const { return _layerCount; }
    unsigned getFeatureCount() const { return _featureCount; }
    
    unsigned getSkippedLayerCount() const { return _skippedLayerCount; }
    unsigned getSkippedFeatureCount() const { return _skippedFeatureCount; }
    
    unsigned getParseErrorCount() const { return _parseErrors; }
    unsigned getBadAttributeCount() const { return _badAttributes; }
    unsigned getUnknownCommandCount() const { return _unknownCommands; }
    unsigned getUknownGeomTypeCount() const { return _unknownGeomTypes; }
    unsigned getUnknownValueTypeCount() const { return _unknownValueTypes; }
    
    bool getParseCanceled() const { return _wasCancelled; }
    
    unsigned getTotalErrorCount() const {
        return _unknownValueTypes +
            _badAttributes +
            _unknownCommands +
            _unknownGeomTypes +
            _parseErrors;
    }

    std::string getErrorString(const char* def) const
    {
        return _parseError.length() ? _parseError : def;
    }
    
    const std::string &getErrorString(const std::string &def) const
    {
        return _parseError.length() ? _parseError : def;
    }

private:
    typedef VectorTilePBFParser This;
    
    static inline int32_t decodeParamInt(int32_t p);
    static inline std::pair<uint8_t, int32_t> decodeCommand(int32_t c);

    // nanopb callbacks
    static bool layerDecode(pb_istream_t *stream, const pb_field_iter_t *field, void **arg);
    static bool featureDecode(pb_istream_t *stream, const pb_field_iter_t *field, void **arg);
    static bool stringDecode(pb_istream_t *stream, const pb_field_iter_t *field, void **arg);
    static bool stringVecDecode(pb_istream_t *stream, const pb_field_iter_t *field, void **arg);
    static bool intVecDecode(pb_istream_t *stream, const pb_field_iter_t *field, void **arg);
    static bool valueVecDecode(pb_istream_t *stream, const pb_field_iter_t *field, void **arg);

    // Wrapped callbacks
    inline bool layerDecode(pb_istream_t *stream, const pb_field_iter_t *field);
    inline bool featureDecode(pb_istream_t *stream, const pb_field_iter_t *field);

    // Parsing methods
    inline bool processTags(const MutableDictionaryCRef &attributes);
    inline bool checkStyles(SimpleIDSet& styleIDs, const MutableDictionaryCRef &attributes);
    inline void parseLineString(const std::vector<uint32_t> &geometry, ShapeSet& shapes);
    inline bool parsePolygon(const std::vector<uint32_t> &geometry, VectorAreal& shape);
    inline bool parsePoints(const std::vector<uint32_t> &geometry, VectorPoints& shape);
    inline void addFeature(const VectorObjectRef &vecObj, const SimpleIDSet &styleIDs);
    inline void layerElement();
    inline bool layerStart();
    inline bool layerFinish();

    static inline int layerKeyHeuristic() { return 10; }
    static inline int layerValueHeuristic() { return 10; }

    // We assume features are mostly geometry
    static inline int featureTagHeuristic(int bytesLeft) { return bytesLeft / 100; }
    static inline int featureGeometryHeuristic(int bytesLeft) { return bytesLeft / 5; }

private:
    // Default state of message structures, for easy setup
    static const vector_tile_Tile_Layer _defaultLayer;
    static const vector_tile_Tile_Feature _defaultFeature;
    static const vector_tile_Tile_Value _defaultValue;

    // This holds a tile value in less space than the one generated by nanopb
    struct SmallValue
    {
        enum SmallValueType : int8_t {
            SmallValNone,
            SmallValString,
            SmallValFloat,
            SmallValDouble,
            SmallValInt,
            SmallValUInt,
            SmallValSInt,
            SmallValBool,
        };
        union {
            std::string_view stringValue;
            float floatValue;
            double doubleValue;
            int64_t intValue;
            uint64_t uintValue;
            int64_t sintValue;
            bool boolValue;
        };
        SmallValueType type;
    };

private:
    // Data parsed and collected
    vector_tile_Tile_Layer* _currentLayer = nullptr;
    bool _layerStarted = false;
    bool _skipLayer = false;
    double _layerScale = 0;
    std::string _layerName;
    std::string_view _layerNameView;
    std::vector<uint32_t> _featureTags;
    std::vector<uint32_t> _featureGeometry;
    std::vector<std::string_view> _layerKeys;
    std::vector<SmallValue> _layerValues;
    std::string _parseError;

private:
    // Data provided by the caller
    const VectorTileData *_tileData;
    VectorStyleDelegateImpl *_styleDelegate;
    PlatformThreadInfo *_styleInst;
    std::map<SimpleIdentity, std::vector<VectorObjectRef>*>& _vecObjByStyle;
    const std::string &_uuidName;
    const std::set<std::string> &_uuidValues;
    const bool _localCoords;
    const bool _parseAll;
    std::vector<VectorObjectRef>* _keepVectors = nullptr;
    std::function<bool()> _checkCancelled;

    // State used during parsing
    const MbrD _bbox;
    const double _bboxWidth;
    const double _bboxHeight;
    
    const double _sx;
    const double _sy;
    const double _tileOriginX;
    const double _tileOriginY;
    
    unsigned _layerCount = 0;
    unsigned _featureCount = 0;
    unsigned _skippedFeatureCount = 0;
    unsigned _skippedLayerCount = 0;
    unsigned _unknownValueTypes = 0;
    unsigned _badAttributes = 0;
    unsigned _unknownCommands = 0;
    unsigned _unknownGeomTypes = 0;
    unsigned _parseErrors = 0;
    bool _wasCancelled = false;

    // Constants
    static constexpr int CmdBits = 3;
    static constexpr int TileSize = 256;
    static constexpr double MAX_EXTENT = 20037508.342789244;
};

} // namespace WhirlyKit

#endif /* VectorTilePBFParser_h */
