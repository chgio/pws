// calibrator.ino
// helper software to record the calibration readings
// and print them out through serial for calibration.h


long soilMoisture;
long soilMoistureTot;
long soilMoistureN;
long soilMoistureAvg;

void setup()
{
   pinMode(D5, OUTPUT);    // soil moisture sensor power pin
   pinMode(A1, INPUT);     // soil moisture sensor data pin
   
   delay(1000);

   Serial.begin(9600);
}

void loop()
{
   /* Soil moisture: place the sensor
   about 3/4 of its way inside the target soil, manipulate its moisture
   and record the readings:
   * completely dry soil:              0        %RH
   * completely wet soil:              100      %RH
   */

   digitalWrite(D5 , HIGH);
   delay(250);

   soilMoisture = analogRead(A1);
   delay(250);
   
   digitalWrite(D5, LOW);
   delay(250);

   soilMoistureTot += soilMoisture;
   soilMoistureN++;
   soilMoistureAvg = soilMoistureTot / soilMoistureN;

   Serial.printlnf("\tSoil Moist: %d\tRunning Avg: %d", soilMoisture, soilMoistureAvg);

   delay(1500);
}