#!/usr/bin/env python3

import sys

if __name__ == '__main__':
    if len(sys.argv) < 2:
        print('Usage: {} <file_name>'.format(sys.argv[0]))
        sys.exit(1)

    code = ''
    for line in open(sys.argv[1], 'r'):
        line = line.strip('\r\n')
        comment = line.split('//')
        if len(comment) == 2 and comment[0].find('http') == -1:
            line = '/*'.join(comment) + '*/'
        code += line + '\n'

    with open(sys.argv[1], 'w') as f:
        f.write(code)
