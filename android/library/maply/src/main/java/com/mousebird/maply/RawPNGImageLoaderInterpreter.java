package com.mousebird.maply;

/**
 * This loader interpreter treats input image data objects as PNGs containing raw data.
 *  The difference is we'll use a direct PNG reader to tease it out, rather than Bitmap.
 */
public class RawPNGImageLoaderInterpreter implements LoaderInterpreter {

    /**
     * Create one.
     */
    public RawPNGImageLoaderInterpreter() { initialise(); }

    public void setLoader(QuadLoaderBase loader) {
    }

    /**
     * Pull the data out of the input image as raw PNG.
     */
    public void dataForTile(LoaderReturn loadReturn,QuadLoaderBase loader) {
        byte[][] images = loadReturn.getTileData();
        for (byte[] image : images) {
            if (loadReturn.isCanceled()) {
                return;
            }
            dataForTileNative(image, loadReturn);
        }
    }

    /**
     * In some cases we just want to pick values out of the input.
     */
    public native void addMappingFrom(int fromVal,int toVal);

    native void dataForTileNative(byte[] image,LoaderReturn loaderReturn);

    static
    {
        nativeInit();
    }
    private static native void nativeInit();
    native void initialise();
    native void dispose();
    private long nativeHandle;
}
