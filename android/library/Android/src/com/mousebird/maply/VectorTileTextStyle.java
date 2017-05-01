/*
 *  MaplyVectorTileTextStyle.java
 *  WhirlyGlobeLib
 *
 *  Created by Ranen Ghosh on 3/27/17.
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

package com.mousebird.maply;


import java.util.ArrayList;
import java.util.HashMap;
import java.util.List;
import java.util.regex.Pattern;
import java.util.regex.Matcher;


public class VectorTileTextStyle extends VectorTileStyle {

    private LabelInfo labelInfo;
    private String textField;
    private Point2d offset;

    public VectorTileTextStyle(LabelInfo labelInfo, Point2d offset, String textField, VectorStyleSettings settings, MaplyBaseController viewC) {
        super(viewC);

        this.labelInfo = labelInfo;
        this.textField = textField;
        this.offset = offset;
    }

    public ComponentObject[] buildObjects(List<VectorObject> objects, MaplyTileID tileID, MaplyBaseController controller) {

        ArrayList<ScreenLabel> labels = new ArrayList<ScreenLabel>();

        for (VectorObject vector : objects) {

            Point2d centroid = vector.centroid();
            if (centroid != null) {
                ScreenLabel screenLabel = new ScreenLabel();
                screenLabel.loc = centroid;
                screenLabel.offset = offset;
                screenLabel.selectable = true;
                screenLabel.text = formatText(textField, vector.getAttributes());
                labels.add(screenLabel);
            }
        }



        ComponentObject compObj = controller.addScreenLabels(labels, labelInfo, MaplyBaseController.ThreadMode.ThreadCurrent);
        if (compObj != null) {
            return new ComponentObject[]{compObj};
        }
        return null;
    }

    public static String formatText(String formatString, AttrDictionary attributes) {

        if (formatString == null)
            return null;

        try {
            // Do variable substitution on [ ... ]
            String patternStr = "\\[[^\\[\\]]+\\]";
            Pattern pattern = Pattern.compile(patternStr);
            Matcher matcher = pattern.matcher(formatString);
            while (matcher.find()) {

                int start = matcher.start();
                int end = matcher.end();

                String matchedStr = formatString.substring(start+1, end-1);
                Object replacement = attributes.get(matchedStr);
                String replacementStr = "";
                if (replacement instanceof String)
                    replacementStr = (String)replacement;
                else if (replacement instanceof Number)
                    replacementStr = ((Number)replacement).toString();

                formatString = formatString.replaceFirst(patternStr, replacementStr);

                matcher = pattern.matcher(formatString);
            }

            // replace \n with a newline
            // TODO: Do I need to do this?

            // replace + and surrounding whitespace
            // TODO: Do I need to do this?

            // replace quotes around quoted strings
            // TODO: Do I need to do this?


            return formatString;

        } catch (Exception e) {
            return null;
        }
    }
}
