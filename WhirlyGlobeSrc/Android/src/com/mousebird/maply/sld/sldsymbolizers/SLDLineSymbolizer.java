/*
 *  SLDLineSymbolizer.java
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


import android.util.Log;

import com.mousebird.maply.VectorStyleSettings;
import com.mousebird.maply.VectorTileStyle;
import com.mousebird.maply.sld.sldstyleset.SLDParseHelper;
import com.mousebird.maply.VectorTileLineStyle;
import com.mousebird.maply.MaplyBaseController;

import org.xmlpull.v1.XmlPullParser;
import org.xmlpull.v1.XmlPullParserException;

import java.io.IOException;

public class SLDLineSymbolizer extends SLDSymbolizer {

    private VectorTileLineStyle vectorTileLineStyle;

    public SLDLineSymbolizer(XmlPullParser xpp, SLDSymbolizerParams symbolizerParams) throws XmlPullParserException, IOException {

        while (xpp.next() != XmlPullParser.END_TAG) {
            if (xpp.getEventType() != XmlPullParser.START_TAG) {
                continue;
            }
            Log.i("SLDLineSymbolizer", xpp.getName());

            if (xpp.getName().equals("Stroke")) {
                vectorTileLineStyle = SLDSymbolizer.vectorTileLineStyleFromStrokeNode(xpp, symbolizerParams.getBaseController(), symbolizerParams.getVectorStyleSettings());
            } else {
                SLDParseHelper.skip(xpp);
            }
        }
    }


    public VectorTileStyle[] getStyles() {
        return new VectorTileStyle[]{vectorTileLineStyle};
    }

    public static boolean matchesSymbolizerNamed(String symbolizerName) {
        if (symbolizerName.equals("LineSymbolizer"))
            return true;
        return false;
    }
}
