/*
 *  SLDOperatorFactory.java
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
package com.mousebird.maply.sld.sldoperators;

import org.xmlpull.v1.XmlPullParser;
import org.xmlpull.v1.XmlPullParserException;

import java.io.IOException;
import java.io.InputStream;

import android.util.Log;

public class SLDOperatorFactory {

    public static SLDOperator operatorForNode(XmlPullParser xpp) throws XmlPullParserException, IOException {
        String operatorName = xpp.getName();
        if (SLDLogicalOperator.matchesElementNamed(operatorName))
            return new SLDLogicalOperator(xpp);
        else if (SLDBinaryComparisonOperator.matchesElementNamed(operatorName))
            return new SLDBinaryComparisonOperator(xpp);
        else if (SLDNotOperator.matchesElementNamed(operatorName))
            return new SLDNotOperator(xpp);
        else if (SLDIsNullOperator.matchesElementNamed(operatorName))
            return new SLDIsNullOperator(xpp);
        else if (SLDIsLikeOperator.matchesElementNamed(operatorName))
            return new SLDIsLikeOperator(xpp);
        else if (SLDIsBetweenOperator.matchesElementNamed(operatorName))
            return new SLDIsBetweenOperator(xpp);
        return null;
    }
}
