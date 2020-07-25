package com.mousebird.maply;

/**
 * A single entry in an AttrDictionary that needs to be handled generically.
 */
public class AttrDictionaryEntry {

    // Mirrors the types in the C++ side of the Dictionary
    public enum Type {DictTypeNone,
        DictTypeString,
        DictTypeInt,
        DictTypeIdentity,
        DictTypeDouble,
        DictTypeObject,
        DictTypeDictionary,
        DictTypeArray};

    /**
     * Construct an empty attribution dictionary
     */
    public AttrDictionaryEntry()
    {
        initialise();
    }

    /**
     * Return the type of this entry.
     */
    public Type getType()
    {
        return Type.values()[getTypeNative()];
    }
    protected native int getTypeNative();

    /**
     * Return a string, if this a string
     */
    public native String getString();

    /**
     * Return an integer, if compatible.
     */
    public native int getInt();

    /**
     * Return a double, if compatible.
     */
    public native double getDouble();

    /**
     * Return a identity, which is a long64, if compatible.
     */
    public native long getIdentity();

    /**
     * Return a dictionary, if this is a dictionary.
     */
    public native AttrDictionary getDict();

    /**
     * Return an array of Entry objects, if this is compatible.
     */
    public native AttrDictionaryEntry[] getArray();

    static
    {
        nativeInit();
    }
    private static native void nativeInit();
    public void finalize()
    {
        dispose();
    }
    native void initialise();
    native void dispose();
    private long nativeHandle;
}
