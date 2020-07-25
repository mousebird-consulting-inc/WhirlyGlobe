/*
 *  ImageLoaderReturn.java
 *  WhirlyGlobeLib
 *
 *  Created by sjg
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

/**
 *  This version of the loader return is used by the ImageLoaderInterpreter.
 *
 *  When image tiles load, the interpeter fills in these contents, which can
 *  include any sort of ComponentObject and, of course, images.
 */
public class ImageLoaderReturn extends LoaderReturn
{
    public ImageLoaderReturn(int generation) {
        super(generation);
    }

    /**
     * Add an image to this loaded return.
     * You can add multiple, but the interpreter should be expecting that
     */
    public native void addImageTile(ImageTile image);

    /**
     * Add a Bitmap to the loader return.
     * This just adds an ImageTile wrapper around the Bitmap and stores it.
     */
    public void addBitmap(Bitmap bitmap)
    {
        ImageTile imageTile = new ImageTile(bitmap);
        addImageTile(imageTile);
    }

    /**
     * Return the images in this loader return.
     */
    public native ImageTile[] getImages();

    /**
     * Clear out the current images.  Presumably to replace them.
     */
    public native void clearImages();
}
