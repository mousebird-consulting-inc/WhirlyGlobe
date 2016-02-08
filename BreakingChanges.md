Breaking Changes:

## 2.4.1:
* MaplyLinearTextureBuilder
    * no longer has an opacityFunc property.
    * initWithSize: constructor has been removed. Just use init.
    * MaplyLinearTextureOpacity enum has been removed
* Wide Vectors(Lines added using addWideVectors:desc:mode) are now about 50% wider in some cases.
* MaplyTileSource.h
    * `-(bool)validTile:(MaplyTileID)tileID bbox:(MaplyBoundingBox *)bbox` has changed to `-(bool)validTile:(MaplyTileID)tileID bbox:(MaplyBoundingBox)bbox`
