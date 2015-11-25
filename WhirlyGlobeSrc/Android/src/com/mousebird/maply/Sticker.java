package com.mousebird.maply;

import android.graphics.Bitmap;

import java.util.ArrayList;

/**
 * The Sticker class is used to represent a rectangular area we stick on top
 * of the globe or map.
 *
 * @author sjg
 */
public class Sticker
{
    /**
     * Construct with empty values.
     */
    public Sticker() { initialise(); }

    public void finalize()
    {
        dispose();
    }

    /**
     * Set the lower left corner of the sticker.
     * This is in the given coordinate system.
     */
    public void setLowerLeft(Point2d pt)
    {
        setLowerLeft(pt.getX(),pt.getY());
    }

    /**
     * Set the upper right corner of the sticker.
     * This is in the given coordinate system.
     */
    public void setUpperRight(Point2d pt)
    {
        setUpperRight(pt.getX(),pt.getY());
    }

    /**
     * Set the lower left corner of the sticker.
     * This is in the given coordinate system.
     */
    public native void setLowerLeft(double x,double y);

    /**
     * Set the upper right corner of the sticker.
     * This is in the given coordinate system.
     */
    public native void setUpperRight(double x,double y);

    /**
     * Set the rotation in radians.  0 by default.
     */
    public native void setRotation(double rot);

    /**
     * Set the coordinate system.  By default this is just geographic.
     */
    public native void setCoordSys(CoordSystem coordSys);

    /**
     * Set the destination image format for images passed in.
     * The images will be converted to this format.
     */
    public void setImageFormat(QuadImageTileLayer.ImageFormat imageFormat)
    {
        setImageFormatNative(imageFormat.ordinal());
    }

    native void setImageFormatNative(int imageFormatEnum);

    /**
     * Images to display on the Sticker.  This may include an image or a blending of
     * multiple, depending on the shader.
     */
    public void setImages(ArrayList<MaplyTexture> textures)
    {
        long[] texIDs = new long[textures.size()];
        int which = 0;
        for (MaplyTexture tex : textures)
        {
            texIDs[which] = tex.texID;
            which++;
        }

        setImagesNative(texIDs);
    }

    native void setImagesNative(long[] texIDs);

    static
    {
        nativeInit();
    }
    private static native void nativeInit();
    native void initialise();
    native void dispose();
    private long nativeHandle;
}
