#include <EEPROM.h>

int scode;
int numValid;

void setup() {
  Serial.begin(9600);
  delay(2000);
  scode = EEPROM.read(0);
  Serial.print("Starting Lock State: ");
  Serial.print(scode);
  Serial.println();
  numValid = EEPROM.read(1);
  Serial.print("Valid Combinations: ");
  Serial.print(numValid);
  Serial.println();
  pinMode(LED_BUILTIN, OUTPUT);
  digitalWrite(LED_BUILTIN, HIGH);
  delay(1000);
  /* EXAMPLE USE
  insertRfidString(4, 0x00, 0x00, 0x00, 0x00);
  setNumValid(1);
  */
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

void setNumValid(int num){
  numValid = num;
  EEPROM.update(1, numValid);
}

void insertRfidString(int loc, unsigned char c1,unsigned char c2,unsigned char c3,unsigned char c4){
  Serial.println("Inserting");
  if (numValid < 255){
    EEPROM.update(loc  , c1);
    EEPROM.update(loc+1, c2);
    EEPROM.update(loc+2, c3);
    EEPROM.update(loc+3, c4);
    Serial.println("Inserting completed!");
  }
}


void printRfidString(int loc){
  for (int i = loc; i < loc + 4; i++){
    Serial.print(i);
    Serial.print(": ");
    Serial.println(EEPROM.read(i));
  }
}

void loop() {

}
