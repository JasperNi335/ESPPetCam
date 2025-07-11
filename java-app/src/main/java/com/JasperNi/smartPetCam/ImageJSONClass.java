package com.JasperNi.smartPetCam;

public class ImageJSONClass {
    private String device;
    private String image;

    public ImageJSONClass(String device, String image){
        this.device = device;
        this.image = image;
    }

    public String getImage(){
        return image;
    }

    public String getDevice(){
        return device;
    }

}
