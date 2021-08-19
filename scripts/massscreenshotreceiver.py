#!/usr/bin/python3
# encoding=utf-8

# Serial receiver
# Teensy sends screenshots, auto-png conversion
# c:d:p for screenshots
# https://www.designer2k2.at
# 2021.07.31


import serial
import screenshotreceive2


def screenshotcatcher(filename, comport, storefile):
    #  open serial
    ser = serial.Serial(comport, 2000000, timeout=0.25)
    # send request
    ser.write(b"c:d:p\n")  # write request
    ser.flush()
    # wait for begin

    stringfile = ""

    running = True
    while running:
        ser_line = ser.read(60)
        if "DATA END" in stringfile:
            print("Receive done.")
            running = False
        stringfile += ser_line.decode()
    ser.close()

    # extract the image part:
    # between "START ====" and "==== PIXEL DATA END"
    start = "START ====\r\n"
    end = "\r\n==== PIXEL DATA END"
    extract = (stringfile.split(start))[1].split(end)[0]
    screenshotreceive2.string2screenshot(extract, filename, False)

    if storefile:
        f = open("screenshotraw.txt", "w")
        f.write(stringfile)
        f.close()


if __name__ == "__main__":
    for x in range(5):
        print("Run {} from {}".format(x, 150))
        imagefilename = "received\{0:05d}.PNG".format(x)
        screenshotcatcher(imagefilename, "COM5", False)
    # run: ffmpeg -framerate 12 -i "%05d.png" video.mp4
