from util import perf_data, lst_to_table

if __name__ == "__main__":
    import sys
    args = sys.argv

    assert len(args) == 3, "Usage: [input.perf data] [output.tex]"

    data: list = perf_data(args[1])
    with open(args[2], 'w') as fobj:
        fobj.write(lst_to_table(data))

