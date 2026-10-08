package com.apexenginenext;

import android.app.Activity;
import android.content.pm.ActivityInfo;
import android.os.Bundle;
import android.view.*;
import android.widget.FrameLayout;
import android.graphics.*;

public final class MainActivity extends Activity implements SurfaceHolder.Callback {
    private GameView game; private Ui ui;
    static { System.loadLibrary("apex"); }

    @Override public void onCreate(Bundle s){
        super.onCreate(s); setRequestedOrientation(ActivityInfo.SCREEN_ORIENTATION_LANDSCAPE);
        getWindow().setNavigationBarColor(Color.BLACK);
        FrameLayout root=new FrameLayout(this);
        game=new GameView(); game.getHolder().addCallback(this); ui=new Ui();
        root.addView(game,new FrameLayout.LayoutParams(-1,-1)); root.addView(ui,new FrameLayout.LayoutParams(-1,-1));
        setContentView(root);
    }
    @Override public void surfaceCreated(SurfaceHolder h){nativeStart(h.getSurface(),getAssets());game.running=true;game.post(game.frame);ui.loading();}
    @Override public void surfaceDestroyed(SurfaceHolder h){game.running=false;nativeStop();}
    @Override public void surfaceChanged(SurfaceHolder h,int f,int w,int z){nativeResize(w,z);}

    private static native void nativeStart(Surface s,android.content.res.AssetManager a);
    private static native void nativeStop(); private static native void nativeResize(int w,int h);
    private static native void nativeFrame(float dt); private static native void nativeToggleCamera();
    private static native String nativeTrackName(); private static native void nativeTouch(float s,float t,float b);
    private static native float nativeSpeed(); private static native float nativeFps();
    private static native int nativeLap(); private static native int nativePosition();
    private static native boolean nativeFinished();

    final class GameView extends SurfaceView {
        boolean running;
        final Runnable frame=new Runnable(){long last=System.nanoTime();public void run(){
            if(!running)return; long n=System.nanoTime(); float dt=Math.min(.05f,(n-last)*1e-9f);last=n;
            nativeFrame(dt);postOnAnimation(this);
        }};
        GameView(){super(MainActivity.this);setFocusable(true);}
        boolean hit(float x,float y,float l,float t,float r,float b){return x>=l&&x<=r&&y>=t&&y<=b;}
        @Override public boolean onTouchEvent(MotionEvent e){
            int a=e.getActionMasked();
            if(a==MotionEvent.ACTION_UP||a==MotionEvent.ACTION_CANCEL){nativeTouch(0,0,0);return true;}
            float w=getWidth(),h=getHeight(),st=0,th=0,br=0,bw=Math.min(w*.13f,h*.20f),top=h*.70f,l=w*.035f,r=l+bw*.95f;
            for(int i=0;i<e.getPointerCount();i++){float x=e.getX(i),y=e.getY(i);
                if(hit(x,y,l,top,l+bw,h*.94f))st=-1;if(hit(x,y,r,top,r+bw,h*.94f))st=1;
                if(hit(x,y,w*.78f,h*.57f,w*.965f,h*.75f))th=1;if(hit(x,y,w*.78f,h*.77f,w*.965f,h*.95f))br=1;
            }
            nativeTouch(st,th,br);return true;
        }
    }

    final class Ui extends View {
        final Paint p=new Paint(3); int screen=0; long loadAt;
        Ui(){super(MainActivity.this);setLayerType(View.LAYER_TYPE_SOFTWARE,null);}
        void loading(){screen=0;loadAt=System.currentTimeMillis();postDelayed(()->{screen=1;invalidate();},1600);}
        void text(Canvas c,String s,float x,float y,float size,int color,boolean bold){
            p.setShader(null);p.setStyle(Paint.Style.FILL);p.setColor(color);p.setTextSize(size);
            p.setTypeface(bold?Typeface.DEFAULT_BOLD:Typeface.DEFAULT);c.drawText(s,x,y,p);
        }
        void panel(Canvas c,float l,float t,float r,float b,boolean active){
            p.setShader(null);p.setStyle(Paint.Style.FILL);p.setColor(active?0xffe51b2e:0xdd0c1117);
            c.drawRoundRect(l,t,r,b,10,10,p);p.setStyle(Paint.Style.STROKE);p.setStrokeWidth(2);
            p.setColor(active?0xffff6672:0xff39434e);c.drawRoundRect(l,t,r,b,10,10,p);p.setStyle(Paint.Style.FILL);
        }
        boolean hit(float x,float y,float l,float t,float r,float b){return x>=l&&x<=r&&y>=t&&y<=b;}
        void car(Canvas c,float x,float y,float s){
            p.setStyle(Paint.Style.FILL);p.setColor(0xff151a20);c.drawRoundRect(x-110*s,y-4*s,x+110*s,y+22*s,15*s,15*s,p);
            p.setColor(0xffe51b2e);Path q=new Path();
            q.moveTo(x-120*s,y+8*s);q.lineTo(x-62*s,y-24*s);q.lineTo(x-28*s,y-30*s);q.lineTo(x-12*s,y-44*s);
            q.lineTo(x+12*s,y-44*s);q.lineTo(x+28*s,y-30*s);q.lineTo(x+62*s,y-24*s);q.lineTo(x+120*s,y+8*s);
            q.lineTo(x+65*s,y+3*s);q.lineTo(x-65*s,y+3*s);q.close();c.drawPath(q,p);
            p.setColor(Color.BLACK);c.drawCircle(x-75*s,y+25*s,27*s,p);c.drawCircle(x+75*s,y+25*s,27*s,p);
            p.setColor(0xffd8dbe0);c.drawRect(x-125*s,y+15*s,x+125*s,y+21*s,p);
            p.setColor(0xff080a0d);c.drawRoundRect(x-27*s,y-47*s,x+27*s,y-5*s,12*s,12*s,p);
        }
        void loadingDraw(Canvas c){
            float w=getWidth(),h=getHeight();
            p.setShader(new LinearGradient(0,0,0,h,0xff030507,0xff171d25,Shader.TileMode.CLAMP));c.drawRect(0,0,w,h,p);p.setShader(null);
            text(c,"APEX",w*.07f,h*.18f,64,Color.WHITE,true);text(c,"ENGINE NEXT",w*.20f,h*.18f,20,0xffc7cdd5,true);
            text(c,"A NEW RACING ERA",w*.07f,h*.25f,14,0xff8d98a5,true);car(c,w*.58f,h*.52f,1.25f);
            p.setColor(0xff303741);c.drawRect(w*.07f,h*.72f,w*.82f,h*.725f,p);
            long e=System.currentTimeMillis()-loadAt;float pr=Math.min(1,e/1600f);
            p.setColor(0xffe51b2e);c.drawRect(w*.07f,h*.72f,w*.07f+w*.75f*pr,h*.725f,p);
            text(c,"LOADING RACE SYSTEMS",w*.07f,h*.82f,15,0xffaab3bf,true);text(c,(int)(pr*100)+"%",w*.84f,h*.82f,14,Color.WHITE,true);
        }
        void menuDraw(Canvas c){
            float w=getWidth(),h=getHeight();
            p.setShader(new LinearGradient(0,0,w,h,0xff030507,0xff121922,Shader.TileMode.CLAMP));c.drawRect(0,0,w,h,p);p.setShader(null);
            p.setColor(0xff222a33);for(int i=0;i<9;i++)c.drawRect(w*.43f+i*w*.06f,h*.46f,w*.445f+i*w*.06f,h,p);
            p.setColor(0xffe51b2e);c.drawRect(w*.43f,h*.485f,w,h*.495f,p);car(c,w*.68f,h*.56f,1.45f);
            text(c,"APEX",w*.045f,h*.12f,58,Color.WHITE,true);text(c,"ENGINE NEXT",w*.19f,h*.12f,19,0xffc6ccd4,true);
            text(c,"REAL TRACKS  /  REAL PHYSICS  /  ANDROID",w*.045f,h*.18f,11,0xff8e98a5,true);
            String[] labels={"CAREER","QUICK RACE","MULTIPLAYER","GARAGE","SETTINGS"};
            boolean[] locked={true,false,true,true,false};
            float l=w*.045f,r=w*.34f,bh=h*.075f,g=h*.018f,t=h*.25f;
            for(int i=0;i<labels.length;i++){boolean on=!locked[i];panel(c,l,t+i*(bh+g),r,t+i*(bh+g)+bh,on);text(c,labels[i],l+32,t+i*(bh+g)+bh*.64f,16,on?Color.WHITE:0xff727b86,true);if(locked[i])text(c,"LOCKED",r-78,t+i*(bh+g)+bh*.64f,10,0xff727b86,true);else text(c,"›",r-26,t+i*(bh+g)+bh*.64f,25,Color.WHITE,true);}
            panel(c,w*.76f,h*.60f,w*.96f,h*.88f,false);text(c,"NEXT RACE",w*.785f,h*.67f,13,0xffe51b2e,true);
            text(c,nativeTrackName(),w*.785f,h*.74f,25,Color.WHITE,true);text(c,"GRAND PRIX",w*.785f,h*.79f,12,0xff9ba4ae,true);
            text(c,"5 LAPS  •  DRY",w*.785f,h*.84f,11,0xffd3d7dc,true);
            text(c,"LEVEL 1",w*.80f,h*.11f,13,Color.WHITE,true);text(c,"12,450 CR",w*.88f,h*.11f,13,0xffffc43d,true);
        }
        @Override protected void onDraw(Canvas c){
            super.onDraw(c);float w=getWidth(),h=getHeight();
            if(screen==0){loadingDraw(c);return;} if(screen==1){menuDraw(c);return;}
            if(screen==3){c.drawColor(0xff070a0e);text(c,"SETTINGS",44,62,34,Color.WHITE,true);
                text(c,"TOUCH CONTROLS",54,128,18,0xffe51b2e,true);text(c,"Landscape / conventional steering",54,158,17,Color.WHITE,false);
                text(c,"CAMERA",54,214,18,0xffe51b2e,true);text(c,"Tap CAM during a race to change view",54,244,17,Color.WHITE,false);
                panel(c,44,300,250,356,true);text(c,"BACK TO MENU",70,336,17,Color.WHITE,true);return;}
            text(c,"P"+nativePosition(),24,38,30,Color.WHITE,true);text(c,nativeTrackName(),24,61,16,Color.WHITE,false);
            text(c,String.format(java.util.Locale.US,"%.0f KM/H",nativeSpeed()),w-145,38,22,Color.WHITE,true);text(c,"LAP "+nativeLap()+"/5",w-95,61,15,Color.WHITE,true);
            float top=h*.70f,bw=Math.min(w*.13f,h*.20f),l=w*.035f,r=l+bw*.95f;
            panel(c,l,top,l+bw,h*.94f,false);panel(c,r,top,r+bw,h*.94f,false);
            text(c,"<",l+bw*.36f,top+(h*.94f-top)*.62f,30,Color.WHITE,true);text(c,">",r+bw*.36f,top+(h*.94f-top)*.62f,30,Color.WHITE,true);
            panel(c,w*.78f,h*.57f,w*.965f,h*.75f,false);panel(c,w*.78f,h*.77f,w*.965f,h*.95f,false);
            text(c,"THR",w*.835f,h*.675f,18,Color.WHITE,true);text(c,"BRK",w*.835f,h*.875f,18,Color.WHITE,true);
            panel(c,w-118,16,w-20,66,false);text(c,"CAM",w-87,47,14,Color.WHITE,true);text(c,String.format(java.util.Locale.US,"FPS %.0f",nativeFps()),24,82,13,0xffb8c0c9,false);
            if(nativeFinished())text(c,"FINISH  P"+nativePosition(),w/2-90,52,30,0xffffd43b,true);
        }
        @Override public boolean onTouchEvent(MotionEvent e){
            if(e.getActionMasked()!=MotionEvent.ACTION_UP)return screen!=2;
            float x=e.getX(),y=e.getY(),w=getWidth(),h=getHeight();
            if(screen==1){float l=w*.045f,r=w*.34f,bh=h*.075f,g=h*.018f,t=h*.25f;
                int index=(int)((y-t)/(bh+g));
                if(index>=0&&index<5){float a=t+index*(bh+g);if(hit(x,y,l,a,r,a+bh)){if(index==1)screen=2;else if(index==4)screen=3;invalidate();return true;}}
                return true;}
            if(screen==3){if(hit(x,y,35,285,270,375)){screen=1;invalidate();}return true;}
            if(screen==2&&x>w-140&&y<90){nativeToggleCamera();return true;} return false;
        }
    }
}