//IR Remote Library and Setup
#include <IRremote.h>
IRrecv rc(11);
decode_results results;

//Pins used for the ultrasonic sensor and variable initalization
const int trigPin = 12;
const int echoPin = 10;
long duration; 
int distance;

//Pins used for slidswitches 
const int switch1 = A0;
const int switch2 = A1; 

//Pin used for POWER LED
const int LED = 2;

//Pin used for IR MODE LED
const int IRLED = 13; 

//Pin used for IR sensor
const int IRsen = 11;

//Pin used for NPN transistor to DC Motor
const int NPN = 3;

//Pin used for potentiometer 
const int pot = A2;

//Pins used to L293D IC
const int enable12 = 9;
const int enable34 = 6;
const int input1 = 5;
const int input2 = 4;
const int input3 = 8;
const int input4 = 7;

//Counters
int sysstart = 0;
int sysoff = 0;
int automode = 0;
int irmode = 0;
boolean goingforward = false;
int dcmotoronoff = 1;

void setup()
{
  //Starts Serial Communication at 9600 Baud Rate
  Serial.begin(9600);
  
  //Pin Modes Set to either OUTPUT or INPUT
  pinMode(trigPin, OUTPUT);
  pinMode(echoPin, INPUT);
  pinMode(switch1, INPUT);
  pinMode(switch2, INPUT);
  pinMode(IRLED, OUTPUT);
  pinMode(LED, OUTPUT);
  pinMode(IRsen, INPUT);
  pinMode(NPN, OUTPUT);
  pinMode(pot, INPUT);
  pinMode(enable12, OUTPUT);
  pinMode(enable34, OUTPUT);
  pinMode(input1, OUTPUT);
  pinMode(input2, OUTPUT);
  pinMode(input3, OUTPUT);
  pinMode(input4, OUTPUT);
  
  //Enables the recieving process
  rc.enableIRIn();
}

void loop()
{
  //Using the Ultrasonic Sensor To Detect Distance
  int checkdist = distancecheck();
  
  //Reading Power Switch
  int power = digitalRead(switch1);
  
  //Reading Mode Switch
  int mode = digitalRead(switch2);
  
  //When Power Switch is ON
  if(power == 1)
  {
   	digitalWrite(LED, HIGH); //Green LED Turns On
    
    //Conditional statement to print status on serial monitor
    if(sysstart == 0)
    {
      Serial.println("System Powering On");
      sysstart++;
      sysoff = 0;
    }
    //Automatic mode when mode switch is set to ON
    if(mode == 1)
    {
      digitalWrite(NPN, HIGH); // DC Motor turns on for suction system
      
      //Conditional statement to print mode status on serial monitor
      if(automode == 0)
      {
        digitalWrite(IRLED, LOW);
        Serial.println("System Mode : Automatic");
        automode++;
        irmode = 0;
      }
      
      //Conditional statement that reverses and turns the vacuum if an obstacle is within 20 inches in front of the vacuum
      if(checkdist<=20)
      {
        stop(); //Calling function to stop vacuum
        delay(300); //Delaying 300 miliseconds
        backward(); //Calling function to reverse motor spin moving the vacuum backwards
        delay(400); //Delaying 400 milliseconds
        checkdist = distancecheck(); //Checks Distance
        stop(); //Calling function to stop vacuum
        delay(300); //Delaying 300 miliseconds
        turnRight(); //Calling function to turn the vacuum right by turning right side motor off and left side motor on
        checkdist = distancecheck(); //Checks Distance
        delay(1000); //Delaying 1 second
        checkdist = distancecheck(); //Checks Distance
      }
      
      //If no obstacle is within 20 inches in front of the vacuum, clean forward
      else
      {
        forward(); //Calling function to turn both motor moving the vacuum forward
      }
    }
    //Switches to Manual Mode with IR Remote when mode switch is off
    if(mode == 0)
    {
      digitalWrite(IRLED, HIGH); //Mode LED turns on
      
      //Conditional Statement to print mode status on serial monitor
      if(irmode == 0)
      {
        Serial.println("System Mode : Remote Controlled");
        irmode++;
        stop();
        automode = 0;
      }
      
      //Recieves and matches input from ir remote to a function
      if(rc.decode(&results))
      {
        switch(results.value){
          case 0xFD00FF: //Power Button
          	if(dcmotoronoff == 1)
            {
              digitalWrite(NPN, HIGH); //Turns DC Motor On
              dcmotoronoff = 0;
            }
          	else
            {
              digitalWrite(NPN, LOW); //Turns DC Motor Off
              dcmotoronoff = 1;
            }
          	delay(50);
          	break;
          case 0xFD807F: //Volume+
          	forward(); //Calling function to turn both motor moving the vacuum forward
          	break;
          case 0xFD20DF: //Left(|<<)
          	turnLeft(); //Calling function to turn vacuum left
          	break; 
          case 0xFD609F: //Right(>>|)
          	turnRight(); //Calling function to turn vacuum right
          	break;        
          case 0xFDA05F: //Stop(>||)
          	stop(); //Calling function to stop vacuum
          	break ; 
          case 0xFD906F: //Volume-
          	backward(); //Calling function to reverse both motors moving the vacuum backwards
          	break;
        } 
        rc.resume();         
      }
    }
  }
  else //If power switch is off, then all motors and LEDS are off
  {
    digitalWrite(LED, LOW);
    digitalWrite(IRLED, LOW);
    digitalWrite(NPN, LOW);
    stop();
    sysstart = 0;
    if(sysoff == 0)
    {
      Serial.println("System Off");
      sysoff++;
    }
  }
}

//Function to use ultrasonic sensor to check distance
long distancecheck()
{
  digitalWrite(trigPin, LOW);
  delayMicroseconds(2);
  digitalWrite(trigPin, HIGH);
  delayMicroseconds(10);
  digitalWrite(trigPin, LOW);
  duration = pulseIn(echoPin, HIGH);
  distance = duration / 74;
  return distance;
}

//Function to move vacuum forward
void forward()
{
  if(!goingforward)
  {
    //Reading Potentiometer 
  	int motorspeed = map(analogRead(pot), 0, 1023, 100, 255);
    analogWrite(enable12, motorspeed);
  	analogWrite(enable34, motorspeed);
  	digitalWrite(input1, HIGH);
  	digitalWrite(input2, LOW);
  	digitalWrite(input3, HIGH);
  	digitalWrite(input4, LOW);
  }
}

//Function to turn vacuum right
void turnRight()
{
  //Reading Potentiometer 
  int motorspeed = map(analogRead(pot), 0, 1023, 100, 255);
  analogWrite(enable12, motorspeed);
  analogWrite(enable34, motorspeed);
  digitalWrite(input1, HIGH);
  digitalWrite(input2, LOW);
  digitalWrite(input3, LOW);
  digitalWrite(input4, HIGH);
}

//Function to turn vacuum left
void turnLeft()
{
  //Reading Potentiometer 
  int motorspeed = map(analogRead(pot), 0, 1023, 100, 255);
  analogWrite(enable12, motorspeed);
  analogWrite(enable34, motorspeed);
  digitalWrite(input1, LOW);
  digitalWrite(input2, HIGH);
  digitalWrite(input3, HIGH);
  digitalWrite(input4, LOW);
}

//Function to move vacuum backwards
void backward()
{
  //Reading Potentiometer 
  int motorspeed = map(analogRead(pot), 0, 1023, 100, 255);
  goingforward = false;
  analogWrite(enable12, motorspeed);
  analogWrite(enable34, motorspeed);
  digitalWrite(input1, LOW);
  digitalWrite(input2, HIGH);
  digitalWrite(input3, LOW);
  digitalWrite(input4, HIGH);
}

//Function to stop vacuum
void stop()
{
  digitalWrite(input1, LOW);
  digitalWrite(input2, LOW);
  digitalWrite(input3, LOW);
  digitalWrite(input4, LOW);
}