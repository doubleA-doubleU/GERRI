package com.example.aaron.gerri;

// libraries

import android.Manifest;
import android.app.Activity;
import android.app.PendingIntent;
import android.content.Context;
import android.content.Intent;
import android.content.pm.PackageManager;
import android.graphics.Bitmap;
import android.graphics.Canvas;
import android.graphics.Paint;
import android.graphics.SurfaceTexture;
import android.hardware.Camera;
import android.hardware.usb.UsbDeviceConnection;
import android.hardware.usb.UsbManager;
import android.os.Bundle;
import android.support.v4.app.ActivityCompat;
import android.support.v4.content.ContextCompat;
import android.view.SurfaceHolder;
import android.view.SurfaceView;
import android.view.TextureView;
import android.view.View;
import android.view.WindowManager;
import android.widget.Button;
import android.widget.TextView;
import android.widget.SeekBar;
import android.widget.SeekBar.OnSeekBarChangeListener;

import com.hoho.android.usbserial.driver.CdcAcmSerialDriver;
import com.hoho.android.usbserial.driver.ProbeTable;
import com.hoho.android.usbserial.driver.UsbSerialDriver;
import com.hoho.android.usbserial.driver.UsbSerialPort;
import com.hoho.android.usbserial.driver.UsbSerialProber;
import com.hoho.android.usbserial.util.SerialInputOutputManager;

import java.io.IOException;
import java.io.UnsupportedEncodingException;
import java.util.List;
import java.util.Scanner;
import java.util.concurrent.ExecutorService;
import java.util.concurrent.Executors;

import static android.graphics.Color.blue;
import static android.graphics.Color.green;
import static android.graphics.Color.red;
import static android.graphics.Color.rgb;

public class MainActivity extends Activity implements TextureView.SurfaceTextureListener {
    private Camera mCamera;
    private TextureView mTextureView;
    private SurfaceView mSurfaceView;
    private SurfaceView mSurfaceView2;
    private SurfaceHolder mSurfaceHolder;
    private SurfaceHolder mSurfaceHolder2;
    private Bitmap bmp = Bitmap.createBitmap(640, 480, Bitmap.Config.ARGB_8888);
    private Canvas canvas = new Canvas(bmp);
    private Bitmap bmp2 = Bitmap.createBitmap(500, 500, Bitmap.Config.ARGB_8888);
    private Paint paint1 = new Paint();
    private TextView myTextView;
    private TextView myTextView2;
    private SeekBar threshold;
    private Button button;
    int thresh = 120;
    //int thresh2 = 190;
    int go = 0;
    int xTrack = 0, yTrack = 0;
    float xCar = 0, yCar = 0, theta = 0, lapTime = 0;

    private UsbManager manager;
    private UsbSerialPort sPort;
    private final ExecutorService mExecutor = Executors.newSingleThreadExecutor();
    private SerialInputOutputManager mSerialIoManager;

    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_main);
        getWindow().addFlags(WindowManager.LayoutParams.FLAG_KEEP_SCREEN_ON); // keeps the screen from turning off

        threshold = (SeekBar) findViewById(R.id.seek1);
        myTextView = (TextView) findViewById(R.id.textView01);
        myTextView2 = (TextView) findViewById(R.id.textView02);
        button = (Button) findViewById(R.id.button1);

        button.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View v) { // change state variable
                go = go + 1;
                if (go > 2) {
                    go = 0;
                    // send 641 to the PIC to stop motors (outside of expected range for COM)
                    int temp = 641;
                    String sendString = String.valueOf(temp) + '\n';
                    for (int j = 0; j < 3; j++) { // send multiple times to make sure the robot "hears" it
                        try {
                            sPort.write(sendString.getBytes(), 10); // 10 is the timeout
                        } catch (IOException e) {
                        }
                    }
                }
            }
        });

        // see if the app has permission to use the camera
        //ActivityCompat.requestPermissions(MainActivity.this, new String[]{Manifest.permission.CAMERA}, 1);
        if (ContextCompat.checkSelfPermission(this, Manifest.permission.CAMERA) == PackageManager.PERMISSION_GRANTED) {
            mSurfaceView = (SurfaceView) findViewById(R.id.surfaceview);
            mSurfaceHolder = mSurfaceView.getHolder();

            mSurfaceView2 = (SurfaceView) findViewById(R.id.surfaceview2);
            mSurfaceHolder2 = mSurfaceView2.getHolder();

            mTextureView = (TextureView) findViewById(R.id.textureview);
            mTextureView.setSurfaceTextureListener(this);

            // set the paintbrush for writing text on the image
            paint1.setColor(0xffff0000); // red
            paint1.setTextSize(24);
        }

        setMyControlListener();
        manager = (UsbManager) getSystemService(Context.USB_SERVICE);
    }

    public void onSurfaceTextureAvailable(SurfaceTexture surface, int width, int height) {
        mCamera = Camera.open(); // might crash here if native camera app is running
        Camera.Parameters parameters = mCamera.getParameters();
        parameters.setPreviewSize(640, 480);
        parameters.setFocusMode(Camera.Parameters.FOCUS_MODE_INFINITY); // no autofocusing
        parameters.setAutoExposureLock(false); // auto exposure is on
        mCamera.setParameters(parameters);
        mCamera.setDisplayOrientation(90); // rotate to portrait mode

        try {
            mCamera.setPreviewTexture(surface);
            mCamera.startPreview();
        } catch (IOException ioe) {
            // Something bad happened
        }
    }

    public void onSurfaceTextureSizeChanged(SurfaceTexture surface, int width, int height) {
        // Ignored, Camera does all the work for us
    }

    public boolean onSurfaceTextureDestroyed(SurfaceTexture surface) {
        mCamera.stopPreview();
        mCamera.release();
        return true;
    }

    // SeekBars
    private void setMyControlListener() {
        threshold.setOnSeekBarChangeListener(new OnSeekBarChangeListener() {

            @Override
            public void onProgressChanged(SeekBar seekBar, int progress, boolean fromUser) {
                thresh = progress;
                myTextView.setText("Blue Greater Than: "+thresh);
            }

            @Override
            public void onStartTrackingTouch(SeekBar seekBar) {
            }

            @Override
            public void onStopTrackingTouch(SeekBar seekBar) {
            }
        });
    }

    // USB functionality
    private final SerialInputOutputManager.Listener mListener =
            new SerialInputOutputManager.Listener() {
                @Override
                public void onRunError(Exception e) {

                }

                @Override
                public void onNewData(final byte[] data) {
                    MainActivity.this.runOnUiThread(new Runnable() {
                        @Override
                        public void run() {
                            MainActivity.this.updateReceivedData(data);
                        }
                    });
                }
            };

    @Override
    protected void onPause(){
        super.onPause();
        stopIoManager();
        if(sPort != null){
            try{
                sPort.close();
            } catch (IOException e){ }
            sPort = null;
        }
        finish();
    }

    @Override
    protected void onResume() {
        super.onResume();

        ProbeTable customTable = new ProbeTable();
        customTable.addProduct(0x04D8,0x000A, CdcAcmSerialDriver.class);
        UsbSerialProber prober = new UsbSerialProber(customTable);

        final List<UsbSerialDriver> availableDrivers = prober.findAllDrivers(manager);

        if(availableDrivers.isEmpty()) {
            //check
            return;
        }

        UsbSerialDriver driver = availableDrivers.get(0);
        sPort = driver.getPorts().get(0);

        if (sPort == null){
            //check
        }else{
            final UsbManager usbManager = (UsbManager) getSystemService(Context.USB_SERVICE);
            UsbDeviceConnection connection = usbManager.openDevice(driver.getDevice());
            if (connection == null){
                //check
                PendingIntent pi = PendingIntent.getBroadcast(this, 0, new Intent("com.android.example.USB_PERMISSION"), 0);
                usbManager.requestPermission(driver.getDevice(), pi);
                return;
            }

            try {
                sPort.open(connection);
                sPort.setParameters(9600, 8, UsbSerialPort.STOPBITS_1, UsbSerialPort.PARITY_NONE);

            }catch (IOException e) {
                //check
                try{
                    sPort.close();
                } catch (IOException e1) { }
                sPort = null;
                return;
            }
        }
        onDeviceStateChange();
    }

    private void stopIoManager(){
        if(mSerialIoManager != null) {
            mSerialIoManager.stop();
            mSerialIoManager = null;
        }
    }

    private void startIoManager() {
        if(sPort != null){
            mSerialIoManager = new SerialInputOutputManager(sPort, mListener);
            mExecutor.submit(mSerialIoManager);
        }
    }

    private void onDeviceStateChange(){
        stopIoManager();
        startIoManager();
    }

    private void updateReceivedData(byte[] data) {

        // store/display received data as xCar,yCar,theta,lapTime variables
        if (go ==1) {
            String rxString = null;
            try {
                rxString = new String(data, "UTF-8"); // put the data you got into a string
                //sscanf(rxString,"%f %f %f %f",&xCar,&yCar,&theta,&lapTime); // need java equivalent of this
                Scanner scan = new Scanner(rxString);
                float buffer[] = new float[4];
                int i = 0;
                while (scan.hasNextFloat()) {
                    buffer[i] = scan.nextFloat();
                    i++;
                }
                xCar = buffer[0];
                yCar = buffer[1];
                theta = buffer[2];
                lapTime = buffer[3];
                myTextView2.setText(rxString);
            } catch (UnsupportedEncodingException e) {
                e.printStackTrace();
            }
        }
    }

    // the important function
    public void onSurfaceTextureUpdated(SurfaceTexture surface) {
        int M, sum, COM, COMavg, count;
        if (go == 1) {
            M = 0;
            sum = 0;
            COMavg = 0;
            count = 0;
            // every time there is a new Camera preview frame
            mTextureView.getBitmap(bmp);

            final Canvas c = mSurfaceHolder.lockCanvas();
            final Canvas c2 = mSurfaceHolder2.lockCanvas();
            if (c != null) {
                int[] pixels = new int[bmp.getWidth()]; // pixels[] is the RGBA data
                for (int startY = 180; startY <= 300; startY = startY + 20) { // which row in the bitmap to analyze to read
                    bmp.getPixels(pixels, 0, bmp.getWidth(), 0, startY, bmp.getWidth(), 1);
                    // in the row, see if the pixel is blue
                    for (int i = 0; i < bmp.getWidth(); i++) {
                        if (blue(pixels[i]) > thresh) {
                            pixels[i] = rgb(0, 255, 0); // over write the pixel with pure green
                            // COM pixel calculation
                            M = M + 1;
                            sum = sum + i;
                        }
                    }
                    // update the row
                    bmp.setPixels(pixels, 0, bmp.getWidth(), 0, startY, bmp.getWidth(), 1);

                    // calculate COM for each row
                    if (M == 0) {
                        COM = 0;
                    } else {
                        COM = (sum / M) + 1;
                        COMavg += COM;
                        count++;
                    }
                }
                // average the COMs
                if (count == 0) {
                    COMavg = 0;
                } else {
                    COMavg = COMavg / count;
                    // calculate a point on the track based on COMavg, xCar, yCar, theta
                    float l = (float) (0.025*COMavg); // distance from left of frame to the COM (in cm)
                    float d = (float) (Math.sqrt(Math.pow(18.2,2) - Math.pow(l,2))); // distance from the car to the COM (in cm)
                    float theta2 = (float) (Math.atan2(8-l,16.35)); // angle from the car to the COM along a line 16.35 cm from car's position (in radians)
                    xTrack = (int) (xCar*100 + d*Math.cos(theta + theta2));
                    yTrack = 500 - (int) (yCar*100 + d*Math.sin(theta + theta2));
                    // add that point to the track bitmap
                    if (c2 != null) {
                        try {
                            bmp2.setPixel(xTrack, yTrack, 0xffffffff);
                        } catch (Exception e) {}
                    }
                }
                // draw circle at COM
                canvas.drawCircle(COMavg, 240, 5, paint1);
                // write the COM as text
                canvas.drawText("COM = " + COMavg, 10, 30, paint1);
                //send COM to PIC
                String sendString = String.valueOf(COMavg) + '\n';
                try {
                    sPort.write(sendString.getBytes(), 10); // 10 is the timeout
                } catch (IOException e) {}
            }
            c.drawBitmap(bmp, 0, 0, null);
            mSurfaceHolder.unlockCanvasAndPost(c);
            c2.drawBitmap(bmp2, 0, 0, null);
            mSurfaceHolder2.unlockCanvasAndPost(c2);

            // add condition for first lap complete (based on position and lapTime), then calculate racing line and change to go ==  2

        }

        // add if (go == 2) {} statement for once the first lap is complete, then use the racing line for feedback control

    }
}