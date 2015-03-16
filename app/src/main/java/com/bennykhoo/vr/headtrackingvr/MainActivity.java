package com.bennykhoo.vr.headtrackingvr;

import android.hardware.Sensor;
import android.hardware.SensorEvent;
import android.hardware.SensorEventListener;
import android.hardware.SensorManager;
import android.support.v7.app.ActionBarActivity;
import android.os.Bundle;
import android.util.Log;
import android.view.Menu;
import android.view.MenuItem;
import android.view.Surface;
import android.view.SurfaceHolder;
import android.view.SurfaceView;
import android.view.View;


public class MainActivity extends ActionBarActivity implements SurfaceHolder.Callback {

    private static final String TAG = "MainActivity";
    private SensorManager _sensorManager;
    LookAtSensorEventListener _lookAtSensorEventListener;

    private Sensor _accelerometer;
    private Sensor _magnetometer;

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);

        View decorView = getWindow().getDecorView();

        // Hide the status bar.

        int uiOptions = View.SYSTEM_UI_FLAG_HIDE_NAVIGATION
                | View.SYSTEM_UI_FLAG_FULLSCREEN
                | View.SYSTEM_UI_FLAG_IMMERSIVE;
        decorView.setSystemUiVisibility(uiOptions);

        setContentView(R.layout.activity_main);

        SurfaceView surfaceView = (SurfaceView)findViewById(R.id.surfaceview);
        surfaceView.getHolder().addCallback(this);

        _sensorManager = (SensorManager)getSystemService(SENSOR_SERVICE);
        _accelerometer = _sensorManager.getDefaultSensor(Sensor.TYPE_ACCELEROMETER);
        _magnetometer = _sensorManager.getDefaultSensor(Sensor.TYPE_MAGNETIC_FIELD);
    }


    @Override
    protected void onResume() {
        super.onResume();
        Log.i(TAG, "onResume()");

        // setup aligning listener. Start panning camera when orientation is initialized properly.
        final InitSensorEventListener aligningSensorEventListener = new InitSensorEventListener(new InitSensorEventListener.FinishCallback() {
            @Override
            public void finish(InitSensorEventListener listener, float[] orientation) {
                _sensorManager.unregisterListener(listener);
                Log.i(TAG, "received stable orientation: " + orientation[0] + " " + orientation[1] + " " + orientation[2]);
                _lookAtSensorEventListener = new LookAtSensorEventListener(orientation, new LookAtSensorEventListener.LookAtCallback() {
                    @Override
                    public void lookAt(float azimuth, float pitch, float roll) {
                        nativeSetLookAtAngles(azimuth, pitch, roll);
                    }
                });

                _sensorManager.registerListener(_lookAtSensorEventListener, _accelerometer, SensorManager.SENSOR_DELAY_GAME);
                _sensorManager.registerListener(_lookAtSensorEventListener, _magnetometer, SensorManager.SENSOR_DELAY_GAME);

                // play click sound?
//                View v = findViewById(R.id.surfaceview);
//                v.playSoundEffect(android.view.SoundEffectConstants.CLICK);
            }
        });

        _sensorManager.registerListener(aligningSensorEventListener, _accelerometer, SensorManager.SENSOR_DELAY_GAME);
        _sensorManager.registerListener(aligningSensorEventListener, _magnetometer, SensorManager.SENSOR_DELAY_GAME);

        // init lookAt angles
        nativeSetLookAtAngles(0f, 0f, 0f);
        nativeOnResume();
    }

    protected void onPause() {
        super.onPause();
        Log.i(TAG, "onPause()");

        _sensorManager.unregisterListener(_lookAtSensorEventListener);
        nativeOnPause();
    }

    static final float ALPHA = 0.2f; // if ALPHA = 1 OR 0, no filter applies.

    static protected float[] LowPass( float[] input, float[] output ) {
        if ( output == null ) return input;
        for ( int i=0; i<input.length; i++ ) {
            output[i] = output[i] + ALPHA * (input[i] - output[i]);
        }
        return output;
    }

    @Override
    public boolean onCreateOptionsMenu(Menu menu) {
        // Inflate the menu; this adds items to the action bar if it is present.
        getMenuInflater().inflate(R.menu.menu_main, menu);
        return true;
    }

    @Override
    public boolean onOptionsItemSelected(MenuItem item) {
        // Handle action bar item clicks here. The action bar will
        // automatically handle clicks on the Home/Up button, so long
        // as you specify a parent activity in AndroidManifest.xml.
        int id = item.getItemId();

        //noinspection SimplifiableIfStatement
        if (id == R.id.action_settings) {
            return true;
        }

        return super.onOptionsItemSelected(item);
    }

    @Override
    protected void onStart() {
        super.onStart();
        Log.i(TAG, "onStart()");
        nativeOnStart();
    }

    @Override
    protected void onStop() {
        super.onStop();
        Log.i(TAG, "onStop()");
        nativeOnStop();
    }

    public void surfaceChanged(SurfaceHolder holder, int format, int w, int h) {
        Log.i(TAG, "nativeSetSurface()");
        nativeSetSurface(holder.getSurface());
    }

    public void surfaceCreated(SurfaceHolder holder) {
    }

    public void surfaceDestroyed(SurfaceHolder holder) {
        nativeSetSurface(null);
    }


    public static native void nativeOnStart();
    public static native void nativeOnResume();
    public static native void nativeOnPause();
    public static native void nativeOnStop();
    public static native void nativeSetSurface(Surface surface);
    public static native void nativeSetLookAtAngles(float azimuth, float pitch, float roll);

    static {
        System.loadLibrary("lynda-demo");
    }

    static class InitSensorEventListener implements SensorEventListener {
        public interface FinishCallback {
            public void finish(InitSensorEventListener listener, float[] orientation);
        }

        private FinishCallback _finishCallback;

        private float[] _lastAccelSet;
        private float[] _lastMagnetoSet;
        private float[] _r = new float[9];

        private float[] _lastOrientationSet;
        private float[] _summedOrientationSet;
        private int _readingCount;
        private long _startTime;
        private final float errorThreshold = (float) (15.0f/180.0f * Math.PI);

        public InitSensorEventListener(FinishCallback _callback) {
            this._finishCallback = _callback;
            start();
        }

        void start() {
            _lastAccelSet = null;
            _lastMagnetoSet = null;
            _summedOrientationSet = new float[3];
            _lastOrientationSet = null;
            _readingCount = 0;
            _startTime = System.currentTimeMillis();
        }

        void finish() {
            float[] finalOrientation = new float[3];
            for (int i = 0; i < 3; i++) {
                finalOrientation[i] = _summedOrientationSet[i] / _readingCount;
            }
            _finishCallback.finish(this, finalOrientation);
        }

        void readOrientation(float []orientation) {
            for (int i = 0; i < 3; i++) {
                _summedOrientationSet[i] += orientation[i];
            }
            _readingCount++;
        }

        public void onSensorChanged(SensorEvent event) {
            if (event.sensor.getType() == Sensor.TYPE_ACCELEROMETER) {
                _lastAccelSet = LowPass(event.values.clone(), _lastAccelSet);
            } else if (event.sensor.getType() == Sensor.TYPE_MAGNETIC_FIELD) {
                _lastMagnetoSet = LowPass(event.values.clone(), _lastMagnetoSet);
            }

            if (_lastAccelSet != null && _lastMagnetoSet != null) {
                SensorManager.getRotationMatrix(_r, null, _lastAccelSet, _lastMagnetoSet);
                float[] orientation = new float [3];
                SensorManager.getOrientation(_r, orientation);

                // if erratic reading restart timer
                boolean okay = true;
                if (_lastOrientationSet != null) {
                    float[] err = new float[3];
                    for (int i = 0; i < 3; i++) {
                        err[i] = orientation[i] - _lastOrientationSet[i];
                        if (err[i] > errorThreshold) {
                            Log.w(TAG, "erratic reading at indice " + i + " with error " + err[i] + " more than preset threshold. Restarting timer");
                            start();
                            okay = false;
                            break;
                        }
                    }
                }

                if (okay) {
                    readOrientation(orientation);
                    long elapsedTime = System.currentTimeMillis() - _startTime;
                    if (elapsedTime > 3000) {
                        // set offset
                        finish();
                    }
                }
                _lastOrientationSet = orientation;
            }
        }


        @Override
        public void onAccuracyChanged(Sensor sensor, int accuracy) {

        }
    }

    static class LookAtSensorEventListener implements SensorEventListener {

        public interface LookAtCallback {
            public void lookAt(float azimuth, float pitch, float roll);
        }

        private LookAtCallback _lookAtCallback;
        private float[] _lastAccelSet;
        private float[] _lastMagnetoSet;
        private float[] _r = new float[9];
        private final float[] _offsets;

        LookAtSensorEventListener(float[] offsets, LookAtCallback _lookAtCallback) {
            this._lookAtCallback = _lookAtCallback;
            this._offsets = offsets;
        }

        @Override
        public void onSensorChanged(SensorEvent event) {
            if (event.sensor.getType() == Sensor.TYPE_ACCELEROMETER) {
                _lastAccelSet = LowPass(event.values.clone(), _lastAccelSet);
            } else if (event.sensor.getType() == Sensor.TYPE_MAGNETIC_FIELD) {
                _lastMagnetoSet = LowPass(event.values.clone(), _lastMagnetoSet);
            }

            if (_lastAccelSet != null && _lastMagnetoSet != null) {
                SensorManager.getRotationMatrix(_r, null, _lastAccelSet, _lastMagnetoSet);
                float[] orientation = new float[3];
                SensorManager.getOrientation(_r, orientation);

                _lookAtCallback.lookAt(_offsets[0] - orientation[0],
                        orientation[1] - _offsets[1],
                        _offsets[2] - orientation[2]);
            }
        }

        @Override
        public void onAccuracyChanged(Sensor sensor, int accuracy) {

        }
    }

}
