import processing.video.*;
import processing.net.*;
import img2opc.*;

/*
 * This sketch uses the img2opc library to display input from a connected
 * webcam.
 */

Capture cam;
Img2Opc i2o;

int displayWidth = 25;
int displayHeight = 25;

void setup() {
  background(0);
  size(640, 480);

  cam = new Capture(this, 640, 480, 30);
  cam.start();

  int target = millis() + 1000;
  while (millis() < target) { }

  i2o = new Img2Opc(this, "127.0.0.1", 7890, displayWidth, displayHeight);
  i2o.setSourceSize(640, 480);
}

void draw() {
  if (cam.available() == true) {
    set(0, 0, cam);

    new PImage(displayWidth, displayHeight, RGB);
    cam.read();

    PImage preview = i2o.sendImg(cam);

    image(preview, 0, 0);
  }
}

