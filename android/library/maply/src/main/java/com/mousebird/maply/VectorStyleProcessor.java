/*
 *  VectorStyleProcessor
 *  com.mousebirdconsulting.maply
 *
 *  Created by Steve Gifford.
 *  Copyright 2013-2019 mousebird consulting
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
package com.mousebird.maply;

import java.util.ArrayList;
import java.util.Arrays;
import java.util.HashMap;

/**
 * Provides a static method to run vectors through a vector style.
 * This can be used as a standalone to visualize vectors from any source
 * that's converted to a list of VectorObjects.
 */
public class VectorStyleProcessor
{
    /**
     * Use the given style to turn the vector objects into visual components.
     * Looking for at least a "layer" or "layer_name" for each object.  Can work anyway.
     * @param vecObjs Vector objects to process.
     * @param styleGen Style to use in processing them
     * @param vc Controller to build the features against.
     * @return ComponentObjects created for the visuals.
     */
    public static ComponentObject[] UseStyle(VectorObject[] vecObjs,VectorStyleInterface styleGen,RenderControllerInterface vc)
    {
        HashMap<Long,ArrayList<VectorObject>> featuresForStyle = new HashMap<Long,ArrayList<VectorObject>>();

        // Pass in fake information for parsing a "tile"
        TileID tileID = new TileID();
        tileID.x = 0;  tileID.y = 0;  tileID.level = 0;
        Mbr bbox = new Mbr(new Point2d(-Math.PI,-Math.PI/90.0),
                new Point2d(Math.PI,Math.PI/2.0));
        VectorTileData tileData = new VectorTileData(tileID,bbox,bbox);

        // Each individual vector feature in each vector object (e.g. splitVectors)
        int whichLayer = 0;
        for (VectorObject thisVecObj : vecObjs) {
            for (VectorObject vecObj : thisVecObj) {
                AttrDictionary attrs = vecObj.getAttributes();

                // Tease out a layer name
                String layer = attrs.getString("layer");
                if (layer == null) {
                    layer = attrs.getString("layer_name");
                }

                // Allow the style to short circuit
                if (layer != null && !styleGen.layerShouldDisplay(layer,tileID)) {
                    continue;
                }

                if (layer == null)
                    layer = "layer" + whichLayer;

                // Convert the geometry type to a number the style may be expecting
                int geomType = MapboxVectorTileParser.GeomTypeUnknown;
                switch (vecObj.getVectorType()) {
                    case MaplyVectorNoneType:
                    case MaplyVectorMultiType:
                        break;
                    case MaplyVectorPointType:
                        geomType = MapboxVectorTileParser.GeomTypePoint;
                        break;
                    case MaplyVectorLinearType:
                        geomType = MapboxVectorTileParser.GeomTypeLineString;
                        break;
                    case MaplyVectorLinear3dType:
                        geomType = MapboxVectorTileParser.GeomTypeLineString;
                        break;
                    case MaplyVectorArealType:
                        geomType = MapboxVectorTileParser.GeomTypePolygon;
                        break;
                }
                attrs.setInt("geometry_type", geomType);

                VectorStyle[] styles = styleGen.stylesForFeature(attrs,tileID,layer,vc);
                if (styles == null || styles.length == 0)
                    continue;

                // Sort the data into buckets for the various styles
                for (VectorStyle thisStyle : styles) {
                    ArrayList<VectorObject> thisVecObjs = featuresForStyle.get(thisStyle.getUuid());
                    if (thisVecObjs == null)
                        thisVecObjs = new ArrayList<VectorObject>();
                    thisVecObjs.add(vecObj);
                    featuresForStyle.put(thisStyle.getUuid(),thisVecObjs);
                }
            }

            whichLayer++;
        }

        // Build all the features sorted by style
        for (long styleID : featuresForStyle.keySet()) {
            ArrayList<VectorObject> vecs = featuresForStyle.get(styleID);
            VectorStyle style = styleGen.styleForUUID(styleID,vc);
            style.buildObjects(vecs.toArray(new VectorObject[0]),tileData,vc);
        }

        ComponentObject[] compObjs = tileData.getComponentObjects();
        vc.enableObjects(new ArrayList<ComponentObject>(Arrays.asList(compObjs)), RenderControllerInterface.ThreadMode.ThreadCurrent);

        return compObjs;
    }
}
