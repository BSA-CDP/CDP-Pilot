//Include Libraries
#include <Wire.h>       // For I2C communication
#include <LIS3MDL.h>    // Library for communicating with LIS3MDL Sensor
#include <LSM6.h>       // Library for communicating with LSM6 Sensor
#include <SD.h>         // Library for reading and writing to SD card             

// Define Objects
LIS3MDL mag;                    // Object to handle magnetometer sensor
LSM6 imu;                       // Object to handle IMU (Inertial Measurement Unit) sensor
File dataFile;                  // SD file object to handle writing data to SD card
unsigned long startTime;        // Start time to track elapsed time

LIS3MDL::vector<int16_t> m_min = {-32767, -32767, -32767}; // Min calibration values for the magnetometer
LIS3MDL::vector<int16_t> m_max = {+32767, +32767, +32767}; // Max calibration values for the magnetometer

// RGB LED pin definitions
const int redPin = ***Fill_In_Here***; // Define pin on Teensy for Red
const int greenPin = ***Fill_In_Here***; // Define pin on Teensy for Green
const int bluePin = ***Fill_In_Here***; // Define pin on Teensy for Blue

void setup()
{
  Serial.begin(9600);
  while (!Serial);

  Wire.begin(); // Initialize I2C communication

  Serial.println(" Pololu IMU-9 v5 Unit LED Test");

  // initialize the digitals pin as an outputs
  pinMode(redPin, OUTPUT);
  pinMode(greenPin, OUTPUT);
  pinMode(bluePin, OUTPUT);

  // Turn the LED off initially
  digitalWrite(***Fill_In_Here***, ***Fill_In_Here***);
  digitalWrite(***Fill_In_Here***, ***Fill_In_Here***);
  digitalWrite(***Fill_In_Here***, ***Fill_In_Here***);

  // Initialize the SD card
  if (!SD.begin(BUILTIN_SDCARD)) {
    Serial.println("SD card initialization failed!");
    digitalWrite(redPin, HIGH);  // Turn on red LED for SD card failure
    while (1);  // Stop the program if SD card initialization fails
  }

  // Delete the existing data file if it exists
  if (SD.exists("IMU_data.csv")) {
    SD.remove("IMU_data.csv");
    Serial.println("Existing data file deleted.");
  }
  
  // Create a new CSV file on the SD card
  dataFile = SD.open("IMU_data.csv", FILE_WRITE);
  if (dataFile) {
    dataFile.println("Timestamp,Heading,Pitch,Roll"); // Write the header line to the CSV file
    Serial.println("New file created: IMU_data.csv");
  } else {
    Serial.println("Error opening IMU_data.csv");
    digitalWrite(***Fill_In_Here***, ***Fill_In_Here***);  // Turn on red LED for SD card failure
    while (1);  // Stop the program if file cannot be opened
  }

  // Check if the magnetometer is still connected through intialization
  if (!mag.init())
  {
    Serial.println("Failed to detect and initialize LIS3MDL magnetometer!");
    digitalWrite(***Fill_In_Here***, ***Fill_In_Here***);  // Turn on red LED for SD card failure
    while (1);
  }
  
  mag.enableDefault(); // Enable Magentometer to default settings 

  // Check if the IMU is still connected through intialization
  if (!imu.init())
  {
    Serial.println("Failed to detect and initialize LSM6 IMU!");
    digitalWrite(***Fill_In_Here***, ***Fill_In_Here***);  // Turn on red LED for SD card failure
    while (1);
  }
  
  imu.enableDefault(); // Enable IMU to default settings 

  startTime = millis(); // Set start time

  pinMode(LED_BUILTIN, OUTPUT); // initialize digital pin LED_BUILTIN as an output.

  Serial.println("Data Collection Starting...");  // Print to serial monitor

}

void loop() {

  // Check if 1 minute has passed
  if (millis() - startTime >= 60000) {  // 1 minute in milliseconds
    dataFile.close();  // Close the file
    Serial.println("1 minute has passed. Data logging stopped.");
    digitalWrite(LED_BUILTIN, LOW);  // turn the Teensy LED off
    digitalWrite(redPin, LOW);  // Turn off red LED
    digitalWrite(greenPin, LOW);  // Turn off green LED
    digitalWrite(***Fill_In_Here***, ***Fill_In_Here***);   // Turn on blue LED
    while (1);  // Stop the program
  }

  if (!mag.init()) {
    Serial.println("Error: Failed to read from LIS3MDL magnetometer!"); // Prints error to serial
    dataFile.println("Error: Failed to read from LIS3MDL magnetometer!"); // Prints error to Data File 
    digitalWrite(greenPin, LOW); // Ensure Green LED is off
    digitalWrite(redPin, HIGH);  // Turn on red LED for magnetometer failure
    while (1); // Stop the program if Magnetometer cannot be read
  }

  if (!imu.init()) {
    Serial.println("Error: Failed to read from LSM6 IMU!"); // Prints error to serial
    dataFile.println("Error: Failed to read from LSM6 IMU!"); // Prints error to Data File
    digitalWrite(greenPin, LOW); // Ensure Green LED is off
    digitalWrite(redPin, HIGH);  // Turn on red LED for magnetometer failure
    while (1); // Stop the program
  }

  // Perform Magnetometer and IMU Readings
  mag.read();
  imu.read();

  // Calculate heading, pitch, and roll angles from sensor data
  float heading = computeHeading();
  float pitch = computePitch();
  float roll = computeRoll();

  // Log data to the file only if the SD card can be initialized (or found)
  if (SD.begin(BUILTIN_SDCARD)) {
    // Calculate the timestamp (in seconds) since the program started
    unsigned long timestamp = millis() / 1000;  // `millis()` returns milliseconds, so divide by 1000 to get seconds

    // Write the timestamp, accelerometer, gyroscope, and magnetometer data to the SD card in CSV format
    dataFile.print(timestamp);               // Write the timestamp
    dataFile.print(",");                     // CSV delimiter (comma)
    dataFile.print(heading);                 // Accel X
    dataFile.print(",");
    dataFile.print(pitch);                 // Accel Y
    dataFile.print(",");
    dataFile.println(roll);                 // Accel Z

    digitalWrite(redPin, LOW);    // Turn off red LED
    digitalWrite(***Fill_In_Here***, ***Fill_In_Here***);   // Turn on green LED for successful logging
  } else {
    // If the file can't be accessed, print an error
    Serial.println("Error writing to IMU_data.csv");
    digitalWrite(redPin, HIGH);  // Turn on red LED for magnetometer failure
  }

  digitalWrite(LED_BUILTIN, HIGH);  // turn the Teensy LED on (HIGH is the voltage level)

  // Delay for 1 second (1000 ms) to log data every second
  delay(1000);
}

//Functions Below... Outside the scope of this project

// Compute the heading
template <typename T> float computeHeading(LIS3MDL::vector<T> from)
{
  LIS3MDL::vector<int32_t> temp_m = {mag.m.x, mag.m.y, mag.m.z};
  LIS3MDL::vector<int16_t> a = {imu.a.x, imu.a.y, imu.a.z};

  temp_m.x -= ((int32_t)m_min.x + m_max.x) / 2;
  temp_m.y -= ((int32_t)m_min.y + m_max.y) / 2;
  temp_m.z -= ((int32_t)m_min.z + m_max.z) / 2;

  LIS3MDL::vector<float> E;
  LIS3MDL::vector<float> N;
  LIS3MDL::vector_cross(&temp_m, &a, &E);
  LIS3MDL::vector_normalize(&E);
  LIS3MDL::vector_cross(&a, &E, &N);
  LIS3MDL::vector_normalize(&N);

  float heading = atan2(LIS3MDL::vector_dot(&E, &from), LIS3MDL::vector_dot(&N, &from)) * 180 / PI;
  if (heading < 0) heading += 360;
  return heading;
}

float computeHeading()
{
  return computeHeading((LIS3MDL::vector<int>){1, 0, 0});
}

// Compute the pitch angle (forward/backward tilt)
float computePitch()
{
  // Convert accelerometer readings to g-force
  float ax = imu.a.x / 32768.0 * 2.0;  // Assuming +/-2g sensitivity
  float ay = imu.a.y / 32768.0 * 2.0;
  float az = imu.a.z / 32768.0 * 2.0;

  // Calculate pitch in degrees
  float pitch = atan2(-ax, sqrt(ay * ay + az * az)) * 180 / PI;
  return pitch;
}

// Compute the roll angle (side-to-side tilt)
float computeRoll()
{
  // Convert accelerometer readings to g-force
  float ay = imu.a.y / 32768.0 * 2.0;  // Assuming +/-2g sensitivity
  float az = imu.a.z / 32768.0 * 2.0;

  // Calculate roll in degrees
  float roll = atan2(ay, az) * 180 / PI;
  return roll;
}