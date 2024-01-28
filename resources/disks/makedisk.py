# This script creates a 800KB floppy disk image file
# The file is filled with 0xE5 bytes, which is the standard value for uninitialized data on a floppy disk

# Create a byte array of 800*1024 bytes, filled with '0xE5'
# 0xE5 is the standard value for uninitialized data on a floppy disk
bytes = bytearray([0xE5] * 800*1024)

# Write the bytes to the file
with open("floppy.img", "wb") as out_file:
    out_file.write(bytes)
