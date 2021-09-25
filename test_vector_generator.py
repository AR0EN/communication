import numpy as np
from numpy import random

SF = 0xF0
EF = 0x0F
VAL_MIN = 0
VAL_MAX = 255
DU_LEN_MAX = 10
NUM_OF_VECTORS = 10

vectors = []

for i in range(NUM_OF_VECTORS):
    v = [SF]
    data_len = random.randint(DU_LEN_MAX)
    v.append(data_len & 0xFF)
    v.append((data_len >> 8) & 0xFF)
    v.append((data_len >> 16) & 0xFF)
    v.append((data_len >> 24) & 0xFF)

    du = random.randint(VAL_MIN, VAL_MAX, size=data_len)
    v += du.tolist()

    v.append(EF)
    vectors.append(v)

with open('test_vectors.cpp', 'w') as fw:
    fw.write('#include <stdint.h>\n\n')
    fw.write('#include <vector>\n\n')

    for i, v in enumerate(vectors):
        fw.write('uint8_t v' + str(i) + '[] = { \\\n')
        tmp = '    '
        for val in v:
            tmp += '0x{0:02X},'.format(val)
        tmp = tmp.strip(',')
        fw.write(tmp)
        fw.write(' \\\n};\n\n')

    fw.write('std::vector<uint8_t *> vectors = {')
    tmp = ''
    for i, v in enumerate(vectors):
        tmp += 'v' + str(i) + ','

    tmp = tmp.strip(',')
    fw.write(tmp)
    fw.write('};\n')

    fw.write('std::vector<int> vectors_sizes = {')
    tmp = ''
    for v in vectors:
        tmp += str(len(v)) + ','

    tmp = tmp.strip(',')
    fw.write(tmp)
    fw.write('};\n')


with open('test_vectors.hpp', 'w') as fw:
    fw.write('#ifndef _TEST_VECTORS_HPP_\n')
    fw.write('#define _TEST_VECTORS_HPP_\n\n')

    fw.write('#include <stdint.h>\n\n')
    fw.write('#include <vector>\n\n')

    fw.write('extern std::vector<uint8_t *> vectors;\n')
    fw.write('extern std::vector<int> vectors_sizes;\n\n')


    fw.write('#endif  // _TEST_VECTORS_HPP_\n')
