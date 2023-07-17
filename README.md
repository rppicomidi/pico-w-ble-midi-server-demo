# pico-w-ble-midi-server-demo
A Bluetooth LE MIDI 1.0 server peripheral demo for the Raspberry Pi Pico W

This project uses a serial port terminal to demonstrate BLE-MIDI 1.0
communication between a Raspberry Pi Pico W board and a BLE-MIDI 1.0
client application. The terminal program prints out advertising and
connection status. It prints out in hex timestamped MIDI 1.0
messages that the client sends to the Pico W. It also accepts a
terminal command `send` that allows you to send arbitrary MIDI 1.0
message byte streams from the Pico W to the client.

# Hardware
To use this demo, you need a Raspberry Pi Pico W (or equivalent)
and some sort of serial port console. I have only tested using
a Picoprobe as a UART to USB serial interface. See Appendix A
of the [Getting started with Raspberry Pi Pico](https://datasheets.raspberrypi.com/pico/getting-started-with-pico.pdf)
document.

# Software
Make sure you have installed the `pico-sdk` toolchain and all libraries
as described in [Getting started with Raspberry Pi Pico](https://datasheets.raspberrypi.com/pico/getting-started-with-pico.pdf).
Read the software license. 

# A word about the software license
Although the original code I have written has a MIT license, the lines of code I
copied from the BlueKitchen software examples, and the underlying  BlueKitchen stack,
have a much more restrictive license. If you plan to do something commercial
with this code, consult with BlueKitchen per the instructions in their license.

# Build instructions
The instructions below have been tested on an Ubuntu Linux 22.04 LTS
system terminal window. They assume that you have already installed
`pico-sdk` version 1.5.1 or later in the directory `${PICO_SDK_PATH}`.
They also assume you want to install the source code for this
project under the directory `${PROJ_PATH}`. 

## Fetch source code
```
cd ${PROJ_PATH}
git clone https://github.com/rppicomidi/pico-w-ble-midi-server-demo
cd pico-w-ble-midi-server-demo
git submodule update --init
mkdir build
```
This project relies on git submodules to get the source code for the
libraries `pico-w-ble-midi-service`, `ring-buffer-lib`, and `embedded-cli`,
so do not neglect the `git submodule update` line or else the project
cannot build.

## Set up your build environment
You will need to execute the following two lines at least once per
terminal window that will doing the build
```
export PICO_SDK_PATH=[path to your pico-sdk directory]
export PICO_BOARD=pico_w
```

## Build the code
```
cd ${PROJ_PATH}/pico-w-ble-midi-server-demo/build
cmake ..
make

```
You may see a number of warning reports when it builds `pico-sdk` code.
You may safely ignore them.

## Using VisualStudio Code
This project contains a `.vscode` directory if you choose to build and
run your code using VS Code. Use `File->Open Folder...` to open your `${PROJ_PATH}`
folder.

# Running the code
Use whatever method you like to download the code to the Pico W board.
Either connect a serial port console to pins 1 and 2 of the Pico W board,
or use the USB serial port and open a `minicom` or equivalent terminal.
The terminal will display useful startup messages. Once the Pico W has
successfully booted, it will start advertising itself as `BLE-MIDI Demo`.

# Connecting from a BLE-MIDI Client
This is not a straightforward as it should be. On an iPad or iPhone, the either
the app you are using has to support connecting to a BLE-MIDI server,
or you need to use an app that creates a MIDI device other apps can use
For example, the MIDIWrench app for iOS will allow you to connect your Apple device to
the BLE-MIDI Demo server and then allows other apps to use the BLE-MIDI Demo server
as if it were any other MIDI device.

On MacOS Monterey, you need to use [Audio MIDI Setup](https://support.apple.com/guide/audio-midi-setup/set-up-bluetooth-midi-devices-ams33f013765/3.5/mac/12.0)
to get the Bluetooth connection. Once the BLE-MIDI Demo server is connected, the
MIDI port is available to other MacOS programs.

I have tested on Android 13 using the TouchDAW app. It is not a free app. If
you have it and your device is running Android 12 or later, then the connection
routine is described in [the TouchDAW manual](https://xmmc.de/touchdaw/man_midi_ble.htm).
There used to be free Android apps for connecting to a BLE-MIDI server like
Android Bluetooth Connect; these apps worked just like MIDIWrench on iOS.
The apps do not seem to be available for my phone anymore.

The BLE-MIDI feature of the Linux BlueZ stack is considered experimental.
Ubuntu 22.04LTS, which is what I normally use for development, builds BlueZ
with BLE-MIDI support disabled. At the time of this writing, I have not
tried replacing the distribution's BlueZ stack with a custom build. The
Internet claims that should be all you need to do to get Linux to work using
the native Bluetooth.

Windows requires a custom driver to connect a device to BLE MIDI. Writing such a driver
is beyond the scope of this project.

The other client I have tried is a CME WIDI BUD. The WIDI BUD connects to the
BLE-MIDI demo and provides a standard USB interface to my Linux workstation or
to my Windows PC.

# Sending data form the BLE-MIDI demo server to the client
Once the client has connected to the server, you can use the serial port
CLI command `send` to send a string hex MIDI bytes to the client.
For example, to send  `Note On` message on channel 1, note number
64, velocity 127, you would enter the following command on the terminal
command line:
```
send 90 40 7F
```
If the client sends a MIDI command to the BLE-MIDI Demo server, then
the serial port CLI will display `TS:(a timestamp)` followed by the MIDI
byte stream (in hex) that the client sent.
