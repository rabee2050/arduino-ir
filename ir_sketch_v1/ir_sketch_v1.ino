/*
  Title  : Arduino IR 
  version: V1.
  Contact: info@tatco.cc
  Done By: TATCO Inc.
  github : https://github.com/rabee2050/arduino-ir
  ios    :
  Android:

  Release Notes:
  - V1 Created 9 Feb 2018

*/

/*
  Connection:
  arduino_rx_pin  ------->   Bluetooth_tx_pin
  arduino_tx_pin  ------->   Bluetooth_rx_pin

  IR Reciever     ------->   Pin 11
  IR Transmitter  ------->   Pin 3

  If you are using Bluefruit module then make sure to connect CTS pin to ground.

*/
#include <SoftwareSerial.h>
#include <IRremote.h>

#define arduino_rx_pin 6  //  arduino_rx_pin 6 ------->   Bluetooth_tx_pin
#define arduino_tx_pin 7  //  arduino_tx_pin 7 ------->   Bluetooth_rx_pin

String ir[3];
int recvPin = 11;
unsigned int irBuf[100];
unsigned int irBufLen;
unsigned int irBufType;
boolean repeat = false;

SoftwareSerial mySerial(arduino_rx_pin, arduino_tx_pin); // RX, TX
IRrecv irrecv(recvPin);//pin 11
IRsend irsend;//pin 3

void setup(void)
{
  Serial.begin(9600);
  //  while (!Serial) {
  //    ; // wait for serial port to connect. Needed for native USB port only
  //  }
  irrecv.enableIRIn();
  Serial.println("Start");
  mySerial.begin(9600);//you have to change this if you change bluetooth baudrate.

}

void loop(void)
{
  if ( mySerial.available() )
  {
    process();
  }

  decode_results  results;
  if (irrecv.decode(&results)) {  // Grab an IR code
    dumpCode(&results);           // Output the results as source code
    delay(50);
    irrecv.resume();              // Prepare for the next value

  }

  if (repeat) {
    sendCode(irBufType, irBuf , irBufLen);
    delay(150);
  }

}

void process() {

  String command = mySerial.readStringUntil('/');

  if (command == "ir") {
    irCommand();
  }

  if (command == "irR") {//R for repeated code
    irCommandR();
  }

  if (command == "allstatus") {
    allstatus();
  }
}

void irCommand() {
  repeat = false;
  String codeType, codeValue, codeLen;
  codeType = mySerial.readStringUntil('/');
  codeValue = mySerial.readStringUntil('/');
  codeLen = mySerial.readStringUntil('\r');
  stringToIntArry(codeValue);
  sendCode(codeType.toInt(), irBuf , codeLen.toInt());
  delay(100);
  //  Serial.println(F("Sent Raw "));
}

void irCommandR() {

  String codeType, codeValue, codeLen;
  codeType = mySerial.readStringUntil('/');
  codeValue = mySerial.readStringUntil('/');
  codeLen = mySerial.readStringUntil('\r');
  if (codeType == "off") {
    repeat = false;
  } else {
    stringToIntArry(codeValue);
    irBufLen = codeLen.toInt();
    irBufType = codeType.toInt();
    repeat = true;
  }
  //  Serial.println(F("Sent Raw "));
}

void allstatus() {
  String data_status = "{\"T\":\"\",\"D\":[]}";
  mySerial.println(data_status);
  Serial.println(F("Connected"));
}

void  dumpCode (decode_results *results)
{
  int codeType = results->decode_type;
  int codeLen = results->bits;
  unsigned long codeValue = results->value;
  String data_status;
  data_status += F("{\"T\":\"");
  data_status += codeType;
  data_status += F("\",\"D\":[\"");
  data_status += codeValue;
  data_status += F("\",\"");
  for (int i = 1; i < results->rawlen ; i++)
  {
    data_status += results->rawbuf[i] * USECPERTICK;
    if (i != results->rawlen - 1)data_status += ",";
  }
  data_status += F(",\",\"");
  data_status += results->rawlen;

  data_status += F("\"]}");
  mySerial.println(data_status);
  Serial.println(F("Dumped Raw"));
}

void sendCode(int codeType, unsigned long codeValue, int codeLen ) {

  irsend.sendRaw(irBuf, codeLen, 38);
  delay(50);
  irrecv.enableIRIn();
}

void stringToIntArry(String irRawString) {
  String dataShort = "";
  int counter = 0;
  for (int i = 0; i <= irRawString.length(); i++) {
    dataShort += irRawString[i];
    if (irRawString[i] == ',') {
      dataShort = dataShort.substring(0, dataShort.length() - 1 );
      irBuf[counter] = dataShort.toInt();
      counter = counter + 1;
      dataShort = "";
    }
  }
}
