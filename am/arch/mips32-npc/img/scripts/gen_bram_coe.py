#!/usr/bin/python
# -*- coding: utf-8 -*-
"""
This file helps transform the bin file to coe file like:
memory_initialization_radix=16;
memory_initialization_vector=
..., ..., ...,

"""
import binascii
import sys

if len(sys.argv) != 3:
    print "Usage: gen_bram_coe.py inputFilePath outputFilePath\n"
    exit()

inPath = sys.argv[1]
outPath = sys.argv[2]

with open(inPath, 'rb') as f:
    bins = f.read()

while len(bins)%4 :
    bins = bins + '\0'

with open(outPath, 'w') as f:
    f.write('memory_initialization_radix=16;\nmemory_initialization_vector=\n')
    for i in range(0, len(bins)/4):
        ii = i * 4
        contents = binascii.b2a_hex(bins[ii + 3] + bins[ii + 2] + bins[ii + 1] + bins[ii])
	f.write( contents )
	if i != (len(bins)/4-1):
	    f.write(',\n')
	else:
	    f.write(';')
    f.write('\n')

print "{0} -> {1} complete!".format(inPath, outPath)
