#include <EEPROM.h>
#include"rfid.h"
#include <Servo.h>
#include <CurieBLE.h>

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


BLEPeripheral blePeripheral; // create peripheral instance
BLEService lockService("19B10010-E8F2-537E-4F6C-D104768A1214");
BLECharCharacteristic lockCharacteristic("19B10011-E8F2-537E-4F6C-D104768A1214", BLERead | BLENotify);
BLEIntCharacteristic passwordCharacteristic("19B10012-E8F2-537E-4F6C-D104768A1214", BLERead | BLEWrite);

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

  pinMode(LED_BUILTIN, OUTPUT);
  
  // RFID
  rfid.begin(7, 5, 4, 3, 6, 2);//rfid.begin(IRQ_PIN,SCK_PIN,MOSI_PIN,MISO_PIN,NSS_PIN,RST_PIN)
  delay(100);
  rfid.init();
  
  // SERVO
  lockServo.attach(8);
  if (lockState == 1){
    lockServo.write(0);
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
  if (lockState == 1){
    Serial.println("Unlocking");
    for (pos = 0; pos <= 90; pos += 1) {
      lockServo.write(pos);
      delay(15);
    }
    /*
     * lockServo.write(180);
    delay(1000);
    lockServo.write(90);
    */
    lockState = 0;
    EEPROM.update(0, lockState);
  }
  Serial.println("Done unlocking!");
}

void lock(){
  if (lockState == 0){
    Serial.println("Locking");
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
  }
  Serial.println("Done locking!");
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
    Serial.print("Password found: ");
    Serial.println(inputtedPass);
    if (inputtedPass == numPass){
      Serial.println("Match found!");
        if (lockState == 1){
          unlock();
        }
        else{
          lock();
        }
    }
    else{
      Serial.println("Incorrect Password");
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
  rfid.halt();
}
