const int AirValue = 783;    //you need to replace this value with Value_1
const int WaterValue = 381;  //you need to replace this value with Value_2
int soilMoistureValue = 0;
int soilmoisturepercent=0;
int motorPin = 9;           // define the pin the motor is connected to
                            // (if you use pin 9,10,11 or 3you can also control speed)
int onSpeed = 100;          // a number between 0 (stopped) and 255 (full speed)
int onTime = 2500;          // the number of milliseconds for the motor to turn on for
int soakTime = 60000;       // the minimum delay time to allow water to soak in before next watering session
  
void setup() {
  Serial.begin(9600); // open serial port, set the baud rate to 9600 bps
}

void loop() {
  soilMoistureValue = analogRead(A0);  //put Sensor insert into soil
  Serial.println(soilMoistureValue);
  soilmoisturepercent = map(soilMoistureValue, AirValue, WaterValue, 0, 100);
  if(soilmoisturepercent >= 100) {
    Serial.println("100 %");
  }
  else if(soilmoisturepercent <=0) {
    Serial.println("0 %");
  }
  else if(soilmoisturepercent >0 && soilmoisturepercent < 100) {
    Serial.print(soilmoisturepercent);
    Serial.println("%");
    if (soilmoisturepercent < 70) {
       analogWrite(motorPin, onSpeed);   // turns the motor On
       delay(onTime);                    // waits for onTime milliseconds
       analogWrite(motorPin, 0);         // turns the motor Off
    } 
  delay(soakTime);                       // allow time for water to soak in
  }
}
