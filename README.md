# IDEA Hacks 2019

This is the repository for the project I (Matthew Wang) made with four other teammates (Allison Chen, Ashvin Nagarajan, Jeff Anderson, and Juan Banchs) made at [IDEA Hacks 2019](http://www.ideahacks.la).

We made a smart lock, powered by an Arduino 101 and a Sunfounder RFID-RC522 reader. We set up our reader using the [Sunfounder Wiki's experiment tutorial](http://wiki.sunfounder.cc/index.php?title=Mifare_RC522_Module_RFID_Reader).

We're still in progress on this project, so get back to us in a bit!

## Project Breakdown

The `getID` folder contains `getId.ino`, which is an ArduinoC file that outputs the ID of any RFID device placed on the RFID reader. We slightly modified the provided `getId.ino` from [Sunfounder](http://wiki.sunfounder.cc/index.php?title=Mifare_RC522_Module_RFID_Reader).

The `lib` folder contains several library files for the RFID board; **they must be copied into `Arduino/libraries` directory for our code to work properly**. We modified the provided libraries provided from [Sunfounder](http://wiki.sunfounder.cc/index.php?title=Mifare_RC522_Module_RFID_Reader), as they caused compilation errors with our Arduino board.

The `main` folder contains `main.ino`, the main program that controls our lock and responds to RFID signals. As it functions currently, the correct RFID ids are hard-coded into the file.

More details coming soon :)
