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

    python_clients/raver_plaid.py - Example client which sends rainbow patterns

To build these programs, run "make" and then look in the bin/ directory.


Quickstart
----------

1. If you're using Linux, first get the dependencies you need (Mac users
   skip to step 2):

    apt-get install mesa-common-dev freeglut3-dev

2. Compile and start the GL simulator using the example "Freespace" layout:

    make
    bin/gl_server layouts/freespace.json

3. Then in another terminal window, send colors to the simulator:

    python_clients/raver_plaid.py
