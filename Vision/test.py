import serial
import time

def read_lidar_data(serial_port):
    # Sending the command to request distance measurement
    serial_port.write(b'\x5A\x05\x82\x02\x66')

    # Waiting for the response
    time.sleep(0.1)

    # Reading the response (9 bytes)
    response = serial_port.read(9)

    # Checking if the response is valid
    if len(response) == 9 and response[0] == 0x5A and response[1] == 0x5A:
        distance_mm = (response[3] << 8) + response[2]
        strength = response[4]
        return distance_mm, strength
    else:
        return None, None

def main():
    # Adjust the port and baudrate based on your setup
    serial_port = serial.Serial("/dev/ttyS0", baudrate=115200, timeout=0.1)

    try:
        while True:
            distance, strength = read_lidar_data(serial_port)

            if distance is not None:
                print(f"Distance: {distance} mm, Strength: {strength}")
            else:
                print("Invalid response. Check the LIDAR connection.")

            time.sleep(1)

    except KeyboardInterrupt:
        # Close the serial port on program exit
        serial_port.close()

if __name__ == "__main__":
    main()
