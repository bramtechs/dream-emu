#!/usr/bin/python
import sys
import os

# script that scans all .MTL files (exported by blender)
# and makes it's texture paths relative ("works on my machine!")

if len(sys.argv) < 2:
    print("No asset path given!")
    sys.exit(-1)

input_path = sys.argv[1]
print("Checking", input_path,"...")

def convert_path(path):
    splitted = path.split(r"\\")
    path = r".\\" + splitted[len(splitted)-1]
    return path

def check(path):
    # print("Checking",path,"...")

    # load file in memory
    file = open(path,"r")
    lines = file.readlines()
    file.close()

    # replace lines 
    changed = False
    new_lines = []
    for line in lines:
        if line.startswith("map_Kd"):
            segs = line.split("map_Kd")
            old_path = segs[1].strip()
            new_path = convert_path(old_path)
            if new_path != old_path:
                changed = True
                new_lines.append("map_Kd " + new_path + "\n")
                continue
        new_lines.append(line)

    if not changed:
        return

    # write new file
    file = open(path,"w", newline='')
    for line in new_lines:
        file.write(line)

    print("Patched",path,"!")


# traverse root directory, and list directories as dirs and files as files
for root, dirs, files in os.walk(input_path):
   for file in files:
       if file.endswith(".mtl"):
           check(os.path.join(root, file))

