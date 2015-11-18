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
