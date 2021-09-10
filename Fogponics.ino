#include <DHT.h>                        //Library for DHT sensor
#include <LiquidCrystal.h>             //Library for LCD display

#define DHTPIN 7                      // Pin for DHT sensor
#define DHTTYPE DHT22                // DHT 22 (AM2302)
DHT dht(DHTPIN, DHTTYPE);           // Initialize DHT sensor for normal 16mhz Arduino

#define PH_PIN A2                 // Pin for pH sensor
unsigned long int avgValue;      //Store the average value of the pH sensor feedback
int buf[10],temp;

#define EC_PIN A1               //Pin for EC sensor
#define VREF 5.0               // analog reference voltage(Volt) of the ADC
#define SCOUNT  30            // sum of sample point
int analogBuffer[SCOUNT];    // store the analog value in the array, read from ADC
int analogBufferTemp[SCOUNT];
int analogBufferIndex = 0;
float averageVoltage = 0,ECvalue = 0,temperature = temp();

const int floatSensor = 15;

//Setting relay pins
const int RELAY_valve1 = 3; // water
const int RELAY_valve2 = 3; // ph- valve
const int RELAY_valve3 = 3; // ph+ valve
const int RELAY_valve4 = 3; // Nutrient valve
const int RELAY_fogger = 3;
const int RELAY_pump = 3;
const int RELAY_fan = 3;
const int RELAY_lights = 3;

LiquidCrystal lcd(1,2,4,5,6,7};

unsigned long Start_time = 0;

void setup(){
  dht.begin();             //Start DHT sensor
  pinMode(PH_PIN,OUTPUT); //set pH sensor output
  pinMode(EC_PIN,INPUT); //Set EC sensor input
  pinMode(lfoatSensor, INPUT); //Set floatSensor input
  //Set all relays output
  pinMode(RELAY_valve1, OUTPUT);
  pinMode(RELAY_valve2, OUTPUT);
  pinMode(RELAY_valve3, OUTPUT);
  pinMode(RELAY_valve4, OUTPUT);
  pinMode(RELAY_fogger, OUTPUT);
  pinMode(RELAY_pump, OUTPUT);
  pinMode(RELAY_fan, OUTPUT);
  pinMode(RELAY_lights, OUTPUT);
  lcd.begin(16,2); //Set LCD dispay
}

void loop() {
  temp = temp();
  ph = pH();
  EC = EC();
  
  unsigned long fog_time = millis();
  if(millis()-fog_time >= 900000) //Every 15 mins activate the fogger
  { 
    digitalWrite(RELAY_fogger, !digitalRead(RELAY_fogger));
    fog_time = millis();
  }

  unsigned long lights_time = millis();
  if(millis()-fan_time >= 28800000) //Every 8 hours activate the lights
  { 
    digitalWrite(RELAY_fan, !digitalRead(RElAY_fan));
    fan_time = millis();
  }

  unsigned long fan_time = millis();
  if(millis()-fan_time >= 1200000) //Every 20 mins activate the fan
  { 
    digitalWrite(RELAY_fan, !digitalRead(RElAY_fan));
    fan_time = millis();
  }

  if(digitalRead(floatSensor) == LOW) //Checks float switch and turns pump on or off
  {
    digitalWrite(RELAY_pump, HIGH);
  }
  else
  {
    digitalWrite(RELAY_pump, LOW);
  }

  if(ph == 6.2) //when ph value reaches the the extrems of the range, valves are activated and turned off when the median value is reached
  {
    digitalWrite(RELAY_valve2, HIGH);
  }
  else if(ph == 5.8)
  {
    digitalWrite(RELAY_valve3, HIGH);
  }
  else if(ph == 6)
  {
    digitalWrite(RELAY_valve2, LOW);
    digitalWrite(RELAY_valve3, LOW);
  }
  else
  {
    continue;
  }

  if(temp == 30) //when temp value reaches the max treshold, the water valve is activated and turned off when a nominal value is reached
  {
    if(millis() >= Start_time + 10000)
    {
        Start_time +=10000;
        digitalWrite(RELAY_valve1, HIGH);
    }
  }
  else
  {
    digitalWrite(RELAY_valve1, LOW);
  }
  
}

// Read temperature data and return
float temp(){
  temp = dht.readTemperature();
  return temp;
}

// Read humidity data and return
float humidity(){
  hum = dht.readHumidity();
  return hum;
}

//Read pH sensor
float pH(){
  unsigned long ph_time = millis();
  if(millis()-ph_time >= 1000)   //Every second read the ph sensor
  {
    ph_time = millis();
    for(int i=0;i<10;i++)       //Get 10 sample value from the sensor for smooth the value
    { 
      buf[i]=analogRead(PH_PIN);
    }
  }
  for(int i=0;i<9;i++)        //sort the analog from small to large
  {
    for(int j=i+1;j<10;j++)
    {
      if(buf[i]>buf[j])
      {
        temp=buf[i];
        buf[i]=buf[j];
        buf[j]=temp;
      }
    }
  }
  avgValue=0;
  for(int i=2;i<8;i++)
  {                      //take the average value of 6 center sample
    avgValue+=buf[i];
  }
  float phValue=(float)avgValue*5.0/1024/6; //convert the analog into millivolt
  phValue=3.5*phValue;                      //convert the millivolt into pH value
  return phValue;
}

//read EC sensor
float EC(){
  static unsigned long analogSampleTimepoint = millis();
   if(millis()-analogSampleTimepoint > 40U)     //every 40 milliseconds,read the analog value from the ADC
   {
     analogSampleTimepoint = millis();
     analogBuffer[analogBufferIndex] = analogRead(TdsSensorPin);    //read the analog value and store into the buffer
     analogBufferIndex++;
     if(analogBufferIndex == SCOUNT)
     { 
         analogBufferIndex = 0;
     }
   }   
   static unsigned long printTimepoint = millis();
   if(millis()-printTimepoint > 800U)
   {
      printTimepoint = millis();
      for(int i=0;i<SCOUNT;i++)
      {
        analogBufferTemp[i]= analogBuffer[i];
      }
      averageVoltage = getMedianNum(analogBufferTemp,SCOUNT) * (float)VREF / 1024.0; // read the analog value more stable by the median filtering algorithm, and convert to voltage value
      float compensationCoefficient=1.0+0.02*(temperature-25.0);    //temperature compensation formula: fFinalResult(25^C) = fFinalResult(current)/(1.0+0.02*(fTP-25.0));
      float compensationVolatge=averageVoltage/compensationCoefficient;  //temperature compensation
      ECValue=(133.42*compensationVolatge*compensationVolatge*compensationVolatge - 255.86*compensationVolatge*compensationVolatge + 857.39*compensationVolatge)*0.5; //convert voltage value to tds value
      return ECvalue
   }
}
int getMedianNum(int bArray[], int iFilterLen) 
{
      int bTab[iFilterLen];
      for (byte i = 0; i<iFilterLen; i++)
      bTab[i] = bArray[i];
      int i, j, bTemp;
      for (j = 0; j < iFilterLen - 1; j++) 
      {
        for (i = 0; i < iFilterLen - j - 1; i++) 
        {
          if (bTab[i] > bTab[i + 1]) 
          {
            bTemp = bTab[i];
            bTab[i] = bTab[i + 1];
            bTab[i + 1] = bTemp;
          }
        }
      }
      if ((iFilterLen & 1) > 0)
      {
        bTemp = bTab[(iFilterLen - 1) / 2];
      }
      else
      {
        bTemp = (bTab[iFilterLen / 2] + bTab[iFilterLen / 2 - 1]) / 2;
      return bTemp;
}

void LCD(){
  lcd.setCursor(0,0);
  lcd.print("Temp: " + temp());
  lcd.print(" pH: " + pH());
  lcd.setCursor(0,1);
  lcd.print("Humidity: " + humidity());
}
