import tkinter
import sys


# def main():
    # shapes_info = parse(sys.argv[1])
    # draw(shapes_info)

# def parse(filename):
#     pass


# def draw(shapes_info):

# parse shapes
coords_to_draw = []
shape = ''
with open('output', 'r+') as f:
    shapes = f.readlines()
    for line in shapes:
        type_coords = line.strip(')').split('(')
        shape = type_coords[0]
        coords = type_coords[1].split(', ')
        for coord in coords:
            coord_pair = coord.split()
            coords_to_draw.append(coord_pair[0])
            coords_to_draw.append(coord_pair[1])


# draw shapes
top = tkinter.Tk()

C = tkinter.Canvas(top, bg="white", height=250, width=300)

if shape == 'LINESTRING':
    C.create_line(coords_to_draw, fill="red")
elif shape == 'POLYGON':
    C.create_polygon(coords_to_draw, fill="red")
elif shape == 'POINT':
    C.create_point(coords_to_draw, fill="red")
elif shape == 'ARC':
    C.create_arc(coords_to_draw, fill="red")


C.pack()
top.mainloop()


# coord = [0, 0, 100, 100]
# arc = C.create_arc(coord, start=0, extent=180, fill="red")
# arc = C.create_arc(200, 200, 300, 300, start=0, extent=180, fill="red")
# arc = C.create_arc(coord, start=0, extent=180, fill="red")


    # from tkinter import *

    # class Window(Frame):

    #     def __init__(self, master=None):
    #         Frame.__init__(self, master)               
    #         self.master = master

    # root = Tk()

    # app = Window(root)

    # root.mainloop()
