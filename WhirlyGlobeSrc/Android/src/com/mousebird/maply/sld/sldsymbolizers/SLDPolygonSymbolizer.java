/*
 *  SLDPolygonSymbolizer.java
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


import android.graphics.Color;
import android.graphics.Bitmap;
import android.util.Log;

import com.mousebird.maply.MaplyBaseController;
import com.mousebird.maply.VectorStyleSettings;
import com.mousebird.maply.VectorTileLineStyle;
import com.mousebird.maply.VectorTilePolygonStyle;
import com.mousebird.maply.VectorTileStyle;
import com.mousebird.maply.WideVectorInfo;
import com.mousebird.maply.VectorInfo;
import com.mousebird.maply.MaplyTexture;
import com.mousebird.maply.sld.sldstyleset.SLDParseHelper;

import org.xmlpull.v1.XmlPullParser;
import org.xmlpull.v1.XmlPullParserException;

import java.io.IOException;
import java.util.ArrayList;

public class SLDPolygonSymbolizer extends SLDSymbolizer {

    private VectorTileLineStyle vectorTileLineStyle;
    private VectorTilePolygonStyle vectorTilePolygonStyle;

    public SLDPolygonSymbolizer(XmlPullParser xpp, SLDSymbolizerParams symbolizerParams) throws XmlPullParserException, IOException {
        while (xpp.next() != XmlPullParser.END_TAG) {
            if (xpp.getEventType() != XmlPullParser.START_TAG) {
                continue;
            }
            Log.i("SLDPolygonSymbolizer", xpp.getName());
            if (xpp.getName().equals("Stroke")) {
                vectorTileLineStyle = SLDSymbolizer.vectorTileLineStyleFromStrokeNode(xpp, symbolizerParams.getBaseController(), symbolizerParams.getVectorStyleSettings());
            } else if (xpp.getName().equals("Fill")) {
                vectorTilePolygonStyle = vectorTilePolygonStyleFromFillNode(xpp, symbolizerParams);
            } else {
                SLDParseHelper.skip(xpp);
            }
        }
    }

    public VectorTileStyle[] getStyles() {
        ArrayList<VectorTileStyle> styles = new ArrayList<VectorTileStyle>();
        if (vectorTileLineStyle != null)
            styles.add(vectorTileLineStyle);
        if (vectorTilePolygonStyle != null)
            styles.add(vectorTilePolygonStyle);
        return styles.toArray(new VectorTileStyle[]{});
    }


    public static boolean matchesSymbolizerNamed(String symbolizerName) {
        if (symbolizerName.equals("PolygonSymbolizer"))
            return true;
        return false;
    }

    public static VectorTilePolygonStyle vectorTilePolygonStyleFromFillNode(XmlPullParser xpp, SLDSymbolizerParams symbolizerParams) throws XmlPullParserException, IOException {
//MaplyBaseController viewC, VectorStyleSettings vectorStyleSettings, String basePath
        VectorInfo wideVectorInfo = new VectorInfo();
        wideVectorInfo.disposeAfterUse = true;
        wideVectorInfo.setEnable(false);
        wideVectorInfo.setFilled(true);

        Integer fillColor = null;
        Float fillOpacity = null;

        while (xpp.next() != XmlPullParser.END_TAG) {
            if (xpp.getEventType() != XmlPullParser.START_TAG) {
                continue;
            }

            if (xpp.getName().equals("SvgParameter") || xpp.getName().equals("CssParameter")) {

                String name = xpp.getAttributeValue(null, "name");
                String value = null;
                while (xpp.next() != XmlPullParser.END_TAG) {
                    if (xpp.getEventType() != XmlPullParser.TEXT) {
                        continue;
                    }
                    value = xpp.getText();
                }
                if (name == null || value == null)
                    continue;

                if (name.equals("fill")) {
                    try {
                        fillColor = Integer.valueOf(Color.parseColor(value));
                    } finally {
                    }
                } else if (name.equals("fill-opacity") || name.equals("opacity")) {
                    try {
                        fillOpacity = Float.valueOf(value);
                    } finally {
                    }
                }

            } else if (xpp.getName().equals("GraphicFill")) {
                while (xpp.next() != XmlPullParser.END_TAG) {
                    if (xpp.getEventType() != XmlPullParser.START_TAG) {
                        continue;
                    }
                    if (xpp.getName().equals("Graphic")) {
                        SLDGraphicParams graphicParams = SLDSymbolizer.graphicParamsForGraphicNode(xpp, symbolizerParams);
                        if (graphicParams != null) {
                            Bitmap bmp = graphicParams.getBitmap();
                            if (bmp != null) {
                                // TODO: how do we use textures with filled polygons in Android?
//                                MaplyTexture tex = symbolizerParams.getBaseController().addTexture(graphicParams.getBitmap(), new MaplyBaseController.TextureSettings(), MaplyBaseController.ThreadMode.ThreadCurrent);
//                                wideVectorInfo.setTexture(tex);
                            }
                        }
                    }
                }
            }
        }

        if (fillColor != null) {
            int color = fillColor.intValue();
            if (fillOpacity != null)
                color = Color.argb(Math.round(fillOpacity.floatValue()*255.f), Color.red(color), Color.green(color), Color.blue(color));

            wideVectorInfo.setColor(color);
        }

        VectorTilePolygonStyle vectorTilePolygonStyle = new VectorTilePolygonStyle(wideVectorInfo, symbolizerParams.getVectorStyleSettings(), symbolizerParams.getBaseController());
        return vectorTilePolygonStyle;
    }

}
