/*
 *  SLDLiteralExpression.java
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
package com.mousebird.maply.sld.sldexpressions;


import com.mousebird.maply.AttrDictionary;

import org.xmlpull.v1.XmlPullParser;
import org.xmlpull.v1.XmlPullParserException;

import java.io.IOException;
import java.io.InputStream;

import android.util.Log;

public class SLDLiteralExpression extends SLDExpression {

    private Object literal;

    public SLDLiteralExpression(XmlPullParser xpp) throws XmlPullParserException, IOException {
        while (xpp.next() != XmlPullParser.END_TAG) {
            if (xpp.getEventType() != XmlPullParser.TEXT) {
                continue;
            }
            Log.i("SLDLiteralExpr", xpp.getText());
        }

    }


    public Object evaluateWithAttrs(AttrDictionary attrs) {
        return null;
    }

    public static boolean matchesElementNamed(String elementName) {
        if (elementName.equals("Literal"))
            return true;
        return false;
    }

}
