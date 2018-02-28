#include <string.h>
#include <Arduino.h>
#include <SPI.h>
#include <SD.h>
#include <Adafruit_Sensor.h>
#include <Wire.h>
#include <Servo.h>
#include <Adafruit_VL53L0X.h>
  
Adafruit_VL53L0X lox = Adafruit_VL53L0X();

#define servoPin 11
#define LED 13

Servo SG92R;

int rangePoint = 0;
int dataPoint[180] = {0};

void setup()
{
  Serial.begin(115200);
  
  while(!Serial)
  {
    delay(1);
  }
  
  Serial.println("Adafruit VL53L0X test");

  if(!lox.begin())
  {
    Serial.println(F("Failed to boot VL53L0X"));
    while(1);
  }
  
  Serial.println(F("VL53L0X Initialized\n\n"));

  SG92R.attach(11);
  SG92R.write(90);
}


void loop()
{
  VL53L0X_RangingMeasurementData_t measure;
  lox.rangingTest(&measure, false);
  rangePoint = measure.RangeMilliMeter;

  int turnToBlock = 3;

  if (measure.RangeStatus != 4)
  {
    Serial.println(rangePoint);
    
    if (rangePoint <= 800)
    {
      sweepfn();
      SG92R.write(90);
      delay(1000);

      // use convolution code to find best index,
      // which can be translated into angle or whatever
      int bestIndex;
      bestIndex = smoothAndFindMin(dataPoint);
      Serial.print("best index to drive to: ");
      Serial.println(bestIndex);
    }
    else
    {
      Serial.println("Searching for object...");
    }
  }
}


void sweepfn()
{
  int i = 0;
  
  while(i <= 180)
  {
    VL53L0X_RangingMeasurementData_t measure;
    lox.rangingTest(&measure, false);
    
    if(measure.RangeStatus != 4);
    {
      SG92R.write(i);
      rangePoint = measure.RangeMilliMeter;
      
      if(rangePoint > 800)
        rangePoint = 800;

      dataPoint[i] = rangePoint;
      Serial.print("dataPoint[");
      Serial.print(i);
      Serial.print("] = ");
      Serial.println(dataPoint[i]);
      delay(50);
    }
    
    i++;
  }
}



/* Max's convolution code. Returns angle 0-179 where smoothed
   lidar reading is a minimum

   Try tweaking the filter or adding another pass 
   for different results
*/

const int dataSize = 180;
const int filterSize = 9;

// this filter is very sharp, a wider one might work better for this
const int filter[9] = {1, 26, 264, 1055, 1672, 1055, 264, 26, 1};

/* here are some other filters to try, from 
this site: http://dev.theomader.com/gaussian-kernel-calculator/
using this conversion: https://gist.github.com/robertmaxwilliams/bd064ac303a0f6b5257cbd6f17ee3c29

Gaussian: Sigma = 3, Kernel Size = 17
{1, 2, 5, 9, 14, 21, 27, 32, 34, 32, 27, 21, 14, 9, 5, 2, 1}

Gaussian: Sigma = 2, Kernel Size = 9
{1, 2, 4, 6, 7, 6, 4, 2, 1}

Rectangular, Kernel Size = 9
{1, 1, 1, 1, 1, 1, 1, 1, 1}
*/

int smoothAndFindMin(int * data){

  int smoothedData[dataSize-filterSize];
  int i, j;
  int minValue;
  int minIndex = -1;

  // iterate over data, nested iterate over filter and multiply in filter
  for (i = 0; i < dataSize + 1 - filterSize; i++){
    smoothedData[i] = 0;
    for (j = 0; j < filterSize; j++){
      smoothedData[i] += data[i+j] * filter[j]
    }

    //capture min value
    if (smoothedData[i] < minValue || minIndex == -1){
      minIndex = i;
      minValue = smoothedData[i];
    }
  }

  // === DEBUG === //
	// print out data and smoothed data side by side
 	// iterate over data again and show where value is centered on
  for (i = 0; i < dataSize; i++){

    // print data then smoothed output shifted 
    Serial.print(i); Serial.print(":\t");
    Serial.print(data[i]); Serial.print("\t");
    // only print smoothed data when not near edges
    if (i >= filterSize/2 && i < dataSize - filterSize/2){
	    Serial.print(smoothedData[i-filterSize/2]);
    }
    Serial.print("\n");
  }

  // shift over by half filter (round down)
  return minIndex + (filterSize / 2);
}
