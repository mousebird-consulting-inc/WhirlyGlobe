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
import java.util.HashMap;

import android.util.Log;

import org.xmlpull.v1.XmlPullParser;
import org.xmlpull.v1.XmlPullParserException;
import org.xmlpull.v1.XmlPullParserFactory;
import com.mousebird.maply.sld.sldstyleset.SLDNamedLayer;

import android.util.Log;

public class SLDStyleSet {

    private boolean useLayerNames;
    private int relativeDrawPriority;
    private HashMap<String, SLDNamedLayer> namedLayers;

    public SLDStyleSet(boolean useLayerNames, int relativeDrawPriority)
    {
        this.useLayerNames = useLayerNames;
        this.relativeDrawPriority = relativeDrawPriority;
        this.namedLayers = new HashMap<String, SLDNamedLayer>();
    }

    public void loadSldInputStream(InputStream in) throws XmlPullParserException, IOException
    {
        XmlPullParserFactory factory = XmlPullParserFactory.newInstance();
        factory.setNamespaceAware(true);
        XmlPullParser xpp = factory.newPullParser();

        xpp.setInput(in, null);

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
                SLDNamedLayer namedLayer = new SLDNamedLayer(xpp);
                this.namedLayers.put(namedLayer.getName(), namedLayer);
            }
        }
    }
}
