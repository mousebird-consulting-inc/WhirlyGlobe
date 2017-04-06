/*
 *  SLDSymbolizerFactory.java
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

import org.xmlpull.v1.XmlPullParser;
import org.xmlpull.v1.XmlPullParserException;

import java.io.IOException;

import com.mousebird.maply.VectorStyleSettings;
import com.mousebird.maply.MaplyBaseController;

public class SLDSymbolizerFactory {

    public static SLDSymbolizer symbolizerForNode(XmlPullParser xpp, SLDSymbolizerParams symbolizerParams) throws XmlPullParserException, IOException {
        String symbolizerName = xpp.getName();
        if (SLDLineSymbolizer.matchesSymbolizerNamed(symbolizerName))
            return new SLDLineSymbolizer(xpp, symbolizerParams);
        else if (SLDPolygonSymbolizer.matchesSymbolizerNamed(symbolizerName))
            return new SLDPolygonSymbolizer(xpp);
        else if (SLDPointSymbolizer.matchesSymbolizerNamed(symbolizerName))
            return new SLDPointSymbolizer(xpp);
        else if (SLDTextSymbolizer.matchesSymbolizerNamed(symbolizerName))
            return new SLDTextSymbolizer(xpp);

        return null;
    }

}
