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
import java.lang.ref.WeakReference;
import java.util.ArrayList;
import java.util.HashMap;
import java.util.List;

import android.content.res.AssetManager;
import android.graphics.Color;
import android.util.DisplayMetrics;
import android.util.Log;

import org.xmlpull.v1.XmlPullParser;
import org.xmlpull.v1.XmlPullParserException;
import org.xmlpull.v1.XmlPullParserFactory;

import com.mousebird.maply.AttrDictionary;
import com.mousebird.maply.RenderControllerInterface;
import com.mousebird.maply.TileID;
import com.mousebird.maply.VectorStyle;
import com.mousebird.maply.VectorTileStyle;
import com.mousebird.maply.VectorStyleInterface;
import com.mousebird.maply.VectorStyleSettings;
import com.mousebird.maply.sld.sldsymbolizers.SLDSymbolizerParams;

import android.app.Activity;

import android.util.Log;

/**
 *
 * A Styled Layer Descriptor (SLD) is an XML document that describes the appearance of map layers.
 * And SLD is made of rules.  Each rule specifies a condition that must be met by a vector feature
 * for the rule to apply to it, and how to display the vector feature if the rule applies.
 *
 * The sld:StyledLayerDescriptor element is the root element of the Styled Layer Descriptor document.
 * Implements the VectorStyleInterface interface for matching and applying styles to vector objects.
 * @see http://schemas.opengis.net/sld/1.1.0/StyledLayerDescriptor.xsd for SLD v1.1.0
 * @see http://schemas.opengis.net/sld/1.0.0/StyledLayerDescriptor.xsd for SLD v1.0.0
 * @see VectorStyleInterface
 */
public class SLDStyleSet implements VectorStyleInterface {

    private boolean useLayerNames;
    private int relativeDrawPriority;
    private HashMap<String, SLDNamedLayer> namedLayers =  new HashMap<String, SLDNamedLayer>();;
    private HashMap<Long, VectorStyle> stylesByUUID = new HashMap<Long, VectorStyle>();
    private WeakReference<RenderControllerInterface> viewC;
    private VectorStyleSettings vectorStyleSettings;
    private SLDSymbolizerParams symbolizerParams;

    private InputStream inputStream;

    /**
     *
     * Constructs a SLDStyleSet object.  This does not load the data in the SLD file.  Use
     * loadSldInputStream() for that.
     *
     * @details After constructing the SLDStyleSet object, call loadSldInputStream() to parse the desired SLD document tree and create the corresponding symbolizers.
     * @param viewC The map or globe view controller
     * @param assetWrapper Wrapper around the asset manager.
     * @param sldFileName The file name of the SLD document
     * @param displayMetrics The DisplayMetrics instance
     * @param useLayerNames Whether to use names of NamedLayer elements as a criteria in matching styles.
     * @param relativeDrawPriority The z-order relative to other vector features. This will be incremented internally for each style rule, so if you have multiple SLDStyleSets, leave some space between the relativeDrawPriority of each.
     * @throws XmlPullParserException
     * @throws IOException
     */
    public SLDStyleSet(RenderControllerInterface viewC, AssetWrapper assetWrapper, String sldFileName, DisplayMetrics displayMetrics, boolean useLayerNames, int relativeDrawPriority) throws XmlPullParserException, IOException
    {
        this.viewC = new WeakReference<RenderControllerInterface>(viewC);
        this.useLayerNames = useLayerNames;
        this.relativeDrawPriority = relativeDrawPriority;

        inputStream = assetWrapper.open(sldFileName);

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

        symbolizerParams = new SLDSymbolizerParams(viewC, assetWrapper, vectorStyleSettings, basePath, relativeDrawPriority);
    }

    /**
     *
     * Parses the desired SLD document tree and create the corresponding symbolizers.
     *
     * @throws XmlPullParserException
     * @throws IOException
     */
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
    public VectorStyle[] stylesForFeature(AttrDictionary attrs, TileID tileID, String layerName, RenderControllerInterface controller)
    {
        List<VectorTileStyle> vectorTileStyles = new ArrayList<VectorTileStyle>();
        boolean matched;
        for (SLDNamedLayer namedLayer : namedLayers.values()) {
            vectorTileStyles.addAll(namedLayer.stylesForFeatureAttributes(attrs));
        }
        return vectorTileStyles.toArray(new VectorStyle[0]);
    }

    @Override
    public boolean layerShouldDisplay(String layerName,TileID tileID)
    {
        return true;
    }

    @Override
    public VectorStyle styleForUUID(long uuid, RenderControllerInterface controller)
    {
        VectorStyle style = stylesByUUID.get(uuid);
        return style;
    }

    @Override
    public VectorStyle[] allStyles()
    {
        return stylesByUUID.values().toArray(new VectorStyle[0]);
    }

    @Override
    public int backgroundColorForZoom(double zoom) { return Color.BLACK; }
}
