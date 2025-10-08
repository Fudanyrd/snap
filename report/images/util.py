
def _display(item) -> str:
    if isinstance(item, dict):
        # has style control.
        try:
            bold: bool = item['bold']
        except KeyError:
            bold = False
        
        try:
            color: str = item['color']
        except KeyError:
            color = None

        ret: str = ""
        itemstr : str= _display(item['data'])
        if color is not None:
            if bold:
                # {\color[HTML]{009933} item}
                ret = "{\\color[HTML]{" + color + "} \\textbf{" + itemstr + "}}"
            else:
                ret = "{\\color[HTML]{" + color + "} " + itemstr + "}"
        else:
            if bold:
                ret = "\\textbf{" + itemstr + "}"
            else:
                ret = itemstr
        return ret
    elif isinstance(item, str):
        return item
    elif isinstance(item, int):
        return f'{item}'
    elif isinstance(item, float):
        return f'{round(item, 4)}'
    else:
        raise RuntimeError(f"cannot handle object type {type(item)}")

def lst_to_table(data: list, caption: str | None = None, 
                 label: str | None = None) -> str:
    assert len(data), "Empty input list!"
    for dat in data:
        assert isinstance(dat, dict), "List element is not dict!"

    ret: str = "" # "\\begin{table}\n" 

    attrs: list = []
    for k in data[0].keys():
        attrs.append(k)
    
    ret += "\t\\begin{tabular}"
    ret += "{|" + ("l|" * len(attrs)) + "} \\hline \n"

    for i in range(len(attrs)):
        ret += '\t\t'
        attr = attrs[i]
        if i > 0:
            ret +=  " & "
        ret += attr
    ret += "\\\\ \\hline \n"

    for row in data:
        ret += '\t\t'
        ret += _display(row[attrs[0]])
        for i in range(1, len(attrs)):
            attr = attrs[i]
            ret += " & "
            item = row[attr]
            ret += _display(item)

        ret += "\\\\ \\hline\n"

    ret += "\t\\end{tabular}\n"
    # ret += "\\end{table}\n"
    return ret

def dict_to_table(data: dict, id_attr: str = 'setup', 
                 label: str | None = None) -> str:
    assert len(data), "Empty input list!"
    setups: list = []
    for setup in data.keys():
        assert isinstance(data[setup], dict), "List element is not dict!"
        setups.append(setup)

    ret: str = "" # "\\begin{table}\n" 

    attrs: list = []
    for k in data.keys():
        for attr in data[k].keys():
            attrs.append(attr)
        break
    
    ret += "\t\\begin{tabular}"
    ret += "{|" + ("l|" * (len(attrs) + 1)) + "} \\hline \n"

    ret += '\t\t'
    ret += id_attr
    for i in range(len(attrs)):
        attr = attrs[i]
        ret +=  " & "
        ret += attr
    ret += "\\\\ \\hline\n"

    for row in data:
        ret += '\t\t'
        ret += row
        record = data[row]
        for i in range(0, len(attrs)):
            attr = attrs[i]
            ret += " & "
            item = record[attr]
            ret += _display(item)

        ret += "\\\\ \\hline\n"

    ret += "\t\\end{tabular}\n"
    # ret += "\\end{table}\n"
    return ret

def perf_data(infile: str) -> list:
    records = []
    counts: int = 0

    # A line of perf output looks like:
    # Overhead  Command     Shared Object         Symbol                                                                                                                
    # 19.60%  agmfitmain  agmfitmain            [.] TAGMFit::SelectLambdaSum
    with open(infile, 'r') as fobj:
        for r in fobj:
            if r.startswith('#'):
                continue

            counts += 1
            color = "FF0000" if counts <= 3 else "009933"
            fields: list[str] = r.split()
            line = {
                "Symbol": (fields[4]),
                "Shared Object": fields[2],
                "Overhead": {"data": float(fields[0].strip('%')) / 100.0, "color": color, "bold": counts <= 3},
            }

            records.append(line)
    
    return records


if __name__ == "__main__":
    lst = [
        {
            "foo": 1,
            "bar": 2,
            "baz": {"data": 3, "color": "FF0000"},
        },
        {
            "foo": {"data": 3, "bold": True},
            "bar": 2,
            "baz": 1,
        }
    ]

    dct = {
        "1": {
            "foo": 1,
            "bar": 2,
            "baz": {"data": 3, "color": "FF0000"},
        },
        "2": {
            "foo": {"data": 3, "bold": True},
            "bar": 2,
            "baz": 1,
        }
    }

    print(lst_to_table(lst))
    print(dict_to_table(dct, 'ID'))

