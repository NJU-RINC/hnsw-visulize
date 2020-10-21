
def parser(fname, actions, data, levels, nebs):
    with open(fname, 'r') as f:
        ls = f.readlines()
        for line in ls:
            t = line.strip().split(' ')
            actions.append(t[0])
            idx = t[1]
            levels.append(int(t[2]))

            if t[0] == 'add':
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

def op_add(tokens, action, data):
    datas = tokens[0].strip().split(',') 
    for d in datas:
        data.append(float(d))

    action['nebs'] = {}
    i = 1
    while i < len(tokens):
        i_nebs = tokens[i].strip().split(',')
        if i_nebs[-1] == '':
            i_nebs = i_nebs[:-1]
        i_nebs = [int(x) for x in i_nebs]
        action['nebs'][i-1] = i_nebs
        i = i + 1

def op_update_nebs(action, tokens):
    nebs = tokens[0].strip().split(',')
    if nebs[-1] == '':
        nebs = nebs[:-1]
    nebs = [int(x) for x in nebs]
    action['nebs'] = nebs

def general_parser(fname):
    actions = []
    data = []
    with open(fname, 'r') as reader:
        lines = reader.readlines()
        for line in lines:
            tokens = line.strip().split(' ')
            action = {}
            action['op'] = tokens[0]
            action['id'] = int(tokens[1])
            action['level'] = int(tokens[2])

            if action['op'] == 'add':
                op_add(tokens[3:], action, data) 
            elif action['op'] == 'update_nebs':
                op_update_nebs(action, tokens[2:])
            else:
                pass
            
            actions.append(action)

    return actions, data

def test_parser():
    fname = 'graph.log1'
    actions = []
    data = []
    levels = []
    nebs = {}
    parser(fname, actions, data, levels, nebs)
    print('actions', actions)
    print('data', data)
    print('levles', levels)
    print('nebs', nebs)

def test_general_parser():
    fname = 'graph.log1'
    actions, data = general_parser(fname)
    print('actions', actions)
    print('data', data)

if __name__ == '__main__':
    test_parser()
    test_general_parser()
