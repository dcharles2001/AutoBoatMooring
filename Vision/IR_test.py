import smbus
import time

IRsensorAddress = 0xB0
slaveAddress = IRsensorAddress >> 1
ledPin = 13
ledState = False
Ix = [0] * 4
Iy = [0] * 4

bus = smbus.SMBus(1)

def write_2bytes(d1, d2):
    try:
        bus.write_i2c_block_data(slaveAddress, d1, [d2])
    except IOError as e:
        print(f"Error writing to I2C device: {e}")

def setup():
    global slaveAddress
    slaveAddress = IRsensorAddress >> 1
    print("Press Ctrl+C to exit.")
    try:
        bus.write_byte(slaveAddress, 0x30)
        time.sleep(0.01)
        write_2bytes(0x30, 0x01)
        time.sleep(0.01)
        write_2bytes(0x30, 0x08)
        time.sleep(0.01)
        write_2bytes(0x06, 0x90)
        time.sleep(0.01)
        write_2bytes(0x08, 0xC0)
        time.sleep(0.01)
        write_2bytes(0x1A, 0x40)
        time.sleep(0.01)
        write_2bytes(0x33, 0x33)
        time.sleep(0.01)
        time.sleep(0.1)
    except IOError as e:
        print(f"Error initializing I2C device: {e}")

def loop():
    global ledState
    ledState = not ledState
    try:
        bus.write_byte(slaveAddress, 0x36)
        time.sleep(0.1)
        data_buf = bus.read_i2c_block_data(slaveAddress, 0, 16)

        for i in range(4):
            Ix[i] = data_buf[1 + 3 * i] + ((data_buf[3 + 3 * i] & 0x30) << 4)
            Iy[i] = data_buf[2 + 3 * i] + ((data_buf[3 + 3 * i] & 0xC0) << 2)

        for i in range(4):
            print(f"{Ix[i]:03},{Iy[i]:03}", end=",")
        print("")
        time.sleep(0.015)
    except IOError as e:
        print(f"Error reading from I2C device: {e}")

if __name__ == "__main__":
    setup()
    try:
        while True:
            loop()
    except KeyboardInterrupt:
        print("\nExiting the program.")
