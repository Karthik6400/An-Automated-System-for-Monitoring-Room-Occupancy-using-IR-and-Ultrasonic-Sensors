#include <WiFi.h>
#include <WiFiClientSecure.h>
#include <UniversalTelegramBot.h>    

// Replace with your Wi-Fi credentials
const char* ssid = "DSL";
const char* password = "gprec#iot";

// Replace with your Telegram bot token from BotFather
#define BOT_TOKEN "8064739614:AAHnlyYGriVl2JERLNeFgyM4KeXXDG0LbB4"

// Replace with your chat ID
#define CHAT_ID "5519942536"

// IR sensor pin
const int IR_PIN = 14;

// Ultrasonic sensor pins
#define TRIG_PIN 21
#define ECHO_PIN 22

// Telegram bot setup
WiFiClientSecure client;
UniversalTelegramBot bot(BOT_TOKEN, client);

// State tracking
bool lastState = HIGH;
bool objectDetected = false;
int irCount = 0;
int outCount = 0;

// Function to measure distance with ultrasonic sensor
long getDistance() {
  digitalWrite(TRIG_PIN, LOW);
  delayMicroseconds(5);
  digitalWrite(TRIG_PIN, HIGH);
  delayMicroseconds(10);
  digitalWrite(TRIG_PIN, LOW);

  long duration = pulseIn(ECHO_PIN, HIGH, 30000);
  if (duration == 0) {
    return -1;
  }
  long distance = duration * 0.034 / 2;
  return distance;
}

// Handle incoming Telegram commands
void handleNewMessages(int numNewMessages) {
  for (int i = 0; i < numNewMessages; i++) {
    String text = bot.messages[i].text;
    String fromChatId = bot.messages[i].chat_id;

    Serial.println("Received message: " + text + " from chat ID: " + fromChatId);

    if (text == "/in") {
      String message = "IR Sensor triggered count: " + String(irCount);
      bot.sendMessage(fromChatId, message, "Markdown");
    }

    if (text == "/out") {
      String message = "Ultrasonic Sensor OUT count (distance < 50cm): " + String(outCount);
      bot.sendMessage(fromChatId, message, "Markdown");
    }

    if (text == "/report") {
      String message = "Total Entered Into the Room: " + String(irCount) + "\n";
      message += "Total Left the Room: " + String(outCount) + "\n";
      message += "Total Currently Inside the Room: " + String(irCount - outCount);
      bot.sendMessage(fromChatId, message, "Markdown");
    }
  }
}

void setup() {
  Serial.begin(115200);

  pinMode(IR_PIN, INPUT);
  pinMode(TRIG_PIN, OUTPUT);
  pinMode(ECHO_PIN, INPUT);

  // Accept all SSL certificates (optional for testing)
  client.setInsecure();

  // Connect to Wi-Fi
  Serial.print("Connecting to WiFi..");
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("\nConnected to WiFi!");

  // Read initial IR state
  lastState = digitalRead(IR_PIN);
}

void loop() {
  // IR sensor detection
  bool currentState = digitalRead(IR_PIN);
  if (lastState == HIGH && currentState == LOW) {
    if (!objectDetected) {
      irCount++;
      Serial.println("IR sensor triggered! Object detected.");
      objectDetected = true;
      bot.sendMessage(CHAT_ID, "🚶‍♂️ Person Entered Into the Room", "Markdown");
    }
  }

  if (lastState == LOW && currentState == HIGH) {
    objectDetected = false;
  }

  // Ultrasonic sensor detection for exit
  long distance = getDistance();
  if (distance >= 0 && distance < 50) {
    if (outCount < irCount) {
      outCount++;
      Serial.println("Distance < 50cm. Person left.");
      bot.sendMessage(CHAT_ID, "🚪 Person Left the Room", "Markdown");
    } else {
      Serial.println("Out count skipped to prevent negative room count.");
    }
  }

  // Handle incoming Telegram messages
  int numNewMessages = bot.getUpdates(bot.last_message_received + 1);
  while (numNewMessages) {
    handleNewMessages(numNewMessages);
    numNewMessages = bot.getUpdates(bot.last_message_received + 1);
  }

  lastState = currentState;
  delay(200);
}



