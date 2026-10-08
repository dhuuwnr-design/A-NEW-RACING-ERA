package com.apexenginenext;

import android.app.Activity;
import android.content.pm.ActivityInfo;
import android.os.Bundle;
import android.view.MotionEvent;
import android.view.SurfaceHolder;
import android.view.SurfaceView;
import android.view.View;
import android.widget.FrameLayout;
import android.graphics.Canvas;
import android.graphics.Color;
import android.graphics.Paint;
import android.graphics.Typeface;

public final class MainActivity extends Activity implements SurfaceHolder.Callback {
    private GameView surface;
    private HudView hud;
    static { System.loadLibrary("apex"); }

    @Override public void onCreate(Bundle state) {
        super.onCreate(state);
        setRequestedOrientation(ActivityInfo.SCREEN_ORIENTATION_LANDSCAPE);
        getWindow().setNavigationBarColor(Color.BLACK);
        FrameLayout root=new FrameLayout(this);
        surface=new GameView();
        surface.getHolder().addCallback(this);
        hud=new HudView();
        root.addView(surface,new FrameLayout.LayoutParams(-1,-1));
        root.addView(hud,new FrameLayout.LayoutParams(-1,-1));
        setContentView(root);
    }

    @Override public void surfaceCreated(SurfaceHolder holder) {
        nativeStart(holder.getSurface(),getAssets());
        surface.running=true;
        surface.post(surface.frame);
        hud.post(hud.tick);
    }
    @Override public void surfaceDestroyed(SurfaceHolder holder) {
        surface.running=false;
        nativeStop();
    }
    @Override public void surfaceChanged(SurfaceHolder holder,int format,int w,int h){nativeResize(w,h);}

    private static native void nativeStart(android.view.Surface s,android.content.res.AssetManager a);
    private static native void nativeStop();
    private static native void nativeResize(int w,int h);
    private static native void nativeFrame(float dt);
    private static native void nativeToggleCamera();
    private static native String nativeTrackName();
    private static native void nativeTouch(float steer,float throttle,float brake);
    private static native float nativeSpeed();
    private static native float nativeFps();
    private static native int nativeLap();
    private static native int nativePosition();
    private static native boolean nativeFinished();

    private final class GameView extends SurfaceView {
        boolean running;
        final Runnable frame=new Runnable(){
            long last=System.nanoTime();
            public void run(){
                if(!running)return;
                long now=System.nanoTime();
                float dt=Math.min(.05f,(now-last)*1e-9f);
                last=now;
                nativeFrame(dt);
                postOnAnimation(this);
            }
        };
        GameView(){super(MainActivity.this);setFocusable(true);}

        private boolean hit(float x,float y,float l,float t,float r,float b){
            return x>=l&&x<=r&&y>=t&&y<=b;
        }
        @Override public boolean onTouchEvent(MotionEvent e){
            int action=e.getActionMasked();
            if(action==MotionEvent.ACTION_UP||action==MotionEvent.ACTION_CANCEL){
                nativeTouch(0,0,0);
                return true;
            }
            float w=getWidth(),h=getHeight(),steer=0,throttle=0,brake=0;
            float top=h*.70f, bw=Math.min(w*.13f,h*.20f);
            float left=w*.035f, l2=left+bw*.95f;
            for(int i=0;i<e.getPointerCount();i++){
                float x=e.getX(i),y=e.getY(i);
                if(hit(x,y,left,top,left+bw,h*.94f))steer=-1;
                if(hit(x,y,l2,top,l2+bw,h*.94f))steer=1;
                if(hit(x,y,w*.78f,h*.57f,w*.965f,h*.75f))throttle=1;
                if(hit(x,y,w*.78f,h*.77f,w*.965f,h*.95f))brake=1;
            }
            nativeTouch(steer,throttle,brake);
            return true;
        }
    }

    private final class HudView extends View {
        final Paint p=new Paint(3);
        final Runnable tick=new Runnable(){public void run(){invalidate();postDelayed(this,100);}};
        HudView(){super(MainActivity.this);setLayerType(View.LAYER_TYPE_SOFTWARE,null);}

        private void button(Canvas c,float l,float t,float r,float b,String text){
            p.setStyle(Paint.Style.FILL);p.setColor(0xaa10151c);c.drawRoundRect(l,t,r,b,22,22,p);
            p.setStyle(Paint.Style.STROKE);p.setStrokeWidth(3);p.setColor(0xccffffff);c.drawRoundRect(l,t,r,b,22,22,p);
            p.setStyle(Paint.Style.FILL);p.setColor(Color.WHITE);p.setTypeface(Typeface.DEFAULT_BOLD);p.setTextSize(text.length()>2?18:30);
            float tw=p.measureText(text);c.drawText(text,(l+r-tw)/2,(t+b)/2+9,p);
        }
        @Override protected void onDraw(Canvas c){
            super.onDraw(c);
            float w=getWidth(),h=getHeight();
            p.setStyle(Paint.Style.FILL);p.setTypeface(Typeface.DEFAULT_BOLD);p.setColor(Color.WHITE);
            p.setTextSize(30);c.drawText("P"+nativePosition(),24,38,p);
            p.setTextSize(16);c.drawText(nativeTrackName(),24,61,p);
            p.setTextSize(22);c.drawText(String.format(java.util.Locale.US,"%.0f KM/H",nativeSpeed()),w-145,38,p);
            p.setTextSize(15);c.drawText("LAP "+nativeLap()+"/5",w-95,61,p);

            float top=h*.70f,bw=Math.min(w*.13f,h*.20f),left=w*.035f,l2=left+bw*.95f;
            button(c,left,top,left+bw,h*.94f,"<");
            button(c,l2,top,l2+bw,h*.94f,">");
            button(c,w*.78f,h*.57f,w*.965f,h*.75f,"THR");
            button(c,w*.78f,h*.77f,w*.965f,h*.95f,"BRK");

            p.setColor(0xaa10151c);c.drawRoundRect(w-118,16,w-20,66,14,14,p);
            p.setColor(Color.WHITE);p.setTextSize(14);c.drawText("CAM",w-87,47,p);
            p.setTextSize(13);c.drawText(String.format(java.util.Locale.US,"FPS %.0f",nativeFps()),24,82,p);
            if(nativeFinished()){p.setColor(0xffffd43b);p.setTextSize(30);c.drawText("FINISH  P"+nativePosition(),w/2-90,52,p);}
        }
        @Override public boolean onTouchEvent(MotionEvent e){
            if(e.getActionMasked()==MotionEvent.ACTION_UP && e.getX()>getWidth()-130 && e.getY()<85){
                nativeToggleCamera();return true;
            }
            return false;
        }
    }
}