package com.mousebird.maply.sld.sldstyleset;

import android.content.res.AssetManager;

import java.io.File;
import java.io.InputStream;
import java.util.ArrayList;

/**
 * Created by sjg on 4/5/18.
 * This wraps the asset manager and lets us insert new directories for use as
 * asset locations.
 */

public class AssetWrapper {
    AssetManager manager;

    public AssetWrapper(AssetManager manager) {
        this.manager = manager;
    }

    ArrayList<String> paths = new ArrayList<String>();

    // Add a path to look for files in addition to the asset manager
    // These will be tried first
    public void addPath(String pathName) {
        paths.add(pathName);
    }

    // Try to find the file with a few variants and open it
    public InputStream open(String fileName) throws java.io.IOException {
        String fullPath = "";

        // Look for it in one of the paths first
        for (String path : paths) {
            fullPath = path + "/" + fileName;
            File fullFile = new File(fullPath);
            if (fullFile.exists()) {
                return manager.open(fullPath);
            }
            File baseName = new File(fileName);
            String fullPath2 = path + "/" + baseName.getName();
            File fullFile2 = new File(fullPath2);
            if (fullFile2.exists()) {
                return manager.open(fullPath2);
            }
        }

        try {
            InputStream iStream = manager.open(fullPath);
            return iStream;
        }
        catch (Exception e) {
            File baseName = new File(fileName);
            return manager.open(baseName.getName());
        }
    }
}
