with open('mips_out/code.txt') as code_file, open ('python/instr.txt') as instr_file:
    instr_list = []
    for line in instr_file:
        instr_list.append(line[:len(line)-1].lower())

    #print instruction set
    print instr_list

    for line in code_file:
        content = line.split('\t\n')
        if len(content) >= 3:
            if content[2] not in instr_list:
                print content[2] + ' is an illegal instruction !'
