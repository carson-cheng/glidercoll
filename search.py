import os, sys
from pathlib import Path
# check if the executable already exists; otherwise, build the executable
# usage: python3 search.py -g <number of gliders> -c <number of searchers>
usage = "usage: python3 search.py -g <number of gliders> -c <number of searchers>"
compile_error = "An error occurred at compile time. Please fix the program and try again."
def catch_index_err(err):
    if err == "IndexError: list index out of range":
        raise SystemExit(usage)
    else:
        raise SystemExit(err)
if len(sys.argv) < 5:
    raise SystemExit(usage)
num_gliders, num_searchers = 0, 0
for i in range(len(sys.argv)):
    if sys.argv[i] == "-g":
        try:
            num_gliders = int(sys.argv[i+1])
        except IndexError as err:
            catch_index_err(err)
    elif sys.argv[i] == "-c":
        try:
            num_searchers = int(sys.argv[i+1])
        except IndexError as err:
            catch_index_err(err)
if num_gliders == 0 or num_searchers == 0:
    raise SystemExit(usage)
if num_gliders < 4 or num_gliders > 6:
    raise SystemExit("the -g parameter should have a value between 4 and 6")
fn = "coll_" + str(num_gliders) + "g.c"
target = "coll_gen_" + str(num_gliders)
if Path(target).exists() == False:
    print("Executable doesn't exist, compiling file...")
    cmd = "gcc " + fn + " -o " + target + " -std=c99 -O3 -Wall -Wextra -Wno-unused-function -march=core2 -D COMPILING_FOR_X86_64_WITH_SSSE3 -D VERSION_FOR_AVX_512F_IS_AVAILABLE"
    compile = os.system(cmd)
    if compile != 0:
        print("The compilation exited with exit code " + str(compile))
        if compile == 2:
            raise SystemExit("The compilation was interrupted. Please try again.")
        elif compile == 256:
            print("Recompiling with specified architecture...")
            compile = os.system("arch -x86_64 " + cmd)
            if compile != 0:
                raise SystemExit(compile_error)
        else:
            raise SystemExit(compile_error)
cmd = "bash "
for item in range(num_searchers):
    tgt = "results/6g_test" + str(item+1) + ".txt"
    cmd = cmd + "./" + target + " db.rle > " + tgt + " & "
cmd = cmd + " wait"
print(cmd)
os.system(cmd)
