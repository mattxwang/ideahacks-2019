# pop 'n lock - IDEA Hacks 2019

![Logo](popnlock.png)

**pop 'n lock was the winning project at IDEA Hacks 2019!**

*Note (2023)*: some of the old documentation links have been lost to link rot. I've done my best to upload various videos (the `.mp4` files in this repository) and a PDF copy of our slides. This project should be relatively replicable given solely the materials in this repository, though I believe this particular approach no longer works for Bruin Cards (which have since updated their security protocol). Thank you to Juan Banchs and Jeff Anderson for animating the original videos in 2019.

This is the repository for the project I (Matthew Wang) made with four other teammates (Allison Chen, Ashvin Nagarajan, Jeff Anderson, and Juan Banchs) at [IDEA Hacks 2019](http://www.ideahacks.la).

Our idea was a smart lock, one that locks and unlocks when an RFID tag (which could be a physical tag, a Bruincard, or an array of other devices) is put near the lock. Compared to other smart locks, ours was designed to be more power and resource efficient, be more tamper-proof (i.e. the lock mechanism cannot be shimmed and the lock stays locked when out of power), and be more convenient (no app required).

Our project was powered by an Arduino 101, a Sunfounder RFID-RC522 reader, and a small servo. In addition to those electronic components, our team also used CAD software to design and 3D print our own custom lock housing and mechanism. We also added LEDs to indicate the status of the lock, as well as a prototype of Bluetooth interaction with the lock (which does include a new vector of attack), possibly for two-factor authentication in the future.

## Project Breakdown/File Structure

The `main` folder contains `main.ino`, the main program that controls our lock and responds to RFID signals, and the file that is supposed to ship with the product. It has a few functions:
* Controls the servo that locks and unlocks the lock.
* Reads the correct RFID IDs that are allowed to lock and unlock the EEPROM - more information in the EEPROM section.
* Controls the LEDs that show the lock status - unlocked is green, locked is red.
* Manages the creation of new valid RFIDs - if an RFID object is read in while the lock is unlocked, it writes it to the EEPROM and locks the lock.
* Creates a `BLEPeripheral` object to interface with a Bluetooth Low Energy device, with two characteristics:
	* `lockCharacteristic`, which returns either `0` or `1` for the lock being unlocked or locked, respectively
	* `passwordCharacteristic`, which receives a numeric password from the user; if it's correct, it toggles the lock. The numeric password is hard-coded into the program: it's currently `1337`

The `utilities` folder contains two files, `getId.ino` and `memory_management.ino`, which are designed to help developers use the application.
* `getId.ino` outputs the ID of any RFID device placed on the RFID reader, which is useful for determing the IDs of any RFID objects. We slightly modified the provided `getId.ino` from [Sunfounder](http://wiki.sunfounder.cc/index.php?title=Mifare_RC522_Module_RFID_Reader).
* `memory_management.ino` contains a set of utility functions that allow the user to easily manipulate the EEPROM onboard the device.

The `lib` folder contains several library files for the RFID board; **they must be copied into `Arduino/libraries` directory for our code to work properly**. We modified the provided libraries provided from [Sunfounder](http://wiki.sunfounder.cc/index.php?title=Mifare_RC522_Module_RFID_Reader), as they caused compilation errors with our Arduino board.

## Arduino Setup

We used an Arduino 101 (with Bluetooth capability) and a Sunfounder RFID-RC522 reader. We set up our reader using the [Sunfounder Wiki's experiment tutorial](http://wiki.sunfounder.cc/index.php?title=Mifare_RC522_Module_RFID_Reader).

|  Input Pin  | Arduino 101 |
| ----------- | ----------- |
| Servo GND   | GND |
| Servo Power | 5V |
| Servo Digital | Digital 8 |
| RFID VCC    | 3.3V |
| RFID RST    | Digital 2 |
| RFID GND    | GND |
| RFID MISO   | Digital 3 |
| RFID MOSI   | Digital 4 |
| RFID SCK    | Digital 5 |
| RFID NSS    | Digital 6 |
| RFID IRQ    | Digital 7 |
| LED Green   | Digital 9 |
| LED Red     | Digital 10|
| Power Positive | vin |
| Power Negative | GND |


## EEPROM Allocation

The first four bytes of the EEPROM (0-3) are reserved for parameters for the program. If they are uninitialized (i.e.) their value is `255`), then `main.ino` sets their values in `setupEEPROM()`.

| Byte # | Data Type | Description of Byte |
| ------ | --------- | ------------------- |
| 0      | `unsigned short int` | Holds the state of the lock: `0` is unlocked, `1` is locked. |
| 1      | `unsigned short int` | Holds the number of valid combinations held in memory. |
| 2      | n/a         | Reserved byte for future use. |
| 3      | n/a         | Reserved byte for future use. |

The rest of the available bytes in the EEPROM are used for RFID ID blocks of four bytes, with each block consisting of four one-byte unsigned characters that represent the value of the RFID ID. Assume `1 <= n <= 255`.

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

Which would look like this in code:

```cpp
// memory_management.ino
insertRfidString(4, 0xA3, 0x48, 0x15, 0xD3);
```

## Lock Mechanism

We used a ball-bearing lock mechanism, which as the advantage of having the entire locking mechanism within the lock body - therefore, there's no physical lock to pick! Our slide deck had a good illustration of how this worked; the videos in this repository are the best aid I could find.
