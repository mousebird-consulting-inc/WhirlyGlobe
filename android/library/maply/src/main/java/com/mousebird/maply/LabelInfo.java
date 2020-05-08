/*
 *  LabelInfo.java
 *  WhirlyGlobeLib
 *
 *  Created by Steve Gifford on 6/2/14.
 *  Copyright 2011-2014 mousebird consulting
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

import android.graphics.Color;
import android.graphics.Paint;
import android.graphics.Typeface;

import static android.R.attr.textColor;
import static android.R.attr.typeface;

/**
 * We use this class to designate attribute shared by a group of labels.
 * Rather than specifying them individually they all go here.  You would
 * use this class in the course of an addLabels() or addScreenLabels()
 * call on the MaplyController.
 *
 * @author sjg
 */
public class LabelInfo extends BaseInfo
{
	/**
	 * Default priority for labels.  Screen labels add a big offset to this.
	 */
	public static int LabelPriorityDefault = 60000;

	public LabelInfo()
	{
		initialise();
		setTextColor(1.f,1.f,1.f,1.f);
		setBackgroundColor(0.f,0.f,0.f,0.f);
		setTypeface(Typeface.DEFAULT);
		setFontSize(24.f);
		setTextJustify(TextJustify.TextLeft);

		setDrawPriority(LabelPriorityDefault);
	}

	public void finalize()
	{
		dispose();
	}

	/**
	 * Set the text color from a standard Android Color value.
	 * @param color Color value, including alpha.
     */
	public void setTextColor(int color)
	{
		setTextColor(Color.red(color)/255.f,Color.green(color)/255.f,Color.blue(color)/255.f,Color.alpha(color)/255.f);
	}

	/**
	 * Return the text color in a form suitable for Android.
	 */
	public native int getTextColor();

	/**
	 * Set the text color as float values from 0.0 to 1.0
	 *
	 * @param r red
	 * @param g green
	 * @param b blue
	 * @param a alpha
	 */
	public native void setTextColor(float r,float g,float b,float a);

	/**
	 * Set the background color from a standard Android Color value.
	 * @param color Color value, including alpha.
	 */
	public void setBackgroundColor(int color)
	{
		setBackgroundColor(Color.red(color)/255.f,Color.green(color)/255.f,Color.blue(color)/255.f,Color.alpha(color)/255.f);
	}

	/**
	 * Set the background color of text.  This is what appears in the rectangles behind the text.
	 * Color components range from 0.0 to 1.0.
	 *
	 * @param r red
	 * @param g green
	 * @param b blue
	 * @param a alpha
	 */
	public native void setBackgroundColor(float r,float g,float b,float a);

	/**
	 * Return the background text color in a form suitable for Android.
	 */
	public native int getBackgroundColor();

	/**
	 * Set the typeface used in the text.
	 */
	public void setTypeface(Typeface typeface)
	{
		setTypefaceNative(typeface);

		updateLineHeight();
	}

	native void setTypefaceNative(Typeface typeface);

	/// Optional font name used for tracking and nothing else
	public String fontName;

	/**
	 * Return the typeface used for the labels.
	 */
	public native Typeface getTypeface();

	float fontSize = 0.f;
	/**
	 * Set the font size for the text.  For screen labels this controls the geometry size as well.
	 */
	public void setFontSize(float size)
	{
		fontSize = size;
		setFontSizeNative(size);

		updateLineHeight();
	}

	native void setFontSizeNative(float size);

	public float getFontSize() {
		return fontSize;
	}

	/**
	 * Set the color of outline.
	 * Color components range from 0.0 to 1.0.
	 *
	 * @param r red
	 * @param g green
	 * @param b blue
	 * @param a alpha
	 */
	public native void setOutlineColor(float r,float g,float b,float a);

	/**
	 * Return the outline color in a form suitable for Android.
	 */
	public native int getOutlineColor();

	/**
	 * Set the color of the outline.
     */
	public void setOutlineColor(int color)
	{
		setOutlineColor(Color.red(color)/255.f,Color.green(color)/255.f,Color.blue(color)/255.f,Color.alpha(color)/255.f);
	}

	/**
	 * Set the outline size for the text.
	 */
	public native void setOutlineSize(float size);

	/**
	 * Return the outline size
	 */
	public native float getOutlineSize();

	/**
	 * Set the color of shadow.
	 * Color components range from 0.0 to 1.0.
	 *
	 * @param r red
	 * @param g green
	 * @param b blue
	 * @param a alpha
	 */
	public native void setShadowColor(float r,float g,float b,float a);

	/**
	 * Set the color of the shadow.
	 */
	public void setShadowColor(int color)
	{
		setShadowColor(Color.red(color)/255.f,Color.green(color)/255.f,Color.blue(color)/255.f,Color.alpha(color)/255.f);
	}

	/**
	 * Set the shadow size for the text.
	 */
	public native void setShadowSize(float size);

	// Importance value for the layout engine
	public float layoutImportance = Float.MAX_VALUE;

	public static int LayoutNone = 1<<0;
	public static int LayoutCenter = 1<<1;
	public static int LayoutRight = 1<<2;
	public static int LayoutLeft = 1<<3;
	public static int LayoutAbove = 1<<4;
	public static int LayoutBelow = 1<<5;

	/**
	 * The layout placement controls where we can put the label relative to
	 * its point.
	 */
	public void setLayoutPlacement(int newPlacement)
	{
		layoutPlacement = newPlacement;
	}
	public int layoutPlacement = -1;

	/**
	 * The layout engine controls how text is displayed.  It tries to avoid overlaps
	 * and takes priority into account.  The layout importance controls which labels
	 * (or other features) are laid out first.  Bigger is more important.
	 * <p>
	 * Defaults to MAXFLOAT, which is off.  That means the layout engine does not control
	 * the associated labels.
	 */
	public void setLayoutImportance(float newImport)
	{
		layoutImportance = newImport;
	}

	/**
	 * Justification values for text.  Can be center, left, or right.
	 */
	public enum TextJustify {TextCenter,TextLeft,TextRight};

	/**
	 * For multi-line labels, you can justify the text to the left, right or center.
	 * Only works for multi-line labels.
	 */
	public void setTextJustify(TextJustify textJustify)
	{
		setTextJustifyNative(textJustify.ordinal());
	}
	native void setTextJustifyNative(int textJustify);

	// Update C++ side for values it needs
	void updateLineHeight()
	{
		if (fontSize == 0.0 || getTypeface() == null)
			return;

		Paint paint = new Paint();
		paint.setTextSize(fontSize);
		paint.setTypeface(getTypeface());
		Paint.FontMetrics fm = paint.getFontMetrics();
		float fontHeight = (float)Math.ceil( Math.abs( fm.bottom ) + Math.abs( fm.top ) );
		setLineHeight(fontHeight);
	}

	/**
	 * Rather than calculate it from the Typeface, set the line height directly.
	 */
	native void setLineHeight(float fontHeight);


	static
	{
		nativeInit();
	}
	private static native void nativeInit();
	native void initialise();
	native void dispose();
}
