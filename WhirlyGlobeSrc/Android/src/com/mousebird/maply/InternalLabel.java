/*
 *  InternalLabel.java
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

/**
 * An internal representation for the labels.  Toolkit users use ScreenLabel instead of this.
 * 
 */
class InternalLabel 
{
	InternalLabel()
	{
		initialise();
	}
	
	/**
	 * Construct from a ScreenLabel and LabelInfo.  Translates to internal values suitable
	 * for the rendering engine.
	 * 
	 * @param label Screen label we're going to represent.
	 * @param info Label info to describe how it looks.
	 */
	InternalLabel(ScreenLabel label,LabelInfo info)
	{
		initialise();
		setLoc(label.loc);
		setRotation(label.rotation);
		if (label.text != null && !label.text.isEmpty()) {
			// Convert text over to code points
			int len = label.text.length();
			int[] codePoints = new int[len];
			int which = 0;
			for (int offset = 0; offset < len; )
			{
				int codePoint = label.text.codePointAt(offset);
				codePoints[which++] = codePoint;
				offset += Character.charCount(codePoint);
			}
			setText(codePoints,which);
		}
		if (label.offset != null)
			setOffset(label.offset);
		setSelectable(label.selectable);
	}
	
	public native void setLoc(Point2d loc);
	public native void setRotation(double rotation);
	public native void setText(int[] codePoints,int len);
//	public native void iconImage(Bitmap image);
//	public native void setIconSize(Point2d iconSize);
	public native void setOffset(Point2d offset);
	// Note: Color
	public native void setSelectable(boolean selectable);
	// Note: Layout placement
	
	static
	{
		nativeInit();
	}
	private static native void nativeInit();
	native void initialise();
	native void dispose();
	private long nativeHandle;	
}
