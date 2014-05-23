package com.mousebird.maply;

import android.graphics.Bitmap;
import android.graphics.Canvas;
import android.graphics.Paint;

/**
 * Convenience object used to render a single character for the 
 * text engine.
 * 
 * @author sjg
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
//		Log.d("Maply","Rendering char: " + charInt);
		
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
		
		return glyph;
	}
}
