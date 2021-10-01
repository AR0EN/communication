import numpy as np
from numpy import random

SF = 0xF0
EF = 0x0F
VAL_MIN = 0
VAL_MAX = 255
DU_LEN_MAX = 10
NUM_OF_VECTORS = 10

VECTOR_HEADER = 'test_vectors.hpp'
VECTOR_SRC = 'test_vectors.cpp'

vectors = []

for i in range(NUM_OF_VECTORS):
    v = []

    du = random.randint(VAL_MIN, VAL_MAX, size=random.randint(1, DU_LEN_MAX))
    v += du.tolist()

    vectors.append(v)

with open(VECTOR_SRC, 'w') as fw:
    fw.write('#include "{}"\n\n'.format(VECTOR_HEADER))
    fw.write('#include <vector>\n\n')
    fw.write('#include <stdint.h>\n\n')

    vectors_txt = 'std::vector<uint8_t *> vectors = {'
    vectors_sizes_txt = 'std::vector<int> vectors_sizes = {'

    for i, v in enumerate(vectors):
        fw.write('uint8_t v' + str(i) + '[] = { \\\n')
        tmp = '    '
        for val in v:
            tmp += '0x{0:02X} ,'.format(val)
        tmp = tmp.strip(' ,')
        fw.write(tmp)
        fw.write(' \\\n};\n\n')

        vectors_txt += 'v' + str(i) + ' ,'
        vectors_sizes_txt += str(len(v)) + ' ,'

    vectors_txt = vectors_txt.strip(',') + '};\n'
    fw.write(vectors_txt)

    vectors_sizes_txt = vectors_sizes_txt.strip(',') + '};\n'
    fw.write(vectors_sizes_txt)

with open(VECTOR_HEADER, 'w') as fw:
    fw.write('#ifndef _TEST_VECTORS_HPP_\n')
    fw.write('#define _TEST_VECTORS_HPP_\n\n')

    fw.write('#include <vector>\n\n')
    fw.write('#include <stdint.h>\n\n')

    fw.write('extern std::vector<uint8_t *> vectors;\n')
    fw.write('extern std::vector<int> vectors_sizes;\n\n')


    fw.write('#endif  // _TEST_VECTORS_HPP_\n')
