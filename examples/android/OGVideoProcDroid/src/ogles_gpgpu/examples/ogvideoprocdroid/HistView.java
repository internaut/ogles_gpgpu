package ogles_gpgpu.examples.ogvideoprocdroid;

import java.util.Arrays;

import android.content.Context;
import android.graphics.Canvas;
import android.graphics.Paint;
import android.util.AttributeSet;
import android.util.Log;
import android.view.View;

public class HistView extends View {
	private final String TAG = this.getClass().getSimpleName();
	
	private float[] hist;
	
	private Paint barPaint = new Paint();
	
	public HistView(Context context, AttributeSet attrs) {
		super(context, attrs);
	}

	
	@Override
	protected void onMeasure(int widthMeasureSpec, int heightMeasureSpec) {
		super.onMeasure(widthMeasureSpec, heightMeasureSpec);
		
		setMeasuredDimension(widthMeasureSpec, heightMeasureSpec);
		
		barPaint.setARGB(255, 255, 0, 0);
	}

	@Override
	protected void onDraw(Canvas canvas) {
		canvas.drawARGB(128, 0, 0, 0);
		
		if (hist == null) return;
		
		float w = canvas.getWidth();
		float h = canvas.getHeight();
		
		int barDist = (int)(w / 256.0f);
		int barW = barDist / 2;
		int x = 0;
		
		for (float v : hist) {
			float barH = v * h;

			canvas.drawRect(x, h - barH, x + barW, h, barPaint);
			
			x += barDist;
		}
		
		super.onDraw(canvas);
	}
	
	public void getHistCopy(float[] arr) {
		this.hist = Arrays.copyOf(arr, arr.length);
	}

	public void setHist(float[] hist) {
		this.hist = hist;
	}
}
