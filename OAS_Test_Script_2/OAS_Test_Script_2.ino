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
      int blockCheck();

      // use convolution code to find best index,
      // which can be translated into angle or whatever
      int bestIndex;
      bestIndex = smoothAndFindMin(dataPoint);
      Serial.print("best index to drive to:");
      Serial.println(bestIndex);

//      blockMaxSum();
      
//      if(checkBothSides(turnToBlock) == "clear")
//      {
//        Serial.print("Turn Rover to Block ");
//        Serial.print(turnToBlock);
//      }
//      else if(checkBothSides(turnToBlock) == "not clear")
//      {
//        Serial.print("Block ");
//        Serial.print(turnToBlock);
//        Serial.println("is not clear. Check next block");
//      }
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
int smoothAndFindMin(int * data){

  int smoothedData[dataSize-filterSize];
  int i, j;
  int minValue = 0;
  int minIndex = -1;

  // iterate over data, nested iterate over filter and multiply in filter
  for (i = 0; i < dataSize + 1 - filterSize; i++){
    smoothedData[i] = 0;
    for (j = 0; j < filterSize; j++){
      smoothedData[i] += data[i+j] * filter[j]
    }

    //capture min value
    if (smoothedData[i] < minValue){
      minIndex = i;
      minValue = smoothedData[i];
    }
  }

  // === DEBUG === //
	// print out data and smoothed data side by side
 	// iterate over data again and show where value is centered on
  for (i = 0; i < dataSize; i++){

    // print data then smoothed output shifted 
    if (i >= filterSize/2 && i < dataSize - filterSize/2){
      printf("%d: %f\t%f\n", i, data[i], smoothedData[i-filterSize/2]);
    } else{
      printf("%d: %f\t\n", i, data[i]);
    }
  }

  // shift over by half filter (round down)
  return minIndex - (filterSize / 2);
}
    
int blockCheck (int blockNum)
{
  int Block1; //= 0;
  int Block2; //= 0;
  int Block3; //= 0;
  int Block4; //= 0;
  int Block5; //= 0;

int sum = 0;
  switch(blockNum)
  {
    case 1:
            for (int i = 0; i = 35; i++)
            {
              sum = sum + dataPoint[i];                             //72 degrees right
              sum = Block1;
              return Block1;
              break;
            }
        
    case 2:
            for (int i = 36; i <= 71; i++)
            {
              sum = sum + dataPoint[i];                             //Turn 54 degrees right
              sum = Block2;
              return Block2;
              break;
            }

    case 3:
            for (int i = 72; i <= 107; i++)
            {
              sum = sum + dataPoint[i];                             //90 degrees
              sum = Block3;
              return Block3;
              break;
            }
  
    case 4:
            for (int i = 108; i <= 143; i++)
            {
              sum = sum + dataPoint[i];                             //Turn 54 degrees left
              sum = Block4;
              return Block4;
              break;
            }
      
    case 5:
            for (int i = 144; i= 179; i++)
            {
              sum = sum + dataPoint[i];                             //72 degrees right
              sum = Block5;
              return Block5;
              break;
            }
   }
}

void blockMaxSum()
{
  int block1Sum = blockCheck(1);
  int block2Sum = blockCheck(2);
  int block3Sum = blockCheck(3);
  int block4Sum = blockCheck(4);
  int block5Sum = blockCheck(5);
  int temp = 0;

  int blockmap[5] = {block1Sum, block2Sum, block3Sum, block4Sum, block5Sum};

  Serial.print("Presorted: ");
  for(int z = 0; z <= 4; z++)
  {
    Serial.print(blockmap[z]);
    Serial.print(" ");
  }
  Serial.println();
  delay(100);
  
  for(int x = 0; x < 4; x++)
  {
    for(int i = x+1; i <= 4; i++)
    {
      if(blockmap[i] > blockmap[x])
      {
        temp = blockmap[i];
        blockmap[i]=blockmap[x];
        blockmap[x]=temp;
      } 
    }
  }

  Serial.println("Sorted Map: ");
  for(int y = 0; y <= 4; y++)
  {
    Serial.print(blockmap[y]);
    Serial.print(" ");
  }
  Serial.println();
}



String checkBothSides (int blockNumToCheck)
{
  int blockNminus1 = blockNumToCheck - 1;
  int blockNplus1 = blockNumToCheck + 1;
  
  if(blockCheck(blockNminus1 >= 20000) && blockCheck(blockNplus1) >= 20000)
  {
    Serial.print(blockNumToCheck);
    Serial.print(" is clear on both sides");
    return "clear";
  }
  else
  {
    Serial.print(blockNumToCheck);
    Serial.print(" is NOT clear on both sides");
    return "not clear";
  }
}
