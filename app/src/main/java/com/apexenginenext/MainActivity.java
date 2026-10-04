package com.apexenginenext;

import android.app.Activity;
import android.graphics.Canvas;
import android.graphics.Paint;
import android.graphics.Path;
import android.graphics.Typeface;
import android.os.Bundle;
import android.view.MotionEvent;
import android.view.SurfaceHolder;
import android.view.SurfaceView;
import android.view.View;
import android.widget.FrameLayout;

public final class MainActivity extends Activity implements SurfaceHolder.Callback {
    private GameView surface; private HudView hud;
    static { System.loadLibrary("apex"); }

    @Override public void onCreate(Bundle state) {
        super.onCreate(state);
        FrameLayout root=new FrameLayout(this);
        surface=new GameView(); surface.getHolder().addCallback(this); surface.setZOrderMediaOverlay(false);
        hud=new HudView();
        root.addView(surface,new FrameLayout.LayoutParams(-1,-1));
        root.addView(hud,new FrameLayout.LayoutParams(-1,-1));
        setContentView(root);
    }
    @Override public void surfaceCreated(SurfaceHolder holder){nativeStart(holder.getSurface(), getAssets());surface.running=true;surface.post(surface.frame);hud.post(hud.tick);}
    @Override public void surfaceDestroyed(SurfaceHolder holder){surface.running=false;nativeStop();}
    @Override public void surfaceChanged(SurfaceHolder holder,int format,int w,int h){nativeResize(w,h);}
    private static native void nativeStart(android.view.Surface surface, android.content.res.AssetManager assets);
    private static native void nativeStop();
    private static native void nativeResize(int width,int height);
    private static native void nativeFrame(float dt);
    private static native void nativeToggleCamera();
    private static native void nativeNextTrack();
    private static native String nativeTrackName();
    private static native void nativeTouch(float steer,float throttle,float brake);
    private static native float nativeSpeed();
    private static native float nativeSteering();
    private static native int nativeLap();
    private static native int nativePosition();
    private static native float nativeRaceTime();
    private static native float nativeGap();
    private static native float nativeInterval();
    private static native String nativeTimingTower();
    private static native boolean nativeFinished();
    private static native int nativeStartState();

    private final class GameView extends SurfaceView {
        boolean running=false;
        final Runnable frame=new Runnable(){long last=System.nanoTime();public void run(){if(!running)return;long now=System.nanoTime();float dt=Math.min(.05f,(now-last)*1e-9f);last=now;nativeFrame(dt);postOnAnimation(this);}};
        GameView(){super(MainActivity.this);setFocusable(true);}
        @Override public boolean onTouchEvent(MotionEvent e){float steer=0,throttle=0,brake=0;for(int i=0;i<e.getPointerCount();i++){float x=e.getX(i),y=e.getY(i);if(x<getWidth()*.44f){
    float center=getWidth()*.22f;
    steer+=(x-center)/center;
}else if(y<getHeight()*.72f)throttle=1;else brake=1;}if(e.getActionMasked()==MotionEvent.ACTION_UP||e.getActionMasked()==MotionEvent.ACTION_CANCEL)steer=throttle=brake=0;nativeTouch(Math.max(-1,Math.min(1,steer)),throttle,brake);return true;}
    }
    private final class HudView extends View {
        Paint p=new Paint(3); Path map=new Path(); Runnable tick=new Runnable(){public void run(){invalidate();postDelayed(this,100);}};
        HudView(){super(MainActivity.this);}
        @Override protected void onDraw(Canvas c){super.onDraw(c);p.setTypeface(Typeface.create(Typeface.DEFAULT,Typeface.BOLD));p.setStyle(Paint.Style.FILL);p.setColor(0xffffffff);
            p.setTextSize(44);c.drawText("P"+nativePosition(),38,58,p); c.drawText(nativeTrackName(),38,30,p);p.setTextSize(28);c.drawText("LAP "+nativeLap()+"/5",38,94,p);
            // Compact original broadcast-style timing tower, driven by native race order.
            String[] rows=nativeTimingTower().split(";");
            float tx=24,ty=126,rowH=31;
            p.setTypeface(Typeface.create(Typeface.DEFAULT,Typeface.BOLD));
            for(int i=0;i<rows.length;i++){
                String[] f=rows[i].split(",");
                if(f.length<6) continue;
                int pos=Integer.parseInt(f[0]); String name=f[1]; int playerId=Integer.parseInt(f[2]);
                float gapRow=Float.parseFloat(f[3]); int move=Integer.parseInt(f[5]);
                boolean player=name.equals("RIT");
                p.setColor(player?0xccf2f2f2:0xaa111820);
                c.drawRoundRect(tx,ty+i*rowH,getWidth()*.40f,ty+i*rowH+27,5,5,p);
                p.setColor(player?0xff111820:0xffffffff);p.setTextSize(15);
                c.drawText(String.format(java.util.Locale.US,"%d",pos),tx+8,ty+i*rowH+18,p);
                c.drawText(name,tx+30,ty+i*rowH+18,p);
                if(move!=0){p.setTextSize(12);c.drawText(move>0?"Ã¢ÂÂ²":"Ã¢ÂÂ¼",tx+70,ty+i*rowH+18,p);}
                p.setTextSize(13);
                String g=(pos==1)?"LEAD":String.format(java.util.Locale.US,"+%.2f",gapRow);
                c.drawText(g,getWidth()*.40f-48,ty+i*rowH+18,p);
            }
            p.setTextSize(38);c.drawText(String.format(java.util.Locale.US,"%.0f",nativeSpeed()),getWidth()-145,58,p);p.setTextSize(18);c.drawText("KM/H",getWidth()-88,82,p);
            int startState=nativeStartState();
            p.setTextSize(22);c.drawText("ERS  MID",getWidth()-170,116,p);
            if(startState>0){p.setColor(0xffffd43b);p.setTextSize(54);c.drawText(Integer.toString(startState),getWidth()/2-15,125,p);}
            else if(startState<0){p.setColor(0xff66ff99);p.setTextSize(38);c.drawText("GO!",getWidth()/2-38,125,p);}
            p.setTextSize(18);
            float gap=nativeGap(), interval=nativeInterval();
            String timing = nativePosition()==1
                    ? String.format(java.util.Locale.US,"LEADER  +%05.2f",gap)
                    : String.format(java.util.Locale.US,"GAP  +%05.2f",gap);
            c.drawText(timing,getWidth()-190,140,p);
            if(nativePosition()>1) c.drawText(String.format(java.util.Locale.US,"INT  +%05.2f",interval),getWidth()-190,160,p);
            c.drawText(String.format(java.util.Locale.US,"TIME %06.2f",nativeRaceTime()),getWidth()-190,180,p);
            if(nativeFinished()){p.setColor(0xffffd43b);p.setTextSize(30);c.drawText("FINISH  P"+nativePosition(),getWidth()/2-100,70,p);}
            p.setColor(0x99ffffff);p.setStyle(Paint.Style.STROKE);p.setStrokeWidth(5);c.drawCircle(90,getHeight()-95,54,p);c.drawCircle(getWidth()-90,getHeight()-95,54,p);
            p.setStyle(Paint.Style.FILL);
            // Live steering-wheel indicator mirrors the native steering input.
            float wheelCx=90, wheelCy=getHeight()-95, wheelR=54, steer=nativeSteering();
            p.setStyle(Paint.Style.STROKE);p.setStrokeWidth(7);p.setColor(0xccffffff);
            c.drawCircle(wheelCx,wheelCy,wheelR,p);
            c.save();c.rotate((float)(steer*52.0),wheelCx,wheelCy);
            p.setStrokeWidth(6);c.drawLine(wheelCx,wheelCy-wheelR+10,wheelCx,wheelCy+wheelR-10,p);
            c.drawLine(wheelCx-wheelR*.58f,wheelCy+wheelR*.35f,wheelCx+wheelR*.58f,wheelCy+wheelR*.35f,p);c.restore();
            p.setStyle(Paint.Style.FILL);p.setColor(0xffffffff);p.setTextSize(15);c.drawText("STEER",61,getHeight()-24,p);
            p.setTextSize(16);c.drawText("BRAKE",getWidth()-120,getHeight()-190,p);c.drawText("THROTTLE",getWidth()-155,getHeight()-155,p);
            p.setColor(0xcc111820);c.drawRoundRect(getWidth()-150,24,getWidth()-24,92,14,14,p);
            p.setColor(0xffffffff);p.setTextSize(17);c.drawText("CAM",getWidth()-118,66,p);
            p.setColor(0xcc111820);c.drawRoundRect(getWidth()-190,132,getWidth()-24,298,18,18,p);
            p.setColor(0xffd8dde2);p.setStyle(Paint.Style.STROKE);p.setStrokeWidth(7);map.reset();map.moveTo(getWidth()-155,155);map.cubicTo(getWidth()-100,145,getWidth()-65,175,getWidth()-75,205);map.cubicTo(getWidth()-88,245,getWidth()-145,230,getWidth()-158,265);c.drawPath(map,p);
            p.setStyle(Paint.Style.FILL);p.setColor(0xffff2f1f);c.drawCircle(getWidth()-117,205,6,p);
        }
        @Override public boolean onTouchEvent(MotionEvent e){
            if(e.getActionMasked()==MotionEvent.ACTION_UP && e.getX()<210 && e.getY()<105){nativeNextTrack();return true;}
            if(e.getActionMasked()==MotionEvent.ACTION_UP && e.getX()>getWidth()-165 && e.getY()<105){nativeToggleCamera();return true;}
            return false;
        }
    }
}
