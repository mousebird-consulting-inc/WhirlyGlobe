/*
 *  SLDStyleSet.java
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
package com.mousebird.maply.sld.sldstyleset;

import java.io.InputStream;
import java.io.IOException;
import java.io.StringReader;
import java.util.ArrayList;
import java.util.HashMap;
import java.util.List;

import android.content.res.AssetManager;
import android.util.DisplayMetrics;
import android.util.Log;

import org.xmlpull.v1.XmlPullParser;
import org.xmlpull.v1.XmlPullParserException;
import org.xmlpull.v1.XmlPullParserFactory;

import com.mousebird.maply.AttrDictionary;
import com.mousebird.maply.MaplyBaseController;
import com.mousebird.maply.MaplyTileID;
import com.mousebird.maply.VectorStyle;
import com.mousebird.maply.VectorTileStyle;
import com.mousebird.maply.sld.sldstyleset.SLDNamedLayer;
import com.mousebird.maply.VectorStyleInterface;
import com.mousebird.maply.MaplyBaseController;
import com.mousebird.maply.VectorStyleSettings;
import com.mousebird.maply.sld.sldsymbolizers.SLDSymbolizerParams;

import android.app.Activity;

import android.util.Log;

public class SLDStyleSet implements VectorStyleInterface {

    private boolean useLayerNames;
    private int relativeDrawPriority;
    private HashMap<String, SLDNamedLayer> namedLayers =  new HashMap<String, SLDNamedLayer>();;
    private HashMap<String, VectorStyle> stylesByUUID = new HashMap<String, VectorStyle>();
    private MaplyBaseController viewC;
    private VectorStyleSettings vectorStyleSettings;
    private SLDSymbolizerParams symbolizerParams;

    private InputStream inputStream;

    public SLDStyleSet(MaplyBaseController viewC, AssetManager assetManager, String sldFileName, DisplayMetrics displayMetrics, boolean useLayerNames, int relativeDrawPriority) throws XmlPullParserException, IOException
    {
        this.viewC = viewC;
        this.useLayerNames = useLayerNames;
        this.relativeDrawPriority = relativeDrawPriority;

        inputStream = assetManager.open(sldFileName);

        String basePath;
        int i = sldFileName.lastIndexOf('/');
        if (i == -1)
            basePath = "";
        else
            basePath = sldFileName.substring(0, i);

        float scale = displayMetrics.density;
        vectorStyleSettings = new VectorStyleSettings();
        vectorStyleSettings.setLineScale(scale);
        vectorStyleSettings.setDashPatternScale(scale);
        vectorStyleSettings.setMarkerScale(scale);
        vectorStyleSettings.setUseWideVectors(true);

        symbolizerParams = new SLDSymbolizerParams(viewC, assetManager, vectorStyleSettings, basePath, relativeDrawPriority);
    }

    public void loadSldInputStream() throws XmlPullParserException, IOException
    {
        XmlPullParserFactory factory = XmlPullParserFactory.newInstance();
        factory.setNamespaceAware(true);
        XmlPullParser xpp = factory.newPullParser();

        xpp.setInput(inputStream, null);

        while (xpp.next() != XmlPullParser.END_DOCUMENT) {
            if (xpp.getEventType() != XmlPullParser.START_TAG) {
                continue;
            }
            if (xpp.getName().equals("StyledLayerDescriptor")) {
                this.loadStyledLayerDescriptorNode(xpp);
            }
        }
    }


    private void loadStyledLayerDescriptorNode(XmlPullParser xpp) throws XmlPullParserException, IOException
    {

        while (xpp.next() != XmlPullParser.END_TAG) {
            if (xpp.getEventType() != XmlPullParser.START_TAG) {
                continue;
            }
            if (xpp.getName().equals("NamedLayer")) {
                SLDNamedLayer namedLayer = new SLDNamedLayer(xpp, symbolizerParams);
                namedLayers.put(namedLayer.getName(), namedLayer);

                List<VectorTileStyle> vectorTileStyles = namedLayer.getStyles();
                for (VectorTileStyle vectorTileStyle : vectorTileStyles) {
                    stylesByUUID.put(vectorTileStyle.getUuid(), vectorTileStyle);
                }
            }
        }
    }

    @Override
    public VectorStyle[] stylesForFeature(AttrDictionary attrs, MaplyTileID tileID, String layerName, MaplyBaseController controller)
    {
        List<VectorTileStyle> vectorTileStyles = new ArrayList<VectorTileStyle>();
        boolean matched;
        for (SLDNamedLayer namedLayer : namedLayers.values()) {
            vectorTileStyles.addAll(namedLayer.stylesForFeatureAttributes(attrs));
        }
        return vectorTileStyles.toArray(new VectorStyle[0]);
    }

    @Override
    public boolean layerShouldDisplay(String layerName,MaplyTileID tileID)
    {
        return true;
    }

    @Override
    public VectorStyle styleForUUID(String uuid,MaplyBaseController controller)
    {
        VectorStyle style = stylesByUUID.get(uuid);
        return style;
    }
}
