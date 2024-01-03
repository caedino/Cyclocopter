#include <HX711_ADC.h>
#include <Wire.h> 
#include <LiquidCrystal_I2C.h>
#include <Servo.h>

// Pin assignments
const int amppin = A1;
const int potpin = A2;
const int potpin2 = A3;
const int RPMpin = 9;
const int loadCellDT = 7;
const int loadCellSCK = 8;
const int voltagePin = A7;

// Constants
const int numReadings = 30;
const float loadCellCalFactor = 213;
const int lcdAddress = 0x27;
const int lcdColumns = 20;
const int lcdRows = 4;
const float voltageDividerRatio = 5.4315;
const int intervalpwm = 2;
const int cuttoffpwm = 1650;
const int pwmincrament = 2;

// Variables
float readings[numReadings];
int ndex = 0;
float total = 0;
float average = 0;
float currentValue = 0;
unsigned long lastTime = 0;
unsigned long intervaltime = 0;
float RPM = 0;
float voltage = 0;
int val = 1150;
int currenttimepwm = 0;
int variabletimepwm = 0;
// Objects
LiquidCrystal_I2C lcd(lcdAddress, lcdColumns, lcdRows);
Servo myservo;
Servo myservo2;
HX711_ADC LoadCell(loadCellDT, loadCellSCK);

void setup() {
  Serial.begin(115200);
  lcd.init();
  lcd.backlight();

  myservo.attach(6);
  myservo2.attach(5);

  pinMode(RPMpin, INPUT_PULLUP);
  attachInterrupt(digitalPinToInterrupt(RPMpin), rpmInterrupt, FALLING);

  LoadCell.begin();
  LoadCell.start(2000);
  LoadCell.setCalFactor(loadCellCalFactor);

myservo.writeMicroseconds(1080);

  delay(10000);

  Serial.println("time,PWM,AMP,Voltage,RPM,g");
}

void loop() {
  measureCurrent();
  controlPWM();
  controlServo();
  measureRPM();
  measureWeight();
  measureVoltage();
  updateLCD();
  serialCsv();
  delay(5); // Adjust the delay as per your requirements
}

void measureCurrent() {
  total -= readings[ndex];
  readings[ndex] = analogRead(amppin);
  readings[ndex] = (readings[ndex] - 510) * 5 / 1024 / 0.04 - 0.52;
  total += readings[ndex];
  ndex++;
  if (ndex >= numReadings)
    ndex = 0;
  average = total / numReadings;
  currentValue = average;
}

void controlPWM() {
  currenttimepwm = millis();
if (currenttimepwm - variabletimepwm >= intervalpwm && val <= cuttoffpwm){
  val = val + pwmincrament;
  variabletimepwm = millis();
  if (val >= cuttoffpwm){
    exit(0);
    }
  }
 
  myservo.writeMicroseconds(val);
}

void controlServo() {
  int val2 = analogRead(potpin2);
  val2 = map(val2, 0, 1023, 0, 180);
  myservo2.write(val2);
}

void measureRPM() {
  if (intervaltime != 0) {
    float RPS = 1000000.0 / intervaltime;
    RPM = 60.0 * RPS;
    lcd.setCursor(0, 3);
    lcd.print(RPM, 1);
    intervaltime = 0;
  }
}

void measureWeight() {
  LoadCell.update();
  float weight = LoadCell.getData();
  weight *= 0.00981;
}

void measureVoltage() {
  float voltage = analogRead(A7);
  voltage = voltage / 1024;
  voltage = voltage * 5;
  voltage = voltage * voltageDividerRatio;
    lcd.setCursor(14, 2);
  lcd.print(voltage); 
  lcd.setCursor(19, 2);
  lcd.print("V");
}

void updateLCD() {
  lcd.setCursor(0, 0);
  lcd.print("amps = ");
  lcd.setCursor(7, 0);
  lcd.print(currentValue);
  lcd.setCursor(12, 0);
  lcd.print("A");
  lcd.setCursor(0, 1);
  lcd.print("PWM = ");
  lcd.setCursor(6, 1);
  lcd.print(myservo.readMicroseconds());
  lcd.setCursor(0, 2);
  lcd.print("servo = ");

  lcd.setCursor(13, 3);
  lcd.print(LoadCell.getData());
  lcd.setCursor(19, 3);
  lcd.print("N");
  lcd.setCursor(6, 3);
  lcd.print("RPM");

  if (myservo2.read() >= 10){
  lcd.setCursor(8, 2);
  lcd.print(myservo2.read());
  } 
    if ( 99 > myservo2.read() >= 10){
  lcd.setCursor(8, 2);
  lcd.print(myservo2.read());
  lcd.setCursor(10, 2);
  lcd.print("  ");
  } 
    if (myservo2.read() < 10){
  lcd.setCursor(8, 2);
  lcd.print(myservo2.read());
  lcd.setCursor(9, 2);
  lcd.print("  ");
  } 
 } 
void serialCsv() {
    float voltage = analogRead(A7);
  voltage = voltage / 1024;
  voltage = voltage * 5;
  voltage = voltage * voltageDividerRatio;

  LoadCell.update();
  float weight = LoadCell.getData();
  weight *= -0.00981;

  unsigned long currenttime = millis();
  Serial.print(currenttime);
  Serial.print(",");
  Serial.print(val);
  Serial.print(",");
  Serial.print(currentValue);
  Serial.print(",");
  Serial.print(voltage);
  Serial.print(",");
  Serial.print(RPM);
  Serial.print(",");
  Serial.println(LoadCell.getData());
}


void rpmInterrupt() {
  unsigned long currentTime = micros();
  intervaltime = currentTime - lastTime;
  lastTime = currentTime;
}
