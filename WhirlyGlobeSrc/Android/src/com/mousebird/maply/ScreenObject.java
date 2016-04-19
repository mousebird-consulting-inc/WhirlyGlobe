/*
 *  ScreenObject.java
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

import android.graphics.Bitmap;
import android.graphics.Color;
import android.util.Size;

import java.util.ArrayList;


public class ScreenObject {

    public class BoundingBox {
        public Point2d ll = new Point2d();
        public Point2d ur = new Point2d();
    }

    private ArrayList<SimplePoly> polys = new ArrayList<>();
    private ArrayList<StringWrapper> strings = new ArrayList<>();

    public ArrayList<SimplePoly> getPolys() {
        return polys;
    }

    public void setPolys(ArrayList<SimplePoly> polys) {
        this.polys = polys;
    }

    public void addPoly(SimplePoly poly){
        this.polys.add(poly);
    }

    public ArrayList<StringWrapper> getStrings() {
        return strings;
    }

    public void setStrings(ArrayList<StringWrapper> strings) {
        this.strings = strings;
    }

    public void addString(StringWrapper string) {
        this.strings.add(string);
    }

    public void addImage(Bitmap image, float[] color, int width, int height) {
        SimplePoly poly = new SimplePoly();
        poly.setImage(image);
        poly.setColor(color);

        poly.addPt(new Point2d(0,0));
        poly.addTexCoord(new Point2d(0,1));
        poly.addPt(new Point2d(width, 0));
        poly.addTexCoord(new Point2d(1,1));
        poly.addPt(new Point2d(width,height));
        poly.addTexCoord(new Point2d(1,0));
        poly.addPt(new Point2d(0, height));
        poly.addTexCoord(new Point2d(0,0));

        this.polys.add(poly);
    }

    public BoundingBox getSize() {
        Mbr mbr = new Mbr();

        for (SimplePoly poly : polys) {
            for (Point2d pt : poly.getPts()) {
                mbr.addPoint(pt);
            }
        }

        for (StringWrapper str : strings) {
            Point3d p0 = str.getMat().multiply(new Point3d(0,0,1));
            Point3d p1 = str.getMat().multiply(new Point3d(str.getWidth(), str.getHeight(), 1));
            mbr.addPoint(new Point2d(p0.getX(), p0.getY()));
            mbr.addPoint(new Point2d(p1.getX(), p1.getY()));
        }

        BoundingBox boundingBox = new BoundingBox();
        boundingBox.ll = new Point2d(mbr.ll.getX(), mbr.ll.getY());
        boundingBox.ur = new Point2d(mbr.ur.getX(), mbr.ur.getY());

        return boundingBox;
    }

    public void scaleX(double x, double y){

        Affine2d scale = new Affine2d(x, y, Affine2d.TYPE_SCALE);
        Matrix3d mat = scale.matrix();

        for (SimplePoly poly : polys){
            for (int i = 0; i < poly.getPts().size(); i++){
                Point2d pt = poly.getPts().get(i);
                Point3d newPt = mat.multiply(new Point3d(pt.getX(), pt.getY(), 1.0));
                poly.getPts().set(i,new Point2d(newPt.getX(), newPt.getY()) );
            }
        }

        for (StringWrapper str : strings){
            str.setMat(mat.multiply(str.getMat()));
        }
    }

    public void translateX(double x, double y) {
        Affine2d trans = new Affine2d(x,y, Affine2d.TYPE_TRANS);

        for (SimplePoly poly : polys){
            for (int i = 0; i < poly.getPts().size(); i++) {
                Point2d pt = trans.multiply(poly.getPts().get(i));
                poly.getPts().set(i, pt);
            }
        }

        Matrix3d mat = trans.matrix();

        for (StringWrapper str : strings) {
            str.setMat(mat.multiply(str.getMat()));
        }
    }

    public void addScreenObj(ScreenObject screenObject){
        for (int i = 0; i < screenObject.getPolys().size(); i++) {
            this.polys.add(screenObject.getPolys().get(i));
        }
        for (int i = 0; i < screenObject.getStrings().size(); i++) {
            this.strings.add(screenObject.getStrings().get(i));
        }
    }

    public class SimplePoly {

        private Bitmap image;

        private float[] color;
        private ArrayList<Point2d> pts = new ArrayList<>();
        private ArrayList<Point2d> textCoords = new ArrayList<>();

        public Bitmap getImage() {
            return image;
        }

        public void setImage(Bitmap image) {
            this.image = image;
        }

        public float[] getColor() {
            return color;
        }

        public void setColor(float[] color) {
            this.color = color;
        }

        public ArrayList<Point2d> getPts() {
            return pts;
        }

        public void setPts(ArrayList<Point2d> pts) {
            this.pts = pts;
        }

        public void addPt(Point2d pt){
            this.pts.add(pt);
        }

        public ArrayList<Point2d> getTextCoords() {
            return textCoords;
        }

        public void addTexCoord(Point2d pt){
            this.textCoords.add(pt);
        }
    }

    private class StringWrapper {

        private Matrix3d mat;
        private int width;
        private int height;

        public int getWidth() {
            return width;
        }

        public void setWidth(int width) {
            this.width = width;
        }

        public int getHeight() {
            return height;
        }

        public void setHeight(int height) {
            this.height = height;
        }

        public Matrix3d getMat() {
            return mat;
        }

        public void setMat(Matrix3d mat) {
            this.mat = mat;
        }

    }

}
