package img2opc;
import processing.core.*;
import processing.net.*;
import java.awt.Color.*;

/*
 * This library sends images to an OpenPixelControl client.
 * Width and height arguments passed to the constructor can specify display
 * size, but currently it assumes a flat display of zigzagging strings like so:
 *
 *   pixel 0     2h-1  2h
 *         1     .     .
 *         2     .     .
 *         .     .     .
 *         .     .     etc.
 *         .     h+1
 *         h-1   h
 */

public class Img2Opc extends PApplet implements PConstants {
  PApplet parent;
  int dispWidth;
  int dispHeight;
  PImage resizedFrame;
  int srcx, srcy, srcw, srch;
  byte[] opcData;
  Client client;
  byte[] gamma;

  public Img2Opc(PApplet parent, String host, int port, int w, int h) {
    this.parent = parent;
    dispWidth = w;
    dispHeight = h;

    gamma = new byte[256];
    for (int i = 0; i < 256; i++) {
      if (true) {
        gamma[i] = (byte)(Math.pow((float)(i) / 255.0, 2.5) * 255.0 + 0.5);
      } else {
        gamma[i] = (byte)(i);
      }
    }

    resizedFrame = new PImage(dispWidth, dispHeight, RGB);

    int numBytes = dispWidth * dispHeight * 3;
    opcData = new byte[4 + numBytes];
    // Channel: 0
    opcData[0] = 0;
    // Command: 0
    opcData[1] = 0;
    // numBytes high and low
    opcData[2] = (byte)((numBytes >> 8) & 0xFF);
    opcData[3] = (byte)(numBytes & 0xFF);

    setSourceSize(dispWidth, dispHeight);

    client = new Client(parent, host, port);
    // The server will hang up after a short period of inactivity.
    // Send a blank image and hope sendImg() is called soon.
    sendImg(new PImage(dispWidth, dispHeight));
  }

  public void setSourceSize(int w, int h) {
    // TODO: handle other sizes.
    srch = h;
    srcw = h;
    srcx = (w - srcw) / 2;
    srcy = 0;
  }

  public PImage sendImg(PImage m) {
    resizedFrame.copy(m, srcx, srcy, srcw, srch, 0, 0, resizedFrame.width, resizedFrame.height);

    for (int x = 0; x < dispWidth; x++) {
      for (int y = 0; y < dispHeight; y++) {
        int c = resizedFrame.pixels[x + y * dispWidth];
        int pixelPos = 4 + 3 * (x * dispHeight + ((x % 2 == 0) ? y : (dispHeight - 1 - y)));
        opcData[pixelPos + 0] = gamma[(byte)(c >> 16 & 0xFF) & 0xFF];
        opcData[pixelPos + 1] = gamma[(byte)(c >> 8  & 0xFF) & 0xFF];
        opcData[pixelPos + 2] = gamma[(byte)(c >> 0  & 0xFF) & 0xFF];
      }
    }
    if (client != null) {
      if (client.output != null) {
        try {
          client.output.write(opcData);
          client.output.flush();
        } catch (Exception e) {
          e.printStackTrace();
        }
      }
    }

    return resizedFrame;
  }
}
