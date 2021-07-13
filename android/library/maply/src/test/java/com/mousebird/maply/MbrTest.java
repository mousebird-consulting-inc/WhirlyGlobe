package com.mousebird.maply;

import android.util.Log;

import org.junit.Test;

import java.io.File;

import static org.junit.Assert.*;

/**
 * Test bounding rects
 */
public class MbrTest {
    // todo: figure out how to test things that touch JNI
    /*
    public MbrTest() {
        File f = new File("build/intermediates/merged_native_libs/debug/out/lib/x86/libwhirlyglobemaply.so");
        if (f.isFile()) {
            try {
                System.load(f.getAbsolutePath());
            } catch (Exception e) {
                Log.w("Maply", "Unable to load " + f);
            }
        }
    }
     */

    @Test public void uninitializedNotValid() {
        assertFalse(new Mbr().isValid());
        assertFalse(new Mbr(null, null).isValid());
        assertFalse(new Mbr(new Mbr()).isValid());
    }

    @Test public void uninitializedAreEqual() {
        assertEquals(new Mbr(), new Mbr());
    }

    /*
    @Test public void initializedValid() {
        assertTrue(new Mbr(ll,ur).isValid());
    }
    @Test public void partlyInitializedNotValid() {
        assertFalse(new Mbr(ll, null).isValid());
        assertFalse(new Mbr(null, ur).isValid());
    }

    @Test public void initializedAreEqual() {
        assertEquals(new Mbr(ll,ur), new Mbr(ll,ur));
    }
    @Test public void partlyInitializedAreEqual() {
        assertEquals(new Mbr(ll,null), new Mbr(ll,null));
        assertEquals(new Mbr(null,ur), new Mbr(null,ur));
    }

    @Test
    public void copConstructedIsEqual() {
        final Mbr mbr = new Mbr(ll,ur);
        assertEquals(mbr, new Mbr(mbr));
    }
    @Test
    public void initializedCopiesPoints() {
        final Point2d ll = new Point2d(this.ll);
        final Point2d ur = new Point2d(this.ur);
        final Mbr mbr = new Mbr(ll,ur);
        ll.setValue(5,6);
        ur.setValue(7,8);
        assertEquals(mbr,new Mbr(this.ll,this.ur));
    }
    @Test
    public void copyConstructedCopiesPoints() {
        final Mbr mbr1 = new Mbr(ll,ur);
        final Mbr mbr2 = new Mbr(mbr1);
        mbr1.ll.setValue(5,6);
        mbr1.ur.setValue(7,8);
        assertEquals(mbr2,new Mbr(this.ll,this.ur));
    }

    private final Point2d ll = new Point2d(123.456789,12.3456789);
    private final Point2d ur = new Point2d(23.45678901, 34.56789012);
     */
}