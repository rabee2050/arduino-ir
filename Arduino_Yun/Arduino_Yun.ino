/*
  Done by TATCO.
  Contact us:
  info@tatco.cc
  tatco.cc
*/
//#include <Pushetta.h>
#include <EEPROM.h>
#include <Bridge.h>
#include <BridgeServer.h>
#include <BridgeClient.h>
#include <Process.h>
#include <Servo.h>

BridgeServer server;
char mode_action[54];
int mode_val[54];
String api, channel, notification, user_id;
Servo myServo[53];
unsigned long last = millis();
void setup() {
  // Bridge startup
  Bridge.begin();
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
  server.listenOnLocalhost();
  server.begin();
}

void update_input() {
  for (byte i = 0; i < sizeof(mode_action); i++) {
    if (mode_action[i] == 'i') {
      mode_val[i] = digitalRead(i);
      if (mode_val[i] == 1) {
        if (millis() - last > 10000) {
          postData(i);
        }
        last = millis();
      }
    }
  }
}

void loop() {
  BridgeClient client = server.accept();
  if (client) {
    process(client);
    client.stop();
  }
  delay(50);
  update_input();

}

void postData(int pin)
{
  if (user_id != "") {

    Process phant;
    String curlCmd = "";
    curlCmd += "curl -k -H \"Content-Type: application/json\" -X POST -d '{\"app_id\":\"e4dbbeda-11d3-4655-aaa3-9d5d32634797\",\"include_player_ids\":[\"";
    curlCmd += user_id;
    curlCmd += "\"],\"contents\":{\"en\":\"";
    curlCmd += "Alarm at pin # " + String(pin);
    curlCmd += "\"}}' https://onesignal.com/api/v1/notifications";

    phant.runShellCommand(curlCmd); // Send command through Shell
  }

}

void process(BridgeClient client) {
  String command = client.readStringUntil('/');

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

  if (command == "allstatus") {
    allstatus(client);
  }

  if (command == "onesignal") {
    onesignal(client);
  }

}


void onesignal(BridgeClient client) {
  user_id = client.readStringUntil('/');

  int action_p = client.parseInt();
  if (action_p == 1) {
    client.println(F("Saved"));
  } else {
    client.println(F("Deleted"));
    user_id = "";

  }

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



void allstatus(BridgeClient client) {
  client.println(F("Status:200"));
  client.println(F("content-type:application/json"));
  client.println();
  client.println(F("{"));
  client.print(F("\"mode\":["));
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

  client.print(F("\"mode_val\":["));
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

  client.print(F("\"analog\":["));
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

  client.print(F("\"boardtype\":\""));
#if defined(__AVR_ATmega1280__) || defined(__AVR_ATmega2560__)//Mega
  client.println(F("kit_mega\","));
#endif
#if defined(__AVR_ATmega32U4__)//Leo
  client.println(F("kit_leo\","));
#endif
#if defined(__AVR_ATmega328P__) || defined(__AVR_ATmega168__) || defined(__AVR_ATmega16U4__)//UNO
  client.println(F("kit_uno\","));
#endif
  client.println(F("\"boardname\":\"yun\","));
  client.println(F("\"boardstatus\":1"));
  client.println(F("}"));
}
