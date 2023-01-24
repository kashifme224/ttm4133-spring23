import os
import tempfile
from urllib.parse import uses_params
from xml.sax.saxutils import prepare_input_source
from pyrsistent import b
import sem
import matplotlib.pyplot as plt
import seaborn as sns
import pandas as pd
import pprint
import matplotlib.ticker as ticker
import sys
import base64
import re
from PIL import Image
from IPython.display import display, Markdown, Latex
import random
from datetime import datetime



sns.set_style("whitegrid")
curr_dt = datetime.now()
timestamp = int(round(curr_dt.timestamp()))
random.seed(timestamp)

runid = random.randint(1,100)

ns_path = os.path.normpath(os.path.join(os.getcwd(), '../../', 'ns-3'))            # Path for simulation purposes
script = 'lab0'                                                                            # NS3 Script to run, (change name here)

txpower = 20                                                                                # Use 40 until react gui parsing  (change for lab1)

##########################################################################################

campaign_dir = []
campaign = []
res_path = []
params = []
    
uePos = "[(341,506),(513,281),]"
eNBPos = "[(0,0)]"

# uePos = "[(341,506),(513,281),(800,706),(1000,381),]"

# eNBPos = "[(0,0),(800,500),]"

    
buildingPos = "[]"

res_path = 'results-rsrp' + '-' + str(txpower) + '-' + str(runid)
campaign_dir = os.path.normpath(os.path.join(os.getcwd(), 'Results', res_path))

campaign = sem.CampaignManager.new(ns_path, script, campaign_dir, overwrite=True,
                                   check_repo = False, max_parallel_processes=4)

params = {
    'arrayPosUEsString': uePos,
    'arrayPosBuildingsString': buildingPos,
    'enablersrp': True,
    'eNBTxPowerDbm': txpower,
    'arrayPoseNBsString' :eNBPos,
    'runId': runid,
}

runs = 1
print("User Received Power simulation started...")

campaign.run_missing_simulations(params, runs=runs)

##########################################################################################
print("User Received Power simulation finished!")
result_signalpower = campaign.db.get_complete_results(params=params) #Results

campaign_dir = []
campaign = []
res_path = []
params = []
##########################################################################################
res_path = 'results-sinr' + '-' + str(txpower) + '-' + str(runid)                        
campaign_dir = os.path.normpath(os.path.join(os.getcwd(), 'Results', res_path))

campaign = sem.CampaignManager.new(ns_path, script, campaign_dir, overwrite=True,          
                                   check_repo = False, max_parallel_processes=8)

params = {
    'arrayPosUEsString': uePos,
    'arrayPosBuildingsString': buildingPos,
    'enablesinrue': True,
    'eNBTxPowerDbm': txpower,
    'arrayPoseNBsString' :eNBPos,
    'runId': runid,
}
runs = 1
campaign.run_missing_simulations(params, runs=runs)

# ##########################################################################################
# print("Simulation SINR finished!")
result_sinr = campaign.db.get_complete_results(params=params) #Results

# ##########################################################################################
campaign_dir = []
campaign = []
res_path = []
params = []
    
print("Throughput simulation started...")

                             
res_path = 'results-flow' + '-' + str(txpower) + '-' + str(runid)                     
campaign_dir = os.path.normpath(os.path.join(os.getcwd(), 'Results', res_path))

campaign = sem.CampaignManager.new(ns_path, script, campaign_dir, overwrite=True,          
                                   check_repo = False, max_parallel_processes=4)

params = {
    'arrayPosUEsString': uePos,
    'arrayPosBuildingsString': buildingPos,
    'enableInstTput': True,
    'eNBTxPowerDbm': txpower,
    'arrayPoseNBsString' :eNBPos,
    'runId': runid,
}

runs = 1
campaign.run_missing_simulations(params, runs=runs)

    ##########################################################################################
print("Throughput simulation finished!")
result_throughput = campaign.db.get_complete_results(params=params) #Results


print('All the simulations have been completed successfully...')

# ##########################################################################################
# print("Simulations finished")    

raise SystemExit


# %reset -f
