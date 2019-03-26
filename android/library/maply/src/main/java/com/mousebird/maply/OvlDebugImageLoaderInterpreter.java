/*
 *  OvlDebugImageLoaderInterpreter.java
 *  WhirlyGlobeLib
 *
 *  Created by jmnavarro on 3/20/19.
 *  Copyright 2011-2019 mousebird consulting
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

import android.graphics.Bitmap;
import android.graphics.BitmapFactory;
import android.graphics.Color;

import java.lang.reflect.Field;

/**
 *  This loader interpreter sticks a designator in the middle of tiles
 *  and a line around the edge.  Nice for debugging.
 */
public class OvlDebugImageLoaderInterpreter extends ImageLoaderInterpreter
{
    OvlDebugImageLoaderInterpreter()
    {
    }

    @Override  public void setLoader(QuadLoaderBase loader)
    {
    }

    // Convert byte arrays into images
    @Override public void dataForTile(LoaderReturn inLoadReturn,QuadLoaderBase loader)
    {
        ImageLoaderReturn loadReturn = (ImageLoaderReturn)inLoadReturn;

        // This interprets the images
        super.dataForTile(loadReturn,loader);

        // Add a label right in the middle
        Mbr bbox = loader.geoBoundsForTile(loadReturn.getTileID());
        ScreenLabel label = new ScreenLabel();
        label.loc = new Point2d((bbox.ll.getX()+bbox.ur.getX())/2.0,(bbox.ll.getY()+bbox.ur.getY())/2.0);
        label.text = loadReturn.getTileID().toString();
        label.layoutImportance = Float.MAX_VALUE;

        LabelInfo labelInfo = new LabelInfo();
        labelInfo.setTextColor(Color.BLACK);
        labelInfo.setOutlineColor(Color.WHITE);
        labelInfo.setOutlineSize(2.0f);
        labelInfo.setEnable(false);

        loadReturn.addComponentObject(
                loader.getController().addScreenLabel(label,labelInfo, RenderController.ThreadMode.ThreadCurrent)
        );

        // A bounding box around the whole thing
        Point2d[] pts = new Point2d[4];
        pts[0] = bbox.ll;
        pts[1] = new Point2d(bbox.ur.getX(),bbox.ll.getY());
        pts[2] = bbox.ur;
        pts[3] = new Point2d(bbox.ll.getX(),bbox.ur.getY());
        VectorObject vecObj = new VectorObject();
        vecObj.addLinear(pts);
        vecObj = vecObj.subdivideToGlobe(0.001);

        VectorInfo vecInfo = new VectorInfo();
        vecInfo.setEnable(false);
        loadReturn.addComponentObject(
                loader.getController().addVector(vecObj,vecInfo, RenderController.ThreadMode.ThreadCurrent)
        );
    }
}
