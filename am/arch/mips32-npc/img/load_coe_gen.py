with open('ram.txt') as inf, open('loader.coe', 'w') as f:
    print >> f, 'memory_initialization_radix = 16;'
    print >> f, 'memory_initialization_vector =',
    for line in inf:
        useless, content, useless2 = line.split(' ')
        print >> f, content + ',',
    print "generated COE file"
