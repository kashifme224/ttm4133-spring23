import sys
args = sys.argv

ran_log = []
##########################################################################        
def get_ran_states(exa):
    for line in exa['output']['stderr'].splitlines():
        print(line)

def preprocess_ran_logs(dat):
    for i in range(1):
        num = dat[i]
        get_ran_states(num)    
##########################################################################
##########################################################################        
def get_epc_states(exa):
    for line in exa['output']['stderr'].splitlines():
        print(line)

#         if (line.startswith("#")):
#             print(line)
#             continue
#         scol = line.split()
#         time = float(scol[0])
#         imsi = int(scol[2])
#         olds.append(scol[4])
#         news.append(scol[5])
        
    
def preprocess_epc_logs(dat):
    for i in range(1):
        num = dat[i]
        get_epc_states(num)    
##########################################################################
######################################################################main functions#################################
if(True):
    print('####################################################')
    print('RAN Connection Setup Logs [MAC, RRC for eNB and UE]')
    print('####################################################')
    preprocess_ran_logs(ran_logs)
    print('\n')
    print('####################################################')
    print('EPC Connection Setup Logs [SGW, PGW, MME]')
    print('####################################################')
    preprocess_epc_logs(epc_logs)
