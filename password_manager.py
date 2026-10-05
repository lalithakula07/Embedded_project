import serial
import time
import getpass

PORT = "COM6"
BAUD_RATE = 9600

def read_until(ser, text):
    while True:
        line = ser.readline().decode("utf-8", errors="ignore").strip()

        if line:
            print("Arduino:", line)

        if text.lower() in line.lower():
            return

def add_account():
    site = input("Site: ").strip()
    username = input("Username: ").strip()
    password = getpass.getpass("Password: ")

    print("\nConnecting to Arduino...")

    ser = serial.Serial(PORT, BAUD_RATE, timeout=1)

    time.sleep(2)

    print("Connected.\n")

    ser.write(b"ADD\n")

    read_until(ser, "Enter site name:")
    ser.write((site + "\n").encode("utf-8"))

    read_until(ser, "Enter username:")
    ser.write((username + "\n").encode("utf-8"))

    read_until(ser, "Enter password:")
    ser.write((password + "\n").encode("utf-8"))

    time.sleep(1)

    while ser.in_waiting:
        line = ser.readline().decode("utf-8", errors="ignore").strip()

        if line:
            print("Arduino:", line)

    ser.close()

    print("\nAccount transfer complete.")

add_account()