/*
  Title  : Arduino MKR1000
  version: V1.
  Contact: info@tatco.cc
  Done By: TATCO Inc.
  github : https://github.com/rabee2050/arduino-mkr1000
  ios    :
  Android:

  Release Notes:
  - V1 Created 1 Jan 2018

*/

#include <SPI.h>
#include <WiFi101.h>
#include <Servo.h>

//#include <WiFiMDNSResponder.h>
//WiFiMDNSResponder mdnsResponder;
//char mdnsName[] = "rabee";

char ssid[] = "Mi rabee";        // your network SSID (name)
char pass[] = "1231231234";    // your network password (use for WPA, or use as key for WEP)

int status = WL_IDLE_STATUS;
WiFiServer server(80);

#define lcd_size 3 //this will define number of LCD on the phone app
int refresh_time = 15; //the data will be updated on the app every 5 seconds.
char mode_action[54];
int mode_val[54];
Servo myServo[53];
String mode_feedback;
String lcd[lcd_size];
unsigned long last_ip = millis();

void setup(void)
{
  Serial.begin(9600);      // initialize serial communication

  // attempt to connect to WiFi network:
  while ( status != WL_CONNECTED) {
    Serial.print("Attempting to connect to Network named: ");
    Serial.println(ssid);                   // print the network name (SSID);

    // Connect to WPA/WPA2 network. Change this line if using open or WEP network:
    status = WiFi.begin(ssid, pass);
    // wait 10 seconds for connection:
    delay(10000);
  }

  server.begin();                           // start the web server on port 80
  printWiFiStatus();                        // you're connected now, so print out the status
  boardInit();                              // Init the board
  
  //  if (!mdnsResponder.begin(mdnsName)) {
  //    Serial.println("Failed to start MDNS responder!");
  //    while(1);
  //  }
  //  Serial.print("Server listening at http://");
  //  Serial.print(mdnsName);
  //  Serial.println(".local/");
}

void loop(void)
{
  //  mdnsResponder.poll();

  lcd[0] = "Test 1 LCD";// you can send any data to your mobile phone.
  lcd[1] = "Test 2 LCD";// you can send any data to your mobile phone.
  lcd[2] = analogRead(1);//  send analog value of A1

  WiFiClient client = server.available();

  if (client) {                             // if you get a client,
    while (client.connected()) {            // loop while the client's connected
      if (client.available()) {             // if there's bytes to read from the client,
        process( client);
        break;
      }
    }
  }
  update_input();
  printWifiSerial();
}

void process(WiFiClient client) {
  String getString = client.readStringUntil('/');
  String arduinoString = client.readStringUntil('/');
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



void terminalCommand(WiFiClient client) {//Here you recieve data form app terminal
  String data = client.readStringUntil('/');
  Serial.println(data);
  client.println("HTTP/1.1 200 OK");
  client.println("Content-type:text/html");
  client.println();
  delay(1); client.stop();

}

void refresh(WiFiClient client) {
  int value;
  value = client.parseInt();
  refresh_time = value;
  client.println("HTTP/1.1 200 OK");
  client.println("Content-type:text/html");
  client.println();
  client.print(value);
  client.println();
  delay(1); client.stop();
}

void digitalCommand(WiFiClient client) {
  int pin, value;
  pin = client.parseInt();
  if (client.read() == '/') {
    value = client.parseInt();
    digitalWrite(pin, value);
    mode_val[pin] = value;
    client.println("HTTP/1.1 200 OK");
    client.println("Content-type:text/html");
    client.println();
    client.print(value);
    client.println();
    delay(1); client.stop();
  }
}

void analogCommand(WiFiClient client) {
  int pin, value;
  pin = client.parseInt();
  if (client.read() == '/') {
    value = client.parseInt();
    analogWrite(pin, value);
    mode_val[pin] = value;
    client.println("HTTP/1.1 200 OK");
    client.println("Content-type:text/html");
    client.println();
    client.print(value);
    client.println();
    delay(1); client.stop();
  }
}

void servo(WiFiClient client) {
  int pin, value;
  pin = client.parseInt();
  if (client.read() == '/') {
    value = client.parseInt();
    myServo[pin].write(value);
    mode_val[pin] = value;
    client.println("HTTP/1.1 200 OK");
    client.println("Content-type:text/html");
    client.println();
    client.print(value);
    client.println();
    delay(1); client.stop();
  }
}

void modeCommand(WiFiClient client) {
  int pin = client.parseInt();
  String mode = client.readStringUntil(' ');
  myServo[pin].detach();
  client.println("HTTP/1.1 200 OK");
  client.println("Content-type:text/html");
  client.println();

  if (mode == "/input") {
    pinMode(pin, INPUT);
    mode_action[pin] = 'i';
    mode_val[pin] = 0;
    digitalWrite(pin, LOW);
    client.print(F("D"));
    client.print(pin);
    client.print(F(" set as INPUT!"));
    client.println();
    delay(1); client.stop();
  }

  if (mode == "/output") {
    pinMode(pin, OUTPUT);
    mode_action[pin] = 'o';
    mode_val[pin] = 0;
    digitalWrite(pin, LOW);
    client.print(F("D"));
    client.print(pin);
    client.print(F(" set as OUTPUT!"));
    client.println();
    delay(1); client.stop();
  }

  if (mode == "/pwm") {
    pinMode(pin, OUTPUT);
    mode_action[pin] = 'p';
    mode_val[pin] = 0;
    digitalWrite(pin, LOW);
    client.print(F("D"));
    client.print(pin);
    client.print(F(" set as PWM!"));
    client.println();
    delay(1); client.stop();
  }

  if (mode == "/servo") {
    digitalWrite(pin, LOW);
    myServo[pin].attach(pin);
    mode_action[pin] = 's';
    mode_val[pin] = 0;
    client.print(F("D"));
    client.print(pin);
    client.print(F(" set as SERVO!"));
    client.println();
    delay(1); client.stop();
  }
}

void allonoff(WiFiClient client) {
  int value = client.parseInt();
  for (byte i = 0; i <= 14; i++) {
    if (mode_action[i] == 'o') {
      digitalWrite(i, value);
      mode_val[i] = value;
    }
  }
  client.println("HTTP/1.1 200 OK");
  client.println("Content-type:text/html");
  client.println();
  client.print(value);
  client.println();
  delay(1); client.stop();
}

void allstatus(WiFiClient client) {
  //Sending all data in JSON format
  client.println(F("HTTP/1.1 200 OK"));
  client.println(F("content-type:application/json"));
  client.println();
  client.println(F("{"));

  client.print(F("\"m\":["));//m for Pin Mode

  for (byte i = 0; i <= 14; i++) {
    client.print(F("\""));
    client.print(mode_action[i]);
    client.print(F("\""));
    if (i != 14)client.print(F(","));
  }
  client.println(F("],"));

  client.print(F("\"v\":["));// v for Mode value
  for (byte i = 0; i <= 14; i++) {
    client.print(mode_val[i]);
    if (i != 14)client.print(F(","));
  }
  client.println(F("],"));

  client.print(F("\"a\":["));// a For Analog
  for (byte i = A0; i <= A6; i++) {
    client.print(analogRead(i));
    if (i != A6)client.print(",");
  }
  client.println("],");


  client.print("\"l\":[");// l for LCD
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
  client.println(F("}"));
  delay(1); client.stop();
}

void update_input() {
  for (byte i = 0; i < sizeof(mode_action); i++) {
    if (mode_action[i] == 'i') {
      mode_val[i] = digitalRead(i);
    }
  }
}

void boardInit() {
  for (byte i = 0; i <= 14; i++) {
    mode_action[i] = 'o';
    mode_val[i] = 0;
    pinMode(i, OUTPUT);
  }
}

void printWiFiStatus() {
  // print the SSID of the network you're attached to:
  Serial.print("SSID: ");
  Serial.println(WiFi.SSID());

  // print your WiFi shield's IP address:
  IPAddress ip = WiFi.localIP();
  Serial.print("IP Address: ");
  Serial.println(ip);

  // print the received signal strength:
  long rssi = WiFi.RSSI();
  Serial.print("signal strength (RSSI):");
  Serial.print(rssi);
  Serial.println(" dBm");

}

void printWifiSerial() {
  if (Serial) {
    if (millis() - last_ip > 5000) {
      printWiFiStatus();
    }
    last_ip = millis();
  }
}
