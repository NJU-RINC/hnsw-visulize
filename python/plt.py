import time
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
        
        self.colors = ['tab:blue','tab:orange','tab:green','tab:red',
            'tab:purple','tab:brown','tab:pink','tab:gray']

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

        self.ax.scatter(x,y,z,s=20,c=self.colors[:n_lvl],alpha=1,label=str(it))
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
            
            for idx in self.nebs[str(it)][lvl]:
                dat1[1][0] = self.data[idx*2]
                dat1[1][1] = self.data[idx*2+1]
                dd = np.copy(dat1)
                x,y,z = dd[:,0],dd[:,1],dd[:,2]
                self.ax.plot(x,y,z,'silver')

        return []

# Fixing random state for reproducibility
np.random.seed(19680801 // 10)

fig = plt.figure()
ax = fig.add_subplot(projection="3d")
ax.grid(False)
#ax.set_xlim3d(-1.1,1.1)
#ax.set_ylim3d(-1.1,1.1)
#ax.set_zlim3d(0,5)
scope = Scope(ax)

pause = False

def onClick(event):
    global pause
    pause ^= True

def Tick():
    i = 0
    while True:
        if not pause:
            i = i + 1
        yield i

fig.canvas.mpl_connect('button_press_event', onClick)

# pass a generator in "emitter" to produce data for the update func
anim = animation.FuncAnimation(fig,scope.update,Tick,interval=20,blit=True)

plt.axis("off")
plt.show()
