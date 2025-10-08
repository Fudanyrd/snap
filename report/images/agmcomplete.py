from matplotlib import pyplot as plt
import json
import statistics
import numpy as np

from util import dict_to_table

# def _stats(arr):
#     return {
#         "mean": np.mean(arr),
#         "stddev": statistics.stdev(arr),
#         "median": statistics.median(arr),
#     }

if __name__ == "__main__":
    # read agmfit.json.
    with open('agmfit.json', 'r') as fobj:
        obj = json.load(fobj)
    
    exectimes = obj['runtime2']
    baseline = np.array(exectimes['baseline'])
    ours = np.array(exectimes['ours'])
    
    plt.boxplot(np.stack([baseline, ours]).T,
                tick_labels=['baseline', 'ours'])
    
    plt.title('Boxplot of Exec Time')
    plt.ylabel("Exec Time(s)")
    plt.grid()
    plt.savefig('agmcomplete.pdf')

    dct = {
        "mean": {},
        "median": {},
        "stddev": {},
    }

    meanb = np.mean(baseline)
    meano = np.mean(ours)
    if meano < meanb:
        dct['mean']['Baseline'] = meanb
        dct['mean']['Ours'] = {"data": meano, "bold": True}
    else:
        dct['mean']['Baseline'] = {"data": meanb, "bold": True}
        dct['mean']['Ours'] = meano
    
    medianb = statistics.median(baseline)
    mediano = statistics.median(ours)
    if mediano < medianb:
        dct['median']['Baseline'] = medianb
        dct['median']['Ours'] = {"data": mediano, "bold": True}
    else:
        dct['median']['Baseline'] = {"data": medianb, "bold": True}
        dct['median']['Ours'] = mediano

    stddevb = statistics.stdev(baseline)
    stddevo = statistics.stdev(ours)
    if stddevo < stddevb:
        dct['stddev']['Baseline'] = stddevb
        dct['stddev']['Ours'] = {"data": stddevo, "bold": True}
    else:
        dct['stddev']['Baseline'] = {"data": stddevb, "bold": True}
        dct['stddev']['Ours'] = stddevo

    with open('agmcomplete-tbl.tex', 'w') as fobj:
        fobj.write(dict_to_table(dct, 'Statistics'))
