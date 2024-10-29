// Include Libraries
#include <Wire.h>              // I2C communication
#include <Adafruit_Sensor.h>   // Adafruit sensor
#include <Adafruit_BMP3XX.h>   // BMP388 pressure/temperature sensor
#include <LIS3MDL.h>    // LIS3MDL Sensor
#include <LSM6.h>       // LSM6 Sensor
#include <SD.h>                // SD card

#define SEALEVELPRESSURE_HPA (1013.25)  // Standard atmospheric pressure at sea level in hPa

// Define Objects
Adafruit_BMP3XX bmp;           // Create an object for the BMP388 sensor
LIS3MDL mag;                    // Object to handle magnetometer sensor
LSM6 imu;                       // Object to handle IMU (Inertial Measurement Unit) sensor
File dataFile;                  // SD file object to handle writing data to SD card
unsigned long startTime;        // Start time to track elapsed time

// Min and Max calibration values for the magnetometer
LIS3MDL::vector<int16_t> m_min = {-32767, -32767, -32767}; 
LIS3MDL::vector<int16_t> m_max = {+32767, +32767, +32767}; 

// RGB LED pin definitions
const int redPin = (Your choice of pin);
const int greenPin = (Your choice of pin);
const int bluePin = (Your choice of pin);

void setup() {
  // Initialize serial communication
  Serial.begin(115200);
  
  // Wait for the serial connection to be established (if connected to a computer)
  while (!Serial);

  // Print a message to the serial monitor to indicate that the setup has started
  Serial.println("Setup Starting...");

  // initialize the digitals pin as an outputs
  pinMode(redPin, OUTPUT);
  pinMode(greenPin, OUTPUT);
  pinMode(bluePin, OUTPUT);

  // Turn the LED off initially
  digitalWrite(redPin, LOW);
  digitalWrite(greenPin, LOW);
  digitalWrite(bluePin, LOW);

  // Initialize the SD card in the Teensy 4.1's built-in SD card slot
  if (!SD.begin(BUILTIN_SDCARD)) {  // Teensy 4.1 has a built-in SD card reader
    // If SD card initialization fails, print an error message and halt execution
    Serial.println("SD card initialization failed on startup!");
    digitalWrite(redPin, HIGH);  // Turn on red LED for SD card failure
    while (1);  // Infinite loop to stop the program
  }

  // Delete the existing data file if it exists
  if (SD.exists("Flight.csv")) {
    SD.remove("Flight.csv");  // Remove existing file
    Serial.println("Existing data file deleted.");
  }

  // Create a new CSV file on the SD card to store data
  dataFile = SD.open("Flight.csv", FILE_WRITE);  // Open (or create) a file for writing
  if (dataFile) {
    // Write the header line to the CSV file (labels for the columns)
    dataFile.println("Timestamp (s),Temperature (C),Pressure (hPa),Altitude (m),Heading (deg),Pitch (deg),Roll (deg)");
  } else {
    // If the file can't be opened, print an error message and halt execution
    Serial.println("Error opening Flight.csv on startup!");
    digitalWrite(redPin, HIGH);
    while (1);  // Infinite loop to stop the program
  }

  Wire1.begin(); // Initialize I2C communication for BMP388

  // Initialize the BMP388 sensor with I2C communication
  if (!bmp.begin_I2C(0x77, &Wire1)) {  // Uses second pair of I2C pins on Teensy 4.1, SDA (pin 17) and SCL (pin 16)
    // If sensor initialization fails, print an error message and halt execution
    Serial.println("Failed to detect and initialize BMP388 on startup!");
    digitalWrite(redPin, HIGH);
    while (1);  // Infinite loop to stop the program
  }

  // Configure BMP388 sensor settings for data acquisition
  bmp.setTemperatureOversampling(BMP3_OVERSAMPLING_8X);  // Set temperature oversampling to 8x for higher precision
  bmp.setPressureOversampling(BMP3_OVERSAMPLING_4X);     // Set pressure oversampling to 4x
  bmp.setIIRFilterCoeff(BMP3_IIR_FILTER_COEFF_3);        // Set the IIR filter to reduce noise
  bmp.setOutputDataRate(BMP3_ODR_100_HZ);                  // Set output data rate to 1 Hz (1 sample per second)

  Wire.begin(); // Initialize I2C communication for IMU
  
  // Initialize IMU (LIS3MDL and LSM6)
  if (!imu.init() || !mag.init()) {
    Serial.println("Failed to detect IMU sensor on startup!");
    digitalWrite(redPin, HIGH);
    while (1); // Halt if IMU initialization fails
  }

  // Default LIS3MDL and LSM6 settings
  imu.enableDefault();
  mag.enableDefault();

  startTime = millis();  // Initialize startTime when the setup is completed

  pinMode(LED_BUILTIN, OUTPUT);   // initialize digital pin LED_BUILTIN as an output.

  Serial.println("Data Collection Starting...");  // SD card initialized successfully

}

void loop() {
  // Check if 10 minutes has passed
  if (millis() - startTime >= ***Fill_In_Here***) { // 10 minutes in milliseconds
    dataFile.close();  // Close the file after 1 minute of data logging
    Serial.println("1 minute has passed. Stopping the data logging.");
    digitalWrite(LED_BUILTIN, LOW);  // turn the LED off (HIGH is the voltage level)
    digitalWrite(redPin, LOW);  // Turn off red LED
    digitalWrite(greenPin, LOW);  // Turn off green LED
    digitalWrite(bluePin, HIGH);   // Turn on blue LED
    while(1);  // Stop the program after data logging completes
  }

  // Verify Datalog file is accessible
  if (SD.exists("Flight.csv")) {
    // Calculate the timestamp (in seconds) since the program started
    unsigned long timestamp = millis() / 1000;  // `millis()` returns milliseconds, so divide by 1000 to get seconds

    dataFile.print(timestamp);               // Write the timestamp
    dataFile.print(",");

    // Take a reading from the BMP388 sensor
    if (bmp.performReading()) {
      dataFile.print(bmp.temperature);
      dataFile.print(",");   
      dataFile.print(bmp.pressure / 100.0);
      dataFile.print(",");   
      dataFile.print(bmp.readAltitude(SEALEVELPRESSURE_HPA));
      dataFile.print(",");   
    } else {
      // If reading fails, print an error message and proceed
      Serial.println("Failed to read BMP388!");
      char temperature = ***Fill_In_Here***;
      char pressure = ***Fill_In_Here***;
      char altitude = ***Fill_In_Here***;
      dataFile.print(temperature);
      dataFile.print(",");   
      dataFile.print(pressure);
      dataFile.print(",");   
      dataFile.print(altitude);
      dataFile.print(",");   
    }

    // Take a reading from the IMU sensor
    if (imu.init() && mag.init()) {
      // Perform Magnetometer and IMU Readings
      mag.read();
      imu.read();
      // Calculate heading, pitch, and roll angles from sensor data
      float heading = computeHeading();
      float pitch = computePitch();
      float roll = computeRoll();
      dataFile.print(heading);
      dataFile.print(",");   
      dataFile.print(pitch);
      dataFile.print(",");   
      dataFile.println(roll);
    } else {
      Serial.println("Failed to read IMU!");
      char heading = ***Fill_In_Here***;
      char pitch = ***Fill_In_Here***;
      char roll = ***Fill_In_Here***;
      dataFile.print(heading);
      dataFile.print(",");   
      dataFile.print(pitch);
      dataFile.print(",");   
      dataFile.println(roll);
    }

    // Addition Here (this will make sense later in the section)
    ***Fill_In_Here***;

    // Turn status LED green if all sensors are connected
    if (bmp.performReading() && imu.init() && mag.init()) {
     digitalWrite(redPin, LOW);    // Turn off red LED
     digitalWrite(greenPin, HIGH);   // Turn on green LED to make yellow
    }

    // Turn status LED yellow if one of the sensors is not connected
    if (!bmp.performReading() || !imu.init() || !mag.init()) {
     digitalWrite(redPin, HIGH);    // Turn on red LED to make yellow
     digitalWrite(greenPin, HIGH);   // Turn on green LED to make yellow
    }

    // Turn status LED red if none of the sensors is not connected
    if (***Fill_In_Here*** && ***Fill_In_Here*** && ***Fill_In_Here***) {
     digitalWrite(redPin, HIGH);    // Turn on red LED to 
     digitalWrite(greenPin, LOW);   // Turn off green LED
    }

  } else {
    Serial.println("Error writing to Flight.csv, SD card may be removed");
    digitalWrite(greenPin, LOW);
    digitalWrite(redPin, HIGH);
    // Start retry attempts
    int retryCount = 0; // Create integer to track retries 
    const int maxRetries = 10; // Set max retry number
    // Enter while loop for up to 10 retries
    while (***Fill_In_Here*** < ***Fill_In_Here***) {
      // Try to reconnect to the SD card 
      if (SD.begin(BUILTIN_SDCARD)) {
        break; // exits the while loop if successful
      } else {
      // Print to serial if reconnect attempt unsuccessful
      Serial.print("Retrying SD card initialization... Attempt ");
      Serial.println(retryCount + 1); 
      retryCount++; //increase count by 1
      delay(500); // wait 0.5 seconds before trying again
      }
    }
    // Stop code once max retry attempts reach
    if (***Fill_In_Here*** == ***Fill_In_Here***) {
      Serial.print("SD is lost...");
      while(1);
    }
    Serial.println("SD connection recovered. Data collection resuming..."); // Signify SD connection recovered
  }

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