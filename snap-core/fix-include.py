import sys

def perform(f: str):
    lst = []
    with open(f, 'r') as fobj:
        for r in fobj:
            lst.append(r)
    
    with open(f, 'w') as fobj:
        include_stmt = "snap_" + f.replace('.', '_').replace('-', '_')
        fobj.write(f'#ifndef {include_stmt}\n')
        fobj.write(f'#define {include_stmt} 1\n\n#include \"glib-config.h\"\n')
        for r in lst:
            fobj.write(r)
        fobj.write(f'\n#endif // {include_stmt} 1\n')


if __name__ == "__main__":
    args = sys.argv
    for i in range(1, len(args)):
        perform(args[i])
