#include <*** FILL_IN_HERE ***>              // For I2C communication
#include <*** FILL_IN_HERE ***>              // Adafruit sensor library
#include "*** FILL_IN_HERE ***"              // Library for BMP388 pressure/temperature sensor
#include <*** FILL_IN_HERE ***>              // SD card library for reading and writing to SD card

#define SEALEVELPRESSURE_HPA (*** FILL_IN_HERE ***)  // Define the sea level pressure in hPa (standard atmospheric pressure)

Adafruit_BMP3XX bmp;           // Create an object for the BMP388 sensor
File dataFile;                 // File object to handle writing data to the SD card

void setup() {
  // Initialize serial communication for debugging (with baud rate 115200)
  Serial.begin(115200);
  
  // Wait for the serial connection to be established (if connected to a computer)
  while (!Serial);

  // Print a message to the serial monitor to indicate that the setup has started
  Serial.println("Teensy 4.1 BMP388 Unit Test");

  // Initialize the SD card. BUILTIN_SDCARD refers to Teensy 4.1's built-in SD card slot
  if (!SD.begin(*** FILL_IN_HERE ***)) {
    // If SD card initialization fails, print an error message and halt execution
    Serial.println("SD card initialization failed!");
    while (1);  // Infinite loop to stop the program
  }
  Serial.println("SD card initialized.");  // SD card initialized successfully

  // Initialize the BMP388 sensor with I2C communication
  if (!bmp.begin_I2C()) {  // Default I2C pins for Teensy 4.1 are SDA (pin 18) and SCL (pin 19)
    // If sensor initialization fails, print an error message and halt execution
    Serial.println("Could not find a valid BMP388 sensor, check wiring!");
    while (1);  // Infinite loop to stop the program
  }

  // Configure BMP388 sensor settings for data acquisition
  bmp.setTemperatureOversampling(BMP3_OVERSAMPLING_8X);  // Set temperature oversampling to 8x for higher precision
  bmp.setPressureOversampling(BMP3_OVERSAMPLING_4X);     // Set pressure oversampling to 4x
  bmp.setIIRFilterCoeff(BMP3_IIR_FILTER_COEFF_3);        // Set the IIR filter to reduce noise
  bmp.setOutputDataRate(BMP3_ODR_*** FILL_IN_HERE ***_HZ);                  // Set output data rate to 1 Hz (1 sample per second)

  // Create a new CSV file on the SD card to store data
  dataFile = SD.open("BMP388_data.csv", FILE_WRITE);  // Open (or create) a file for writing
  if (dataFile) {
    // Write the header line to the CSV file (labels for the columns)
    dataFile.println("Timestamp,Temperature (C),Pressure (hPa),Altitude (m)");
    dataFile.close();  // Close the file after writing the header
  } else {
    // If the file can't be opened, print an error message and halt execution
    Serial.println("Error opening BMP388_data.csv");
    while (1);  // Infinite loop to stop the program
  }
}

void loop() {
  // Take a reading from the BMP388 sensor
  if (!bmp.performReading()) {
    // If reading fails, print an error message and skip to the next loop iteration
    Serial.println("Failed to perform reading :(");
    return;
  }

  // Open the data file on the SD card for appending data
  dataFile = SD.open("BMP388_data.csv", FILE_WRITE);  // Open file in append mode
  if (dataFile) {
    // Calculate the timestamp (in seconds) since the program started
    unsigned long timestamp = millis() / 1000;  // `millis()` returns milliseconds, so divide by 1000 to get seconds

    // Write the timestamp and sensor data to the SD card in CSV format
    dataFile.print(timestamp);               // Write the timestamp
    dataFile.print(",");                     // CSV delimiter (comma)
    dataFile.print(bmp.temperature);         // Write the temperature in Celsius
    dataFile.print(",");                     // CSV delimiter
    dataFile.print(bmp.pressure / 100.0);    // Write the pressure in hPa (Pa to hPa conversion)
    dataFile.print(",");                     // CSV delimiter
    dataFile.println(bmp.readAltitude(SEALEVELPRESSURE_HPA));  // Write the calculated altitude in meters

    // Close the file after writing
    dataFile.close();
  } else {
    // If the file can't be opened, print an error message (no infinite loop here since it's in the main loop)
    Serial.println("Error writing to BMP388_data.csv");
  }

  // Wait for 1 second before taking another reading
  delay(*** FILL_IN_HERE ***);  // Units are in milliseconds
}
