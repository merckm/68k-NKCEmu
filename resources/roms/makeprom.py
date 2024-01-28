# This script creates EPRM fles file
# The file is filled with 0xE5 bytes, which is the standard value for uninitialized data on a floppy disk

import sys
print ('argument list', sys.argv)
type = 3
if len(sys.argv) > 1:
    type = int(sys.argv[1])

if( type <1 or type > 3):
    print ('Valid arguments are 1 (2716 PROM),2 (2732 PROM) or 3 (2764 PROM)')
    print ('Defaulting to 2764 PROM if no argument is passed')
    exit(1)

if( type == 1):
    size = 2048
if( type == 2):
    size = 4096
if( type == 3):
    size = 8192

# Create a byte array of 800*1024 bytes, filled with '0xE5'
# 0xE5 is the standard value for uninitialized data on a floppy disk
bytes = bytearray([0xFF] * size)

# Write the bytes to the file
with open("prom.bin", "wb") as out_file:
    out_file.write(bytes)

if( type == 1):
    print ('Created 2716 PROM file prom.bin!')    

if( type == 2):
    print ('Created 2732 PROM file prom.bin!')
    
if( type == 3):
    print ('Created 2764 PROM file prom.bin!')