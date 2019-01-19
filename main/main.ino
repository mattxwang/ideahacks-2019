#include <Servo.h>

#include <Wire.h>
#include"rfid.h"

RFID rfid;
uchar serNum[5];

Servo lockServo;  // create servo object to control a servo

int pos = 0;    // variable to store the servo position


// the setup function runs once when you press reset or power the board
void setup() {
  // initialize digital pin LED_BUILTIN as an output.
  pinMode(LED_BUILTIN, OUTPUT);
  // RFID
  Serial.begin(9600);
  rfid.begin(7, 5, 4, 3, 6, 2);//rfid.begin(IRQ_PIN,SCK_PIN,MOSI_PIN,MISO_PIN,NSS_PIN,RST_PIN)
  delay(100);
  rfid.init();
  // SERVO
  lockServo.attach(9);
  Serial.print("Done setup!");
}

void unlock(){
  lockServo.write(180);
  delay(1000);
  lockServo.write(90);
  Serial.print("Done unlocking!");
}

void lock(){
  lockServo.write(0);
  delay(1000);
  lockServo.write(90);
  Serial.print("Done locking!");
}
/*
void doCorrect(){
  Serial.println("Match found!");
  digitalWrite(LED_BUILTIN, HIGH);   // turn the LED on (HIGH is the voltage level)
  for (pos = 0; pos <= 180; pos += 1) { // goes from 0 degrees to 180 degrees
    // in steps of 1 degree
    lockServo.write(pos);              // tell servo to go to position in variable 'pos'
    delay(15);                       // waits 15ms for the servo to reach the position
  }
  for (pos = 180; pos >= 0; pos -= 1) { // goes from 180 degrees to 0 degrees
    lockServo.write(pos);              // tell servo to go to position in variable 'pos'
    delay(15);                       // waits 15ms for the servo to reach the position
  }
  id[0] = 0x00;
  id[1] = 0x00;
  id[2] = 0x00;
  id[3] = 0x00;
}
*/
// the loop function runs over and over again forever
void loop() {
  digitalWrite(LED_BUILTIN, LOW);
  uchar status;
  uchar str[MAX_LEN];
  status = rfid.request(PICC_REQIDL, str);
  if (status != MI_OK)
  {
    return;
  }
  
  rfid.showCardType(str);
  status = rfid.anticoll(str);

  if (status == MI_OK) {
    Serial.print("The card's number is: ");
    memcpy(serNum, str, 5);
    rfid.showCardID(serNum);
    uchar* id = serNum;
    if( id[0]==0xA3 && id[1]==0x48 && id[2]==0x15 && id[3]==0xD3 ){
      Serial.println("Match found! Unlocking!");
      digitalWrite(LED_BUILTIN, HIGH);
      unlock();
    }
    else if( id[0]==0x93 && id[1]==0x88 && id[2]==0x78 && id[3]==0x89 ){
      Serial.println("Match found! Locking!");
      digitalWrite(LED_BUILTIN, HIGH);
      lock();
    }
    else{
      digitalWrite(LED_BUILTIN, LOW);
      Serial.println("Not correct!");
    }
  }
  
  delay(2000);
  rfid.halt(); //command the card into sleep mode 
}
