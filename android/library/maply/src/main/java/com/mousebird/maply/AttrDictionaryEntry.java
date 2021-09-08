/*  AttrDictionaryEntry.java
 *  WhirlyGlobeLib
 *
 *  Created by Steve Gifford
 *  Copyright 2011-2021 mousebird consulting
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
 */

package com.mousebird.maply;

import org.jetbrains.annotations.Nullable;

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
    public AttrDictionaryEntry() {
        initialise();
    }

    /**
     * Return the type of this entry.
     */
    public Type getType() {
        return Type.values()[getTypeNative()];
    }
    protected native int getTypeNative();

    /**
     * Return a string, if this a string
     */
    public native @Nullable String getString();

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
    public native @Nullable AttrDictionary getDict();

    /**
     * Return an array of Entry objects, if this is compatible.
     */
    public native @Nullable AttrDictionaryEntry[] getArray();

    static {
        nativeInit();
    }
    private static native void nativeInit();
    public void finalize() {
        dispose();
    }
    native void initialise();
    native void dispose();

    @SuppressWarnings("unused") // Used by JNI
    private long nativeHandle;
}
