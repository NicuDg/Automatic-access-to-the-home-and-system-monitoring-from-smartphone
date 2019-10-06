package com.example.remotecontrol;

import android.Manifest;
import android.location.Location;
import android.os.Handler;
import android.support.v4.app.ActivityCompat;
import android.support.v7.app.AppCompatActivity;
import android.os.Bundle;
import android.util.Log;
import android.view.View;
import android.widget.Button;
import android.widget.TextView;
import android.widget.Toast;

import com.android.volley.Request;
import com.android.volley.Response;
import com.android.volley.VolleyError;
import com.android.volley.toolbox.JsonObjectRequest;
import com.android.volley.toolbox.Volley;
import com.google.gson.Gson;
import com.google.gson.GsonBuilder;
import com.google.gson.reflect.TypeToken;


import org.json.JSONObject;
import java.util.Arrays;

public class MainActivity extends AppCompatActivity {

    TextView LightStatus, BarrierStatus, GateStatus, DoorStatus, GarageStatus;
    Button btnOn, btnOff, btnOpenBarrier, btnCloseBarrier, btnOpenGate,
            btnCloseGate, btnOpenDoor, btnCloseDoor;
    private Handler handler = new Handler();
    private Handler handler1 = new Handler();
    double dist;
    boolean stop = false;

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_main);

        btnOn = (Button) findViewById(R.id.buttonON);
        btnOff = (Button) findViewById(R.id.buttonOFF);
        btnOpenBarrier = (Button) findViewById(R.id.buttonOpen);
        btnCloseBarrier = (Button) findViewById(R.id.buttonClose);
        btnOpenGate = (Button) findViewById(R.id.buttonOpenGate);
        btnCloseGate = (Button) findViewById(R.id.buttonCloseGate);
        btnOpenDoor = (Button) findViewById(R.id.buttonOpenDoor);
        btnCloseDoor = (Button) findViewById(R.id.buttonCloseDoor);

        LightStatus = (TextView) findViewById(R.id.textViewLightStatus);
        BarrierStatus = (TextView) findViewById(R.id.textBarrierStatus);
        GateStatus = (TextView) findViewById(R.id.textViewGateStatus);
        DoorStatus = (TextView) findViewById(R.id.textViewDoorStatus);
        GarageStatus = (TextView) findViewById(R.id.textViewGarageStatus);



        ActivityCompat.requestPermissions(MainActivity.this, new String[] {Manifest.permission.ACCESS_FINE_LOCATION}, 123);
        try{
            handler2();
        }
        catch (Exception e){

        }



        try {
            handler1();
        }
        catch (Exception e){
            Log.d("eroare inchide","suka");
            Log.e("Erorr", Arrays.toString(e.getStackTrace()));
            e.printStackTrace();
        }



        btnOn.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View v) {
                arduino("http://192.168.1.102/?cmd=1");
            }
        });

        btnOff.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View v) {
                arduino("http://192.168.1.102/?cmd=2");
            }
        });

        btnOpenBarrier.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View v) {
                arduino("http://192.168.1.102/?cmd=3");

            }
        });

        btnCloseBarrier.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View v) {
                arduino("http://192.168.1.102/?cmd=4");
            }
        });

        btnOpenGate.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View v) {
                arduino("http://192.168.1.102/?cmd=5");
            }
        });

        btnCloseGate.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View v) {
                arduino("http://192.168.1.102/?cmd=6");
            }
        });

        btnOpenDoor.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View v) {
                arduino("http://192.168.1.102/?cmd=7");
            }
        });

        btnCloseDoor.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View v) {
                arduino("http://192.168.1.102/?cmd=8");
            }
        });

    }


    public void arduino(String url){
        try {
            JsonObjectRequest jsonObjReq = new JsonObjectRequest(Request.Method.GET, url, null,
                    new Response.Listener<JSONObject>() {
                        @Override
                        public void onResponse(JSONObject response) {
                            //
                        }
                    },
                    new Response.ErrorListener() {
                        @Override
                        public void onErrorResponse(VolleyError error) {
                            //
                        }
                    });

            Volley.newRequestQueue(this.getApplicationContext()).add(jsonObjReq);
        }catch(Exception e){
            Log.e("Log", Arrays.toString(e.getStackTrace()));
            e.printStackTrace();
        }
        finally {

        }
    }

    public void click() {
        try {
            String url = "http://192.168.1.102/";

            JsonObjectRequest jsObjRequest = new JsonObjectRequest
                    (Request.Method.GET, url, null, new Response.Listener<JSONObject>() {
                        @Override
                        public void onResponse(JSONObject response) {
                            Gson gson = new GsonBuilder().create();
                            Status status = gson.fromJson(response.toString(), new TypeToken<Status>() {
                            }.getType());
                            LightStatus.setText(status.lightStatus());
                            BarrierStatus.setText(status.barrierStatus());
                            GateStatus.setText(status.gateStatus());
                            DoorStatus.setText(status.doorStatus());
                            GarageStatus.setText(status.garageStatus());
                        }
                    }, new Response.ErrorListener() {
                        @Override
                        public void onErrorResponse(VolleyError error) {
                        }
                    });

            Volley.newRequestQueue(this.getApplicationContext()).add(jsObjRequest);
        } catch(Exception e){
            Log.e("Log", Arrays.toString(e.getStackTrace()));
            e.printStackTrace();
        }
        finally {

        }
    }

    public void handler1() {
        handler.postDelayed(new Runnable() {
            public void run() {
                click();          // this method will contain your almost-finished HTTP calls
                handler.postDelayed(this, 1000);
            }
        }, 1000);
    }

    public void handler2(){
        handler.postDelayed(new Runnable() {
            public void run() {
                coord();          // this method will contain your almost-finished HTTP calls
                if(!stop) {
                    handler.postDelayed(this, 2000);
                }
            }
        }, 2000);
    }


    public double distance(double lat, double lon){
        double lat1 = 46.78209195;
        double lon1 = 23.62096608;
        dist = Math.sqrt(Math.pow(lat-lat1, 2) + (Math.pow(lon-lon1,2)))*110000;
        return dist;
    }

    public void coord(){
        GpsTracker gt = new GpsTracker(getApplicationContext());
        Location l = gt.getLocation();
        if( l == null){
            Toast.makeText(getApplicationContext(),"GPS unable to get Value",Toast.LENGTH_SHORT).show();
        }else {
            double lat = l.getLatitude();
            double lon = l.getLongitude();
            distance(lat, lon);
            //Toast.makeText(getApplicationContext(),"GPS Lat = "+lat+"\n lon = "+lon,Toast.LENGTH_SHORT).show();
        }
        if(dist < 10 ){
            //Toast.makeText(getApplicationContext(),"distance "+dist,Toast.LENGTH_SHORT).show();
            arduino("http://192.168.1.102/?cmd=9");
            stop = true;
        }
        else
        {
            //Toast.makeText(getApplicationContext(),"departe " + dist,Toast.LENGTH_SHORT).show();
        }
    }
}
