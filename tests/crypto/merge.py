import os

files = os.listdir('src')
srcfile = 'src/crypto.S'

fd = open(srcfile, 'w')

for f in files:
    record = 0
    fs = open('src/'+f, 'r+')
    for line in fs.readlines():
        if 'RVTEST_CODE_BEGIN' in line:
            record = 1
        elif 'RVMODEL_HALT' in line:
            record = 0

        if record :
            if 'inst_' in line:
                fd.write(f[0:-2].lower().replace('-','_')+'_')
                fd.write(line.lstrip())
            else:
                fd.write(line)


fs.close()
fd.close()
