##rsrp pre processing
from collections import UserList
from multiprocessing.util import close_all_fds_except


cellid = []
rnti = []
time = []
cellid1 = []
rnti1 = []
time1 = []

ueList = []
cellList = []

global uid


ueList1 = []
cellList1 = []

global uid1

########################################################################
##############################RSRP_RSRQ##########################################


def eval_rsrp_rsrq(exa,r):
    global uid
    ueArr = []
    cellArr = []

    # Get amount of cells and ue's
    
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
    cellid_info = [[[] for j in range(len(ueList))] for i in range(len(cellList))]
    rnti_info = [[[] for j in range(len(ueList))] for i in range(len(cellList))]

    
    
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

        #rename    ## might have tom swap values of cellid and rnti
        rsrp[cellid][rnti].append(float(col[3]))
        rsrq[cellid][rnti].append(float(col[4]))
        tt[cellid][rnti].append(float(col[0]))
        cellid_info[cellid][rnti].append(float(col[1]))
        rnti_info[cellid][rnti].append(float(col[2]))

                            
    
    
    
    uid = [[[] for j in range(len(ueList))] for i in range(len(cellList))]

    
    # print(cellid_info)
    # print(rnti_info)
    k = 0
    l = 0
    p = 0
    dataSet = []
    keys = [] 
    
    
    

    for i in emptyList:
        for j in i:
            # print("(l,k)= (" + str(l) + ", " + str(k) + ")")
            # print(l,k)
            # print(tt[l][k])
            
            #print(len(tt))
            #print(len(rsrp))
            #print(len(rsrq))
            
#             if l == 1 and k ==1:
#                 print(tt[l][k])
#                 print(rsrp[l][k])
#                 print(rsrq[l][k])

#                 print(cellid_info[l][k])
#                 print(rnti_info[l][k])

            celln = cellList[l]
            uen = ueList[k]
            uid[l][k].append(str(celln) + str(uen))
        
            dataRes = pd.DataFrame({'time': tt[l][k], 'sig_power': rsrp[l][k], 'sig_quality': rsrq[l][k], 'uid': 'user{}{}'.format(celln,uen)}, columns=['time', 'sig_power', 'sig_quality', 'uid'])
            
            dataSet.append(dataRes)
            # print("there has been added a dataset")
            keys.append('user{}{}'.format(celln,uen))
            # print(keys)

            p=p+1
            k=k+1

        l=l+1
        k = 0

    # print(uid)

    result = pd.concat(dataSet, keys=keys)

    return result



def preprocess_file_rsrp(dat):
    for i in range(len(dat)):
        num = dat[i]
        ress = eval_rsrp_rsrq(num, i)
    return ress

########################################################################
##################################SNR###################################

def eval_sinr(exa,r):
    
    global uid1

    ueArr1 = []
    cellArr1 = []

    # Get amount of cells and ue's
    
    # #debug
    # for line in exa['output']['stdout'].splitlines():
    #     col = line.split()
    #     if col[3] == '-nan':
    #         continue
    #     if col[4] == '-nan':
    #         continue
    #     if(int(col[2]) == 0):
    #         continue
    
    
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
        
    uid1 = [[[] for j in range(len(ueList1))] for i in range(len(cellList1))]

    
    k1 = 0
    l1 = 0
    p1 = 0
    dataSet1 = []
    keys1 = [] 

    for i in emptyList1:
        for j in i:
#             print(k1,l1)
            
#             print(len(tt1))
#             print(len(sinr))
            celln = cellList1[l1]
            uen = ueList1[k1]
        
            uid1[l1][k1].append(str(celln)+str(uen))
            
            dataRes1 = pd.DataFrame({'time': tt1[l1][k1], 'sinr': sinr[l1][k1], 'uid': 'user{}{}'.format(celln,uen)}, columns=['time', 'sinr', 'uid'])
            dataSet1.append(dataRes1)
            # print("there has been added a dataset")
            keys1.append('user{}{}'.format(celln,uen))
            # print(keys1)

            p1+=1
            k1+=1

        l1+=1

    # print(uid)

    # print(uid1)    

    # print(uid1)

    result1 = pd.concat(dataSet1, keys=keys1)

    return result1



def preprocess_file_sinr(dat):
    for i in range(len(dat)):
        num = dat[i]
        ress1 = eval_sinr(num, i)
    return ress1

########################################################################
########################################################################


res_signalpower = preprocess_file_rsrp(result_signalpower)
res_sinr = preprocess_file_sinr(result_sinr)

# print(uid)

# print(uid1)

if sectors == True:
    
    res_cu00 = pd.DataFrame({'time': res_signalpower.loc[res_signalpower['uid'] == 'user00', 'time'], 'sig_power': res_signalpower.loc[res_signalpower['uid'] == 'user00', 'sig_power'], 'sig_quality': res_signalpower.loc[res_signalpower['uid'] == 'user00', 'sig_quality'], 'uid': res_signalpower.loc[res_signalpower['uid'] == 'user00', 'uid']} , columns=['time', 'sig_power', 'sig_quality', 'uid'])

    res_cu01 = pd.DataFrame({'time': res_signalpower.loc[res_signalpower['uid'] == 'user01', 'time'], 'sig_power': res_signalpower.loc[res_signalpower['uid'] == 'user01', 'sig_power'], 'sig_quality': res_signalpower.loc[res_signalpower['uid'] == 'user01', 'sig_quality'], 'uid': res_signalpower.loc[res_signalpower['uid'] == 'user01', 'uid']} , columns=['time', 'sig_power', 'sig_quality', 'uid'])

    res_cu02 = pd.DataFrame({'time': res_signalpower.loc[res_signalpower['uid'] == 'user02', 'time'], 'sig_power': res_signalpower.loc[res_signalpower['uid'] == 'user02', 'sig_power'], 'sig_quality': res_signalpower.loc[res_signalpower['uid'] == 'user02', 'sig_quality'], 'uid': res_signalpower.loc[res_signalpower['uid'] == 'user02', 'uid']} , columns=['time', 'sig_power', 'sig_quality', 'uid'])

    res_cu10 = pd.DataFrame({'time': res_signalpower.loc[res_signalpower['uid'] == 'user10', 'time'], 'sig_power': res_signalpower.loc[res_signalpower['uid'] == 'user10', 'sig_power'], 'sig_quality': res_signalpower.loc[res_signalpower['uid'] == 'user10', 'sig_quality'], 'uid': res_signalpower.loc[res_signalpower['uid'] == 'user10', 'uid']} , columns=['time', 'sig_power', 'sig_quality', 'uid'])

    res_cu11 = pd.DataFrame({'time': res_signalpower.loc[res_signalpower['uid'] == 'user11', 'time'], 'sig_power': res_signalpower.loc[res_signalpower['uid'] == 'user11', 'sig_power'], 'sig_quality': res_signalpower.loc[res_signalpower['uid'] == 'user11', 'sig_quality'], 'uid': res_signalpower.loc[res_signalpower['uid'] == 'user11', 'uid']} , columns=['time', 'sig_power', 'sig_quality', 'uid'])

    res_cu12 = pd.DataFrame({'time': res_signalpower.loc[res_signalpower['uid'] == 'user12', 'time'], 'sig_power': res_signalpower.loc[res_signalpower['uid'] == 'user12', 'sig_power'], 'sig_quality': res_signalpower.loc[res_signalpower['uid'] == 'user12', 'sig_quality'], 'uid': res_signalpower.loc[res_signalpower['uid'] == 'user12', 'uid']} , columns=['time', 'sig_power', 'sig_quality', 'uid'])

    res_cu20 = pd.DataFrame({'time': res_signalpower.loc[res_signalpower['uid'] == 'user20', 'time'], 'sig_power': res_signalpower.loc[res_signalpower['uid'] == 'user20', 'sig_power'], 'sig_quality': res_signalpower.loc[res_signalpower['uid'] == 'user20', 'sig_quality'], 'uid': res_signalpower.loc[res_signalpower['uid'] == 'user20', 'uid']} , columns=['time', 'sig_power', 'sig_quality', 'uid'])

    res_cu21 = pd.DataFrame({'time': res_signalpower.loc[res_signalpower['uid'] == 'user21', 'time'], 'sig_power': res_signalpower.loc[res_signalpower['uid'] == 'user21', 'sig_power'], 'sig_quality': res_signalpower.loc[res_signalpower['uid'] == 'user21', 'sig_quality'], 'uid': res_signalpower.loc[res_signalpower['uid'] == 'user21', 'uid']} , columns=['time', 'sig_power', 'sig_quality', 'uid'])

    res_cu22 = pd.DataFrame({'time': res_signalpower.loc[res_signalpower['uid'] == 'user22', 'time'], 'sig_power': res_signalpower.loc[res_signalpower['uid'] == 'user22', 'sig_power'], 'sig_quality': res_signalpower.loc[res_signalpower['uid'] == 'user22', 'sig_quality'], 'uid': res_signalpower.loc[res_signalpower['uid'] == 'user22', 'uid']} , columns=['time', 'sig_power', 'sig_quality', 'uid'])

    res_signalpower_sec1 = pd.concat([res_cu00, res_cu01, res_cu02])
    res_signalpower_sec2 = pd.concat([res_cu10, res_cu11, res_cu12])
    res_signalpower_sec3 = pd.concat([res_cu20, res_cu21, res_cu22])

    res1_cu00 = pd.DataFrame({'time': res_sinr.loc[res_sinr['uid'] == 'user00', 'time'], 'sinr': res_sinr.loc[res_sinr['uid'] == 'user00', 'sinr'], 'uid': res_sinr.loc[res_sinr['uid'] == 'user00', 'uid']} , columns=['time', 'sinr', 'uid'])

    res1_cu01 = pd.DataFrame({'time': res_sinr.loc[res_sinr['uid'] == 'user01', 'time'], 'sinr': res_sinr.loc[res_sinr['uid'] == 'user01', 'sinr'], 'uid': res_sinr.loc[res_sinr['uid'] == 'user01', 'uid']} , columns=['time', 'sinr', 'uid'])

    res1_cu02 = pd.DataFrame({'time': res_sinr.loc[res_sinr['uid'] == 'user02', 'time'], 'sinr': res_sinr.loc[res_sinr['uid'] == 'user02', 'sinr'], 'uid': res_sinr.loc[res_sinr['uid'] == 'user02', 'uid']} , columns=['time', 'sinr', 'uid'])

    res1_cu10 = pd.DataFrame({'time': res_sinr.loc[res_sinr['uid'] == 'user10', 'time'], 'sinr': res_sinr.loc[res_sinr['uid'] == 'user10', 'sinr'], 'uid': res_sinr.loc[res_sinr['uid'] == 'user10', 'uid']} , columns=['time', 'sinr', 'uid'])

    res1_cu11 = pd.DataFrame({'time': res_sinr.loc[res_sinr['uid'] == 'user11', 'time'], 'sinr': res_sinr.loc[res_sinr['uid'] == 'user11', 'sinr'], 'uid': res_sinr.loc[res_sinr['uid'] == 'user11', 'uid']} , columns=['time', 'sinr', 'uid'])

    res1_cu12 = pd.DataFrame({'time': res_sinr.loc[res_sinr['uid'] == 'user12', 'time'], 'sinr': res_sinr.loc[res_sinr['uid'] == 'user12', 'sinr'], 'uid': res_sinr.loc[res_sinr['uid'] == 'user12', 'uid']} , columns=['time', 'sinr', 'uid'])

    res1_cu20 = pd.DataFrame({'time': res_sinr.loc[res_sinr['uid'] == 'user20', 'time'], 'sinr': res_sinr.loc[res_sinr['uid'] == 'user20', 'sinr'], 'uid': res_sinr.loc[res_sinr['uid'] == 'user20', 'uid']} , columns=['time', 'sinr', 'uid'])

    res1_cu21 = pd.DataFrame({'time': res_sinr.loc[res_sinr['uid'] == 'user21', 'time'], 'sinr': res_sinr.loc[res_sinr['uid'] == 'user21', 'sinr'], 'uid': res_sinr.loc[res_sinr['uid'] == 'user21', 'uid']} , columns=['time', 'sinr', 'uid'])

    res1_cu22 = pd.DataFrame({'time': res_sinr.loc[res_sinr['uid'] == 'user22', 'time'], 'sinr': res_sinr.loc[res_sinr['uid'] == 'user22', 'sinr'], 'uid': res_sinr.loc[res_sinr['uid'] == 'user22', 'uid']} , columns=['time', 'sinr', 'uid'])

    res_sinr_sec1 = pd.concat([res1_cu00, res1_cu01, res1_cu02])
    res_sinr_sec2 = pd.concat([res1_cu10, res1_cu11, res1_cu12])
    res_sinr_sec3 = pd.concat([res1_cu20, res1_cu22, res1_cu22])
