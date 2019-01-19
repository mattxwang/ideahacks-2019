#include <EEPROM.h>
#include"rfid.h"
#include <Servo.h>
//#include <CurieBLE.h>

int addr; // eeprom address

RFID rfid; // rfid object
uchar serNum[5]; // serialnum

Servo lockServo;  // create servo object to control a servo
int pos = 0;    // variable to store the servo position

int lockState;
int numValid;

/*
BLEPeripheral blePeripheral; // create peripheral instance
BLEService ledService("19B10010-E8F2-537E-4F6C-D104768A1214"); // create service
BLECharCharacteristic ledCharacteristic("19B10011-E8F2-537E-4F6C-D104768A1214", BLERead | BLEWrite);
BLECharCharacteristic buttonCharacteristic("19B10012-E8F2-537E-4F6C-D104768A1214", BLERead | BLENotify); // allows remote device to get notifications
*/

// the setup function runs once when you press reset or power the board
void setup() {
  Serial.begin(9600);
  delay(2000);
  int scode = EEPROM.read(0);
  Serial.print("Starting Lock State: ");
  Serial.print(scode);
  Serial.println();
  //setupEEPROM();
  if (scode != 0 && scode != 1){
    setupEEPROM();
  }
  numValid = EEPROM.read(1);
  Serial.print("Valid Combinations: ");
  Serial.print(numValid);
  Serial.println();
  lockState = scode;
  // initialize digital pin LED_BUILTIN as an output.
  pinMode(LED_BUILTIN, OUTPUT);
  
  // RFID
  rfid.begin(7, 5, 4, 3, 6, 2);//rfid.begin(IRQ_PIN,SCK_PIN,MOSI_PIN,MISO_PIN,NSS_PIN,RST_PIN)
  delay(100);
  rfid.init();
  
  // SERVO
  lockServo.attach(9);
  lockServo.write(90);
  
  // BLUETOOTH
  /*
  blePeripheral.setLocalName("ButtonLED");
  blePeripheral.setAdvertisedServiceUuid(ledService.uuid());
  blePeripheral.addAttribute(ledService);
  blePeripheral.addAttribute(ledCharacteristic);
  blePeripheral.addAttribute(buttonCharacteristic);
  ledCharacteristic.setValue(0);
  buttonCharacteristic.setValue(0);
  blePeripheral.begin();
  */
  digitalWrite(LED_BUILTIN, HIGH);
  Serial.println("Done setup!");
}

void setupEEPROM(){
  Serial.println("Setting up EEPROM");
  EEPROM.update(0, 0); // 0 is unlocked, 1 is locked
  EEPROM.update(1, 2); // num of valid combo blocks
  // Combo block 1 - badge
  EEPROM.update(4, 0xA3);
  EEPROM.update(5, 0x48);
  EEPROM.update(6, 0x15);
  EEPROM.update(7, 0xD3);
  // Combo block 2 - card
  EEPROM.update(8, 0x93);
  EEPROM.update(9, 0x88);
  EEPROM.update(10, 0x78);
  EEPROM.update(11, 0x89);
  Serial.println("Done setting up EEPROM");
}

void unlock(){
  Serial.println("Unlocking");
  lockServo.write(180);
  delay(1000);
  lockServo.write(90);
  lockState = 0;
  EEPROM.update(0, lockState);
  Serial.println("Done unlocking!");
}

void lock(){
  Serial.println("Locking");
  lockServo.write(0);
  delay(1000);
  lockServo.write(90);
  lockState = 1;
  EEPROM.update(0,lockState);
  Serial.println("Done locking!");
}

void loop() {
  uchar status;
  uchar str[MAX_LEN];
  status = rfid.request(PICC_REQIDL, str);
  if (status != MI_OK)
  {
    return;
  }
  
  //rfid.showCardType(str);
  status = rfid.anticoll(str);

  if (status == MI_OK) {
    Serial.print("RFID Number: ");
    memcpy(serNum, str, 5);
    rfid.showCardID(serNum);
    Serial.println();
    uchar* id = serNum;
    addr = 4;
    bool found = false;
    for (int i = 0; i < numValid; i++){
      if(
        id[0]==EEPROM.read(4*i+addr)   &&
        id[1]==EEPROM.read(4*i+addr+1) &&
        id[2]==EEPROM.read(4*i+addr+2) &&
        id[3]==EEPROM.read(4*i+addr+3)){
          found = true;
          Serial.println("Match found!");
          if (lockState == 1){
            unlock();
          }
          else{
            lock();
          }
          break;
       }
    }
    if (!found){
      Serial.println("Not correct!");
    }
  }
  
  delay(500);
  rfid.halt(); //command the card into sleep mode 
}
