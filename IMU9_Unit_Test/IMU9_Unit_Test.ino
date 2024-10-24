//Include Libraries
#include <***FILL_IN_HERE***>       // For I2C communication
#include <***FILL_IN_HERE***>    // Library for communicating with LIS3MDL Sensor
#include <***FILL_IN_HERE***>       // Library for communicating with LSM6 Sensor
#include <***FILL_IN_HERE***>         // Library for reading and writing to SD card             

// Define Objects
LIS3MDL mag;                    // Object to handle magnetometer sensor
LSM6 imu;                       // Object to handle IMU (Inertial Measurement Unit) sensor
File dataFile;                  // SD file object to handle writing data to SD card
unsigned long startTime;        // Start time to track elapsed time

// Min and Max calibration values for the magnetometer
LIS3MDL::vector<int16_t> m_min = {-32767, -32767, -32767};
LIS3MDL::vector<int16_t> m_max = {+32767, +32767, +32767}; 

void setup()
{
  Serial.begin(9600);
  while (!Serial);

  Wire.begin(); // Initialize I2C communication

  Serial.println("Starting Pololu IMU-9 v5 Unit Test");

  // Initialize the SD card
  if (!SD.begin(***FILL_IN_HERE***)) {
    Serial.println("SD card initialization failed!");
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
    dataFile.println("***FILL_IN_HERE***,***FILL_IN_HERE***,***FILL_IN_HERE***,***FILL_IN_HERE***"); // Write the header line to the CSV file
    Serial.println("New file created: IMU_data.csv");
  } else {
    Serial.println("Error opening IMU_data.csv");
    while (1);  // Stop the program if file cannot be opened
  }

  // Initialize Magnetomter
  if (!mag.init()) {
    Serial.println("Failed to detect and initialize LIS3MDL magnetometer!");
    while (1);
  }
  
  ***FILL_IN_HERE***; // Enable Magentometer to default settings 

  // Initialize IMU
  if (!imu.init()) {
    Serial.println("Failed to detect and initialize LSM6 IMU!");
    while (1);
  }
  
  ***FILL_IN_HERE***; // Enable IMU to default settings 

  startTime = millis(); // Set start time

  pinMode(LED_BUILTIN, OUTPUT); // initialize digital pin LED_BUILTIN as an output.

  Serial.println("***FILL_IN_HERE***");  // Print to serial monitor that setup finished

}

void loop() {

  // Check if 1 minute has passed
  if (millis() - startTime >= ***FILL_IN_HERE***) {  // 1 minute in milliseconds
    dataFile.close();  // Close the file
    Serial.println("1 minute has passed. Data logging stopped.");
    digitalWrite(LED_BUILTIN, LOW);  // turn the Teensy LED off
    while (1);  // Stop the program
  }

  // Check if the Magnetometer is still connected through intialization
  if (!mag.init()) {
    Serial.println("Error: Failed to read from LIS3MDL magnetometer!"); // Prints error to serial
    dataFile.println("Error: Failed to read from LIS3MDL magnetometer!"); // Prints error to Data File 
    while (1); // Stop the program if Magnetometer cannot be read
  }

  // Check if the IMU is still connected through intialization
  if (!imu.init()) {
    Serial.println("Error: Failed to read from LSM6 IMU!"); // Prints error to serial
    dataFile.println("Error: Failed to read from LSM6 IMU!"); // Prints error to Data File
    while (1); // Stop the program
  }

  // Perform Magnetometer and IMU Readings
  ***FILL_IN_HERE***; 
  ***FILL_IN_HERE***;

  // Calculate heading, pitch, and roll angles from sensor data
  float heading = computeHeading();
  float pitch = computePitch();
  float roll = computeRoll();

  // Log data to the file only if it was opened successfully in setup()
  if (dataFile) {
    // Calculate the timestamp (in seconds) since the program started
    unsigned long timestamp = millis() / ***FILL_IN_HERE***;  // `millis()` returns milliseconds, so divide by a number to get the seconds timestamp

    // Write the timestamp, accelerometer, gyroscope, and magnetometer data to the SD card in CSV format
    dataFile.print(timestamp);               // Write the timestamp
    dataFile.print(",");                     // CSV delimiter (comma)
    dataFile.print(heading);                 // Heading Value
    dataFile.print(",");
    dataFile.print(pitch);                 // Pitch Value
    dataFile.print(",");
    dataFile.println(roll);                 // Accel Value

  } else {
    // If the file can't be accessed, print an error
    Serial.println("Error writing to IMU_data.csv");
  }

  digitalWrite(***FILL_IN_HERE***, ***FILL_IN_HERE***);  // turn the Teensy LED on

  // Delay for 1 second to log data every second. Remember time is in milliseconds
  delay(***FILL_IN_HERE***);
}

//Functions... Outside the scope of this project

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