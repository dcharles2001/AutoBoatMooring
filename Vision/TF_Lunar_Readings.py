######################################################
# Copyright (c) 2021 Maker Portal LLC
# Author: Joshua Hrisko
######################################################
#
# TF-Luna Mini LiDAR wired to a Raspberry Pi via UART
# --- testing the distance measurement from the TF-Luna
#
#
######################################################
#
import serial,time
import numpy as np
#
##########################
# TFLuna Lidar
##########################
#
ser = serial.Serial("/dev/ttyS0", 115200,timeout=0) # mini UART serial device
#
############################
# read ToF data from TF-Luna
############################
#
############################
# Adustable Variables
############################
sampleSize = 100        #How many measurements per Averaged reading
readingsReq = 10        #How many Averaged readings outputted
def main():
    reading = 0
    while reading<readingsReq:
        sample = 1
        totalDist = 0
        while sample<sampleSize:
            def read_tfluna_data():
                while True:
                    counter = ser.in_waiting # count the number of bytes of the serial port
                    if counter > 8:
                        bytes_serial = ser.read(9) # read 9 bytes
                        ser.reset_input_buffer() # reset buffer

                        if bytes_serial[0] == 0x59 and bytes_serial[1] == 0x59: # check first two bytes
                            distance = bytes_serial[2] + bytes_serial[3]*256 # distance in next two bytes
                            strength = bytes_serial[4] + bytes_serial[5]*256 # signal strength in next two bytes
                            return distance/100.0,strength

            if ser.isOpen() == False:
                ser.open() # open serial port if not open

            distance,strength = read_tfluna_data() # read values
            totalDist = totalDist + distance
            sample+=1
        avgDist = totalDist/100
        print('Average Distance : {0:2.2f} m'.\
            format(avgDist))
        reading+=1
if __name__ == "__main__":
    main()
ser.close() # close serial port