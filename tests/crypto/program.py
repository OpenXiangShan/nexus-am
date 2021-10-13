import os

spath = './tmp/'
dpath = './src/'

files = os.listdir(spath)

for f in files:
    prefix = f[0:-2].lower().replace('-','_')
    entry = prefix + '_entry'
    init = prefix + '_init'
    code_begin = prefix + '_code_begin'
    code_end = prefix + '_code_end'

    fs = open(spath+f, 'r+')
    fd = open(dpath+f, 'w')

    record = 1
    for line in fs.readlines():
        # if ('RVTEST_SIGBASE' in line): #('RVTEST_CASE' in line) | 
        #     continue
        # if 'RVTEST_CODE_END' in line:
        #     record = 0
        # if record:
        if '.globl rvtest_entry_point' in line:
            fd.write('.globl ' + entry + '\n')
        elif 'rvtest_entry_point:' in line:
            fd.write(entry+':\n')
        elif 'RVTEST_CODE_BEGIN' in line:
            fd.write('RVTEST_CODE_BEGIN ' + init + ' ' + code_begin + '\n')
        elif 'RVTEST_CODE_END' in line:
            fd.write('RVTEST_CODE_END ' + code_end + '\n')
        elif '.section .text.init' in line:
            fd.write(line)
            fd.write('.globl main\n')
            fd.write('main:\n')
        else:
            fd.write(line)

    # fd.write('mv a0, x0;\n')
    # fd.write('.word 0x0000006b\n')


fs.close()
fd.close()
            
