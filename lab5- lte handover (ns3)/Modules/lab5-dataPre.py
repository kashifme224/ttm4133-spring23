##rsrp pre processing
from collections import UserList
from multiprocessing.util import close_all_fds_except
import numpy as np
import re

userids = []
hotime = []

cellid = []
rnti = []
time = []
cellid1 = []
rnti1 = []
time1 = []
########################################################################
##############################RSRP_RSRQ##########################################


def eval_rsrp_rsrq(exa,r):
    ueArr = []
    cellArr = []
    
    for line in exa['output']['stdout'].splitlines():
        if (line.startswith("#")):
            continue
        col = line.split()
        if col[3] == '-nan':
            continue
        if col[4] == '-nan':
            continue
        if(int(col[2]) == 0):
            continue
        ueArr.append(col[2])
        cellArr.append(col[1])

    ueSet = set(ueArr)
    cellSet = set(cellArr)
    ueList = list(ueSet)
    cellList = list(cellSet)
    ueList.sort()
    cellList.sort()
    
    emptyList = [[[] for j in range(len(ueList))] for i in range(len(cellList))]
    rsrp = [[[] for j in range(len(ueList))] for i in range(len(cellList))]
    rsrq = [[[] for j in range(len(ueList))] for i in range(len(cellList))]
    tt = [[[] for j in range(len(ueList))] for i in range(len(cellList))]

    for line in exa['output']['stdout'].splitlines():
        if (line.startswith("#")):
            continue
        col = line.split()
        if col[3] == '-nan':
            continue
        if col[4] == '-nan':
            continue
        if(int(col[2]) == 0):
            continue
                  
        cellid = int(cellList.index(str(col[1])))   ##find the index of element n in list
        rnti = int(ueList.index(str(col[2])))

        rsrp[cellid][rnti].append(float(col[3]))
        rsrq[cellid][rnti].append(float(col[4]))
        tt[cellid][rnti].append(float(col[0]))
    
    k = 0
    l = 0
    p = 0
    dataSet = []
    keys = [] 

    for i in emptyList:
        for j in i:

            dataRes = pd.DataFrame({'time': tt[l][k], 'sig_power': rsrp[l][k], 'sig_quality': rsrq[l][k], 'ue': 'user{}{}'.format(l+1,k+1)}, columns=['time', 'sig_power', 'sig_quality', 'ue'])
            dataSet.append(dataRes)
            keys.append('user{}{}'.format(l+1,k+1))

            p=p+1
            k=k+1

        l=l+1
        k = 0
        
    result = pd.concat(dataSet, keys=keys)
    global userids
    userids = result['ue'].unique()
    return result

def preprocess_file_rsrp(dat):
    for i in range(len(dat)):
        num = dat[i]
        ress = eval_rsrp_rsrq(num, i)
    return ress

########################################################################
##################################SNR###################################

def eval_sinr(exa,r):
    ueArr1 = []
    cellArr1 = []

    for line in exa['output']['stdout'].splitlines():
        if (line.startswith("#")):
            continue
        col = line.split()
        if col[3] == '-nan':
            continue
        if col[4] == '-nan':
            continue
        if(int(col[2]) == 0):
            continue
        ueArr1.append(col[2])
        cellArr1.append(col[1])

    ueSet1 = set(ueArr1)
    cellSet1 = set(cellArr1)

    ueList1= list(ueSet1)
    cellList1 = list(cellSet1)
    
    ueList1.sort()
    cellList1.sort()

    
    emptyList1 = [[[] for j in range(len(ueList1))] for i in range(len(cellList1))]
    sinr = [[[] for j in range(len(ueList1))] for i in range(len(cellList1))]
    tt1 = [[[] for j in range(len(ueList1))] for i in range(len(cellList1))]
    
    

    for line in exa['output']['stdout'].splitlines():
        if (line.startswith("#")):
            continue
        col = line.split()
        if col[3] == '-nan':
            continue
        if col[4] == '-nan':
            continue
        if(int(col[2]) == 0):
            continue

        
        cellid1 = int(cellList1.index(str(col[1])))   ##find the index of element n in list
        rnti1 = int(ueList1.index(str(col[2])))

        #rename    ## might have tom swap values of cellid and rnti
        sinr[cellid1][rnti1].append(float(col[4]))
        tt1[cellid1][rnti1].append(float(col[0]))
    
    k1 = 0
    l1 = 0
    p1 = 0
    dataSet1 = []
    keys1 = [] 

    for i in emptyList1:
        for j in i:
            
            dataRes1 = pd.DataFrame({'time': tt1[l1][k1], 'sinr': sinr[l1][k1], 'ue': 'user{}{}'.format(l1+1,k1+1)}, columns=['time', 'sinr', 'ue'])
            dataSet1.append(dataRes1)
            keys1.append('user{}{}'.format(l1+1,k1+1))

            p1+=1
            k1+=1

        l1=l1+1
        k1 = 0
        
    result1 = pd.concat(dataSet1, keys=keys1)
    
        
    r = re.compile("([a-zA-Z]+)([0-9]+)")
    m1 = r.match(userids[0])
    userid1 = m1.group(2)
    m2 = r.match(userids[1])
    userid2 = m2.group(2)
    
    userid1_first = [int(a) for a in str(userid1)]
    userid1_second = [int(a) for a in str(userid2)]

    # print(userid1_first[0] != userid1_second[0])
    # print(userid1_second[0])
    
    global hotime
    
    if len(result1['ue'].unique()) > 1 and len(result1.loc[result1['ue'] == userids[1]]) > 0 and userid1_first[0] != userid1_second[0]:
        # print(userids[0])
        # print(userids[1])
        hotime = float(result1.loc[result1['ue'] == userids[1]]['time'][0])

    
    return result1

def preprocess_file_sinr(dat):
    for i in range(len(dat)):
        num = dat[i]
        ress1 = eval_sinr(num, i)
    return ress1

########################################################################
########################################################################

total_ues = []
time = []
def get_tput(exa):
    for line in exa['output']['stdout'].splitlines():
        if (line.startswith("#")):
            col = line.split()
            total_ues = len(col) - 1
            continue
            
        col = line.split()
        time.append(float(col[0]))

    cols = [[[]for j in range(len(time))] for i in range(total_ues)] # dimension is (# of ues) x (# of data values)

    for var in range(total_ues):
        i = 0
        for line in exa['output']['stdout'].splitlines():
            if (line.startswith("#")):
                continue
        
            col = line.split()
            cols[var][i] = float(col[var])
            i = i+1

    dataSet1 = []
    keys1 = []
    
    for var in range(total_ues-1):
        d = {'time': time, 'tput': cols[var+1], 'ue': userids[var]} #USE THIS IF YOU WANT THE #UE AT THE END OF THE TABLE.
        dataRes1 = pd.DataFrame(data=d)      
        dataSet1.append(dataRes1)
        keys1.append(userids[var])

        
    result2 = pd.concat(dataSet1, keys=keys1)
    
    if len(userids) > 1 and hotime != []:
        result2.loc[result2['time'] >= hotime, 'ue'] = userids[1] 

    
    

#     tput_min = []
#     tput_max = []

#     for var in range(total_ues-1):
#         if var==0:
#             tput_min = result2.loc[result2['ue'] == userids[var]]['tput'].min()
#             tput_max = result2.loc[result2['ue'] == userids[var]]['tput'].max()
            
#         else:
#             tput_min = result2.loc[result2['ue'] == userids[var]]['tput'].min()
#             tput_max = result2.loc[result2['ue'] == userids[var]]['tput'].max()
#         result2 = result2.replace(tput_max, tput_max/len(time))
    
    # result2['tput'] = result2['tput'].expanding().mean()
    result2['tput'] = result2['tput'].rolling(15).mean()
    return result2

def preprocess_tput(dat):
    for i in range(1):
        num = dat[i]
        res22 = get_tput(num)
        return res22

########################################################################
########################################################################
res_signalpower = preprocess_file_rsrp(result_signalpower)
res_sinr = preprocess_file_sinr(result_sinr)
res_throughput = preprocess_tput(result_throughput)
# print(hotime)
