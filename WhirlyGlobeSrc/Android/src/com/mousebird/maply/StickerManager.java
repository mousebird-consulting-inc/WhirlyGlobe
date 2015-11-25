package com.mousebird.maply;

/**
 * The sticker managers interfaces to the Maply C++/JNI side of things
 * and is invisible to the toolkit users.
 */
public class StickerManager
{
    private StickerManager()
    {}

    StickerManager(Scene scene) { initialise(scene); }

    public void finalize() { dispose(); }

    // Add a single sticker to the scene and return an ID for tracker
    public native long addSticker(Sticker sticker,StickerInfo stickerInfo,ChangeSet changes);

    // Modify the given chunk with new texture IDs
    public native boolean modifyChunkTextures(long id,long texIDs[],ChangeSet changes);

    // Enable/disable stickers by ID
    public native void enableStickers(long ids[],boolean enable,ChangeSet changes);

    // Remove stickers by ID
    public native void removeStickers(long ids[],ChangeSet changes);

    static
    {
        nativeInit();
    }
    private static native void nativeInit();
    native void initialise(Scene scene);
    native void dispose();
    private long nativeHandle;
    private long nativeSceneHandle;
}
