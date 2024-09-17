import sys

mode = sys.argv[1].strip()
output_path = sys.argv[2].strip()
iterations_number = int(sys.argv[3])

generation_table = {
    'opt::option': {
        'prologue': '#include <opt/option.hpp>',
        'class_name': 'opt::option'
    },
    'std::optional': {
        'prologue': '#include <optional>',
        'class_name': 'std::optional'
    }
}

gen_info = generation_table.get(mode)
if gen_info is None:
    print(f'Invalid mode: {mode}')
    sys.exit(1)

prologue = gen_info['prologue']
class_name = gen_info['class_name']

with open(output_path, 'w') as output_file:
    output_file.write(prologue + '\n')
    for i in range(iterations_number):
        output_file.write(f'struct S{i} {{ int x; }};\n')
        output_file.write(f'{class_name}<S{i}> b{i};\n')
