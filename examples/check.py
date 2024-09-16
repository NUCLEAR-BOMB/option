import sys
import subprocess

prefix = '//$'

files = sys.argv[1].split(';')
targets = sys.argv[2].split(';')

has_error = False

def try_int(string):
    try: return int(string)
    except ValueError: return None

for file_path, target_path in zip(files, targets):
    target_result = subprocess.run(target_path, capture_output=True, text=True)
    if target_result.returncode != 0:
        print(f'Error while running "{target_path}":\n{target_result.stderr}', file=sys.stderr)
        has_error = True
        continue
    target_stdout = [line.strip() for line in target_result.stdout.splitlines()]

    with open(file_path) as example_file:
        expected_lines = [(idx + 1, exp.strip()) for idx, line in enumerate(example_file.readlines()) if (exp := line.partition(prefix)[2])]

    for (line, expected), received in zip(expected_lines, target_stdout):
        
        if expected == '[number]' and try_int(received) is not None:
            pass
        elif expected == '[nullptr]' and (try_int(received) == 0 or received == '(nil)'):
            pass
        elif expected != received:
            print(f'Expected: "{expected}", received: "{received}".\nAt line {line}, file: "{file_path}"\n', file=sys.stderr)
            has_error = True

    if len(expected_lines) > len(target_stdout):
        print(f'Abundance of {len(expected_lines) - len(target_stdout)} lines.\nLast match at line {expected_lines[-1][0]}, file: "{file_path}"', file=sys.stderr)
        has_error = True
    elif len(expected_lines) < len(target_stdout): 
        print(f'Lack of {len(target_stdout) - len(expected_lines)} lines.\nLast match at line {expected_lines[-1][0]}, file: "{file_path}"', file=sys.stderr)
        has_error = True

if has_error:
    sys.exit(1)
else:
    print('Completed successfully')
