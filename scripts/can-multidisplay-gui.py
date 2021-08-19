#!/usr/bin/python3
# encoding=utf-8

# Serial receiver
# https://www.designer2k2.at
# 2021.08.19

import tkinter as tk
from tkinter import *
import serial.tools.list_ports
import time

import massscreenshotreceiver


if __name__ == "__main__":
    # comports:
    print()

    OptionList = [comport.device for comport in serial.tools.list_ports.comports()]

    app = tk.Tk()

    app.geometry('400x300')

    variable = tk.StringVar(app)
    variable.set(OptionList[0])

    opt = tk.OptionMenu(app, variable, *OptionList)
    opt.config(width=90, font=('Helvetica', 12))
    opt.pack()



    labelTest = tk.Label(text="", font=('Helvetica', 12), fg='blue')
    labelTest.pack(side="top")

    canvas = Canvas(app, width=320, height=240)
    canvas.pack()
    img = PhotoImage(file="test.png")
    image_on_canvas = canvas.create_image(0, 0, anchor=NW, image=img)


    def callback(*args):
        labelTest.configure(text="The selected port is {}".format(variable.get()))
        massscreenshotreceiver.screenshotcatcher("test.png", variable.get(), False)
        img = PhotoImage(file="test.png")
        canvas.itemconfig(image_on_canvas, image=img)
        app.update_idletasks()


    variable.trace("w", callback)

    app.mainloop()

# Selectable Comport from available list

# shows remote firmware version (c:v command)

# Fetches Filelist and gives selection  to download (c:l command)

# File transfer (other script)

# Screenshot button (other script)
