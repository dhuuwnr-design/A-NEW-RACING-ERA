package com.apexenginenext;

import android.app.Activity;
import android.graphics.Canvas;
import android.graphics.Paint;
import android.graphics.Path;
import android.graphics.Typeface;
import android.hardware.Sensor;
import android.hardware.SensorEvent;
import android.hardware.SensorEventListener;
import android.hardware.SensorManager;
import android.os.Bundle;
import android.view.MotionEvent;
import android.view.SurfaceHolder;
import android.view.SurfaceView;
import android.view.View;
import android.widget.FrameLayout;

public final class MainActivity extends Activity implements SurfaceHolder.Callback, SensorEventListener {
    private GameView surface; private HudView hud;
    private SensorManager sensors; private Sensor rotationSensor;
    private float tiltSteer=0;
    private boolean leftHanded=false, tiltSteerEnabled=false, steeringAssist=false;
    static { System.loadLibrary("apex"); }

    @Override public void onCreate(Bundle state) {
        super.onCreate(state);
        sensors=(SensorManager)getSystemService(SENSOR_SERVICE);
        rotationSensor=sensors==null?null:sensors.getDefaultSensor(Sensor.TYPE_ROTATION_VECTOR);
        FrameLayout root=new FrameLayout(this);
        surface=new GameView(); surface.getHolder().addCallback(this); surface.setZOrderMediaOverlay(false);
        hud=new HudView();
        root.addView(surface,new FrameLayout.LayoutParams(-1,-1));
        root.addView(hud,new FrameLayout.LayoutParams(-1,-1));
        setContentView(root);
    }
    @Override protected void onResume(){super.onResume();if(tiltSteerEnabled&&rotationSensor!=null)sensors.registerListener(this,rotationSensor,SensorManager.SENSOR_DELAY_GAME);}
    @Override protected void onPause(){if(sensors!=null)sensors.unregisterListener(this);super.onPause();}
    @Override public void onSensorChanged(SensorEvent e){
        if(e.sensor.getType()!=Sensor.TYPE_ROTATION_VECTOR)return;
        float[] r=new float[9];SensorManager.getRotationMatrixFromVector(r,e.values);
        // Screen-horizontal phone roll becomes steering. Clamp and smooth to avoid gyro noise.
        float target=Math.max(-1f,Math.min(1f,r[2]/0.45f));
        tiltSteer+= (target-tiltSteer)*0.18f;
    }
    @Override public void onAccuracyChanged(Sensor s,int accuracy){}

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
        boolean running=false; float lastSteer=0,lastThrottle=0,lastBrake=0;
        final Runnable frame=new Runnable(){long last=System.nanoTime();public void run(){if(!running)return;long now=System.nanoTime();float dt=Math.min(.05f,(now-last)*1e-9f);last=now;nativeFrame(dt);postOnAnimation(this);}};
        GameView(){super(MainActivity.this);setFocusable(true);}

        private boolean inRect(float x,float y,float l,float t,float r,float b){return x>=l&&x<=r&&y>=t&&y<=b;}
        @Override public boolean onTouchEvent(MotionEvent e){
            final int action=e.getActionMasked(),skip=(action==MotionEvent.ACTION_POINTER_UP)?e.getActionIndex():-1;
            if(action==MotionEvent.ACTION_CANCEL||action==MotionEvent.ACTION_UP){lastSteer=lastThrottle=lastBrake=0;nativeTouch(0,0,0);return true;}
            float steer=0; boolean throttle=false,brake=false;
            final float w=getWidth(),h=getHeight(),controlW=w*.30f;
            final float steerL=leftHanded?w-controlW:0, steerR=leftHanded?w:controlW;
            final float pedalL=leftHanded?0:w*.58f, pedalR=leftHanded?w*.42f:w;
            for(int i=0;i<e.getPointerCount();i++){
                if(i==skip)continue;
                float x=e.getX(i),y=e.getY(i);
                if(!tiltSteerEnabled && x>=steerL && x<=steerR && y>h*.18f){
                    float center=(steerL+steerR)*.5f;
                    float s=(x-center)/(controlW*.42f);
                    if(Math.abs(s)>Math.abs(steer))steer=Math.max(-1,Math.min(1,s));
                }
                if(inRect(x,y,pedalL,h*.58f,pedalR,h*.76f))throttle=true;
                if(inRect(x,y,pedalL,h*.78f,pedalR,h*.96f))brake=true;
            }
            if(tiltSteerEnabled)steer=tiltSteer;
            if(steeringAssist){
                float speed=Math.max(0,nativeSpeed());
                float factor=0.72f+0.20f/(1f+speed/120f);
                steer*=factor;
            }
            lastSteer=steer;lastThrottle=throttle?1:0;lastBrake=brake?1:0;
            nativeTouch(lastSteer,lastThrottle,lastBrake);
            return true;
        }
    }

    private final class HudView extends View {
        Paint p=new Paint(3); Path map=new Path();
        Runnable tick=new Runnable(){public void run(){invalidate();postDelayed(this,100);}};
        HudView(){super(MainActivity.this);}
        @Override protected void onDraw(Canvas c){
            super.onDraw(c);p.setTypeface(Typeface.create(Typeface.DEFAULT,Typeface.BOLD));p.setStyle(Paint.Style.FILL);p.setColor(0xffffffff);
            p.setTextSize(44);c.drawText("P"+nativePosition(),38,58,p);c.drawText(nativeTrackName(),38,30,p);p.setTextSize(28);c.drawText("LAP "+nativeLap()+"/5",38,94,p);
            String[] rows=nativeTimingTower().split(";");
            float tx=24,ty=126,rowH=31;
            for(int i=0;i<rows.length;i++){String[] f=rows[i].split(",");if(f.length<6)continue;int pos=Integer.parseInt(f[0]);String name=f[1];float gapRow=Float.parseFloat(f[3]);int move=Integer.parseInt(f[5]);boolean player=name.equals("RIT");p.setColor(player?0xccf2f2f2:0xaa111820);c.drawRoundRect(tx,ty+i*rowH,getWidth()*.40f,ty+i*rowH+27,5,5,p);p.setColor(player?0xff111820:0xffffffff);p.setTextSize(15);c.drawText(Integer.toString(pos),tx+8,ty+i*rowH+18,p);c.drawText(name,tx+30,ty+i*rowH+18,p);if(move!=0){p.setTextSize(12);c.drawText(move>0?"▲":"▼",tx+70,ty+i*rowH+18,p);}p.setTextSize(13);c.drawText(pos==1?"LEAD":String.format(java.util.Locale.US,"+%.2f",gapRow),getWidth()*.40f-48,ty+i*rowH+18,p);}
            p.setTextSize(38);c.drawText(String.format(java.util.Locale.US,"%.0f",nativeSpeed()),getWidth()-145,58,p);p.setTextSize(18);c.drawText("KM/H",getWidth()-88,82,p);
            int startState=nativeStartState();p.setTextSize(22);c.drawText("ERS  MID",getWidth()-170,116,p);if(startState>0){p.setColor(0xffffd43b);p.setTextSize(54);c.drawText(Integer.toString(startState),getWidth()/2-15,125,p);}else if(startState<0){p.setColor(0xff66ff99);p.setTextSize(38);c.drawText("GO!",getWidth()/2-38,125,p);}
            p.setTextSize(18);float gap=nativeGap(),interval=nativeInterval();c.drawText(nativePosition()==1?String.format(java.util.Locale.US,"LEADER  +%05.2f",gap):String.format(java.util.Locale.US,"GAP  +%05.2f",gap),getWidth()-190,140,p);if(nativePosition()>1)c.drawText(String.format(java.util.Locale.US,"INT  +%05.2f",interval),getWidth()-190,160,p);c.drawText(String.format(java.util.Locale.US,"TIME %06.2f",nativeRaceTime()),getWidth()-190,180,p);
            if(nativeFinished()){p.setColor(0xffffd43b);p.setTextSize(30);c.drawText("FINISH  P"+nativePosition(),getWidth()/2-100,70,p);}
            final float sw=getWidth()*.30f,pl=leftHanded?0:getWidth()*.70f,pr=leftHanded?getWidth()*.30f:getWidth();
            p.setColor(0x55111118);p.setStyle(Paint.Style.FILL);c.drawRoundRect(pl+8,getHeight()*.54f,pr-8,getHeight()*.98f,24,24,p);
            p.setColor(0x99ffffff);p.setStyle(Paint.Style.STROKE);p.setStrokeWidth(3);c.drawRoundRect(pl+8,getHeight()*.58f,pr-8,getHeight()*.75f,20,20,p);c.drawRoundRect(pl+8,getHeight()*.78f,pr-8,getHeight()*.96f,20,20,p);
            p.setStyle(Paint.Style.FILL);p.setTextSize(22);p.setColor(0xffffffff);c.drawText("THROTTLE",pl+24,getHeight()*.69f,p);c.drawText("BRAKE",pl+24,getHeight()*.90f,p);
            p.setStyle(Paint.Style.STROKE);p.setStrokeWidth(5);p.setColor(0xccffffff);float wc=leftHanded?getWidth()-80:80,wy=getHeight()-95;c.drawCircle(wc,wy,54,p);c.save();c.rotate((float)(nativeSteering()*52.0),wc,wy);c.drawLine(wc,wy-44,wc,wy+44,p);c.drawLine(wc-31,wy+19,wc+31,wy+19,p);c.restore();p.setStyle(Paint.Style.FILL);p.setTextSize(15);c.drawText(tiltSteerEnabled?"TILT":"STEER",wc-27,getHeight()-24,p);
            p.setColor(0xcc111820);c.drawRoundRect(getWidth()-150,24,getWidth()-24,92,14,14,p);p.setColor(0xffffffff);p.setTextSize(17);c.drawText("CAM",getWidth()-118,66,p);
            p.setColor(0xcc111820);c.drawRoundRect(getWidth()-300,24,getWidth()-174,92,14,14,p);p.setColor(0xffffffff);p.setTextSize(14);c.drawText("CTRL",getWidth()-270,66,p);
            p.setTextSize(13);String mode=(leftHanded?"R":"L")+" "+(tiltSteerEnabled?"TILT":"TOUCH")+" "+(steeringAssist?"ASSIST":"RAW");c.drawText(mode,getWidth()-300,108,p);
            p.setColor(0x99ffffff);p.setStyle(Paint.Style.STROKE);p.setStrokeWidth(5);map.reset();map.moveTo(getWidth()-155,155);map.cubicTo(getWidth()-100,145,getWidth()-65,175,getWidth()-75,205);map.cubicTo(getWidth()-88,245,getWidth()-145,230,getWidth()-158,265);c.drawPath(map,p);p.setStyle(Paint.Style.FILL);p.setColor(0xffff2f1f);c.drawCircle(getWidth()-117,205,6,p);
        }
        @Override public boolean onTouchEvent(MotionEvent e){
            if(e.getActionMasked()==MotionEvent.ACTION_UP){
                float x=e.getX(),y=e.getY();
                if(x>getWidth()-165&&y<105){nativeToggleCamera();return true;}
                if(x>getWidth()-310&&x<getWidth()-165&&y<105){
                    if(!leftHanded&&!tiltSteerEnabled&&!steeringAssist){leftHanded=true;}
                    else if(leftHanded&&!tiltSteerEnabled&&!steeringAssist){tiltSteerEnabled=true;if(rotationSensor!=null)sensors.registerListener(MainActivity.this,rotationSensor,SensorManager.SENSOR_DELAY_GAME);}
                    else if(tiltSteerEnabled){tiltSteerEnabled=false;steeringAssist=true;if(sensors!=null)sensors.unregisterListener(MainActivity.this);}
                    else {steeringAssist=false;leftHanded=false;}
                    return true;
                }
            }
            return false;
        }
    }
}
