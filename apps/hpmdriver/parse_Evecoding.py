import os
import re

xs = os.environ['NOOP_HOME']
am = os.environ['AM_HOME']
assert(xs)
assert(am)
# grep -r -n generatePerfEvent src > generatePerfEventlog
generatePerfEventlog = os.path.join(xs, 'generatePerfEvent_log')
# make emu EMU_THREADS=8 -j24 > generate_log 2>&1 &
generate_log = os.path.join(xs,'generate_log')
events_header =  os.path.join(am, 'apps/hpmdriver/events.h')

# open src file that has perfEvents and parse
def parse_events_src():
    with open(generatePerfEventlog, 'r') as f:
        c = f.read()
    regexp = '(.*scala):'
    paths = re.findall(regexp, c)

    all_events = []
    for path in paths:
        module = path.split('/')[-1].split('.')[0]

        with open(os.path.join(xs, path), 'r') as f:
            c = f.read()

        # the following is to match content in Seq()
        pf = 'val perfEvents = Seq'
        st = c.find(pf)
        if st == -1:
            continue
        st = st + len(pf)
        cnt = 0
        for i in range(10000):
            ch = c[st+i]
            if ch == '(':
                cnt += 1
            elif ch == ')':
                cnt -= 1
            if cnt == 0:
                content = c[st:st+i+1]
                break  
        
        # turn Seq(...) to (module, name, signal, coding)
        for eve in re.findall(r'\("(.*)", (.*)\),',content):
            name = eve[0].strip()
            signal = eve[1].strip()
            # search for coding in generate_log
            with open(generate_log, 'r') as f:
                g = f.read()
            regexp = f'\((\w+) perfEvents Set,({name})\s*,(.+),(\d+)'
            matches = re.findall(regexp,g)
            coding = 'NA'
            block = 'NA'
            if len(matches) == 1:
                coding = matches[0][3]
                block = matches[0][0]
            elif len(matches) > 1:
                coding = ','.join([x[3] for x in matches])
                block = matches[0][0]            
            all_events.append((block, module, name, signal, coding))

    for event in all_events:
        print('@'.join([x for x in event]))
    # use excel to export this, split = '@'


def parse_encoding():
    with open(generate_log, 'r') as f:
        c = f.read()
    regexp = '\((\w+) perfEvents Set,(\w+)\s*,(.+),(\d+)'
    matches = re.findall(regexp,c)
    for match in matches:
        block = match[0]
        name = match[1]
        coding = match[3]
        print(f'#define {block}_{name} {coding}')

if __name__ == '__main__':
    parse_events_src()
