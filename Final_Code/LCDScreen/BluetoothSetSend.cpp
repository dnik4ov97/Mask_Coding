#include "ArduinoBLE.h"
#include "UVSensorSetSend.h"
#include "ScreenDraw.h"
#include "ButtonControl.h"

BLEService uvMaskService("180C"); //User define service

// create percentage characteristic and allow remote device to get notifications
BLEIntCharacteristic percentageCharacteristic("2A57", BLERead | BLENotify);
// create UV intesity characteristic and allow remote device to get notifications
BLEIntCharacteristic intesityCharacteristic("2A58", BLERead | BLENotify);
// creat time chracteristic and allow remote device to get notifications
BLEIntCharacteristic timeChracteristic("2A59", BLERead | BLENotify);
// creat buttonOnOff chracteristic and allow remote device to get notifications
BLEIntCharacteristic buttonOnOffChar("111E", BLERead | BLENotify | BLEWrite);

// Setup everything related to bluetooth and data sending/receiving. 
void bluetoothSetup(){
  Serial.begin(9600); // initialize serial communication
  
  if(!BLE.begin()){     // initialize BLE
    Serial.println("starting BLE failed!");
    while(1);
  }
  BLE.setLocalName("Nano33BLE");  // Set name for connection
  BLE.setAdvertisedService(uvMaskService); // Advertise unMaskService
  uvMaskService.addCharacteristic(percentageCharacteristic); // Add percentage Characteristic
  uvMaskService.addCharacteristic(intesityCharacteristic);  // Add intesity Characteristic
  uvMaskService.addCharacteristic(timeChracteristic);  // Add time Characteristic
  uvMaskService.addCharacteristic(buttonOnOffChar);  // Add buttonOnOff Characteristic
  
  
  BLE.addService(uvMaskService); // Add service
  
  percentageCharacteristic.writeValue(0);
  intesityCharacteristic.writeValue(500);
  timeChracteristic.writeValue(0);
  buttonOnOffChar.writeValue(0);
  
  BLE.advertise();  // Start advertising

}

// Checks Bluetooth, and whether its connected to the Arduino or not. 
void bluetoothStatus(){
  BLEDevice central = BLE.central();  // Wait for a BLE central to connect

}

// Checks the status of the battery percentage, and outputs it to selected screen and the app. 
void percentageStatus(int percentage){

  int checking = checkScreenSelect();

  // has the value changed since the last read
  boolean percentageChanged = (percentageCharacteristic.value() != percentage); // is the percentage in the loop

  
    percentageCharacteristic.writeValue(percentage); // Send data to the app

    // Check which screen you've selected. 
    // If screen = 1, you're on the first screen, print out these statements
    // If screen = 2, you're on the second screen, execute these statements
    
    if (checking == 1)
    {
      //percentageOutput(inc); // Removing from first screen and placing into the second screen.
    }

    else if (checking == 2)
    {
      printPercentage_SECONDSCREEN(percentage); // Printing the battery percentage into the second screen. 
    }

    else
    { 
      // PRINT THE ERROR IF THERE IS AN ERROR.
      int randomNumber = 123;
      percentageOutput(randomNumber);
      printPercentage_SECONDSCREEN(randomNumber);
    }
    
}


double convertFromADC (){
  double sum = 0;
  for(int i = 0; i < 1000; i++){
    double v = analogRead(A0);
    v = (3.3/1023)*v;
    sum = v + sum;
  }
  return (double)(sum / 1000 * 100); // returns mV value
}

// Based of ma2shita's Arduino library for GUVA-S12SD, to calculate UV Index. 
double calculateUVIndex(double i){
  double mV = convertFromADC();
  double uv_index = (mV / 1024) / 0.1;
  //Serial.print("guvaUV_index: ");Serial.print(uv_index);Serial.println("   ");  // Print to serial monitor for testing. 
  return uv_index;
}

// Checks the UV intensity from the UV sensor, and applies our calibration curve. 
void intensityStatus(){
  // read the current percentage value
  //int percentage = analogRead(A0);
  double mV = convertFromADC();
  double indexUV = calculateUVIndex(mV);

  double averageV = AverageFilter(mV);

  //////////////////////// Calibration Curve Equation //////////////////////////
  // Calibration Curve, Third Iteration USING 265nm DIODES: 3.03x + 335
  double calibrationCurveValue = (3.03*averageV) + 335;
  double sendToApp = (3.03*mV) + 335;


  if ( ledStatus() == 0)
  {
    sendToApp = 0.0;
  }
  // Sending raw analog voltage after sending it through the calibration curve.
  intesityCharacteristic.writeValue(sendToApp);
}

void printVal (char string[] , float data){
  Serial.println(string);
  Serial.print(data);
}

// Function that checks the buttonOnOff characteristic. 
// - If the app sets it to 0, turn OFF the UV LED's
// - If the app sets it to 1, turn ON  the UV LED's
void buttonOnOff(){
  if(buttonOnOffChar.written()){
    if(buttonOnOffChar.value() == 1){
      digitalWrite(D3, HIGH);
    } else {
      digitalWrite(D3, LOW);
    }
  }
}
