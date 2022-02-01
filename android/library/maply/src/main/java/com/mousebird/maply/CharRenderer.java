/*  CharRenderer.java
 *  WhirlyGlobeLib
 *
 *  Created by Steve Gifford on 6/2/14.
 *  Copyright 2011-2022 mousebird consulting
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

import android.graphics.Bitmap;
import android.graphics.Canvas;
import android.graphics.Color;
import android.graphics.Paint;

import static android.graphics.Paint.*;

/**
 * Convenience object used to render a single character for the
 * text engine.  You should not ever be using this.
 */
class CharRenderer
{
	static int fontPadX = 2,fontPadY = 2;

	// Encapsulate the glyph data we need to return
	public static class Glyph
	{
		public Bitmap bitmap = null;
		// texture size (not currently used)
		public float sizeX,sizeY;
		// size of the glyph within the texture, excluding offsets
		public float glyphSizeX,glyphSizeY;
		public float offsetX,offsetY;
		public float textureOffsetX,textureOffsetY;
	}

	CharRenderer()
	{
	}

	// Must match `BogusFontScale` in FontTextureManager_Android.cpp
	// Setting this to 2 or more makes text, particularly text with outlines, look worse.
	// Possibly with min filtering better than bilinear it would be an improvement.
	private static final float FontSizeScale = 1.0f;

	@SuppressWarnings("unused")	// called from C++ code
	Glyph renderChar(int charInt,LabelInfo labelInfo,float fontSize)
	{
		if (labelInfo == null) {
			return null;
		}

		final char[] chars = Character.toChars(charInt);
		final int textColor = labelInfo.getTextColor();

		final int textPaintFlags =
			Paint.ANTI_ALIAS_FLAG |
			Paint.FAKE_BOLD_TEXT_FLAG | // allow synthetic bolding
			Paint.LINEAR_TEXT_FLAG |    // enable smooth linear scaling of text
			Paint.SUBPIXEL_TEXT_FLAG;   // glyph advances computed with subpixel accuracy

		Paint textFillPaint = new Paint(textPaintFlags);
		textFillPaint.setTypeface(labelInfo.getTypeface());
		textFillPaint.setTextSize(fontSize * FontSizeScale);
		textFillPaint.setColor(textColor);
		textFillPaint.setAntiAlias(true);
		textFillPaint.setHinting(HINTING_ON);      // enable freetype's auto-hinter
		textFillPaint.setElegantTextHeight(true);  // elegant must be good, right?
		textFillPaint.setSubpixelText(true);       // enable subpixel glyph rendering
		//textFillPaint.setFontFeatureSettings("\"dlig\" 1");	// enable discretionary ligatures

		final Paint.FontMetrics fm = textFillPaint.getFontMetrics();
		final float fontHeight = (float)Math.ceil( Math.abs( fm.bottom ) + Math.abs( fm.top ) );
		//final float fontAscent = (float)Math.ceil( Math.abs( fm.ascent ) );
		final float fontDescent = (float)Math.ceil( Math.abs( fm.descent ) );

		final float[] widths = new float[2];
		textFillPaint.getTextWidths(chars, 0, 1, widths);

		final float width = widths[0] + fontPadX * 2.0f;
		final float height = fontHeight + fontPadY * 2.0f;
		final float outlineSize = labelInfo.getOutlineSize() * FontSizeScale;

		final Bitmap bitmap = Bitmap.createBitmap(
				(int)Math.ceil(width + 2.0f * outlineSize),
				(int)Math.ceil(height + 2.0f * outlineSize),
				Bitmap.Config.ARGB_8888);
		bitmap.eraseColor(Color.TRANSPARENT);

		final Canvas canvas = new Canvas(bitmap);

		//draw char outline
		//paint for outline
		if (outlineSize > 0) {
			final Paint textOutlinePaint = new Paint(textFillPaint);
			textOutlinePaint.setStyle(Paint.Style.STROKE);
			textOutlinePaint.setStrokeWidth(2.0f * outlineSize);
			textOutlinePaint.setColor(labelInfo.getOutlineColor());
			textOutlinePaint.setTypeface(textFillPaint.getTypeface());
			canvas.drawText(chars, 0, 1,
					fontPadX + outlineSize,
					height - fontDescent - fontPadY + outlineSize,
					textOutlinePaint);
		}

		//draw char fill
		canvas.drawText(chars, 0, 1,
				fontPadX + outlineSize,
				height - fontDescent - fontPadY + outlineSize,
				textFillPaint);

		// Send back some useful info
		final Glyph glyph = new Glyph();
		glyph.bitmap = bitmap;
		glyph.sizeX = width;
		glyph.sizeY = height;
		glyph.textureOffsetX = fontPadX + outlineSize;
		glyph.textureOffsetY = fontPadY + outlineSize;
		glyph.offsetX = 0;
		glyph.offsetY = 1;
		glyph.glyphSizeX = widths[0];
		glyph.glyphSizeY = fontHeight;

//		Log.d("Maply","Rendering char: " + charInt + " sizeX = " + width + " sizeY = " + height);

		return glyph;
	}
}
