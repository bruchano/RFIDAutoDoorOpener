#include <SPI.h>
#include <MFRC522.h>
#include <pitches.h>
#include <Servo.h>
#include <Dictionary.h>
#include <LiquidCrystal.h>

#define SDApin 53
#define RSTpin 8

int R = 4;
int G = 3;
int B = 2;

int T1 = 2000;
int T2 = 500;
int T3 = 2500;

MFRC522 reader(SDApin, RSTpin);
MFRC522::MIFARE_Key key;

String Admin = " 49 CB 07 E5";
String unknown_card;
Dictionary &Users = *(new Dictionary(4));

int BuzzPort = 7;
int melody[] = {
  NOTE_D5, NOTE_A2, NOTE_C6
  };

int MotorPort = 11;
Servo motor;

int RS = 22;
int E = 24;
int D4 = 28;
int D5 = 30;
int D6 = 32;
int D7 = 34;

LiquidCrystal *LCD = new LiquidCrystal(RS, E, D4, D5, D6, D7);

int state = 0;
int x = 0;
int n = 5000 / T2;

void Draw(int row, String x) {
  LCD->setCursor(0, row);
  LCD->print(x);
}

void DrawAll(int row, String x) {
  LCD->clear();
  LCD->setCursor(0, row);
  LCD->print(x);
}

void Clear(int i) {
  LCD->setCursor(0, i);
  LCD->print("                ");
}

void Pass() {
  DrawAll(0, "Access Accepted");
  digitalWrite(G, HIGH);
  tone(BuzzPort, melody[0], T2);
  motor.write(180);
  delay(T2);
  digitalWrite(G, LOW);
  delay(T3);
  motor.write(0);
}

void Deny() {
  DrawAll(0, "Access Denied");
  digitalWrite(R, HIGH);
  tone(BuzzPort, melody[1], T2);
  delay(T2);
  digitalWrite(R, LOW);
}

void Add_New() {
  DrawAll(0, "Adding New ID");
  digitalWrite(B, HIGH);
  tone(BuzzPort, melody[2], T2);
  delay(T2);
  digitalWrite(B, LOW);
}

void Add(String i) {
  DrawAll(0, "New ID Added");
  Users(i, "1");
  digitalWrite(R, HIGH);
  digitalWrite(G, HIGH);
  digitalWrite(B, HIGH);
  tone(BuzzPort, melody[2], T2);
  delay(T2);
  digitalWrite(R, LOW);
  digitalWrite(G, LOW);
  digitalWrite(B, LOW);
}

void setup() {
  Serial.begin(9600);  
  while (!Serial);
  
  pinMode(R, OUTPUT);
  pinMode(G, OUTPUT);
  pinMode(B, OUTPUT);
  digitalWrite(R, LOW);
  digitalWrite(G, LOW);
  digitalWrite(B, LOW);
  
  motor.attach(MotorPort);
  motor.write(0);
  
  LCD->begin(16, 2);
  
  SPI.begin();
  reader.PCD_Init();

}

void loop() {

  if (!reader.PICC_IsNewCardPresent() || !reader.PICC_ReadCardSerial()) {
    delay(T2);
    if (state != 2) {
      Draw(0, "Present Valid ID");
    } 
    
    if (state == 1 && x < n) {
      x++;
      if (x * T2 / 1000 != 0) {
        Draw(1, String(x * T2 / 1000));
      }
      return;
    }
    if (state == 1 && x == n) {
      Clear(1);
      state = 0;
      x = 0;
      return;
    }
    return;
  }

  x = 0;
  reader.PICC_HaltA();
  
  unknown_card = "";
  byte *id = reader.uid.uidByte;
  byte id_size = reader.uid.size;
  Serial.print(F("Card UID: "));
  for (byte i = 0; i < id_size; i++) {
    Serial.print(id[i] < 0x10? " 0" : " ");
    Serial.print(id[i], HEX);
    unknown_card.concat(String(id[i] < 0x10? " 0" : " "));
    unknown_card.concat(String(id[i], HEX));
    
  }
  Serial.println();
  unknown_card.toUpperCase();

  if (state == 0) {
    if (Users(unknown_card) || unknown_card == Admin) {
      Pass();
      return;
    }
    else {
      state = 1;
      Deny();
      return;
    }
  }

  if (state == 1) {
    if (unknown_card == Admin) {
      state = 2;
      Add_New();
      return;
    }
    
    if (Users(unknown_card)) {
      state = 0;
      Pass();
      return;
    }
    else {
      Deny();
      return;
    }
  }

  if (state == 2) {
    if (Users(unknown_card) || unknown_card == Admin) {
      state = 0;
      Pass();
      return;
    }

    if (!Users(unknown_card)) {
      Add(unknown_card);
      state = 0;
      return;
    }
  }

  

  
}
