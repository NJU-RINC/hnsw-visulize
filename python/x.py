import numpy as np
import matplotlib
import matplotlib.pyplot as plt
import matplotlib.animation as animation


class Scope:
    def __init__(self, ax, maxt=2, dt=0.02):
        self.ax = ax
        self.arts = []
        self.arts.append(self.ax.scatter([0], [0], [0], c='b'))

    def update(self, y):
        x,y,z = np.random.rand(3,3)
        self.arts.append(self.ax.scatter(x,y,z,s=20,c='b',alpha=1))
        self.arts.append(self.ax.plot(x,y,z,'g'))

        return self.arts

# Fixing random state for reproducibility
np.random.seed(19680801 // 10)

fig = plt.figure()
ax = fig.add_subplot(projection="3d")

scope = Scope(ax)

# pass a generator in "emitter" to produce data for the update func
ani = animation.FuncAnimation(fig, scope.update, interval=1000)

plt.axis("off")
plt.show()
