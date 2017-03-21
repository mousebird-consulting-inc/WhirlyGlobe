package com.mousebird.maply;

import android.graphics.Color;

import java.util.ArrayList;

/**
 * This class holds the visual information for a set of stickers.
 * Rather than have each of those represent their own visual information,
 * we share it here.
 * <p>
 * Toolkit users fill this class out and pass it into the addStickers().
 *
 * @author sjg
 *
 */
public class StickerInfo extends BaseInfo
{
    /**
     * Default draw priority for stickers.
     */
    static int StickerPriorityDefault = 30000;

    public StickerInfo() {
        initialise();
        setColor(1.f,1.f,1.f,1.f);
        setDrawPriority(StickerPriorityDefault);
    }

    /**
     * Set the color used by the geometry.
     * @param color Color, including alpha.
     */
    public void setColor(int color)
    {
        setColor(Color.red(color)/255.f,Color.green(color)/255.f,Color.blue(color)/255.f,Color.alpha(color)/255.f);
    }

    /**
     * Set the color used by the geometry.  Color values range from 0 to 1.0.
     * You must specify all four values.  Alpha controls transparency.
     * @param r Red component.
     * @param g Green component.
     * @param b Blue component.
     * @param a Alpha component.
     */
    public native void setColor(float r,float g,float b,float a);

    /**
     * Images to display on the Sticker.  This may include an image or a blending of
     * multiple, depending on the shader.
     * <br>
     * Note: This is only used for changeSticker() calls.
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

    /**
     * Set the scene name of the shader to use for the sticker objects.
     */
    public void setShaderName(MaplyBaseController control,String name)
    {
        setShaderName(control.getScene(),name);
    }

    native void setShaderName(Scene scene,String name);

    public void finalize() {
            dispose();
    }

    static {
        nativeInit();
    }
    private static native void nativeInit();
    native void initialise();
    native void dispose();
}
