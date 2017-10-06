/*
  Done by TATCO.
  
  Contact us:
  info@tatco.cc

  Release Notes:
  - Created 10 Oct 2015
  - V2 Updated 01 Jan 2016
  - V3 Updated 15 Apr 2016
  - V4 Updated 06 Oct 2017
  
*/
//#include <Pushetta.h>
#include <EEPROM.h>
#include <Bridge.h>
#include <BridgeServer.h>
#include <BridgeClient.h>
#include <Process.h>
#include <Servo.h>

#define lcd_size 3 //this will define number of LCD on the phone app
int refresh_time = 15; //the data will be updated on the app every 15 seconds.

BridgeServer server;
Servo myServo[53];

char mode_action[54];
int mode_val[54];
String mode_feedback;
String lcd[lcd_size];

unsigned long last = millis();
unsigned long last_ip = millis();

void setup() {
  Bridge.begin();
  boardInit();
  server.listenOnLocalhost();
  server.begin();
}

void loop() {
  lcd[0] = "Test 1 LCD";// you can send any data to your mobile phone.
  lcd[1] = "Test 2 LCD";// you can send any data to your mobile phone.
  lcd[2] = analogRead(1);//  send analog value of A1

  BridgeClient client = server.accept();
  if (client) {
    process(client);
    client.stop();
  }
  delay(50);
  update_input();
  print_wifiStatus();
}

void process(BridgeClient client) {
  String command = client.readStringUntil('/');

  if (command == "terminal") {
    terminalCommand(client);
  }

  if (command == "digital") {
    digitalCommand(client);
  }

  if (command == "analog") {
    analogCommand(client);
  }

  if (command == "servo") {
    servo(client);
  }

  if (command == "mode") {
    modeCommand(client);
  }

  if (command == "allonoff") {
    allonoff(client);
  }

  if (command == "refresh") {
    refresh(client);
  }

  if (command == "allstatus") {
    allstatus(client);
  }
}

void terminalCommand(BridgeClient client) {//Here you recieve data form app terminal
  String data = client.readStringUntil('\r');
  Serial.println(data);
}

void digitalCommand(BridgeClient client) {
  int pin, value;
  pin = client.parseInt();
  if (client.read() == '/') {
    value = client.parseInt();
    digitalWrite(pin, value);
    mode_val[pin] = value;
    client.print(value);
  }
}

void analogCommand(BridgeClient client) {
  int pin, value;
  pin = client.parseInt();
  if (client.read() == '/') {
    value = client.parseInt();
    analogWrite(pin, value);
    mode_val[pin] = value;

  }

}

void servo(BridgeClient client) {
  int pin, value;
  pin = client.parseInt();
  if (client.read() == '/') {
    value = client.parseInt();
    myServo[pin].write(value);
    mode_val[pin] = value;
    client.print(value);
  }
}

void modeCommand(BridgeClient client) {
  int pin;
  pin = client.parseInt();
  String mode = client.readStringUntil('\r');

  if (mode == "/input") {
    pinMode(pin, INPUT);
    mode_action[pin] = 'i';
    client.print(F("Pin D"));
    client.print(pin);
    client.print(F(" configured as INPUT!"));
    return;
  }

  if (mode == "/output") {
    pinMode(pin, OUTPUT);
    mode_action[pin] = 'o';
    client.print(F("Pin D"));
    client.print(pin);
    client.print(F(" configured as OUTPUT!"));
    return;
  }

  if (mode == "/pwm") {
    pinMode(pin, OUTPUT);
    mode_action[pin] = 'p';
    client.print(F("Pin D"));
    client.print(pin);
    client.print(F(" configured as PWM!"));
    return;
  }

  if (mode == "/servo") {
    myServo[pin].attach(pin);
    mode_action[pin] = 's';
    client.print(F("Pin D"));
    client.print(pin);
    client.print(F(" configured as SERVO!"));
    return;
  }
}



void allonoff(BridgeClient client) {
  int pin, value;
  value = client.parseInt();

#if defined(__AVR_ATmega328P__) || defined(__AVR_ATmega168__) || defined(__AVR_ATmega32U4__) || defined(__AVR_ATmega16U4__)
  for (byte i = 0; i <= 13; i++) {
    if (mode_action[i] == 'o') {
      digitalWrite(i, value);
      mode_val[i] = value;
    }
  }
#endif
#if defined(__AVR_ATmega1280__) || defined(__AVR_ATmega2560__)
  for (byte i = 0; i <= 53; i++) {
    if (mode_action[i] == 'o') {
      digitalWrite(i, value);
      mode_val[i] = value;
    }
  }
#endif

}

void refresh(BridgeClient client) {
  int value;
  value = client.parseInt();
  refresh_time = value;

}

void allstatus(BridgeClient client) {
  client.println(F("Status:200"));
  client.println(F("content-type:application/json"));
  client.println();
  client.println(F("{"));
  client.print(F("\"m\":["));//m for Pin Mode
#if defined(__AVR_ATmega1280__) || defined(__AVR_ATmega2560__)
  for (byte i = 0; i <= 53; i++) {
    client.print(F("\""));
    client.print(mode_action[i]);
    client.print(F("\""));
    if (i != 53)client.print(F(","));
  }
#endif
#if defined(__AVR_ATmega328P__) || defined(__AVR_ATmega168__) || defined(__AVR_ATmega32U4__) || defined(__AVR_ATmega16U4__)
  for (byte i = 0; i <= 13; i++) {
    client.print(F("\""));
    client.print(mode_action[i]);
    client.print(F("\""));
    if (i != 13)client.print(F(","));
  }
#endif
  client.println(F("],"));

  client.print(F("\"v\":["));// v for Mode value
#if defined(__AVR_ATmega1280__) || defined(__AVR_ATmega2560__)
  for (byte i = 0; i <= 53; i++) {
    client.print(mode_val[i]);
    if (i != 53)client.print(F(","));
  }
#endif
#if defined(__AVR_ATmega328P__) || defined(__AVR_ATmega168__) || defined(__AVR_ATmega32U4__) || defined(__AVR_ATmega16U4__)
  for (byte i = 0; i <= 13; i++) {
    client.print(mode_val[i]);
    if (i != 13)client.print(F(","));
  }
#endif
  client.println(F("],"));

  client.print(F("\"a\":["));// a For Analog
#if defined(__AVR_ATmega1280__) || defined(__AVR_ATmega2560__)
  for (byte i = 0; i <= 15; i++) {
    client.print(analogRead(i));
    if (i != 15)client.print(",");

  }
#endif
#if defined(__AVR_ATmega328P__) || defined(__AVR_ATmega168__) || defined(__AVR_ATmega32U4__) || defined(__AVR_ATmega16U4__)
  for (byte i = 0; i <= 5; i++) {
    client.print(analogRead(i));
    if (i != 5)client.print(",");
  }
#endif
  client.println("],");

  client.print("\"l\":[");// // l for LCD
  for (byte i = 0; i <= lcd_size - 1; i++) {
    client.print("\"");
    client.print(lcd[i]);
    client.print("\"");
    if (i != lcd_size - 1)client.print(",");
  }
  client.println("],");

  client.print("\"f\":\"");// f for Feedback.
  client.print(mode_feedback);
  client.println("\",");
  client.print("\"t\":\"");//t for refresh Time .
  client.print(refresh_time);
  client.println("\"");
  client.println("}");
}

void boardInit() {
#if defined(__AVR_ATmega1280__) || defined(__AVR_ATmega2560__)
  for (byte i = 0; i <= 53; i++) {
    if (i == 0 || i == 1 ) {
      mode_action[i] = 'x';
      mode_val[i] = 'x';
    }
    else {
      mode_action[i] = 'o';
      mode_val[i] = 0;
      pinMode(i, OUTPUT);
    }
  }

#endif

#if defined(__AVR_ATmega328P__) || defined(__AVR_ATmega168__) || defined(__AVR_ATmega32U4__) || defined(__AVR_ATmega16U4__)
  for (byte i = 0; i <= 13; i++) {
    if (i == 0 || i == 1  ) {
      mode_action[i] = 'x';
      mode_val[i] = 'x';
    }
    else {
      mode_action[i] = 'o';
      mode_val[i] = 0;
      pinMode(i, OUTPUT);
    }
  }
#endif

}

void print_wifiStatus() {
  if (Serial) {
    if (millis() - last_ip > 2000) {
      Process wifiCheck;
      wifiCheck.runShellCommand("/usr/bin/pretty-wifi-info.lua");
      while (wifiCheck.available() > 0) {
        char c = wifiCheck.read();
        SerialUSB.print(c);
      }
      SerialUSB.println();
    }
    last_ip = millis();
  }
}

void update_input() {
  for (byte i = 0; i < sizeof(mode_action); i++) {
    if (mode_action[i] == 'i') {
      mode_val[i] = digitalRead(i);
    }
  }
}
