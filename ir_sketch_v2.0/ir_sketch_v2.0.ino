/*
  Title  : Arduino IR
  version: V2.0
  Contact: info@tatco.cc
  Done By: TATCO Inc.
  github : https://github.com/rabee2050/arduino-ir
  iOS    : https://itunes.apple.com/us/app/arduino-ir-kit/id1332867848
  Android: https://play.google.com/store/apps/details?id=com.tatco.ir

  Release Notes:
  - V1 Created 9 Feb 2018
  - V1.1 Updated 06 Apr 2019 / Minor Changes
  - V2.0 Updated 09 Mar 2022 / Support last IRremote library

*/

/*
  Connection:
  arduino_rx_pin  ------->   Bluetooth_tx_pin
  arduino_tx_pin  ------->   Bluetooth_rx_pin
  - If you are using Bluefruit module then make sure to connect CTS pin to ground.

  Notes:
  - Make sure to install IRremote library from Sketch/Include Library/Manage Libraries then search for "IRremote" then install it.

*/
#include <SoftwareSerial.h>
#include <IRremote.hpp>

#define arduino_rx_pin 10  //  arduino_rx_pin 10 ------->   Bluetooth_tx_pin
#define arduino_tx_pin 8  //  arduino_tx_pin 8 ------->   Bluetooth_rx_pin

unsigned int irBuf[100];
unsigned int irBufLen;
unsigned int irBufType;
boolean repeat = false;

SoftwareSerial mySerial(arduino_rx_pin, arduino_tx_pin); // RX, TX

#define IR_RECEIVE_PIN  11 // Reciever Pin
#define IR_SEND_PIN     3  // Send Pin

IRData IRSendData;
bool rawFlag; //Recieving raw or encoded singnal

void setup(void)
{
  Serial.begin(9600);
  IrReceiver.begin(IR_RECEIVE_PIN, DISABLE_LED_FEEDBACK);
  IrSender.begin(IR_SEND_PIN, DISABLE_LED_FEEDBACK);
  Serial.println("Start");

  mySerial.begin(9600);//If you need to change this, then you have to change bluetooth module baudrate to the same.
}

void loop(void)
{
  if ( mySerial.available() )
  {
    process();
  }

  if (IrReceiver.decode()) {  // Grab an IR code
    dumpCode();           // Output the results as source code
    delay(50);
    IrReceiver.resume();              // Prepare for the next value
  }

  if (repeat) {
    rawFlag == true ? sendCode(1) : sendCode(0) ;//sent repeat raw signal or encoded signal

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

  if (codeType.toInt()) {//If encoded signal
    int commaIndex = codeValue.indexOf(',');
    int secondCommaIndex = codeValue.indexOf(',', commaIndex + 1);

    String codeAdress = codeValue.substring(0, commaIndex);//Commands like: mode, digital, analog, servo,etc...
    String codeCommand = codeValue.substring(commaIndex + 1, secondCommaIndex);//Pin number.
    String decodedRawData = codeValue.substring(secondCommaIndex + 1 );// value of the pin.
    // prepare data
    IRSendData.protocol = codeType.toInt();
    IRSendData.address = codeAdress.toInt();
    IRSendData.command = codeCommand.toInt();
    IRSendData.decodedRawData = decodedRawData.toInt();
    sendCode(0);
    //    Serial.println(F("Sent Encoded "));
  } else {//If raw signal
    irBufLen = codeLen.toInt();
    irBufType = codeType.toInt();
    stringToIntArry(codeValue);
    sendCode(1);
    //    Serial.println(F("Sent Raw "));
  }
}

void irCommandR() {

  String codeType, codeValue, codeLen;
  codeType = mySerial.readStringUntil('/');

  if (codeType == "off") {
    repeat = false;
    Serial.println(F("Don't repeat"));
  } else {
    codeValue = mySerial.readStringUntil('/');
    codeLen = mySerial.readStringUntil('\r');
    if (codeType.toInt()) {//If encoded signal
      int commaIndex = codeValue.indexOf(',');
      int secondCommaIndex = codeValue.indexOf(',', commaIndex + 1);

      String codeAdress = codeValue.substring(0, commaIndex);//Commands like: mode, digital, analog, servo,etc...
      String codeCommand = codeValue.substring(commaIndex + 1, secondCommaIndex);//Pin number.
      String decodedRawData = codeValue.substring(secondCommaIndex + 1 );// value of the pin.
      // prepare data
      IRSendData.protocol = codeType.toInt();
      IRSendData.address = codeAdress.toInt();
      IRSendData.command = codeCommand.toInt();
      IRSendData.decodedRawData = decodedRawData.toInt();//IRDATA_FLAGS_EMPTY
      repeat = true;
      rawFlag = false;
      //      Serial.println(F("Sent Repeat Encoded "));

    } else {//If raw signal

      stringToIntArry(codeValue);
      irBufLen = codeLen.toInt();
      irBufType = codeType.toInt();
      repeat = true;
      rawFlag = true;
      //      Serial.println(F("Sent Repeat Raw "));
    }
  }

}

void allstatus() {
  String data_status = "{\"T\":\"\",\"D\":[]}";
  mySerial.println(data_status);
  Serial.println(F("Connected"));
}

void  dumpCode ()
{
  int codeType = IrReceiver.decodedIRData.protocol;
  int codeAdress = IrReceiver.decodedIRData.address;
  int codeCommand = IrReceiver.decodedIRData.command;
  int codeExtra = IrReceiver.decodedIRData.extra;
  int codeLen = IrReceiver.decodedIRData.rawDataPtr->rawlen;
  unsigned long codeValue = IrReceiver.decodedIRData.decodedRawData;
  //  const uint16_t codeBuf = IrReceiver.decodedIRData.rawDataPtr->rawbuf;
  String data_status;
  data_status += F("{\"T\":\"");
  data_status += codeType;
  data_status += F("\",\"D\":[\"");
  data_status += codeValue;
  data_status += F("\",\"");
  if (codeType) {
    data_status += codeAdress;
    data_status += F(",");
    data_status += codeCommand;
    data_status += F(",");
    data_status += codeValue;
  }
  else {
#if RAW_BUFFER_LENGTH <= 254        // saves around 75 bytes program space and speeds up ISR
    uint8_t i;
#else
    uint16_t i;
#endif
    for (int i = 1; i < codeLen ; i++)
    {
      if (i & 1) {
        // Mark
        data_status += IrReceiver.decodedIRData.rawDataPtr->rawbuf[i] * MICROS_PER_TICK - MARK_EXCESS_MICROS;
      } else {
        data_status += IrReceiver.decodedIRData.rawDataPtr->rawbuf[i] * MICROS_PER_TICK + MARK_EXCESS_MICROS;
      }
      if (i != codeLen - 1 )data_status += ",";
    }
  }

  data_status += F("\",\"");
  data_status += codeLen;
  data_status += F("\"]}");
  mySerial.println(data_status);
  //  Serial.println(F("Got IR Code"));
  //  Serial.println(data_status);
  IrReceiver.printIRResultShort(&Serial);
}


void sendCode(int x) {
  if (x == 0) {//If encoded signal
    IrSender.write(&IRSendData, 2);
    delay(50);
    IrReceiver.resume();

  }

  if (x == 1) {//If raw signal
    IrSender.sendRaw(irBuf, irBufLen, 38);
    delay(50);
    IrReceiver.resume();

  }
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
