/*  SLDPolygonSymbolizer.java
 *  WhirlyGlobeLib
 *
 *  Created by Ranen Ghosh on 3/14/17.
 *  Copyright 2011-2021 mousebird consulting
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
 */
package com.mousebird.maply.sld.sldsymbolizers;

import android.graphics.Color;
import android.graphics.Bitmap;

import com.mousebird.maply.RenderController;
import com.mousebird.maply.RenderControllerInterface;
import com.mousebird.maply.VectorStyleSettings;
import com.mousebird.maply.VectorTileLineStyle;
import com.mousebird.maply.VectorTilePolygonStyle;
import com.mousebird.maply.VectorTileStyle;
import com.mousebird.maply.VectorInfo;
import com.mousebird.maply.MaplyTexture;
import com.mousebird.maply.sld.sldstyleset.SLDParseHelper;

import org.xmlpull.v1.XmlPullParser;
import org.xmlpull.v1.XmlPullParserException;

import java.io.IOException;
import java.util.ArrayList;

/**
 *  Class corresponding to the PolygonSymbolizer element
 * see http://schemas.opengis.net/se/1.1.0/Symbolizer.xsd for SLD v1.1.0
 * see http://schemas.opengis.net/sld/1.0.0/StyledLayerDescriptor.xsd for SLD v1.0.0
 */
public class SLDPolygonSymbolizer extends SLDSymbolizer {

    private VectorTileLineStyle vectorTileLineStyle;
    private VectorTilePolygonStyle vectorTilePolygonStyle;

    public SLDPolygonSymbolizer(XmlPullParser xpp, SLDSymbolizerParams symbolizerParams) throws XmlPullParserException, IOException {
        while (xpp.next() != XmlPullParser.END_TAG) {
            if (xpp.getEventType() != XmlPullParser.START_TAG) {
                continue;
            }
            if (xpp.getName().equals("Stroke")) {
                vectorTileLineStyle = SLDSymbolizer.vectorTileLineStyleFromStrokeNode(xpp, symbolizerParams);
                symbolizerParams.incrementRelativeDrawPriority();
            } else if (xpp.getName().equals("Fill")) {
                vectorTilePolygonStyle = vectorTilePolygonStyleFromFillNode(xpp, symbolizerParams);
                symbolizerParams.incrementRelativeDrawPriority();
            } else {
                SLDParseHelper.skip(xpp);
            }
        }
    }

    public VectorTileStyle[] getStyles() {
        ArrayList<VectorTileStyle> styles = new ArrayList<>();
        if (vectorTileLineStyle != null)
            styles.add(vectorTileLineStyle);
        if (vectorTilePolygonStyle != null)
            styles.add(vectorTilePolygonStyle);
        return styles.toArray(new VectorTileStyle[]{});
    }


    public static boolean matchesSymbolizerNamed(String symbolizerName) {
        return symbolizerName.equals("PolygonSymbolizer");
    }

    public static VectorTilePolygonStyle vectorTilePolygonStyleFromFillNode(XmlPullParser xpp, SLDSymbolizerParams symbolizerParams) throws XmlPullParserException, IOException {

        RenderControllerInterface viewC = symbolizerParams.getBaseController();
        VectorStyleSettings vectorStyleSettings = symbolizerParams.getVectorStyleSettings();

        VectorInfo vectorInfo = new VectorInfo();
        vectorInfo.setEnable(false);
        vectorInfo.setFilled(true);

        if (symbolizerParams.getMinScaleDenominator() != null) {
            if (symbolizerParams.getMaxScaleDenominator() == null)
                vectorInfo.setMaxVis(Float.MAX_VALUE);
            vectorInfo.setMinVis((float) viewC.heightForMapScale(symbolizerParams.getMinScaleDenominator().floatValue()));
        }
        if (symbolizerParams.getMaxScaleDenominator() != null) {
            if (symbolizerParams.getMinScaleDenominator() == null)
                vectorInfo.setMinVis(0.0f);
            vectorInfo.setMaxVis((float) viewC.heightForMapScale(symbolizerParams.getMaxScaleDenominator().floatValue()));
        }

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
                    fillColor = Color.parseColor(value);
                } else if (name.equals("fill-opacity") || name.equals("opacity")) {
                    fillOpacity = Float.valueOf(value);
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
                                RenderController.TextureSettings texSettings = new RenderController.TextureSettings();
                                texSettings.wrapU = true;  texSettings.wrapV = true;
                                MaplyTexture tex = viewC.addTexture(graphicParams.getBitmap(), texSettings, RenderController.ThreadMode.ThreadCurrent);
                                vectorInfo.setTexture(tex);

                                float scaleX = 50000.0f;
                                float scaleY = -100000.0f;
                                Integer width = graphicParams.getWidth();
                                Integer height = graphicParams.getHeight();
                                if (width != null && width.floatValue() != 0.0f)
                                    scaleX = 50000.0f / width.floatValue();
                                if (height != null && height.floatValue() != 0.0f)
                                    scaleY = -100000.0f / height.floatValue();

                                vectorInfo.setTexScale(scaleX, scaleY);
                                vectorInfo.setTextureProjection(VectorInfo.TextureProjection.TangentPlane);

                            }
                        }
                    }
                }
            }
        }

        if (fillColor != null) {
            int color = fillColor;
            if (fillOpacity != null)
                color = Color.argb(Math.round(fillOpacity *255.f), Color.red(color), Color.green(color), Color.blue(color));

            vectorInfo.setColor(color);
        }

        vectorInfo.setDrawPriority(symbolizerParams.getRelativeDrawPriority() + RenderController.FeatureDrawPriorityBase);
        return new VectorTilePolygonStyle(null,null,vectorInfo, vectorStyleSettings,viewC);
    }

}
