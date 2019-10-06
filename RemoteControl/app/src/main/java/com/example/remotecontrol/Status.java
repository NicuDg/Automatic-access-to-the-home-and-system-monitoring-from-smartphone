package com.example.remotecontrol;

import com.google.gson.annotations.Expose;
import com.google.gson.annotations.SerializedName;

public class Status {
    @Expose
    @SerializedName("status_1")
    private String status1;
    @SerializedName("status_2")
    private String status2;
    @SerializedName("status_3")
    private String status3;
    @SerializedName("status_4")
    private String status4;
    @SerializedName("status_5")
    private String status5;



    public Status(String status1, String status2, String status3, String status4, String status5) {
        this.status1 = status1;
        this.status2 = status2;
        this.status3 = status3;
        this.status4 = status4;
        this.status5 = status5;
    }

    public String lightStatus() {
        return status1;
    }

    public String barrierStatus() {

        return status2;
    }

    public String gateStatus(){

        return status3;
    }

    public String doorStatus(){

        return status4;
    }

    public String garageStatus(){

        return status5;
    }

}
