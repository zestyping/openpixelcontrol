openpixelcontrol
================

A simple stream protocol for controlling RGB lighting, particularly RGB LEDs.

This source repository provides a reference implementation of the protocol
and a few simple programs:

    dummy_client - Sends OPC commands for the RGB values that you type in.

    dummy_server - Receives OPC commands from a client and prints them out.

    gl_server (Mac or Linux only) - Receives OPC commands from a client and
        displays the LED pixels in an OpenGL simulator.  Takes a "layout file"
        that specifies the locations of the pixels in a JSON array; each item
        in the array should be a JSON object of the form {"point": [x, y, z]}
        where x, y, z are the coordinates of the pixel in space.

    tcl_server - Receives OPC commands from a client and uses them to control
        Total Control Lighting pixels (see http://coolneon.com/) that are
        connected to the SPI port on a Beaglebone.

To build these programs, run "make" and then look in the bin/ directory.

NOTE: On Linux, "apt-get install mesa-common-dev freeglut3-dev" will get the
      dependencies you need to run "make".
