
def parser(fname, actions, data, levels, nebs):
    with open(fname, 'r') as f:
        ls = f.readlines()
        for line in ls:
            t = line.strip().split(' ')
            actions.append(t[0])
            idx = t[1]
            levels.append(int(t[2]))
            fs = t[3].split(',')
            for f in fs:
                data.append(float(f))
            nn = t[4:]
            nebs[idx] = {}
            for i in range(len(nn)):
                a = nn[i].split(',')
                if a[-1] == '':
                    a = a[:-1]
                a = [int(b) for b in a]
                nebs[idx][i] = a

def test_parser():
    fname = 'graph.log'
    actions = []
    data = []
    levels = []
    nebs = {}
    parser(fname, actions, data, levels, nebs)
    print('actions', actions)
    print('data', data)
    print('levles', levels)
    print('nebs', nebs)


if __name__ == '__main__':
    test_parser()
