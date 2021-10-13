

filename = '/home/zfw/nexus-am/apps/bitasm/src/test.Source'
fileoutname = '/home/zfw/nexus-am/apps/bitasm/src/test.S'
fs = open(filename ,'r+')
fout = open(fileoutname, 'w')

interest = ['include', 'pcnt', 'clz', 'ctz', 'pack', 'packw', 'packh', 'clmul', 'max', 'min','andn','orn','xnor','sbclr','sbinv','sbext','sbset','rol','ror']
uninterest = ['clmulw', 'clmulrw', 'clmulhw','packu','packuw','sbclrw','sbinvw','sbextw','sbsetw','sbclriw','sbinviw','sbextiw','sbsetiw']

complete = [
'test_complete:\n',
'    la x2, test_pass\n',
'    jr x2\n',
'\n',
'.align 4\n',
'test_result:\n',
'    .fill 1, 4, -1\n',
'str_Test_Pass:\n',
'    .string "Test Pass"\n',
'str_Test_Fail:\n',
'    .string "Test Fail"\n']

record = 0
for line in fs.readlines():
    if(line[0] == '#'):
        for i in interest:
            if i in line:
                
                for j in uninterest:
                    if(j in line):
                        record = 0
                        break
                if(('clmulw'in line) | ('clmulrw' in line) | ('clmulhw' in line) | ('packu' in line) | ('packuw' in line) | 
                ('sbclrw' in line) | ('sbinvw' in line) | ('sbextw' in line) | ('sbsetw' in line) | 
                ('sbclriw' in line) | ('sbinviw' in line) | ('sbextiw' in line) | ('sbsetiw' in line)):
                    record = 0
                else:
                    record = 1
                    break
            else:
                record = 0
    if record:
        fout.write(line)

for line in complete:
    fout.write(line)

fout.close()
fs.close()
