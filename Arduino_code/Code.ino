#include <ThingerESP8266.h>
#include <DHT.h>

// Thinger.io device credentials
#define USERNAME "Enter your username"
#define DEVICE_ID "Enter your Device ID"
#define DEVICE_CREDENTIAL "Enter your Credential"

// Sensor and actuator pin definitions
const int IRPIN = 2;             // IR sensor pin
#define FLAME_PIN 5             // Flame sensor pin
#define DHTPIN 14               // DHT11 sensor pin
#define Buzzer 3                // Buzzer pin
#define FANPIN 10               // Fan pin
#define MOTOR_PIN 12            // Motor pin
#define DHTTYPE DHT11           // DHT11 sensor type

// Create DHT object
DHT dht11(DHTPIN, DHTTYPE);

// Product count variables
int count = 0;
int lastState = LOW;
int currentState = LOW;

// Wi-Fi credentials
#define SSID "Enter Wi-Fi Name"
#define SSID_PASSWORD "Enter Wi-fi Password"

// Thinger.io object
ThingerESP8266 thing(USERNAME, DEVICE_ID, DEVICE_CREDENTIAL);

// Variables for temperature and humidity
float temperature, humidity;

void setup() {
  Serial.begin(9600);
  
  // Initialize pins
  pinMode(IRPIN, INPUT);
  pinMode(FLAME_PIN, INPUT);
  pinMode(Buzzer, OUTPUT);
  pinMode(FANPIN, OUTPUT);
  pinMode(MOTOR_PIN, OUTPUT);

  // Connect to Wi-Fi
  thing.add_wifi(SSID, SSID_PASSWORD);

  // Thinger.io endpoints
  thing["count"] >> [](pson& out) {
    out = count;
  };

  thing["dht11"] >> [](pson& out) {
    out["temperature"] = temperature;
    out["humidity"] = humidity;
  };

  thing["flame"] >> [](pson& out) {
    out = digitalRead(FLAME_PIN);
  };

  thing["fan"] << digitalPin(FANPIN);
  thing["motor"] << digitalPin(MOTOR_PIN);

  // Start DHT11 sensor
  dht11.begin();
}

void loop() {
  // Handle Thinger.io communication
  thing.handle();

  // Read sensor values
  temperature = dht11.readTemperature();
  humidity = dht11.readHumidity();
  int flame_value = digitalRead(FLAME_PIN);

  // Stream DHT11 data to dashboard
  thing.stream(thing["dht11"]);

  // Product count logic using IR sensor
  lastState = currentState;
  currentState = digitalRead(IRPIN);

  if (lastState == LOW && currentState == HIGH) {
    count++;
    Serial.print("Product count: ");
    Serial.println(count);
  }

  // Fan control based on temperature and humidity
  if (temperature > 35 || humidity > 70) {
    digitalWrite(FANPIN, LOW); // Turn ON fan
    pson data;
    data["temperature"] = temperature;
    data["humidity"] = humidity;
    thing.call_endpoint("HVAC_Notification", data);
  } else {
    digitalWrite(FANPIN, HIGH); // Turn OFF fan
  }

  // Flame detection and motor/buzzer control
  if (flame_value == LOW) {
    digitalWrite(MOTOR_PIN, LOW); // Turn ON motor
    pson data;
    data["Flame"] = flame_value;
    thing.call_endpoint("Flame_Notification", data);
    
    // Buzzer alert
    digitalWrite(Buzzer, HIGH);
    delay(100);
    digitalWrite(Buzzer, LOW);
    delay(100);
  } else {
    digitalWrite(MOTOR_PIN, HIGH); // Turn OFF motor
  }
}
