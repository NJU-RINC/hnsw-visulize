
from time import sleep


import matplotlib.pyplot as plt
from mpl_toolkits.mplot3d import Axes3D


fig = plt.figure()
ax = fig.add_subplot(111, projection='3d')

x = [0, 1, 2, 3]
y = [2, 1, 3, 0]
z1 = [1, 1, 1, 1]
z2 = [i + 2 for i in z1]

ax.scatter(x, y, z1)
plt.show()

sleep(1)

ax.scatter(x, y, z2)
plt.show()

if __name__ == '_main__':
    pass
