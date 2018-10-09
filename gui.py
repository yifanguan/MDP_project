import tkinter

top = tkinter.Tk()

C = tkinter.Canvas(top, bg="blue", height=250, width=300)

coord = 10, 50, 240, 210
arc = C.create_arc(coord, start=0, extent=150, fill="red")

C.pack()
top.mainloop()


# from tkinter import *

# class Window(Frame):

#     def __init__(self, master=None):
#         Frame.__init__(self, master)               
#         self.master = master

# root = Tk()

# app = Window(root)

# root.mainloop()
