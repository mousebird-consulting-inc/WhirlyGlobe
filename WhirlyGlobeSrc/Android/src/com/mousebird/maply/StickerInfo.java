package com.mousebird.maply;

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
        public StickerInfo() {
                initialise();
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
