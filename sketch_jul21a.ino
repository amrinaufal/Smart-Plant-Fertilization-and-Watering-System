#include <WiFi.h> 
#include <FirebaseESP32.h>
#include <SoftwareSerial.h>
#include <ESP32Servo.h>
#include <Wire.h>

Servo myservo1;
Servo myservo2;
Servo myservo3;


#define SSID "GH14/33"
#define PASS "8DDF38RYMJA"


FirebaseData firebaseData;
FirebaseAuth auth;
FirebaseConfig config;

const byte nitro[] = {0x01, 0x03, 0x00, 0x1e, 0x00, 0x01, 0xe4, 0x0c};
const byte phos[] = {0x01, 0x03, 0x00, 0x1f, 0x00, 0x01, 0xb5, 0xcc};
const byte pota[] = {0x01, 0x03, 0x00, 0x20, 0x00, 0x01, 0x85, 0xc0};

byte values[11];
SoftwareSerial mod(23, 22);
int pHPin = 34;
#define RE 4
#define DE 2
int sudut;
int sensorPin = 35;
int samples = 10;
int pos = 0;
int servoPin1 = 18;
int servoPin2 = 19;
int servoPin3 = 21;
int relay2 = 32;
int waterLevelPin = 33;
float relaytime;
unsigned long int avgValue;
float b;
int kelembabanTanah;
float bacapH;
int nilaipH;
int Moist;
String Wateringtime;
String addK;
String addN;
String addP;
// float adc_resolution = 1024.0;
unsigned long previousMillis = 0; 
const unsigned long interval = 120000;

void setup() {
    Serial.begin(115200);
    mod.begin(4800);
    pinMode(waterLevelPin, INPUT);
    pinMode(RE, OUTPUT);
    pinMode(DE, OUTPUT);

    // Koneksi ke Wi-Fi
    WiFi.begin(SSID, PASS);
    Serial.print("Connecting to Wi-Fi");
    while (WiFi.status() != WL_CONNECTED) {
        Serial.print(".");
        delay(300);
    }
    Serial.println();
    Serial.print("Connected with IP: ");
    Serial.println(WiFi.localIP());

    // Koneksi ke Firebase
    config.host = "https://kebuncabe-6f515-default-rtdb.firebaseio.com";
    config.signer.tokens.legacy_token = "W0WzT5HTiFO599n9UdCqpNJV2Rn2lR7SJvopQZbE";
    Firebase.begin(&config, &auth);
    Firebase.reconnectWiFi(true);

    pinMode(sensorPin, INPUT);
    pinMode(pHPin, INPUT);
    myservo1.attach(servoPin1);
    myservo2.attach(servoPin2);
    myservo3.attach(servoPin3);
    pinMode(relay2, OUTPUT);
    
    delay(500);
}

byte nitrogen() {
    digitalWrite(DE, HIGH);
    digitalWrite(RE, HIGH);
    delay(10);
    if (mod.write(nitro, sizeof(nitro)) == 8) {
        digitalWrite(DE, LOW);
        digitalWrite(RE, LOW);
        for (byte i = 0; i < 7; i++) {
            values[i] = mod.read();
            Serial.print(values[i], HEX);
        }
        Serial.println();
    }
    return values[4];
}

byte phosphorous() {
    digitalWrite(DE, HIGH);
    digitalWrite(RE, HIGH);
    delay(10);
    if (mod.write(phos, sizeof(phos)) == 8) {
        digitalWrite(DE, LOW);
        digitalWrite(RE, LOW);
        for (byte i = 0; i < 7; i++) {
            values[i] = mod.read();
            Serial.print(values[i], HEX);
        }
        Serial.println();
    }
    return values[4];
}

byte potassium() {
    digitalWrite(DE, HIGH);
    digitalWrite(RE, HIGH);
    delay(10);
    if (mod.write(pota, sizeof(pota)) == 8) {
        digitalWrite(DE, LOW);
        digitalWrite(RE, LOW);
        for (byte i = 0; i < 7; i++) {
            values[i] = mod.read();
            Serial.print(values[i], HEX);
        }
        Serial.println();
    }
    return values[4];
}

void loop() {
  byte val1, val2, val3;
    val1 = nitrogen();
    delay(250);
    val2 = phosphorous();
    delay(250);
    val3 = potassium();
    delay(250);

    int sensorValue = analogRead(sensorPin);
    int Moist = (100.0 - ((sensorValue / 4095.0) * 100.0));
    delay(50);
    bacapH = analogRead(pHPin);
    nilaipH = (4+(bacapH / 4095.0) * 14);
    delay(50);
    int waterLevelValue = analogRead(waterLevelPin);
    float waterLevelPercentage = (waterLevelValue / 4095.0) * 100.0;
    Serial.print(waterLevelPercentage);
    Serial.println("%)");
    delay(50);


    unsigned long currentMillis = millis();
    if (currentMillis - previousMillis >= interval) {
        previousMillis = currentMillis; 
 if (Firebase.getFloat(firebaseData, "/predictions_realtime/0/Waktu Air (detik)")) {
    relaytime = firebaseData.floatData();
    Serial.println(relaytime);
    
    if (waterLevelPercentage > 20.0) {
        if (Moist <= 70) {
            digitalWrite(relay2, HIGH);
            delay(relaytime * 1000); // Mengubah relaytime ke milidetik
            digitalWrite(relay2, LOW);
        } else {
            digitalWrite(relay2, LOW);
            Serial.println("Penyiraman nonaktif");
        }
    } else {
        digitalWrite(relay2, LOW);
        Serial.println("air habis");
    }
} else {
    Serial.println("Failed to get float data from Firebase");
    Serial.println(firebaseData.errorReason());
}

if (Firebase.getString(firebaseData, "/predictions_realtime/0/Penambahan Pupuk KCL (K)")) {
    addK = firebaseData.stringData();
    int Ktime = addK.toInt();
    int i = 0;
    while (i < Ktime) {
        myservo1.write(180);
        delay(750); // Waktu untuk mencapai posisi 180 derajat
        myservo1.write(0);
        delay(750); // Waktu untuk kembali ke posisi 0 derajat
        Serial.println("Tambah K: ");
        Serial.print(Ktime);
        i++;
    }
    delay(100);
} else {
    myservo1.write(0);
    Serial.println("Servo KCL moved to 0 degrees");
}

if (Firebase.getString(firebaseData, "/predictions_realtime/0/Penambahan Pupuk Urea (N)")) {
    addN = firebaseData.stringData();
    int Ntime = addN.toInt();
    int i = 0;
    while (i < Ntime) {
        myservo2.write(180);
        delay(750); // Waktu untuk mencapai posisi 180 derajat
        myservo2.write(0);
        delay(750); // Waktu untuk kembali ke posisi 0 derajat
        Serial.println("Tambah N: ");
        Serial.print(Ntime);
        i++;
    }
    delay(100);
} else {
    myservo2.write(0);
    Serial.println("Servo Urea moved to 0 degrees");
}

if (Firebase.getString(firebaseData, "/predictions_realtime/0/Penambahan Pupuk SP36 (P)")) {
    addP = firebaseData.stringData();
    int Ptime = addP.toInt();
    int i = 0;
    while (i < Ptime) {
        myservo3.write(180);
        delay(750); // Waktu untuk mencapai posisi 180 derajat
        myservo3.write(0);
        delay(750); // Waktu untuk kembali ke posisi 0 derajat
        Serial.println("Tambah P: ");
        Serial.print(Ptime);
        i++;
    }
    delay(100);
} else {
    myservo3.write(0);
    Serial.println("Servo SP36 moved to 0 degrees");
}

 
        FirebaseJson json;
        json.set("sensorData/moisture", Moist);
        json.set("sensorData/ph", nilaipH);
        json.set("sensorData/N", val1);
        json.set("sensorData/P", val2);
        json.set("sensorData/K", val3);
        json.set("waterLevel", waterLevelPercentage);

        if (Firebase.ready()) {
            Firebase.setJSON(firebaseData, "/sensorData", json);
            Serial.println("Data terkirim ke Firebase");
        } else {
            Serial.println("Gagal mengirim data ke Firebase");
            Serial.println(firebaseData.errorReason());
        }
    }

if (Firebase.getString(firebaseData, "/relay2")) {
    if (firebaseData.dataType() == "string") {
      String relayState = firebaseData.stringData();
      int relayValue = relayState.toInt(); 

      Serial.print("Relay state: ");
      Serial.println(relayValue);

      if (relayValue == 1) {
        digitalWrite(relay2, HIGH); 
      } else if (relayValue == 0) {
        digitalWrite(relay2, LOW);  
      }
    }
  } else {
    Serial.println("Failed to get data from Firebase");
    Serial.println(firebaseData.errorReason());
  }

    
    if (Firebase.getString(firebaseData, "/KCL")) {
        String servoState1 = firebaseData.stringData();
        if (servoState1 == "1") {
            myservo1.write(180);
            delay(750);
            myservo1.write(0);
            Serial.println("Servo KCL moved to 90 degrees");
        } else {
            myservo1.write(0);
            Serial.println("Servo KCL moved to 0 degrees");
        }
    }

    
    if (Firebase.getString(firebaseData, "/SP36")) {
        String servoState2 = firebaseData.stringData();
        if (servoState2 == "1") {
            myservo2.write(180);
            delay(750);
            myservo2.write(0);
            Serial.println("Servo SP36 moved to 90 degrees");
        } else {
            myservo2.write(0);
            Serial.println("Servo SP36 moved to 0 degrees");
        }
    }

    
    if (Firebase.getString(firebaseData, "/Urea")) {
        String servoState3 = firebaseData.stringData();
        if (servoState3 == "1") {
            myservo3.write(180);
            delay(750);
            myservo3.write(0);
            Serial.println("Servo Urea moved to 90 degrees");
        } else {
            myservo3.write(0);
            Serial.println("Servo Urea moved to 0 degrees");
        }
    } else {
        Serial.print("Failed to get data from Firebase: ");
        Serial.println(firebaseData.errorReason());
    }
    delay(10);

    Serial.print("Nitrogen: ");
    Serial.print(val1);
    Serial.println(" ppm");
    Serial.print("Phosphorous: ");
    Serial.print(val2);
    Serial.println(" ppm");
    Serial.print("Potassium: ");
    Serial.print(val3);
    Serial.println(" ppm");
    delay(10);
}
