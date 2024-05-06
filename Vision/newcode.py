import serial
import time

# Define UART parameters
uart_port = '/dev/ttyS0'  # UART port on Raspberry Pi (adjust as needed)
baud_rate = 115200  # Baud rate of TF Luna Lidar sensor

# Create Serial object
ser = serial.Serial(uart_port, baud_rate, timeout=1)

def read_lidar_distance():
    # Send command to TF Luna Lidar to request distance measurement
    ser.write(b'\x42\x57\x02\x00\x00\x00\x01\x06')

    # Wait for the response (delay time may vary based on the sensor)
    time.sleep(0.1)

    # Read the response from TF Luna Lidar (7 bytes)
    response = ser.read(7)

    # Parse the response to get the distance value
    distance = (response[3] << 8) + response[4]

    return distance

try:
    while True:
        distance = read_lidar_distance()
        print(f"Distance: {distance} mm")
        time.sleep(1)

except KeyboardInterrupt:
    print("Program terminated by user.")
finally:
    ser.close()

