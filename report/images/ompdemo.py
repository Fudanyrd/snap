from matplotlib import pyplot as plt

if __name__ == "__main__":
    n = [2, 3, 4, 5]
    exetimes = [0.122, 1.33, 16.57, 193.4]
    ideals = []

    for i in range(len(n)):
        ideals.append(exetimes[0] * (10 ** i))

    plt.xlabel('log10(N)')
    plt.ylabel("Execution Time(s)")
    plt.grid()
    plt.plot(n, exetimes, label='Actual')
    plt.plot(n, ideals, color='green', linestyle='dashed', label='Ideal')
    plt.legend()
    plt.savefig('ompdemo.pdf')
    plt.close()
