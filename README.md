# IDEA Hacks 2019

This is the repository for the project I (Matthew Wang) made with four other teammates (Allison Chen, Ashvin Nagarajan, Jeff Anderson, and Juan Banchs) at [IDEA Hacks 2019](http://www.ideahacks.la).

We made a smart lock, powered by an Arduino 101 and a Sunfounder RFID-RC522 reader. We set up our reader using the [Sunfounder Wiki's experiment tutorial](http://wiki.sunfounder.cc/index.php?title=Mifare_RC522_Module_RFID_Reader).

We're still in progress on this project, so get back to us in a bit!

## Project Breakdown

The `main` folder contains `main.ino`, the main program that controls our lock and responds to RFID signals, and the file that is supposed to ship with the product. It has a few functions:
* Controls the servo that locks and unlocks the lock
* Reads the correct RFID IDs that are allowed to lock and unlock the EEPROM - more information in the EEPROM section
* Creates a `BLEPeripheral` object to interface with a Bluetooth Low Energy device, with two characteristics:
	* `lockCharacteristic`, which returns either `0` or `1` for the lock being unlocked or locked, respectively
	* `passwordCharacteristic`, which receives a numeric password from the user; if it's correct, it toggles the lock. The numeric password is hard-coded into the program: it's currently `1337`

The `utilities` folder contains two files, `getId.ino` and `memory_management.ino`, which are designed to help developers use the application.
* `getId.ino` outputs the ID of any RFID device placed on the RFID reader, which is useful for determing the IDs of any RFID objects. We slightly modified the provided `getId.ino` from [Sunfounder](http://wiki.sunfounder.cc/index.php?title=Mifare_RC522_Module_RFID_Reader).
* `memory_management.ino` contains a set of utility functions that allow the user to easily manipulate the EEPROM onboard the device. Currently, this is the only way to add "correct" RFID IDs to the lock.

The `lib` folder contains several library files for the RFID board; **they must be copied into `Arduino/libraries` directory for our code to work properly**. We modified the provided libraries provided from [Sunfounder](http://wiki.sunfounder.cc/index.php?title=Mifare_RC522_Module_RFID_Reader), as they caused compilation errors with our Arduino board.

## Arduino Setup

We used an Arduino 101 (with Bluetooth capability) and a Sunfounder RFID-RC522 reader.

|  Input Pin  | Arduino 101 |
| ----------- | ----------- |
| Mtr GND     | GND |
| Mtr Power   | 5V |
| Mtr Digital | Digital 8 |
| RFID VCC    | 3.3V |
| RFID RST    | Digital 2 |
| RFID GND    | GND |
| RFID MISO   | Digital 3 |
| RFID MOSI   | Digital 4 |
| RFID SCK    | Digital 5 |
| RFID NSS    | Digital 6 |
| RFID IRQ    | Digital 7 |


## EEPROM Allocation

The first four bytes of the EEPROM (0-3) are reserved for parameters for the program. If they are uninitialized (i.e.) their value is `255`), then `main.ino` sets their values in `setupEEPROM()`.

| Byte # | Data Type | Description of Byte |
| ------ | --------- | ------------------- |
| 0      | `unsigned short int` | Holds the state of the lock: `0` is unlocked, `1` is locked. |
| 1      | `unsigned short int` | Holds the number of valid combinations held in memory. |
| 2      | n/a         | Reserved byte for future use. |
| 3      | n/a         | Reserved byte for future use. |

The rest of the available bytes in the EEPROM are used for RFID ID blocks of four bytes, with each block consisting of four one-byte unsigned characters that represent the value of the RFID ID. Assume `0 <= n <= 255`.

| Byte # | Data Type | Description of Byte |
| ------ | --------- | ------------------- |
| 4n     | `unsigned char` | The 1st unsigned char-encoded value of the RFID  |
| 4n+1   | `unsigned char` | The 2nd unsigned char-encoded value of the RFID  |
| 4n+2   | `unsigned char` | The 3rd unsigned char-encoded value of the RFID  |
| 4n+3   | `unsigned char` | The 4th unsigned char-encoded value of the RFID  |

For example, to make the RFID `A34815D3` valid as the first correct RFID, you'd do this:

| Byte # | Data Type | Byte Value |
| ------ | --------- | ---------- |
| 4     | `unsigned char` | 0xA3 |
| 5     | `unsigned char` | 0x48 |
| 6     | `unsigned char` | 0x15 |
| 7     | `unsigned char` | 0xD3 |


More details coming soon :)
