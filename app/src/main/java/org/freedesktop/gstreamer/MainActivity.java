package org.freedesktop.gstreamer;

import android.content.Context;
import android.graphics.Bitmap;
import android.graphics.BitmapFactory;
import android.support.v7.app.AppCompatActivity;
import android.os.Bundle;
import android.util.Log;
import android.view.Surface;
import android.view.SurfaceHolder;
import android.view.SurfaceView;
import android.widget.Toast;

import org.freedesktop.gstreamer.play.R;

public class MainActivity extends AppCompatActivity {

    private final String defaultMediaUri = "http://192.168.0.104/videos/oceans.mp4";
    private final String defaultMediaUri2 = "http://192.168.0.104/videos/world.mp4";
    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        try {
            init(this);
        } catch (Exception e) {
            Toast.makeText(this, e.getMessage(), Toast.LENGTH_LONG).show();
            finish();
            return;
        }
        setContentView(R.layout.activity_main);
        SurfaceView surfaceView = (SurfaceView)findViewById(R.id.surfaceView);
        surfaceView.getHolder().addCallback(new SurfaceHolder.Callback() {
            @Override
            public void surfaceCreated(SurfaceHolder surfaceHolder) {
                Log.i("NATIVE_LOG","set surface!");
                setSurface(surfaceHolder.getSurface());

            }

            @Override
            public void surfaceChanged(SurfaceHolder surfaceHolder, int i, int i1, int i2) {

            }

            @Override
            public void surfaceDestroyed(SurfaceHolder surfaceHolder) {
                Log.i("NATIVE_LOG","destroy surface!");
                destroySurface();

            }
        });
        setUri(defaultMediaUri,defaultMediaUri2);
        Bitmap bm = BitmapFactory.decodeResource(getResources(),R.drawable.pic1)
                .copy(Bitmap.Config.ARGB_8888,true);
        setBitmap(bm);
    }
    public static void init(Context context) throws Exception {
        System.loadLibrary("gstreamer_android");
        GStreamer.init(context);
        System.loadLibrary("gstplayer");
    }

    public native static void setSurface(Surface surface);
    public native static void setUri(String uri1,String uri2);
    public native static void destroySurface();
    public native static void setBitmap(Bitmap bitmap);
}
