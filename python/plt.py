import time
import numpy as np
import matplotlib
import matplotlib.pyplot as plt
import matplotlib.animation as animation

from utils import parser, general_parser


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

pause = True

class Apple:
    def __init__(self, ax, fname='graph.log'):
        self.ax = ax
        
        self.actions, self.data = general_parser(fname)

        self.colors = ['tab:blue','tab:orange','tab:green','tab:red',
            'tab:purple','tab:brown','tab:pink','tab:gray']

        self.arts = []

        path = np.fromfile("path", dtype=np.float32)
        self.path = path.reshape(-1, 3)
        self.i = 1
        self.lines = []

    def update(self, n):
        global pause
        if n >= len(self.actions):
            if len(self.lines) == 0:
                dx,dy = 0.1,0.1
                self.ax.scatter([dx],[dy],[0],s=50,c='r',alpha=1)
                self.ax.plot([dx,dy],[dx,dy],[-0.5,4.5],'--',c='cyan',linewidth=2)

                path = np.fromfile("path", dtype=np.float32)
                path = path.reshape(-1, 3)[1:,::]
                x,y,z = path[:,0], path[:,1], path[:,2]
                l = x.shape[0]
                l = min((self.i % l) + 1, l)
                self.i = self.i + 1
                for i in range(l-1):
                    a=self.ax.plot(x[i:i+2],y[i:i+2],z[i:i+2],'r--',linewidth=3)
                    self.lines.append(a)
            
            else:
                for i in range(len(self.lines)):
                    self.lines[i].pop(0).remove()
                self.lines = []
                
            return []
        
        action = self.actions[n]
        n_lvl = action['level'] + 1
        dat = np.random.rand(n_lvl,3)
        dat[:,0] = self.data[n * 2]
        dat[:,1] = self.data[n * 2 + 1]
        dat[:,2] = np.arange(n_lvl)

        x,y,z = dat[:,0],dat[:,1],dat[:,2]

        self.ax.scatter(x,y,z,s=40,c=self.colors[:n_lvl],alpha=1)
        self.ax.plot(x,y,z,'y--')

        #self.arts.append(self.ax.plot(x,y,z,'g'))

        nebs = action['nebs']
        for lvl in range(n_lvl):
            if not nebs:
                continue

            if not lvl in nebs:
                continue

            dat1 = np.random.rand(2,3)
            dat1[0,:] = dat[lvl,:] 
            dat1[1][2] = lvl
            
            for idx in nebs[lvl]:
                dat1[1][0] = self.data[idx*2]
                dat1[1][1] = self.data[idx*2+1]
                dd = np.copy(dat1)
                x,y,z = dd[:,0],dd[:,1],dd[:,2]
                self.ax.plot(x,y,z,'k',alpha=0.5)

        return []

# Fixing random state for reproducibility
np.random.seed(19680801 // 10)

fig = plt.figure()
ax = fig.add_subplot(projection="3d")
ax.grid(False)

base = np.fromfile("data", dtype=np.float32)
base = base.reshape(-1, 2)
x = base[:,0]
y = base[:,1]
z = np.empty(x.shape)
z[:] = 5
#ax.scatter(x,y,z,s=40,c='tab:brown',alpha=1)

scope = Apple(ax)


def onClick(event):
    global pause
    pause ^= True

def Tick():
    global pause
    i = 0
    while True:
        if not pause:
            i = i + 1
            pause ^= True
        yield i

#fig.canvas.mpl_connect('button_press_event', onClick)
fig.canvas.mpl_connect('key_press_event', onClick)

# pass a generator in "emitter" to produce data for the update func
#anim = animation.FuncAnimation(fig,scope.update,Tick,interval=50,blit=True)
anim = animation.FuncAnimation(fig,scope.update,interval=200)

ax.text(0,0.5,-2,"HNSW search process",size=14)
plt.axis("off")
plt.show()
