/*
 *  SLDPointSymbolizer.java
 *  WhirlyGlobeLib
 *
 *  Created by Ranen Ghosh on 3/14/17.
 *  Copyright 2011-2017 mousebird consulting
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
package com.mousebird.maply.sld.sldsymbolizers;


import android.graphics.Bitmap;
import android.util.Log;

import com.mousebird.maply.MaplyBaseController;
import com.mousebird.maply.VectorStyleSettings;
import com.mousebird.maply.VectorTileMarkerStyle;
import com.mousebird.maply.VectorTileStyle;
import com.mousebird.maply.sld.sldstyleset.SLDParseHelper;
import com.mousebird.maply.MaplyTexture;
import com.mousebird.maply.MarkerInfo;

import org.xmlpull.v1.XmlPullParser;
import org.xmlpull.v1.XmlPullParserException;

import java.io.IOException;
import java.util.HashMap;

/**
 *
 * Class corresponding to the PointSymbolizer element
 * @see http://schemas.opengis.net/se/1.1.0/Symbolizer.xsd for SLD v1.1.0
 * @see http://schemas.opengis.net/sld/1.0.0/StyledLayerDescriptor.xsd for SLD v1.0.0
 */
public class SLDPointSymbolizer extends SLDSymbolizer {

    private VectorTileMarkerStyle vectorTileMarkerStyle;

    public SLDPointSymbolizer(XmlPullParser xpp, SLDSymbolizerParams symbolizerParams) throws XmlPullParserException, IOException {

        MaplyBaseController viewC = symbolizerParams.getBaseController();
        VectorStyleSettings vectorStyleSettings = symbolizerParams.getVectorStyleSettings();

        Bitmap bitmap = null;
        MarkerInfo markerInfo = new MarkerInfo();
        markerInfo.setEnable(false);
        markerInfo.setLayoutImportance(1.f);

        if (symbolizerParams.getMinScaleDenominator() != null) {
            if (symbolizerParams.getMaxScaleDenominator() == null)
                markerInfo.setMaxVis(Float.MAX_VALUE);
            markerInfo.setMinVis((float) viewC.heightForMapScale(symbolizerParams.getMinScaleDenominator().floatValue()));
        }
        if (symbolizerParams.getMaxScaleDenominator() != null) {
            if (symbolizerParams.getMinScaleDenominator() == null)
                markerInfo.setMinVis(0.0f);
            markerInfo.setMaxVis((float) viewC.heightForMapScale(symbolizerParams.getMaxScaleDenominator().floatValue()));
        }

        while (xpp.next() != XmlPullParser.END_TAG) {
            if (xpp.getEventType() != XmlPullParser.START_TAG) {
                continue;
            }
            if (xpp.getName().equals("Graphic")) {
                SLDGraphicParams graphicParams = SLDSymbolizer.graphicParamsForGraphicNode(xpp, symbolizerParams);
                if (graphicParams != null) {
                    bitmap = graphicParams.getBitmap();

                    Number width = graphicParams.getWidth();
                    Number height = graphicParams.getHeight();
                    HashMap<String, Object> crossSymParams = symbolizerParams.getCrossSymbolizerParams();
                    if (width != null && height != null && crossSymParams != null) {
                        crossSymParams.put("width", width);
                        crossSymParams.put("height", height);

                    }
                }

            } else {
                SLDParseHelper.skip(xpp);
            }
        }

        markerInfo.setDrawPriority(symbolizerParams.getRelativeDrawPriority() + MaplyBaseController.MarkerDrawPriorityDefault);
        vectorTileMarkerStyle = new VectorTileMarkerStyle(markerInfo, bitmap, vectorStyleSettings, viewC);
    }

    public VectorTileStyle[] getStyles() {
        if (vectorTileMarkerStyle != null)
            return new VectorTileStyle[]{vectorTileMarkerStyle};
        return new VectorTileStyle[]{};
    }


    public static boolean matchesSymbolizerNamed(String symbolizerName) {
        if (symbolizerName.equals("PointSymbolizer"))
            return true;
        return false;
    }

}
