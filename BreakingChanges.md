Breaking Changes:

## 2.5.1:
* We moved directories around.  Look for the AutoTester app in ios/AutoTester
* If you’re linking to WhirlyGlobeMaplyComponent look for it in ios/library/WhirlyGlobe-MaplyComponent

## 2.4.1:
* MaplyLinearTextureBuilder
    * no longer has an opacityFunc property.
    * initWithSize: constructor has been removed. Just use init.
    * MaplyLinearTextureOpacity enum has been removed
* Wide Vectors(Lines added using addWideVectors:desc:mode) are now about 50% wider in some cases.
* MaplyTileSource.h
    * `-(bool)validTile:(MaplyTileID)tileID bbox:(MaplyBoundingBox *)bbox` has changed to `-(bool)validTile:(MaplyTileID)tileID bbox:(MaplyBoundingBox)bbox`
* MaplyViewController
    * `- (float)findHeightToViewBounds:(MaplyBoundingBox *)bbox pos:(MaplyCoordinate)pos` has changed to `- (float)findHeightToViewBounds:(MaplyBoundingBox)bbox pos:(MaplyCoordinate)pos` 
* WhirlyGlobeViewController
    * `- (float)findHeightToViewBounds:(MaplyBoundingBox *)bbox pos:(MaplyCoordinate)pos` has changed to `- (float)findHeightToViewBounds:(MaplyBoundingBox)bbox pos:(MaplyCoordinate)pos`
* MaplyMapnikVectorTiles
    * Files have been renamed to MapboxVectorTiles.{mm/h}
    * Class has been renamed to MapboxVectorTiles
* MaplyVectorStyle
    * `MaplyVectorTileStyleSettings` has been renamed `MaplyVectorStyleSettings`
* AFNetworking has been removed
* MaplyRemoteTileSource
    * MaplyRemoteTileInfo now implements MaplyRemoteTileInfoProtocol
    * MaplyRemoteTileSource `tileInfo` property has changed from type `MaplyRemoteTileInfo *` to `NSObject<MaplyRemoteTileInfoProtocol> *`  
    * `- (instancetype)initWithInfo:(MaplyRemoteTileInfo *)info;` has changed to `- (nullable instancetype)initWithInfo:(NSObject<MaplyRemoteTileInfoProtocol> *__nonnull)info;`
