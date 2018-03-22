openpixelcontrol
================

A simple stream protocol for controlling RGB lighting, particularly RGB LEDs.
See http://openpixelcontrol.org/ for a spec.

Using this implementation, you can write your own patterns and animations,
test them in a simulator, and run them on real RGB light arrays.  This
repository includes these programs:

* `dummy_client`: Sends OPC commands for the RGB values that you type in.

* `dummy_server`: Receives OPC commands from a client and prints them out.

* `gl_server` (Mac or Linux only): Receives OPC commands from a client and
  displays the LED pixels in an OpenGL simulator.  Takes a "layout file"
  that specifies the locations of the pixels in a JSON array; each item
  in the array should be a JSON object of the form {"point": [x, y, z]}
  where x, y, z are the coordinates of the pixel in space.  Click and drag
  to rotate the 3-D view; hold shift and drag up or down to zoom.

* `tcl_server`: Receives OPC commands from a client and uses them to
  control Total Control Lighting pixels (see http://coolneon.com/) that
  are connected to the SPI port on a Beaglebone.

* `python/opc.py`: A Python client library for connecting and sending pixels.

* `python/color_utils.py`: A Python library for manipulating colors.

* `python/raver_plaid.py`: An example client that sends rainbow patterns.

To build these programs, run "make" and then look in the bin/ directory.


Quickstart (simulator on Linux/OSX)
-----------------------------------

**Step 1.** If you're using Linux, first get the dependencies you need
(**OSX users skip to step 2**):

    apt-get install mesa-common-dev freeglut3-dev

**Step 2.** Compile and start the GL simulator using the example "Freespace" layout:

    make
    bin/gl_server -l layouts/freespace.json

**Step 3.** In another terminal window, send colors to the simulator:

    python/raver_plaid.py


Quickstart (simulator on Windows)
---------------------------------

Install CMake: https://cmake.org/
Install Visual Studio: https://www.visualstudio.com/

NOTE! Assuming use of **git bash** below, but normal **cmd.exe** works as well.

**Step 1.** Download and unpack GLUT in a known path (For example `c:\glut\`):

https://www.opengl.org/resources/libraries/glut/glutdlls37beta.zip
Or google for "glut windows" if above URL is out of date. You want `glut32.lib`, `glut32.dll` and `glut.h`.

**Step 2.** Generate the CMake project, specify the **full absolute path** to the directory were you unpacked GLUT:

    mkdir build
    cd build
    cmake -DGLUT_DIR=c:/glut/ ..  # NOTE that you should use / not \ for the paths!

**Step 3.** Open the Visual Studio Solution and build:

    start openpixelcontrol.sln

**Step 4.** After compiling you can run it in the terminal window (assuming default build type Debug):

    bin/Debug/gl_server.exe -l ../layout/freespace.json

**Step 6.** In another terminal window, send colors to the simulator:

    python/raver_plaid.py


Quickstart (Beaglebone)
-----------------------

**Step 1.** Log in to your Beaglebone and add these two lines to the
`/boot/uEnv.txt` file.

    cape_disable=capemgr.disable_partno=BB-BONELT-HDMI,BB-BONELT-HDMIN
    cape_enable=capemgr.enable_partno=BB-SPIDEV0,BB-SPIDEV1

**Step 2.** Type `reboot` to reboot your Beaglebone.  You should now
see two files at `/dev/spidev1.0` and `/dev/spidev2.0`.

**Step 3.** Copy the code to your Beaglebone and compile the server
appropriate for your LED chipset; pick one:

    make bin/apa102_server   # APA102 (Adafruit "DotStar")
    make bin/lpd8806_server  # LPD8806, 21-bit colour
    make bin/tcl_server      # P9813 ("Total Control Lighting"), 24-bit colour
    make bin/ws2801_server   # WS2801, 24-bit colour

**Step 4.** Connect the ground, data, and clock wires of your LED strand
to the appropriate breakout pins on the Beaglebone.  For `/dev/spidev1.0`,
data is P9 pin 18 and clock is P9 pin 22.  For `/dev/spidev2.0`, data
is P9 pin 30 and clock is P9 pin 31.  Ground is on P9 pin 1. See
http://beagleboard.org/Support/bone101/#headers-spi for a nice picture.

**Step 5.** Connect the power wire of your LED strand _either_ to a
separate 5V power source, _or_ to your Beaglebone's power on P9 pin 5;
choose one or the other, not both.  If you are using USB to power your
Beaglebone and also using your Beaglebone to power the LEDs, there will
only be enough power to light a small number of LEDs at full brightness
at any given moment.  If you need to light a lot of LEDs and are adding
external power sources to provide more power down the line, use _only_
external power sources and do not connect to the Beaglebone's power pin.

**Step 6.** Run the server on your Beaglebone, specifying the SPI speed,
port number, and device path you want to use.  For example, to run the
TCL server at 8 MHz (the default SPI speed) on port 7890 (the default
port), controlling an LED strand connected to P9 pins 18 and 22:

    bin/tcl_server 8 7890 /dev/spidev1.0

**Step 7.** Run a client on the Beaglebone to make it send data to itself
(the default server address is 127.0.0.1:7890):

    python/raver_plaid.py

**Step 8.** Run a client on your laptop to send data to the Beaglebone
(if you're using a USB network connection, the Beaglebone's address
is most likely 192.168.7.2; otherwise substitute the Beaglebone's
IP address):

    python/raver_plaid.py 192.168.7.2:7890

