#include <EEPROM.h>
#include "rfid.h"
#include <Servo.h>
#include <CurieBLE.h>

//uncomment this for dev mode
//#define DEVMODE 1

int addr; // eeprom address

RFID rfid; // rfid object
uchar serNum[5]; // serialnum

Servo lockServo;  // create servo object to control a servo
int pos = 0;    // variable to store the servo position

int lockState;
int numValid;
/*
String inputString = "";
String command = "";
String value = "";
String password = "enginefars";
boolean stringComplete = false;
*/

int numPass = 1337;

int redPin = 10;
int greenPin = 9;


BLEPeripheral blePeripheral; // create peripheral instance
BLEService lockService("19B10010-E8F2-537E-4F6C-D104768A1214");
BLECharCharacteristic lockCharacteristic("19B10011-E8F2-537E-4F6C-D104768A1214", BLERead | BLENotify);
BLEIntCharacteristic passwordCharacteristic("19B10012-E8F2-537E-4F6C-D104768A1214", BLERead | BLEWrite);

void setup() {
  #if defined(DEVMODE)
  Serial.begin(9600);
  delay(2000);
  #endif
  int scode = EEPROM.read(0);
  #if defined(DEVMODE)
  Serial.print("Starting Lock State: ");
  Serial.print(scode);
  Serial.println();
  //setupEEPROM();
  #endif
  if (scode != 0 && scode != 1){
    setupEEPROM();
  }
  numValid = EEPROM.read(1);
  #if defined(DEVMODE)
  Serial.print("Valid Combinations: ");
  Serial.print(numValid);
  Serial.println();
  #endif
  lockState = scode;

  pinMode(LED_BUILTIN, OUTPUT);
  pinMode(redPin, OUTPUT);
  pinMode(greenPin, OUTPUT);
  
  // RFID
  rfid.begin(7, 5, 4, 3, 6, 2);//rfid.begin(IRQ_PIN,SCK_PIN,MOSI_PIN,MISO_PIN,NSS_PIN,RST_PIN)
  delay(100);
  rfid.init();
  
  // SERVO
  lockServo.attach(8);
  if (lockState == 1){
    lockServo.write(0);
    digitalWrite(redPin, HIGH);
    digitalWrite(greenPin, LOW);
  }
  else{
    digitalWrite(redPin, LOW);
    digitalWrite(greenPin, HIGH);
  }
  //lockServo.write(90);
  
  // BLUETOOTH

  blePeripheral.setLocalName("EFSmartLock");
  blePeripheral.setAdvertisedServiceUuid(lockService.uuid());
  blePeripheral.addAttribute(lockService);
  blePeripheral.addAttribute(lockCharacteristic);
  blePeripheral.addAttribute(passwordCharacteristic);
  lockCharacteristic.setValue(lockState);
  passwordCharacteristic.setValue(0);
  blePeripheral.begin();

  /*
  inputString.reserve(50);
  command.reserve(50);
  value.reserve(50);
  */
  
  digitalWrite(LED_BUILTIN, HIGH);
  #if defined(DEVMODE)
  Serial.println("Done setup!");
  #endif
}

void setupEEPROM(){
  #if defined(DEVMODE)
  Serial.println("Setting up EEPROM");
  #endif
  EEPROM.update(0, 1); // 0 is unlocked, 1 is locked
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
  #if defined(DEVMODE)
  Serial.println("Done setting up EEPROM");
  #endif
}

void appendRfidString(unsigned char c1,unsigned char c2,unsigned char c3,unsigned char c4){
  #if defined(DEVMODE)
  Serial.println("Appending");
  #endif
  if (numValid < 255){
    int loc = 4*(numValid+1);
    EEPROM.update(loc  , c1);
    EEPROM.update(loc+1, c2);
    EEPROM.update(loc+2, c3);
    EEPROM.update(loc+3, c4);
    numValid++;
    EEPROM.update(1, numValid);
    #if defined(DEVMODE)
    Serial.println("Inserting completed!");
    #endif
  }
  else{
    #if defined(DEVMODE)
    Serial.println("Max lock # reached");
    #endif
    for (int i = 0; i < 10; i++){
      digitalWrite(redPin, HIGH);
      delay(50);
      digitalWrite(redPin, LOW);
      delay(50);
    }
  }
}

void unlock(){
  if (lockState == 1){
    #if defined(DEVMODE)
    Serial.println("Unlocking");
    #endif
    
    for (pos = 0; pos <= 90; pos += 1) {
      lockServo.write(pos);
      delay(15);
    }
    /*
    lockServo.write(180);
    delay(1000);
    lockServo.write(90);
    */
    lockState = 0;
    EEPROM.update(0, lockState);
    digitalWrite(redPin, LOW);
    digitalWrite(greenPin, HIGH);
  }
  #if defined(DEVMODE)
  Serial.println("Done unlocking!");
  #endif
}

void lock(){
  if (lockState == 0){
    #if defined(DEVMODE)
    Serial.println("Locking");
    #endif
    for (pos = 90; pos >= 0; pos -= 1) {
      lockServo.write(pos);
      delay(15);
    }
    /*
    lockServo.write(0);
    delay(1000);
    lockServo.write(90);
    */
    lockState = 1;
    EEPROM.update(0,lockState);
    digitalWrite(redPin, HIGH);
    digitalWrite(greenPin, LOW);
  }
  #if defined(DEVMODE)
  Serial.println("Done locking!");
  #endif
}

/*
void serialEvent() {
  while (Serial.available()) {
    char inChar = (char)Serial.read(); 
    //Serial.write(inChar);
    inputString += inChar;
    if (inChar == '\n' || inChar == '\r') {
      stringComplete = true;
    } 
  }
}
*/

void loop() {
  uchar status;
  uchar str[MAX_LEN];

  blePeripheral.poll();
  if (lockCharacteristic.value() != lockState) {
    lockCharacteristic.setValue(lockState);
  }
  int inputtedPass = passwordCharacteristic.value();
  if (inputtedPass != 0) {
    #if defined(DEVMODE)
    Serial.print("Password found: ");
    Serial.println(inputtedPass);
    #endif
    if (inputtedPass == numPass){
      #if defined(DEVMODE)
      Serial.println("Match found!");
      #endif
      for (int i = 0; i < 10; i++){
        digitalWrite(greenPin, HIGH);
        delay(50);
        digitalWrite(greenPin, LOW);
        delay(50);
      }
      if (lockState == 1){
        unlock();
      }
      else{
        lock();
      }
    }
    else{
      #if defined(DEVMODE)
      Serial.println("Incorrect Password");
      #endif
      for (int i = 0; i < 10; i++){
        digitalWrite(redPin, HIGH);
        delay(50);
        digitalWrite(redPin, LOW);
        delay(50);
      }
    }
    passwordCharacteristic.setValue(0);
  }
  /* This is for serial input testing - not part of final product
  if (stringComplete) {
    //Serial.println(inputString);
    delay(100);
    int pos = inputString.indexOf('=');
    if (pos > -1) {
      command = inputString.substring(0, pos);
      value = inputString.substring(pos+1, inputString.length()-1);
      //Serial.println(command);
      //Serial.println(value);
      if(!password.compareTo(value) && (command == "OPEN")){
        unlock(); // call openDoor() function
        Serial.println(" OPEN");
        delay(100);
      }
      else if(!password.compareTo(value) && (command == "CLOSE")){
        lock();
        Serial.println(" CLOSE");
        delay(100);
      }
      else if(password.compareTo(value)){
        Serial.println(" WRONG");
        delay(100);
      } 
    } 
    inputString = "";
    stringComplete = false;
  }  
  */
  status = rfid.request(PICC_REQIDL, str);
  if (status != MI_OK)
  {
    return;
  }
  
  //rfid.showCardType(str);
  status = rfid.anticoll(str);

  if (status == MI_OK) {
    #if defined(DEVMODE)
    Serial.print("RFID Number: ");
    #endif
    memcpy(serNum, str, 5);
    #if defined(DEVMODE)
    rfid.showCardID(serNum);
    Serial.println();
    #endif
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
          for (int i = 0; i < 10; i++){
            digitalWrite(greenPin, HIGH);
            delay(50);
            digitalWrite(greenPin, LOW);
            delay(50);
          }
          #if defined(DEVMODE)
          Serial.println("Match found!");
          #endif
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
      if (lockState == 0){
        #if defined(DEVMODE)
        Serial.println("Creating new valid combination");
        #endif
        for (int i = 0; i < 10; i++){
          digitalWrite(greenPin, HIGH);
          digitalWrite(redPin, HIGH);
          delay(50);
          digitalWrite(greenPin, LOW);
          digitalWrite(redPin, LOW);
          delay(50);
        }
        appendRfidString(id[0],id[1],id[2],id[3]);
        lock();
      }
      else{
        #if defined(DEVMODE)
        Serial.println("Not correct!");
        #endif
        for (int i = 0; i < 10; i++){
          digitalWrite(redPin, HIGH);
          delay(50);
          digitalWrite(redPin, LOW);
          delay(50);
        }
        digitalWrite(redPin, HIGH);
      }
    }
  }
  
  delay(500);
  rfid.halt();
}
