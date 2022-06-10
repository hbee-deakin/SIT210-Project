# Imports the RPi.GPIO module and names it GPIO
import RPi.GPIO as GPIO
# Imports the library used the RC522 RFID sensor
from mfrc522 import SimpleMFRC522
# Import the time library
import time
# Import the sleep module from the time library
from time import sleep
# Import the SMBus module from the smbus libray
from smbus import SMBus
# Import the os.path library
import os.path
# Import the data time library
import datetime
# Import the CSV library
import csv

# Set the GPIO warnings off
GPIO.setwarnings(False)

# Set the GPIO mode to board
GPIO.setmode(GPIO.BOARD)

# Initialise the address variable as master and set to address 8
addr = 0x8

# Initialise the bus variable for I2C
bus = SMBus(1)

# Initialise the reader variable to the RFID RC522 library function
reader = SimpleMFRC522()

# Used to get the current date
def date_now():
    today = datetime.datetime.now().strftime("%d-%m-%Y")
    today = str(today)
    return(today)

# Used to get the current time
def time_now():
    now = datetime.datetime.now().strftime("%H:%M:%S")
    now = str(now)
    return(now)

# Used to write data from reading to a CSV file
def write_to_csv(id, name, status):
    # Check if the CSV file exists, if not, create it with the header labels
    if os.path.exists('./petdoor_record.csv') == False:
        with open('./petdoor_record.csv', mode='a') as sensor_readings:
            sensor_write = csv.writer(sensor_readings, delimiter=',', quotechar='"', quoting=csv.QUOTE_MINIMAL)
            write_to_log = sensor_write.writerow(["pet_name", "tag_id", "status", "date","time"])

    # Add the data from the reading to the CSV file
    with open('./petdoor_record.csv', mode='a') as sensor_readings:
        sensor_write = csv.writer(sensor_readings, delimiter=',', quotechar='"', quoting=csv.QUOTE_MINIMAL)
        write_to_log = sensor_write.writerow([name, id, status, date_now(),time_now()])
        return(write_to_log)

# The initialise function resets the state to the beginning
# This makes sure that both devices are in sync when the devices start up
def init():
    # Print to the console "Pet is Home", used for testing
    print("Pet is Home")
    # Print to the console "Ready...", used for testing
    print("Ready...")
    # Send a message to the slave device to set the state to initialise
    bus.write_byte(addr, 0x2)
    # Get the current reading from the RFID sensor and save the ID and name
    id, name = reader.read()
    # Write the new data to the CSV file
    write_to_csv(id, name[:4], "departed")
    # Send a message to the slave device to set the state to pet Exited
    bus.write_byte(addr, 0x1)
    # Run the away function
    away()

# The away function sets the state to away and waits for new reading
def away():
    # Print to the console "Pet Departed", used for testing
    print("Pet Departed")
    # Delay the program for 15 seconds to prevent overloading particle.io publish events
    sleep(15)
    # Print to the console "Ready...", used for testing
    print("Ready...")
    # Get the current reading from the RFID sensor and save the ID and name
    id, name = reader.read()
    # Write the new data to the CSV file
    write_to_csv(id, name[:4], "returned")
    # Send a message to the slave device to set the state to pet Entered
    bus.write_byte(addr, 0x0)
    # Run the home function
    home()

# The away function sets the state to home and waits for new reading
def home():
    # Print to the console "Pet Returned", used for testing
    print("Pet Returned")
    # Delay the program for 15 seconds to prevent overloading particle.io publish events
    sleep(15)
    # Print to the console "Ready...", used for testing
    print("Ready...")
    # Get the current reading from the RFID sensor and save the ID and name
    id, name = reader.read()
    # Write the new data to the CSV file
    write_to_csv(id, name[:4], "departed")
    # Send a message to the slave device to set the state to pet Exited
    bus.write_byte(addr, 0x1)
    # Run the away function
    away()

# Sets the program to the initialise function when the program begins
init()

# Cleans up the GPIO pins at the end of the program
GPIO.cleanup()