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
    ImageLoaderReturn(QuadLoaderBase loader) {
        super(loader);
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

    /**
     * If any component objects are associated with the tile, these are them.
     * They need to start disabled.  The system will enable and delete them when it is time.
     */
    public void addComponentObjects(ComponentObject[] compObjs)
    {
        addComponentObjects(compObjs,false);
    }

    /**
     * Clear out the component objects, presumably to replace them.
     */
    public void clearComponentObjects() {
        clearComponentObjectsNative(false);
    }

    /**
     * Add a single component object to the tile
     */
    public void addComponentObject(ComponentObject compObj)
    {
        ComponentObject[] compArr = new ComponentObject[1];
        compArr[0] = compObj;
        addComponentObjects(compArr);
    }

    /**
     * These component objects are assumed to be overlaid and so only one
     * set will be displayed at a time.
     */
    public void addOverlayComponentObjects(ComponentObject[] compObjs)
    {
        addComponentObjects(compObjs,true);
    }

    public void clearOverlayComponentObjects() {
        clearComponentObjectsNative(true);
    }

    private native void addComponentObjects(ComponentObject[] compObjs,boolean isOverlay);
    private native void clearComponentObjectsNative(boolean isOverlay);
}
