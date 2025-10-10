from matplotlib import pyplot as plt
import json

if __name__ == "__main__":
    with open('agmcpu.json') as fobj:
        d = json.load(fobj)
        ours: list[float] = d['ours']
        baseline: list[float] = d['baseline']

    plt.title("CPU Percentage of agmfitmain")
    plt.grid()
    if ours:
        x = [1.5 * i for i in range(len(ours))]
        plt.plot(x, ours, label='Ours')
    if baseline:
        x = [1.5 * i for i in range(len(baseline))]
        plt.plot(x, baseline, label='Original')
    plt.ylabel("CPU Percentage")
    plt.xlabel("Execution Time(s)")
    plt.legend()
    plt.savefig('agmcpu.pdf')
    plt.close()
