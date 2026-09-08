#include <SPI.h>
#include <MFRC522.h>
#include <WiFi.h>
#include <HTTPClient.h>
#include <WiFiClientSecure.h>
#include <Wire.h>
#include <LiquidCrystal_I2C.h>

MFRC522 mfrc522(SS_PIN, RST_PIN);
LiquidCrystal_I2C lcd(LCD_ADDRESS, LCD_COLS, LCD_ROWS);

void showMessage(String line1, String line2 = "")
{
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print(line1);
  lcd.setCursor(0, 1);
  lcd.print(line2);
}

long readDistanceCm()
{
  digitalWrite(TRIG_PIN, LOW);
  delayMicroseconds(2);
  digitalWrite(TRIG_PIN, HIGH);
  delayMicroseconds(10);
  digitalWrite(TRIG_PIN, LOW);
  long duration = pulseIn(ECHO_PIN, HIGH, 30000);
  if (duration == 0)
    return -1;
  return duration / 58;
}

bool objectDetected()
{
  long dist = readDistanceCm();
  return dist > 0 && dist < DISTANCE_THRESHOLD;
}

int sendApiRequest(String endpoint, String method, String payload = "")
{
  WiFiClientSecure secureClient;
  secureClient.setInsecure();

  HTTPClient http;
  String url = String(API_BASE) + endpoint;

  Serial.print(method);
  Serial.print(" ");
  Serial.println(url);

  if (!http.begin(secureClient, url))
  {
    Serial.println("HTTP begin failed");
    return -1;
  }

  http.setTimeout(5000);
  if (method == "POST")
  {
    http.addHeader("Content-Type", "application/json");
  }

  int code = -1;
  if (method == "GET")
  {
    code = http.GET();
  }
  else if (method == "POST")
  {
    code = http.POST(payload);
  }

  Serial.print("Response code: ");
  Serial.println(code);
  http.end();
  return code;
}

void setup()
{
  Serial.begin(115200);
  Serial.println("=== Starting system ===");

  IPAddress primaryDns(1, 1, 1, 1);
  IPAddress secondaryDns(1, 0, 0, 1);
  WiFi.config(IPAddress(0, 0, 0, 0), IPAddress(0, 0, 0, 0), IPAddress(255, 255, 255, 0), primaryDns, secondaryDns);

  SPI.begin();
  mfrc522.PCD_Init();
  Serial.println("RFID reader initialized");

  pinMode(TRIG_PIN, OUTPUT);
  pinMode(ECHO_PIN, INPUT);
  digitalWrite(TRIG_PIN, LOW);

  pinMode(GREEN_LED_PIN, OUTPUT);
  pinMode(RED_LED_PIN, OUTPUT);
  pinMode(BUZZER_PIN, OUTPUT);
  digitalWrite(GREEN_LED_PIN, LOW);
  digitalWrite(RED_LED_PIN, LOW);
  digitalWrite(BUZZER_PIN, LOW);

  Wire.begin(I2C_SDA_PIN, I2C_SCL_PIN);
  lcd.init();
  lcd.backlight();
  showMessage("Booting...");

  Serial.print("Connecting to WiFi: ");
  Serial.println(WIFI_SSID);
  WiFi.mode(WIFI_STA);
  WiFi.begin(WIFI_SSID, WIFI_PASS);
  while (WiFi.status() != WL_CONNECTED)
  {
    delay(300);
    Serial.print(".");
  }
  Serial.println();
  Serial.print("WiFi connected. IP: ");
  Serial.println(WiFi.localIP());

  showMessage("Ready", "Waiting for box");
  Serial.println("System ready. Waiting for box...");
}

String getTagRfid()
{
  String tag = "";
  for (byte i = 0; i < mfrc522.uid.size; i++)
  {
    if (mfrc522.uid.uidByte[i] < 0x10)
      tag += "0";
    tag += String(mfrc522.uid.uidByte[i], HEX);
  }
  tag.toUpperCase();
  return tag;
}

void notifyRequesting(String tag)
{
  showMessage("Sending...", tag);
  Serial.println("Sending request to API...");
  digitalWrite(BUZZER_PIN, HIGH);
  delay(100);
  digitalWrite(BUZZER_PIN, LOW);
}

void signalSuccess(String tag, bool wasInput)
{
  Serial.println(wasInput ? "SUCCESS: input registered" : "SUCCESS: output registered");
  digitalWrite(GREEN_LED_PIN, HIGH);
  digitalWrite(BUZZER_PIN, HIGH);
  showMessage(wasInput ? "Input OK" : "Output OK", tag);
  delay(200);
  digitalWrite(BUZZER_PIN, LOW);
  delay(1300);
  digitalWrite(GREEN_LED_PIN, LOW);
}

void signalError(String reason)
{
  Serial.print("ERROR: ");
  Serial.println(reason);
  digitalWrite(RED_LED_PIN, HIGH);
  showMessage("Error", reason);
  for (int i = 0; i < 3; i++)
  {
    digitalWrite(BUZZER_PIN, HIGH);
    delay(100);
    digitalWrite(BUZZER_PIN, LOW);
    delay(100);
  }
  delay(1200);
  digitalWrite(RED_LED_PIN, LOW);
}

bool boxExists(String tag)
{
  int code = sendApiRequest("/" + tag, "GET");
  return code == 200;
}

bool registerInput(String tag)
{
  int code = sendApiRequest("/box/input/" + tag, "POST");
  return code == 201;
}

bool registerOutput(String tag)
{
  int code = sendApiRequest("/box/output/" + tag, "POST");
  return code == 200;
}

void loop()
{
  if (objectDetected())
  {
    Serial.println("Object detected by ultrasonic sensor");
    showMessage("Reading...", "Hold tag near");
    unsigned long start = millis();
    bool read = false;

    while (millis() - start < 3000 && !read)
    {
      if (mfrc522.PICC_IsNewCardPresent() && mfrc522.PICC_ReadCardSerial())
      {
        read = true;
      }
    }

    if (read)
    {
      String tag = getTagRfid();
      Serial.print("Tag read: ");
      Serial.println(tag);

      if (tag.length() >= 5 && tag.length() <= 20)
      {
        notifyRequesting(tag);

        bool exists = boxExists(tag);
        bool success;
        if (exists)
        {
          success = registerOutput(tag);
        }
        else
        {
          success = registerInput(tag);
        }

        if (success)
        {
          signalSuccess(tag, !exists);
        }
        else
        {
          signalError("API failed");
        }
      }
      else
      {
        signalError("Invalid tag");
      }
      mfrc522.PICC_HaltA();
      mfrc522.PCD_StopCrypto1();
    }
    else
    {
      signalError("No tag detected");
    }

    showMessage("Ready", "Waiting for box");
    Serial.println("Waiting for next box...");
    delay(1500);
  }
}