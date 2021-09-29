#include <SPI.h>                                                              //library for SPI communication.
#include <LoRa.h>                                                             // library for lora protocal.

#include <Wire.h>                                                             // library for I2C communication.
#include "SparkFunHTU21D.h"                                                   // library for temperature sensor.
HTU21D htu;                                                                   // making a class object og temp sensor

#define Apin A3                                                               // analog I/O pin for methane sensor  
int buzzerPin = 3;                                                            // I/O pin for Alarm


// making a funtion to generate alarm tone
void alarm(unsigned int t)
{
  analogWrite(buzzerPin, 20); //Setting pin to high
  delay(t); //Delaying
  digitalWrite(buzzerPin ,0); //Setting pin to LOW
  delay(t);
}

int counter = 0;                                                                // to print counting of messages sent

//funtion for calculating ppm of methane 
double cal_ppm()
{
float volt_sensor;                                      //sensor voltage
  float Rs_air;                                         // sensor resistance Rs
  float R0;                                             //variable for resistance R0 to e checked in air, which was calibrated and found its value. 
  float R0_calibrated=15.62;
  float Avg=0;                                          //Define variable for analog readings
  for (int i = 0 ; i < 100 ; i++)                       //Start for loop
  {
    Avg =Avg + analogRead(Apin)/100.00;                 //Add analog values of sensor 500 times
  }
  volt_sensor= Avg*(5.00/1023.00);
  Rs_air=(50.00/volt_sensor) - 10.00;
  R0=Rs_air/4.4;
  float m = -0.318;                                     //Slope taken from sensor data sheet with help of sme encyclopedia
float b = 1.133;                                        //Y-Intercept from data sheet
  double ppm_log = (log10(Rs_air/R0_calibrated)-b)/m;
  double ppm = pow(10, ppm_log);                        // ppm of gas
  return ppm;
}
void setup() {
  pinMode(buzzerPin, OUTPUT);                                             //Set buzzerPin as output
  alarm(100);                                                             // starting whole setup with alarm beep
  delay(100);
 pinMode(Apin,0);                                                         //setting methane gas pin as input 
  Serial.begin(9600);                                                     // setting buadrate of serial monitor
  Serial.println("LoRa Sender");
htu.begin();
  if (!LoRa.begin(868E6))                                                 //setting lora frequency 868 MHz and Spreading factor 6
  {
    Serial.println("Starting LoRa failed!");
    while (1);
  }
   LoRa.setSpreadingFactor(8);
}

void loop() {
  float humd = htu.readHumidity();                    //reading humidity
  float temp = htu.readTemperature();                 //reading temperature in 'C
  double p=cal_ppm();       //reading methane ppm
  Serial.print("Time:");
  Serial.print(millis()/1000);
  Serial.print("   Temperature:");
  Serial.print(temp, 1);
  Serial.print("   Humidity:");
  Serial.print(humd, 1);
  Serial.print("%  ");
  Serial.print(p);
  String s=String(humd)+"-"+String(temp)+"-"+String(p);     // making a charater string of all data to be sent
  Serial.print("Sending packet: ");
  Serial.println(counter);

  // send packet
  LoRa.beginPacket();                                       //begin transmission of lora protocal
  LoRa.print(s);
  delay(200);
  LoRa.print("::humd-temp-ppm");
  delay(200);
  LoRa.endPacket();                                         //ending the lora data packet
counter++;
  delay(5000);
}
