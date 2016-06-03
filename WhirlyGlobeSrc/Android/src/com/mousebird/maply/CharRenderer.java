/*
 *  CharRenderer.java
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

import android.graphics.Bitmap;
import android.graphics.Canvas;
import android.graphics.Paint;
import android.util.Log;

/**
 * Convenience object used to render a single character for the 
 * text engine.  You should not ever be using this.
 * 
 */
class CharRenderer 
{
	static int fontPadX = 2,fontPadY = 2;
	
	// Encapsulate the glyph data we need to return
	public class Glyph 
	{
		public Bitmap bitmap = null;
		public float sizeX,sizeY;
		public float glyphSizeX,glyphSizeY;
		public float offsetX,offsetY;
		public float textureOffsetX,textureOffsetY;
	}
	
	CharRenderer()
	{		
	}
	
	Glyph renderChar(int charInt,LabelInfo labelInfo,float fontSize)
	{
		Paint p = new Paint();
		String str = new String(Character.toChars(charInt));
		p.setTextSize(fontSize);
		int textColor = labelInfo.getTextColor();
		p.setColor(textColor);
		if (labelInfo != null)
			p.setTypeface(labelInfo.getTypeface());
		Paint.FontMetrics fm = p.getFontMetrics();
		float fontHeight = (float)Math.ceil( Math.abs( fm.bottom ) + Math.abs( fm.top ) );
//		float fontAscent = (float)Math.ceil( Math.abs( fm.ascent ) );
		float fontDescent = (float)Math.ceil( Math.abs( fm.descent ) );
		
		float widths[] = new float[2];
		p.getTextWidths(str, widths);
		
		int width = (int) (widths[0] + fontPadX*2);
		int height = (int) (fontHeight + fontPadY*2);
		
		Bitmap bitmap = Bitmap.createBitmap(width, height, Bitmap.Config.ARGB_8888);
		bitmap.eraseColor( 0x00000000 );
		Canvas canvas = new Canvas (bitmap);
		canvas.drawText(str, 0, 1, fontPadX, height - fontDescent - fontPadY, p);
		
		// Send back some useful info
		Glyph glyph = new Glyph();
		glyph.bitmap = bitmap;
		glyph.sizeX = width;  glyph.sizeY = height;
		glyph.textureOffsetX = 1;  glyph.textureOffsetY = 1;
		// Note: Porting. Probably not right
		glyph.offsetX = 0;  glyph.offsetY = 0;
		glyph.glyphSizeX = widths[0];  glyph.glyphSizeY = fontHeight;

//		Log.d("Maply","Rendering char: " + charInt + " sizeX = " + width + " sizeY = " + height);

		return glyph;
	}
}
