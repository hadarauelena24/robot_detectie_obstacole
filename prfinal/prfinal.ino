#include <Wire.h>
#include <Servo.h>
#include<SoftwareSerial.h>
#include <LiquidCrystal_I2C.h>

LiquidCrystal_I2C lcd(0x27,16,2);

// Pinii motor 1
#define mpin00 5
#define mpin01 6

// Pinii motor 2
#define mpin10 3
#define mpin11 11

//senzor ultrasunet
int trigPin = 13;    // Trigger
int echoPin = 12;    // Echo
long duration, cm,cm2, inches;                                                                    

// control mode, true pentru autocontrol, false pentru control cu comenzi
bool mode = false;
bool switched=false;
char dir = 'a';
int speedDCL = 128;
int speedDCR = 128;

//create serial for bt
SoftwareSerial bt(4,2); /* (Rx,Tx) */

Servo srv;

void setup() {
  //lcd
  lcd.init();
  lcd.clear();         
  lcd.backlight();
  //bt
  pinMode(4, INPUT);
  pinMode(2, OUTPUT);
  // put your setup code here, to run once:
  bt.begin(9600);  /* Define baud rate for software serial communication */
  Serial.begin(9600);
  bt.write("Please enter your commands: l->left,r->right, f->foward, b->back,m->switch mode, anything else-> stop");
  
  // configurarea pinilor motor ca iesire, initial valoare 0
   digitalWrite(mpin00, 0);
   digitalWrite(mpin01, 0);
   digitalWrite(mpin10, 0);
   digitalWrite(mpin11, 0);
   pinMode (mpin00, OUTPUT);
   pinMode (mpin01, OUTPUT);
   pinMode (mpin10, OUTPUT);
   pinMode (mpin11, OUTPUT);
   // pin LED
   pinMode(13, OUTPUT);
  
   //Setup sunet
   pinMode(trigPin, OUTPUT);
   pinMode(echoPin, INPUT);

   //servo cu blink initial
   playWithServoInit(8);
   // Blink rapid. Scoateți cablul USB!!!!
   for (int i=0; i<10; i++)
   {
   digitalWrite(13, 1);
   delay(100);
   digitalWrite(13, 0);
   delay(100);
   }
   digitalWrite(13, 1);
}

bool checkDistance(){
    cm=getDistance();
    cm2=getDistance();
    bool ver=((cm>20) || (cm2>20));
    bt.print(ver);
    lcd.setCursor(6,0);
    if(ver){
      lcd.print("YES");
    }else
    {
      lcd.print("NO ");
    }
    return ver;
}

long getDistance(){
    digitalWrite(trigPin, LOW);
    delayMicroseconds(5);
    digitalWrite(trigPin, HIGH);
    delayMicroseconds(10);
    digitalWrite(trigPin, LOW);
    duration = pulseIn(echoPin, HIGH);
    // Convert the time into a distance
    return ((duration/2) / 29.1);     // Divide by 29.1 or multiply by 0.0343
}

// Funcție pentru controlul unui motor
// Intrare: pinii m1 și m2, direcția și viteza
void StartMotor (int m1, int m2, int forward, int speedDC)
{

 if (speedDC==0) // oprire
 {
 digitalWrite(m1, 0);
 digitalWrite(m2, 0);
 }
 else
 {
 if (forward)
   {
   digitalWrite(m2, 0);
   analogWrite(m1, speedDC); // folosire PWM
   }
 else
   {
   digitalWrite(m1, 0);
   analogWrite(m2, speedDC);
   }
 }
}
// Funcție de siguranță
// Execută oprire motoare, urmată de delay
void delayStopped(int ms)
{
 StartMotor (mpin00, mpin01, 0, 0);
 StartMotor (mpin10, mpin11, 0, 0);
 delay(ms);
}
// Utilizare servo
// Poziționare în trei unghiuri
// La final, rămâne în mijloc (90 grade)
void playWithServoInit(int pin)
{
   srv.attach(pin);
   srv.write(0);
   delay(50);
   for (int i=0; i<10; i++)
     {
       digitalWrite(13, 1);
       delay(300);
       digitalWrite(13, 0);
       delay(300);
       if(i==3){
          srv.write(180);
       }
       if(i==7){
          srv.write(90);
       }
     }
   srv.detach();
}

void playWithServo(int pin,int grades)
{
   srv.attach(pin);
   srv.write(grades);
   delay(1000);
   srv.detach();
}

void dirForward()
{
  StartMotor (mpin00, mpin01, 0, speedDCL);
  StartMotor (mpin10, mpin11, 0, speedDCR);
}

void dirBack()
{
  StartMotor (mpin00, mpin01, 1, speedDCL);
  StartMotor (mpin10, mpin11, 1, speedDCR);
}

void dirRight()
{
  playWithServo(8,0);
  if(checkDistance()){
    // Acum se porneste doar un motor
    StartMotor (mpin00, mpin01, 0, speedDCL);
    delay(300);
  }
  playWithServo(8,90);
  delayStopped(400);
}

void dirRightAuto()
{
   // Acum se porneste doar un motor
   StartMotor (mpin00, mpin01, 0, speedDCL);
   delay(300);
}

long distanceRight()
{
  playWithServo(8,0);
  long dis= getDistance(); 
  playWithServo(8,90);
  delayStopped(300);
  bt.print("Right: ");
  bt.print(dis);
  bt.print("   ");
  lcd.setCursor(8,1);
  lcd.print("R:");
  lcd.print(dis);
  return dis;
}

void dirLeft()
{
  playWithServo(8,180);
  if(checkDistance()){
    // Acum se porneste doar un motor
    StartMotor (mpin10, mpin11, 0, speedDCR);
    delay(300);
  }
  playWithServo(8,90);
  delayStopped(400);
}

void dirLeftAuto()
{
   // Acum se porneste doar un motor
   StartMotor (mpin10, mpin11, 0, speedDCR);
   delay(300);
}

long distanceLeft()
{
  playWithServo(8,180);
  long dis= getDistance(); 
  playWithServo(8,90);
  delayStopped(300);
  bt.print("Left: ");
  bt.print(dis);
  bt.print(" ");
  lcd.setCursor(0,1);
  lcd.print("L:");
  lcd.print(dis);
  return dis;
}

void loop() {
  if (checkDistance()==false){
    delayStopped(500);
  }
  // put your main code here, to run repeatedly:
   if (bt.available()) // Read from Bluetooth
   {
        char command=bt.read();
        Serial.write(command);
        if(command == 'm'){
          mode = !mode;
        }else
        {
          switched=true;
          dir = command;
        }
    }
    if(mode){
      //code pentru control automat
       if (checkDistance()==false){
        delayStopped(600);
        long r= distanceRight();
        long l= distanceLeft();
          if(r>l){
            dirRightAuto();
          }else{
            dirLeftAuto();
          }
       } 
       
       dirForward();
    }else{
      if(switched){
         switch(dir){
            case 'l':
              //call move left function
              delayStopped(500);
              dirLeft();
              dirForward();
              break;
            case 'r':
              //call move right function
              delayStopped(500);
              dirRight();
              dirForward();
              break;
            case 'f':
              //call move foward function
              delayStopped(500);
              dirForward();
              break;
            case 'b':
              //call move back function
              delayStopped(500);
              dirBack();
              delay(1000);
              delayStopped(500);
              break;
            default:
              break;      
          }
        switched=false;
      }
    }
}


//codul folosit pentru masurarea rotatiilor pe minut
////For counting RPM
////variables
//volatile unsigned int counter;
//int RPM;
////setup
//  pinMode(7,INPUT);
//  digitalWrite(7, HIGH);
//  attachInterrupt(0,countpulse,RISING);
////count rpm
//void countpulse(){
//        counter++;
//}
////loop
//https://electronicsprojects.in/speedometer-using-arduino-lm393-speed-sensor-l298n-motor-driver-and-16x2-lcd-display/
// for measuring RPM to balance 
//  
//  static uint32_t previousMillis;
//  if (millis() - previousMillis >= 1000) {
//            RPM = (counter/12)*60;
//            counter = 0;
//            previousMillis += 1000;
//  }
//  // Left-3960 Right-3960
//  bt.println(RPM);
//  delay(1);
