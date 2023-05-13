#include <SPI.h>
#include <MFRC522.h>
#include <Arduino.h>

#define black A0
#define red A1 
#define blue A2
#define green A3

#define blackValue 1
#define redValue 2
#define blueValue 3
#define greenValue 4

int order[10] = {};
// 2. 1t 
// 4. 3t
// 1. 2t
// 3. 4t
// 1. sırada mavi 2 defa, 2. sırada siyah 1 defa, 3. sırada yeşil 4 defa, 4. sırada kırmızı 3 defa
int correctOrder[10] = {3, 3, 1, 4, 4, 4, 4, 2, 2, 2};

#define DEBUG
#define DEBUG_MODE
//#define PRINT_MODE

#define SS_PIN 10
#define RST_PIN 9


#define door_push 6
#define door_pull 7

#define cabinet_push 4
#define cabinet_pull 5

#define box_piston 3

#define sensor 2

#define buzzer 8

String correctCard = "322414536";
String correctCardBackup = "6721018522";
String gameCards[] = {
    "2272813922",
    "19510723820",
    "321953936",
    "21020213228"};

MFRC522 mfrc522[1];

String dump_byte_array(byte *buffer, byte bufferSize);
bool is_game_card(String cardId);

void open_door(bool active);
void open_cabinet(bool active);

void addValue(int value);
bool checkOrder ();
void printOrder ();


void setup()
{
#ifdef DEBUG
    Serial.begin(9600);
    Serial.println("Start");
#endif

    pinMode(door_push, OUTPUT);
    pinMode(door_pull, OUTPUT);
    pinMode(cabinet_push, OUTPUT);
    pinMode(cabinet_pull, OUTPUT);
    pinMode(box_piston, OUTPUT);
    pinMode(sensor, INPUT);
    pinMode(buzzer, OUTPUT);

    SPI.begin();
    mfrc522[0].PCD_Init(SS_PIN, RST_PIN);
#ifdef DEBUG
    mfrc522[0].PCD_DumpVersionToSerial();
#endif
}

void loop()
{

    #ifdef PRINT_MODE
    while (true)
    {
        Serial.print("A0: ");
        Serial.print(analogRead(black));
        Serial.print(" A1: ");
        Serial.print(analogRead(red));
        Serial.print(" A2: ");
        Serial.print(analogRead(blue));
        Serial.print(" A3: ");
        Serial.println(analogRead(green));
        delay(300);
    }
    #endif
    
    if (mfrc522[0].PICC_IsNewCardPresent() && mfrc522[0].PICC_ReadCardSerial())
    {
        String cardId = dump_byte_array(mfrc522[0].uid.uidByte, mfrc522[0].uid.size);
        mfrc522[0].PICC_HaltA();
#ifdef DEBUG
        Serial.print(F("Card ID: "));
        Serial.println(cardId);
#endif
        if (cardId == correctCard || cardId == correctCardBackup)
        {
#ifdef DEBUG
            Serial.print(F("Correct card\n"));
#endif
            open_door(true);
        }
        else if (is_game_card(cardId))
        {
#ifdef DEBUG
            Serial.println(F("Wrong card"));
#endif
        }
        else
        {
#ifdef DEBUG
            Serial.println(F("Admin card"));
#endif
            open_door(false);
            open_cabinet(false);
        }
    }

    int blackv = digitalRead(black);
    int redv = digitalRead(red);
    int bluev = digitalRead(blue);
    int greenv = digitalRead(green);

    if (blackv || redv || bluev || greenv) {
      #ifdef DEBUG
      Serial.print("A0: ");
        Serial.print(blackv);
        Serial.print(" A1: ");
        Serial.print(redv);
        Serial.print(" A2: ");
        Serial.print(bluev);
        Serial.print(" A3: ");
        Serial.println(greenv);
        #endif
        digitalWrite(buzzer, HIGH);
        delay(150);
        digitalWrite(buzzer, LOW);
        if (blackv) {
            addValue(blackValue);
            #ifdef DEBUG_MODE
            Serial.println("siyah sıraya eklendi");
            # endif
            while (analogRead(black) == 0) {delay(20);}
        }
        else if (redv) {
            addValue(redValue);
            #ifdef DEBUG_MODE
            Serial.println("Kırmızı sıraya eklendi");
            #endif
            while (analogRead(red) == 0) {delay(20);}
        }
        else if (bluev) {
            addValue(blueValue);
            #ifdef DEBUG_MODE
            Serial.println("Mavi sıraya eklendi");
            #endif
            while (analogRead(blue) == 0) {delay(20);}
        }
        if (greenv) {
            addValue(greenValue);
            #ifdef DEBUG_MODE
            Serial.println("Yeşil sıraya eklendi");
            #endif
            while (analogRead(green) == 0) {delay(20);}
        }

        if (checkOrder()) {
            #ifdef DEBUG_MODE
            Serial.println("Sıra doğru");
            #endif
            digitalWrite(box_piston, HIGH);
            digitalWrite(buzzer, HIGH);
            delay(1000);
            digitalWrite(box_piston, LOW);
            digitalWrite(buzzer, LOW);
        }
        #ifdef DEBUG_MODE
        printOrder();
        #endif
        delay(650);
    }

    if (digitalRead(sensor)) {
        #ifdef DEBUG_MODE
        Serial.println("Sensor okundu");
        #endif
        open_cabinet(true);
    }
    delay(100);
}

String dump_byte_array(byte *buffer, byte bufferSize)
{
    String data = "";
    for (byte i = 0; i < bufferSize; i++)
    {
        data += (String)buffer[i];
    }
    return data;
}

bool is_game_card(String cardId)
{
    for (int i = 0; i < sizeof(gameCards) / sizeof(gameCards[0]); i++)
    {
        if (cardId == gameCards[i])
        {
            return true;
        }
    }
    return false;
}

void open_door(bool active)
{
    if (active)
    {
        digitalWrite(door_push, LOW);
        digitalWrite(door_pull, HIGH);
        delay(1000);
        digitalWrite(door_pull, LOW);
    }
    else
    {
        digitalWrite(door_push, HIGH);
        digitalWrite(door_pull, LOW);
        delay(1000);
        digitalWrite(door_push, LOW);
    }
}

void open_cabinet(bool active)
{
    if (active)
    {
        digitalWrite(cabinet_push, LOW);
        digitalWrite(cabinet_pull, HIGH);
        delay(1000);
        digitalWrite(cabinet_pull, LOW);
    }
    else
    {
        digitalWrite(cabinet_push, HIGH);
        digitalWrite(cabinet_pull, LOW);
        delay(1000);
        digitalWrite(cabinet_push, LOW);
    }
}

void addValue(int value) {
    for (int i = 0; i < 9; i++) {
        order[i] = order[i + 1];
    }
    order[9] = value;
    delay(150);
}

bool checkOrder () {
    for (int i = 0; i < 10; i++) {
        if (order[i] != correctOrder[i]) {
            return false;
        }
    }
    return true;
}

void printOrder () {
    for (int i = 0; i < 10; i++) {
        Serial.print(order[i]);
        Serial.print(" ");
    }
    Serial.println();
}
