/*
 *  AnnotationManager.java
 *  WhirlyGlobeLib
 *
 *  Created by jmnavarro
 *  Copyright 2011-2016 mousebird consulting
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
 *
 */
package com.mousebird.maply;

import android.content.Context;

import java.util.ArrayList;
import java.util.Map;


public class AnnotationManager implements ActiveObject
{
    private Point3d oldCameraPos = new Point3d();
    private MaplyBaseController viewC;
    private Context context;
    private boolean changed = false;
    private boolean started = false;
    private ArrayList<Annotation> annotations = new ArrayList<>();

    public AnnotationManager(Context context, MaplyBaseController viewC) {
        this.viewC = viewC;
        this.context = context;
    }

    public boolean addAnnotation(Annotation annotation) {
        if (annotations.contains(annotation)) {
            return false;
        }
        annotations.add(annotation);
        changed = true;
        return true;
    }

    public boolean removeAnnotation(Annotation annotation) {
        if (this.annotations.remove(annotation)) {
            this.changed = true;
            return true;
        }
        return false;
    }

    public void modifyAnnotation (Annotation annotation) {
        if (this.annotations.contains(annotation)) {
            this.annotations.set(this.annotations.indexOf(annotation), annotation);
        } else {
            this.annotations.add(annotation);
        }
        changed = true;
    }

    public void setVisible (boolean visible, Annotation annotation) {
        int index = this.annotations.indexOf(annotation);
        if (index != -1) {
            this.annotations.get(index).setVisible(visible);
            changed = true;
        }
    }

    @Override
    public void activeUpdate() {
        if (!hasChanges()) {
            return;
        }
        for (Annotation that : this.annotations) {
            if (that.isVisible()) {
                if (viewC instanceof MapController) {
                    ((MapController)viewC).showAnnotation(that);
                } else if (viewC instanceof GlobeController) {
                    // TODO globe
                    // ((GlobeController)viewC).showAnnotation(that);
                }
            }
        }
        changed = false;
    }

    @Override
    public boolean hasChanges() {
        Point3d cameraPos = new Point3d();
        if (viewC instanceof GlobeController) {
            GlobeController gb = (GlobeController) viewC;
            cameraPos = gb.getGlobeView().getEyePosition();
        } else if (viewC instanceof MapController) {
            MapController mp = (MapController) viewC;
            cameraPos = mp.getPositionGeo();
        }

        if (!changed && started && cameraPos.equals(oldCameraPos)) {
            return false;
        }
        return true;
    }
}
