package com.mousebird.maply;

public class MapboxVectorStyleSet {
//    - (id __nullable)initWithJSON:(NSData * __nonnull)styleJSON
//    settings:(MaplyVectorStyleSettings * __nonnull)settings
//    viewC:(NSObject<MaplyRenderControllerProtocol> * __nonnull)viewC;
//
///// @brief Where we can fetch the sprites
//    @property (nonatomic, strong, nullable) NSString *spriteURL;
//
///// Tile sources
//    @property (nonatomic, strong, nonnull) NSArray *sources;
//
///// If there is a background layer, calculate the color for a given zoom level.
///// Otherwise return nil
//- (UIColor * __nullable)backgroundColorForZoom:(double)zoom;
//
//    @property (nonatomic, weak, nullable) NSObject<MaplyRenderControllerProtocol> *viewC;

    MapboxVectorStyleSet(String styleJSON,VectorStyleSettings settings,RenderController control) {

    }

    // If there's a sprite sheet, where it's at
    String spriteURL;

    enum SourceType {Vector,Raster};

    // Source for vector tile (or raster) data
    class Source {
        // Name as it appears in the file
        String name;

        // Either vector or raster at present
        SourceType type;

        // TileJSON URL, if present
        String url;

        // If the TileJSON spec is inline, it's here
        AttrDictionary tileSpec;

        Source(String inName,AttrDictionary styleEntry, MapboxVectorStyleSet styleSet,RenderController control) {
            name = inName;

            String typeStr = styleEntry.getString("type");
            if (typeStr == "vector") {
                type = SourceType.Vector;
            } else if (typeStr == "raster") {
                type = SourceType.Raster;
            } else {
                throw new IllegalArgumentException("Unexpected type string in Mapbox Source");
            }

            url = styleEntry.getString("url");
            tileSpec = styleEntry.getDict("tiles");

            if (url == null && tileSpec == null) {
                throw new IllegalArgumentException("Expecting either URL or tileSpec in source " + name);
            }
        }
    }
}
