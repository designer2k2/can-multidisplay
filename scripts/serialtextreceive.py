#!/usr/bin/python3
# encoding=utf-8

# Serial receiver
# Teensy sends file over Serial
# https://www.designer2k2.at
# 2021.04.22

import serial


def serialtextreceiver(comport, filename):
    print("Receive file: {} over port: {}".format(filename, comport))
    #  open serial
    ser = serial.Serial(comport, 2000000, timeout=1)
    # send request
    sendreq = 'c:s:{}.TXT\n'.format(filename)
    sendreq = sendreq.encode()
    ser.write(sendreq)
    ser.flush()
    # wait for begin

    running = True
    stringfile = ""

    while running:
        ser_byte = ser.read(60)
        if b"\x04" in ser_byte:
            running = False
        stringfile += ser_byte.decode()
    ser.close()
    print("Seems to have worked, Size received: {} bytes".format(len(stringfile)))

    # trim the result:
    start = chr(2)
    end = chr(4)
    extract = (stringfile.split(start))[1].split(end)[0]

    # the file has \r\r\n so CR CR LF,  the double CR must go.
    extract = extract.replace(r"\r\r\n", r"\r\n")

    f = open("{}.txt".format(filename), "w")
    f.write(extract)
    f.close()


if __name__ == "__main__":
    serialtextreceiver("COM5", "L210807")
