import os
import sem
import matplotlib.pyplot as plt
import seaborn as sns
import pandas as pd
import pprint
import matplotlib.ticker as ticker
import sys
sns.set_style("whitegrid")

import random
from datetime import datetime

curr_dt = datetime.now()
timestamp = int(round(curr_dt.timestamp()))
random.seed(timestamp)

runid = random.randint(1,100)


param1 = 'y'
sectors = []
txpowers = []
apnd = []

for txpower in txpowers:
    ###################################Parameter Inputs#################################################
    print("Simulation will now begin shortly...")
    print("Please enter the following required parameters:")
    while True:
        print("Do you want to have sectors in the cell?(yes OR no OR y OR n)")
        if param1 == 'yes':
            sectors = True
            apnd = 'Sectored'
            break
        elif param1 == 'no':
            sectors = False
            apnd = 'Non-Sectored'
            break
        elif param1 == 'y':
            sectors = True
            apnd = 'Sectored'
            break
        elif param1 == 'n':
            sectors = False
            apnd = 'Non-Sectored'
            break
        if sectors == []:
            continue

    while True:
        print("Enter the value of eNB transmit power: (between 15 and 45dBm)")
        if txpower >= 15 and txpower <= 45:
              break
        else: 
            print("Invalid value for eNB transmit power (must be within 15 and 45dBm) : Entered value = " , txpower)
            continue

    print("The simulation parameters are : ")
    print("Sectors = ", sectors, ", eNB TxPower = ", txpower)
    ####################################################################################################
    #for rsrp values
    ########################################################################################
    ns_path = os.path.normpath(os.path.join(os.getcwd(), '../../', 'ns-3'))
    script = 'lab1' # ns-3 script in ../ns-3/scratch/
    res_path = 'results-signalpower' + '-' + str(int(txpower));
    campaign_dir = os.path.normpath(os.path.join(os.getcwd(), 'Results', str(apnd), res_path))
    print("Simulations started... (1/2)")
    campaign = sem.CampaignManager.new(ns_path, script, campaign_dir, overwrite=True, check_repo = False, max_parallel_processes=4) #create a new sem campaign

    params = {
        'enablesignalpower': True,
        'enableSectors': sectors,
        'bsTxPower': txpower,
        'runId': runid,
    }
    runs = 1

    campaign.run_missing_simulations(params, runs=runs)
    print("Simulations completed. (1/2)")

    ########################################################################################

    #for sinr values
    ########################################################################################
    res_path1 = 'results-sinr' + '-' + str(int(txpower));
    campaign_dir1 = os.path.normpath(os.path.join(os.getcwd(), 'Results', str(apnd), res_path1))
    campaign1 = sem.CampaignManager.new(ns_path, script, campaign_dir1, overwrite=True,check_repo = False, max_parallel_processes=4) #create a new sem campaign
    print("Simulations started... (2/2)")


    params1 = {
        'enablesinr': True,
        'enableSectors': sectors,
        'bsTxPower': txpower,
        'runId': runid,
    }
    runs1 = 1

    campaign1.run_missing_simulations(params1, runs=runs1)
    print("Simulations completed. (2/2)")

