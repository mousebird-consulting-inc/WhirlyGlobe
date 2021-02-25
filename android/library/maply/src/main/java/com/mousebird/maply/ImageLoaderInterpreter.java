/*
 *  ImageLoaderInterpreter.java
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
import android.os.Build;

import java.lang.reflect.Field;

/**
 *  Image loader intrepreter turns data objects into ImageTiles.
 *
 *  This is the default interpreter used by the QuadImageLoader.
 */
public class ImageLoaderInterpreter implements LoaderInterpreter
{
    // Set if we can use the premultiply option for images
    boolean hasPremultiplyOption = false;

    // Turn this off we need the loader to not mess with values in the channels
    // This happens for data loading
    public boolean usePremultiply = false;

    public ImageLoaderInterpreter()
    {
        // See if the premultiplied option is available
        try {
            Object opts = new BitmapFactory.Options();
            Class<?> theClass = opts.getClass();
            Field field = theClass.getField("inPremultiplied");
            int version = Build.VERSION.SDK_INT;
            if (field != null && version >= 23) {
                hasPremultiplyOption = true;
            }
        }
        catch (Exception x)
        {
            // Premultiply is missing
        }
    }

    public void setLoader(QuadLoaderBase loader)
    {
    }

    // Convert byte arrays into images
    public void dataForTile(LoaderReturn inLoadReturn,QuadLoaderBase loader)
    {
        ImageLoaderReturn loadReturn = (ImageLoaderReturn)inLoadReturn;

        BitmapFactory.Options options = new BitmapFactory.Options();
// 		                options.inScaled = false;
        if (hasPremultiplyOption && usePremultiply)
            options.inPremultiplied = false;

        byte[][] images =loadReturn.getTileData();
        for (byte[] image : images) {
            Bitmap bm = BitmapFactory.decodeByteArray(image,0, image.length,options);
            if (bm != null)
                loadReturn.addBitmap(bm);
            else
                loadReturn.errorString = "Failed to decode bitmap";
        }
    }

}
