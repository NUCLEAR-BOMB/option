import sys
import subprocess
import re
import collections
from pprint import pprint

llvm_objdump_path = sys.argv[1].strip()
target_path = sys.argv[2].strip()
source_path = sys.argv[3].strip()
current_compiler = sys.argv[4].strip().lower()

prefix = '//$'

llvm_objdump_version = subprocess.run([llvm_objdump_path, '--version'], capture_output=True, text=True)
if len(llvm_objdump_version.stderr) > 0 or llvm_objdump_version.returncode != 0:
    sys.exit(1)
print('llvm-objdump version output:\n', ''.join(llvm_objdump_version.stdout.splitlines(True)[:2]))

disasm_result = subprocess.run([llvm_objdump_path, '-d', '--no-leading-addr', '--no-show-raw-insn', '--demangle', '--x86-asm-syntax=intel', target_path], capture_output=True, text=True)
if len(disasm_result.stderr) > 0:
    print('Disassembly error: {} (return code: {})'.format(disasm_result.stderr, disasm_result.returncode))
    sys.exit(1)
if disasm_result.returncode != 0:
    print('Disassembly return code: {}'.format(disasm_result.returncode))
    sys.exit(1)

disasm_target_list = []
function_name_pattern = re.compile(r'<.*?(\S+)\(.*\)>')
instruction_pattern = re.compile(r'\s+(.*)', re.DOTALL)
decimal_number = re.compile(r'(\W|\A)(\d+)(\W|\Z)')

for line in disasm_result.stdout.splitlines():
    if function_name := re.match(function_name_pattern, line):
        disasm_target_list.append((function_name[1], []))
    elif len(disasm_target_list) != 0 and (instruction := re.match(instruction_pattern, line)):
        converted_instruction = re.sub(decimal_number, lambda m: (m[1] + hex(int(m[2])) + m[3]), instruction[1].replace('\t', ' '))

        disasm_target_list[-1][1].append(converted_instruction)

disasm_target = dict(disasm_target_list)

function_pattern = re.compile(r'@(.+?)\s*{(.+)}\s*:')
expected_disasm = []
with open(source_path) as source_file:
    section = None
    for idx, line in enumerate(source_file, start=1):
        exp = line.partition(prefix)[2].strip()
        if exp == '': continue

        if fn := function_pattern.match(exp):
            fn_name, compilers = fn[1], fn[2]
            if compilers is None:
                print('Invalid compiler attribute in function name. Line: {}, Function: "{}"'.format(idx, fn_name))
                sys.exit(1)

            if section is not None: expected_disasm.append(section)
            section = (fn_name, (compilers.lower().split(','), []))
        else:
            section[1][1].append((idx, exp))
    if section is not None:
        expected_disasm.append(section)

is_successful = True
checked_function = 0

for fn_name, (compilers, expected_asm) in expected_disasm:
    is_successful_current_iteration = True
    resulted_asm = disasm_target.get(fn_name, None)
    if resulted_asm is None:
        print('\nUnknown function name: "{}"\nList of known function names: {}\n'.format(fn_name, ", ".join(disasm_target.keys())))
        sys.exit(1)

    if current_compiler not in compilers:
        continue

    checked_function += 1
    is_any_last = False

    for (line_number, expected_asm_line), resulted_asm_line in zip(expected_asm, resulted_asm):
        if expected_asm_line == '*':
            is_any_last = True
            continue
        is_any_last = False

        if expected_asm_line != resulted_asm_line:
            print('Line: {}\nFunction: "{}"\n Expected: "{}"\n Received: "{}"\n'.format(line_number, fn_name, expected_asm_line, resulted_asm_line))
            is_successful_current_iteration = False

    if len(expected_asm) > len(resulted_asm):
        print('Function: "{}"\n Expected too many instructions: {}\n Received: {}\n'.format(fn_name, len(expected_asm), len(resulted_asm)))
        is_successful_current_iteration = False
    elif not is_any_last and len(expected_asm) < len(resulted_asm):
        print('Function: "{}"\n Received too many instructions: {}\n Expected: {}\n'.format(fn_name, len(resulted_asm), len(expected_asm)))
        is_successful_current_iteration = False

    if not is_successful_current_iteration:
        print('Full received assembly:\n {}'.format("\n ".join(resulted_asm)))
        is_successful = False

if not is_successful:
    sys.exit(1)
else:
    print('\nSuccessfully completed (functions checked: {})\n'.format(checked_function))
