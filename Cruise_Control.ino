#define BLYNK_TEMPLATE_ID "TMPL344KpYuQY"
#define BLYNK_TEMPLATE_NAME "Cruise Controller"
#define BLYNK_AUTH_TOKEN "avYC7BNdX9pyZ7lZfAUonfcfdCbLRk0i"

#include <WiFi.h>
#include <BlynkSimpleEsp32.h>
#include <Adafruit_GFX.h>
#include <Adafruit_ST7735.h>
#include <SPI.h>

// TFT SPI pins
#define TFT_CS     5
#define TFT_DC     16
#define TFT_RST    17
Adafruit_ST7735 tft = Adafruit_ST7735(TFT_CS, TFT_DC, TFT_RST);

// WiFi credentials
char ssid[] = "manoj";
char pass[] = "manoj@123";

// Ultrasonic Sensor
#define TRIG_PIN 23
#define ECHO_PIN 22

// Motors
#define ENA 25
#define IN1 26
#define IN2 27
#define ENB 33
#define IN3 32
#define IN4 21

// Brake LED
#define LED_WARN 19

// Variables
int distance = 0;
int targetSpeed = 0;
float currentSpeed = 0;
float smoothing = 0.05;
bool systemOn = true;
bool ledState = false;
bool forceStopped = false;
bool dangerNotified = false;

unsigned long lastDisplay = 0;
unsigned long lastBlink = 0;
unsigned long tooCloseStart = 0;

const unsigned long displayInterval = 100;
const unsigned long blinkInterval = 100;

// Blynk V0 Switch
BLYNK_WRITE(V0) {
  systemOn = param.asInt();
}

void setup() {
  Serial.begin(115200);

  // TFT setup
  tft.initR(INITR_BLACKTAB);
  tft.setRotation(1);
  tft.fillScreen(ST77XX_BLACK);
  tft.setTextSize(2);
  tft.setTextColor(ST77XX_GREEN);
  tft.setCursor(10, 10);
  tft.print("Adaptive Cruise");

  delay(1000);
  tft.setCursor(10, 35);
  tft.setTextColor(ST77XX_YELLOW);
  tft.print("Connecting WiFi...");

  Blynk.begin(BLYNK_AUTH_TOKEN, ssid, pass);

  tft.fillScreen(ST77XX_BLACK);
  tft.setCursor(10, 10);
  tft.setTextColor(ST77XX_GREEN);
  tft.print("WiFi Connected!");
  delay(2000);
  tft.fillScreen(ST77XX_BLACK);

  // Pin setup
  pinMode(TRIG_PIN, OUTPUT);
  pinMode(ECHO_PIN, INPUT);
  pinMode(ENA, OUTPUT); pinMode(IN1, OUTPUT); pinMode(IN2, OUTPUT);
  pinMode(ENB, OUTPUT); pinMode(IN3, OUTPUT); pinMode(IN4, OUTPUT);
  pinMode(LED_WARN, OUTPUT);
  digitalWrite(LED_WARN, LOW);
}

int readDistanceCM() {
  digitalWrite(TRIG_PIN, LOW); delayMicroseconds(2);
  digitalWrite(TRIG_PIN, HIGH); delayMicroseconds(2);
  digitalWrite(TRIG_PIN, LOW);
  long duration = pulseIn(ECHO_PIN, HIGH, 30000);
  return (duration == 0) ? 999 : duration * 0.034 / 2;
}

void setMotors(int pwm) {
  pwm = constrain(pwm, 0, 255);
  if (pwm > 0) {
    digitalWrite(IN1, HIGH); digitalWrite(IN2, LOW);
    digitalWrite(IN3, HIGH); digitalWrite(IN4, LOW);
  } else {
    digitalWrite(IN1, LOW); digitalWrite(IN2, LOW);
    digitalWrite(IN3, LOW); digitalWrite(IN4, LOW);
  }
  analogWrite(ENA, pwm);
  analogWrite(ENB, pwm);
}

void updateLED(int pwm, int prevPwm) {
  if (pwm == 0) {
    digitalWrite(LED_WARN, HIGH);
  } else if (prevPwm > pwm) {
    if (millis() - lastBlink >= blinkInterval) {
      ledState = !ledState;
      digitalWrite(LED_WARN, ledState);
      lastBlink = millis();
    }
  } else {
    digitalWrite(LED_WARN, LOW);
  }
}

void updateDisplay(int pwm, int dist) {
  tft.setTextSize(1);
  tft.setTextWrap(false);

  // Line 1
  tft.fillRect(0, 0, 160, 20, ST77XX_BLACK);
  tft.setCursor(5, 5);
  tft.setTextColor(ST77XX_CYAN);
  tft.print("Speed: ");
  tft.print(pwm);
  tft.print(" /255");

  // Line 2
  tft.fillRect(0, 25, 160, 20, ST77XX_BLACK);
  tft.setCursor(5, 30);
  tft.setTextColor(ST77XX_YELLOW);
  tft.print("Dist:  ");
  tft.print(dist);
  tft.print(" cm");

  // Blynk Terminal
  Blynk.virtualWrite(V1, "Speed: " + String(pwm) );
  Blynk.virtualWrite(V2, "Dist: " + String(dist) + "cm");
}

void loop() {
  Blynk.run();

  if (!systemOn) {
    setMotors(0);
    digitalWrite(LED_WARN, LOW);
    return;
  }

  distance = readDistanceCM();

  if (distance <= 10 && !dangerNotified) {
    Blynk.logEvent("too_close", "🚨 Obstacle too close!");
    dangerNotified = true;
  } else if (distance > 15) {
    dangerNotified = false;
  }

  if (distance <= 10) {
    if (tooCloseStart == 0) tooCloseStart = millis();
    else if (millis() - tooCloseStart > 2000 && !forceStopped) {
      forceStopped = true;
      currentSpeed = 0;
      targetSpeed = 0;
      setMotors(0);
      Blynk.logEvent("force_stop", "🛑 Obstacle held >2s");
    }
  } else {
    tooCloseStart = 0;
    forceStopped = false;
  }

  if (forceStopped) return;

  if (distance > 40) targetSpeed = 255;
  else if (distance > 20) targetSpeed = map(distance, 21, 40, 100, 200);
  else if (distance > 10) targetSpeed = map(distance, 11, 20, 50, 100);
  else targetSpeed = 0;

  int prevSpeed = round(currentSpeed);
  currentSpeed += (targetSpeed - currentSpeed) * smoothing;
  int pwm = round(currentSpeed);

  setMotors(pwm);
  updateLED(pwm, prevSpeed);

  if (millis() - lastDisplay >= displayInterval) {
    updateDisplay(pwm, distance);
    lastDisplay = millis();
  }
}
