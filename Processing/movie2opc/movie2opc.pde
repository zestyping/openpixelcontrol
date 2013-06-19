import img2opc.*;
import processing.video.*;
import processing.net.*;

/*
 * This sketch uses the img2opc library to display video files in the data
 * directory at random.
 */

Movie m;
Img2Opc i2o;
PImage resized;
int displayWidth = 25;
int displayHeight = 25;
Movie endingMovie = null;

void setup() {

  background(0);
  size(1024, 768);

  i2o = new Img2Opc(this, "127.0.0.1", 7890, displayWidth, displayHeight);
  loadMovie();
}

void mousePressed() {
  if (endingMovie != null) {
    endingMovie.stop();
    endingMovie = null;
  }
  endingMovie = m;
  loadMovie();
  endingMovie.stop();
  endingMovie = null;
}

void loadMovie() {
  m = null;

  java.io.File folder = new java.io.File(dataPath(""));
  String[] filenames = folder.list();

  String f;
  do {
   f = filenames[int(random(filenames.length))]
  } while (f.equals(".DS_Store"));
  print("loading '");
  print(f);
  println("'");
  m = new Movie(this, f);
  m.play();
  m.read();
  i2o.setSourceSize(m.width, m.height);
}

void movieEvent(Movie mov) {
  if (mov == endingMovie) {
    return;
  }
  mov.read();
  resized = i2o.sendImg(mov);

  if (mov.time() >= mov.duration() - 0.1) {
    if (endingMovie != null) {
      endingMovie.stop();
      endingMovie = null;
    }
    endingMovie = mov;
    loadMovie();
  }
}

void draw() {
  if (m != null) {
    image(m, displayWidth, 0);
  }
  if (resized != null) {
    image(resized, 0, 0);
  }
}
