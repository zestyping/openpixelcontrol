openpixelcontrol
================

A simple stream protocol for controlling RGB lighting, particularly RGB LEDs.

This source repository provides a reference implementation of the protocol
and a few simple programs:

    dummy_client - Sends OPC commands for the RGB values that you type in.

    dummy_server - Receives OPC commands from a client and prints them out.

    gl_server (Mac only) - Receives OPC commands from a client and visualizes
        the LED pixels in an OpenGL simulator.  Takes a "layout file" that
        specifies the locations of the LED pixels; each line of the file
        gives the (x, y, z) coordinates for a pixel as three floating-point
        numbers separated by spaces.

    tcl_server - Receives OPC commands from a client and uses them to control
        Total Control Lighting pixels (see http://coolneon.com/) that are
        connected to the SPI port on a Beaglebone.

    python_clients/raver_plaid.py - Example client which sends rainbow patterns

To build these programs, run "make" and then look in the bin/ directory.

Quickstart
----------

Compile and start the GL simulator using the example "wall" layout

    make
    ./bin/gl_server layouts/wall.l

And then in another shell, send colors to the simulator:

    ./python_clients/raver_plaid.py

