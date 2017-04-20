#!/usr/bin/python
# -*- coding: utf-8 -*-
"""
This file helps transform the bin file to ascii text, and rearrange them into the format 
that can be accepted by readmem of verilog task. The format is like this:
@0 xxxxxxxx
@1 xxxxxxxx
@2 xxxxxxxx

"""
import binascii
import sys

if len(sys.argv) != 3:
    print "Usage: bin2text.py inputFilePath outputFilePath\n"
    exit()

inPath = sys.argv[1]
outPath = sys.argv[2]
# fileSize = 8192
with open(inPath, 'rb') as f:
    bins = f.read()
with open(outPath, 'w') as f:
    for i in range(0, len(bins)/4):
        contents = binascii.b2a_hex(bins[i*4:i*4+4])
        contents = contents[:2]+contents[2:4]+contents[4:6]+contents[6:]
        # f.write("ram[{0:d}]=32'h{1:s}; \n".format(i, contents))
        f.write("@{0:x} {1:s} \n".format(i, contents))
    # for i in range(len(bins)/4,fileSize):
        # f.write("@{0:x} {1:d} \n".format(i, 0))
print "Process complete!"