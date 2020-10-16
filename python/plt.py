import numpy as np
import matplotlib
import matplotlib.pyplot as plt
import matplotlib.animation as animation

from utils import parser


class Scope:
    def __init__(self, ax, fname='graph.log'):
        self.ax = ax
        
        actions = []
        data = []
        levels = []
        nebs = {}

        parser(fname, actions, data, levels, nebs)

        self.actions = actions
        self.data = data
        self.levels = levels
        self.nebs = nebs
        

    def update(self, it):
        arts = []
        if it >= len(self.actions):
            return []

        n_lvl = self.levels[it] + 1 
        dat = np.random.rand(n_lvl,3)
        dat[:,0] = self.data[it * 2]
        dat[:,1] = self.data[it * 2 + 1]
        dat[:,2] = np.arange(n_lvl)

        x,y,z = dat[:,0],dat[:,1],dat[:,2]

        self.ax.scatter(x,y,z,s=20,c='b',alpha=1,label=str(it))
        self.ax.plot(x,y,z,'y--')

        #self.arts.append(self.ax.plot(x,y,z,'g'))

        for lvl in range(n_lvl):
            if not self.nebs[str(it)]:
                continue

            if not lvl in self.nebs[str(it)]:
                continue

            dat1 = np.random.rand(2,3)
            dat1[0,:] = dat[lvl,:] 
            dat1[1][2] = lvl

            pp = 0
            for idx in self.nebs[str(it)][lvl]:
                dat1[1][0] = self.data[idx*2]
                dat1[1][1] = self.data[idx*2+1]
                dd = np.copy(dat1)
                x,y,z = dd[:,0],dd[:,1],dd[:,2]
                self.ax.plot(x,y,z,'g')

        return []

# Fixing random state for reproducibility
np.random.seed(19680801 // 10)

fig = plt.figure()
ax = fig.add_subplot(projection="3d")
ax.grid(False)

scope = Scope(ax)

for i in range(4):
    scope.update(i)

# pass a generator in "emitter" to produce data for the update func
#ani = animation.FuncAnimation(fig,scope.update,interval=1000,blit=True)

plt.axis("off")
plt.show()
