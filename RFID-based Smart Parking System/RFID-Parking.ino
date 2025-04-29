#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include <Servo.h>
#include <SPI.h>
#include <MFRC522.h>

#define SS_PIN 10
#define RST_PIN 9
#define servoPin 2

String UID = "C3 29 C8 A9"; // Enter your card or keychain ID

MFRC522 rfid(SS_PIN, RST_PIN);
Servo myservo;
LiquidCrystal_I2C lcd(0x27, 16, 2);

int IR1 = 4; // Entry sensor
int IR2 = 3; // Exit sensor

int Slot = 4; // Total number of parking Slots
const int MaxSlots = 4; // Maximum number of slots

bool carEntering = false; // Track if a car is entering
bool carExiting = false; // Track if a car is exiting

void setup() {
  Serial.begin(9600);
  lcd.init(); // Initialize the LCD
  lcd.backlight(); // Open the backlight
  pinMode(IR1, INPUT);
  pinMode(IR2, INPUT);
  myservo.attach(servoPin);
  myservo.write(100); // Close the gate initially
  lcd.setCursor(0, 0);
  lcd.print("    ARDUINO    ");
  lcd.setCursor(0, 1);
  lcd.print(" PARKING SYSTEM ");
  delay(3000);
  lcd.clear();

  SPI.begin();
  rfid.PCD_Init();
}

void loop() {
  // Check if a car is entering (IR1 -> IR2)
  if (digitalRead(IR1) == LOW && !carEntering && !carExiting) {
    carEntering = true;
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("Car Detected!");
    delay(1000);

    if (Slot > 0) {
      int pass = rfunc(); // Scan RFID card
      if (pass == 1) {
        smoothOpenGate(); // Open the gate smoothly
        Slot--; // Decrement available slots
        lcd.clear();
        lcd.setCursor(0, 0);
        lcd.print("Gate Opened!");
        delay(2000);

        // Wait for the car to pass through IR2
        while (digitalRead(IR2) == HIGH) {
          delay(100);
        }

        // Wait for the car to completely leave IR2
        while (digitalRead(IR2) == LOW) {
          delay(100);
        }

        delay(2000); // Wait 2 seconds before closing the gate
        smoothCloseGate(); // Close the gate smoothly
        lcd.clear();
        lcd.setCursor(0, 0);
        lcd.print("Gate Closed!");
        delay(2000);
      }
    } else {
      lcd.clear();
      lcd.setCursor(0, 0);
      lcd.print("    SORRY :(    ");
      lcd.setCursor(0, 1);
      lcd.print("  Parking Full  ");
      delay(3000);
      lcd.clear();
    }
    carEntering = false;
  }

  // Check if a car is exiting (IR2 -> IR1)
  if (digitalRead(IR2) == LOW && !carExiting && !carEntering) {
    if (Slot == MaxSlots) {
      // If all slots are available, show "No Cars Left!" message
      lcd.clear();
      lcd.setCursor(0, 0);
      lcd.print(" No Cars Left! ");
      delay(3000);
      lcd.clear();
    } else {
      carExiting = true;
      smoothOpenGate(); // Open the gate smoothly
      if (Slot < MaxSlots) {
        Slot++; // Increment available slots
      }
      lcd.clear();
      lcd.setCursor(0, 0);
      lcd.print("Car Exiting...");
      delay(2000);

      // Wait for the car to pass through IR1
      while (digitalRead(IR1) == HIGH) {
        delay(100);
      }

      // Wait for the car to completely leave IR1
      while (digitalRead(IR1) == LOW) {
        delay(100);
      }

      delay(2000); // Wait 2 seconds before closing the gate
      smoothCloseGate(); // Close the gate smoothly
      lcd.clear();
      lcd.setCursor(0, 0);
      lcd.print("Gate Closed!");
      delay(2000);
      carExiting = false;
    }
  }

  // Display available slots
  lcd.setCursor(0, 0);
  lcd.print("    WELCOME!    ");
  lcd.setCursor(0, 1);
  lcd.print("Slot Left: ");
  lcd.print(Slot);
}

// Function to smoothly open the gate
void smoothOpenGate() {
  for (int pos = 100; pos >= 0; pos -= 1) { // Move from 100° to 0°
    myservo.write(pos);
    delay(10); // Adjust delay for smoother/faster motion
  }
}

// Function to smoothly close the gate
void smoothCloseGate() {
  for (int pos = 0; pos <= 100; pos += 1) { // Move from 0° to 100°
    myservo.write(pos);
    delay(10); // Adjust delay for smoother/faster motion
  }
}

int rfunc() {
  lcd.clear();
  lcd.setCursor(4, 0);
  lcd.print("Welcome!");
  lcd.setCursor(1, 1);
  lcd.print("Put your card");
  delay(100);

  while (true) {
    if (rfid.PICC_IsNewCardPresent() && rfid.PICC_ReadCardSerial()) {
      break;
    }
    delay(300);
  }

  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Scanning");

  String ID = "";
  for (byte i = 0; i < rfid.uid.size; i++) {
    lcd.print(".");
    ID.concat(String(rfid.uid.uidByte[i] < 0x10 ? " 0" : " "));
    ID.concat(String(rfid.uid.uidByte[i], HEX));
    delay(300);
  }

  ID.toUpperCase();

  if (ID.substring(1) == UID) {
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("Allowed.");
    delay(1500);
    lcd.clear();
    return 1;
  } else {
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("Wrong card!");
    delay(1500);
    lcd.clear();
    return 0;
  }
}
