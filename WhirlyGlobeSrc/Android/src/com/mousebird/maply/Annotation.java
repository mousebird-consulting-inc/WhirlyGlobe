/*
 *  Annotation.java
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
import android.graphics.Bitmap;
import android.graphics.Color;
import android.view.Gravity;
import android.view.ViewGroup;
import android.widget.FrameLayout;
import android.widget.TextView;

import java.util.ArrayList;


public class Annotation
{
    private String title;
    private String subTitle;
    private Bitmap leftAccessoryView;
    private Bitmap rightAccessoryView;
    private Bitmap titleView;
    private Bitmap subtitleView;
    private Bitmap contentView;
    private Point2d loc;
    private Context context;
    private int width;
    private int height;
    private FrameLayout annView = null;
    private long id = Identifiable.genID();
    private boolean visible = false;

    public enum AnnoType {
        Custom, 
        TitleText, 
        TitleAndSubTitleText,
        None
    };

    public Annotation(Context context, int width, int height) {
        this.context = context;
        this.loc = new Point2d();
        this.width = width;
        this.height = height;
    }

    public boolean isVisible() {
        return visible;
    }

    public void setVisible(boolean visible) {
        this.visible = visible;
    }

    public String getTitle() {
        return title;
    }

    public void setTitle(String title) {
        this.title = title;
    }

    public String getSubTitle() {
        return subTitle;
    }

    public void setSubTitle(String subTitle) {
        this.subTitle = subTitle;
    }

    public Bitmap getLeftAccessoryView() {
        return leftAccessoryView;
    }

    public void setLeftAccessoryView(Bitmap leftAccessoryView) {
        this.leftAccessoryView = leftAccessoryView;
    }

    public Bitmap getRightAccessoryView() {
        return rightAccessoryView;
    }

    public void setRightAccessoryView(Bitmap rightAccessoryView) {
        this.rightAccessoryView = rightAccessoryView;
    }

    public Bitmap getTitleView() {
        return titleView;
    }

    public void setTitleView(Bitmap titleView) {
        this.titleView = titleView;
    }

    public Bitmap getSubtitleView() {
        return subtitleView;
    }

    public void setSubtitleView(Bitmap subtitleView) {
        this.subtitleView = subtitleView;
    }

    public Bitmap getContentView() {
        return contentView;
    }

    public void setContentView(Bitmap contentView) {
        this.contentView = contentView;
    }

    public Point2d getLoc() {
        return loc;
    }

    public void setLoc(Point2d loc) {
        this.loc = loc;
    }

    public void generateLayout(AnnoType type, FrameLayout customLayout) {
        this.annView = new FrameLayout(this.context);
        this.annView.setBackgroundColor(Color.WHITE);

        switch (type) {
            case None:
                break;

            case Custom:
                if (customLayout != null) {
                    this.annView = customLayout;
                }
                break;

            case TitleAndSubTitleText:
                addTitleText();
                addSubTitleText();
                break;

            case TitleText:
                addTitleText();
                break;
        }
    }

    public FrameLayout getLayout() {
        if (this.annView == null) {
            return null;
        }
        return this.annView;
    }

    private void addTitleText() {
        if (title != null) {
            TextView titleTextView = new TextView(this.context);
            titleTextView.setText(this.title);
            titleTextView.setTextSize(5);
            titleTextView.setLayoutParams(new FrameLayout.LayoutParams(FrameLayout.LayoutParams.WRAP_CONTENT, FrameLayout.LayoutParams.WRAP_CONTENT, Gravity.CENTER_HORIZONTAL));
            annView.addView(titleTextView);
        }
    }

    private void addSubTitleText() {
        if (subTitle != null) {
            TextView subtitleTextView = new TextView(this.context);
            subtitleTextView.setText(this.subTitle);
            subtitleTextView.setTextSize(5);
            subtitleTextView.setLayoutParams(new FrameLayout.LayoutParams(FrameLayout.LayoutParams.WRAP_CONTENT, FrameLayout.LayoutParams.WRAP_CONTENT, Gravity.CENTER_HORIZONTAL+Gravity.BOTTOM));
            annView.addView(subtitleTextView);
        }
    }

    @Override
    public boolean equals(Object obj) {
        if (obj == null) {
            return false;
        }
        if (!(obj instanceof Annotation)) {
            return false;
        }

        Point2d that = ((Annotation) obj).getLoc();

        return this.loc.getX() == that.getX() && this.loc.getY() == that.getY();
    }
}
