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
sns.set_style("whitegrid")
from PIL import Image
from IPython.display import display, Markdown, Latex
#include json library
import json

import random
from datetime import datetime

curr_dt = datetime.now()
timestamp = int(round(curr_dt.timestamp()))
random.seed(timestamp)

runid = random.randint(1,100)

print("List of created simulations: ")
print(" ")

ns_path = os.path.normpath(os.path.join(os.getcwd(), '../../', 'ns-3')) # Path for simulation purposes

script = 'lab3'  # NS3 Script to run, (change name here)

listOfSimulationParams = [] # List with all simulations

file_directory = os.path.normpath(os.path.join(os.getcwd(), '../../', 'ttm4133-servers', 'files-upload', 'Labs', 'Lab2'))  # Directory with simulation params

img_directory = os.path.normpath(os.path.join(os.getcwd(), '../../', 'ttm4133-servers', 'files-upload', 'Images', 'Lab2'))  # Directory with simulation params

imgout_dir = os.path.normpath(os.path.join(os.getcwd(), '../../', 'ttm4133-spring23', 'lab3- lte connection setup (ns3)', 'Figures'))  # Directory with simulation params

txpower = 40   # Use 40 until react gui parsing  (change for lab1)

# print("Current working directory: {0}".format(os.getcwd())) # To get cwd

for filename in os.listdir(file_directory):
  f = os.path.join(file_directory, filename)
  if os.path.isfile(f) and filename.endswith('.txt'):
    temp = filename.split('-')
    temp2 = temp[1].split('.')
    print(temp2[0])

print("")


simulations = input("List the simulation to run: ")                                         # String with simulation names
simulationsArray = simulations.split(',')                                                   # Split by ,

showTopos = input("Show a picture of the topology? (y/n)")
# runSim = input("Run simulation aswell? (y/n)")

# while True:
#     print("Enter the value of transmit power: (between 15 and 45dBm)")
#     txpower = float(input())
#     if txpower >= 15 and txpower <= 45:
#           break
#     else: 
#         print("Invalid value for transmit power (must be within 15 and 45dBm) : Entered value = " , txpower)
#         continue

if showTopos == "y":
    print("Generating topology pictures...")

    for simName in simulationsArray:
        #print('Simulation {}'.format(simName))
        
        p = open(img_directory + "/base64Image-{}.txt".format(simName), "r")
        image_result = open(imgout_dir + '/GeneratedImages/image {}.png'.format(simName), 'wb')
        image = p.read().split(",")
        imageRaw = image[1]
        image_result.write(base64.b64decode(imageRaw))
        markdownDisplay = '<img src="./Figures/GeneratedImages/image {}.png" alt = "test pic" style="background-color: white;"> '.format(simName)
        display(Markdown(markdownDisplay))

# if runSim == "n": # Error code if we dont want to proceed further
#     raise SystemExit

# for filename in os.listdir(file_directory): # All files from the directory Lab0
#     f = os.path.join(file_directory, filename)
#     if os.path.isfile(f) and filename.endswith('.txt'):# If the file is a txt file.
#         temp = filename.split('-')  # Split by - to get the [simName].txt
#         #print(temp[1])  # Print all simualtions
#         for simName in simulationsArray:    # If one of the simulations mentioned by the student is that simualtion,
#             if temp[1].startswith(simName): # add the simulation to the listOfSimulationParams which we run later.
#                 with open(f, 'r') as infp:
#                     listOfSimulationParams.append(infp.read())

json_data = []
for filename in os.listdir(file_directory): # All files from the directory Lab0
    f = os.path.join(file_directory, filename)
    if os.path.isfile(f) and filename.endswith('.txt'):# If the file is a txt file.
        temp = filename.split('-')
        for simName in simulationsArray:    # If one of the simulations mentioned by the student is that simualtion,
            if temp[1].startswith(simName): # add the simulation to the listOfSimulationParams which we run later.
                with open(f, 'r') as fobj:
                    json_data = json.load(fobj)

if len(json_data) == 0:                                        # If no files match, raise an error
  print("No topologies saved which match the criteria")
  raise SystemExit                                                          # Exit

counter = 1                                                                 # Keep track of which simulation we are running

##########################################################################################
campaign_dir = []
campaign = []

uetemp = []
for ue in json_data["ueList"]:
    uetemp.append('(' + str(ue['x']) + ',' + str(ue['y']) + '),')

enbtemp = []
for ue in json_data["eNBList"]:
    enbtemp.append('(' + str(ue['x']) + ',' + str(ue['y']) + '),')


txpower = int(json_data["txPower"])



uePos = ''.join(uetemp)
eNBPos = ''.join(enbtemp)

# print(uePos)
# print(eNBPos)
# print(txpower)


res_path = 'results-uestates' + '-' + ''.join(json_data["name"]) + '-' + str(txpower) + '-' + str(runid)                                # result path for naming outstreams
campaign_dir = os.path.normpath(os.path.join(os.getcwd(), 'Results', res_path))


print("Connection Tracing Simualtions for RAN...")

campaign = sem.CampaignManager.new(ns_path, script, campaign_dir, overwrite=True, optimized=False, check_repo = False, max_parallel_processes=8)           #create a new sem campaign


params = {
    'eNBTxPowerDbm': txpower,
    'arrayPosUEsString': uePos,
    'arrayPoseNBsString' :eNBPos,
    'runId': runid,
}

log_components = {
    'LteEnbMac': 'info',
    'LteEnbRrc': 'info',
    'LteRrcProtocolIdeal': 'info',
    'LteUeRrc': 'info',
    'LteEnbRrc': 'debug',
    'LteRrcProtocolIdeal': 'debug',

}

runs = 1
print("Starting simulation {}...".format(counter))
campaign.run_missing_simulations(params, runs=runs, log_components=log_components)

##########################################################################################
print("Simulation {} finished!".format(counter))
ran_logs = campaign.db.get_complete_results(params=params, log_components=log_components) #Results

counter = counter+1

print("There are %s results in the database\n" % len(list(ran_logs)))

##########################################################################################

campaign_dir = []
campaign = []


res_path = 'results-enbstates' + '-' + ''.join(json_data["name"]) + '-' + str(txpower) + '-' + str(runid)                                # result path for naming outstreams
campaign_dir = os.path.normpath(os.path.join(os.getcwd(), 'Results', res_path))


print("Connection Tracing Simualtions for EPC...")

campaign = sem.CampaignManager.new(ns_path, script, campaign_dir, overwrite=True, optimized=False, check_repo = False, max_parallel_processes=8)           #create a new sem campaign


params = {
    'eNBTxPowerDbm': txpower,
    'arrayPosUEsString': uePos,
    'arrayPoseNBsString' :eNBPos,
    'runId': runid,
}

log_components = {
    'EpcMmeApplication': 'logic',
    'EpcPgwApplication': 'debug',
    'EpcEnbApplication': 'info',
    'PointToPointEpcHelper': 'logic',
}

# log_components = {
#     'EpcMmeApplication': 'debug',
#     'EpcPgwApplication': 'debug',
#     'EpcEnbApplication': 'info',
#     'EpcSgwApplication': 'info',
# }

runs = 1
print("Starting simulation {}...".format(counter))
campaign.run_missing_simulations(params, runs=runs, log_components=log_components)

##########################################################################################
print("Simulation {} finished!".format(counter))
epc_logs = campaign.db.get_complete_results(params=params, log_components=log_components) #Results

print("There are %s results in the database\n" % len(list(epc_logs)))




# args = sys.argv

# distance = float(args[1])
# # radius = float(args[2])
# # buidling =[]
# # apnd = []
# # if args[3] == 'True':
# #     building = True
# #     apnd = 'urban'
# # elif args[3] == 'False':
# #     building = False
# #     apnd = 'rural'


# #for rsrp values
# ########################################################################################
# ns_path = os.path.join(os.path.expanduser('~'), 'repos', 'sem-example', 'ns-3')
# script = 'task2-1' # ns-3 script in ../ns-3/scratch/
# res_path = 'results-uestates' + str(distance);
# campaign_dir = os.path.join(os.path.expanduser('~'), 'repos', 'sem-example', 'ttm4133-notebooks', 'lab3', 'results', res_path)
# campaign = sem.CampaignManager.new(ns_path, script, campaign_dir, overwrite=True,optimized=False, #create a new sem campaign
#                                    check_repo = False, max_parallel_processes=8)

# params = {
#     'd1': distance,
# }
# log_components = {
#     'LteEnbMac': 'info',
#     'LteEnbRrc': 'info',
#     'LteRrcProtocolIdeal': 'info',
#     'LteUeRrc': 'info',
#     'LteEnbRrc': 'debug',
#     'LteRrcProtocolIdeal': 'debug',
# }
# runs = 1

# campaign.run_missing_simulations(params, runs=runs, log_components=log_components)
# ########################################################################################
# script = 'task2-1' # ns-3 script in ../ns-3/scratch/
# res_path = 'results-enbstates' + str(distance);
# campaign_dir = os.path.join(os.path.expanduser('~'), 'repos', 'sem-example', 'ttm4133-notebooks', 'lab3', 'results', res_path)
# campaign1 = sem.CampaignManager.new(ns_path, script, campaign_dir, overwrite=True,optimized=False, #create a new sem campaign
#                                    check_repo = False, max_parallel_processes=8)

# params = {
#     'd1': distance,
# }
# runs = 1
# log_components = {
#     'EpcMmeApplication': 'debug',
#     'EpcPgwApplication': 'debug',
#     'EpcEnbApplication': 'info',
#     'EpcSgwApplication': 'info',
# }
# campaign1.run_missing_simulations(params, runs=runs, log_components=log_components)
# ########################################################################################
# script = 'task2-1' # ns-3 script in ../ns-3/scratch/
# res_path = 'results-flow' + str(distance);
# campaign_dir = os.path.join(os.path.expanduser('~'), 'repos', 'sem-example', 'ttm4133-notebooks', 'lab3', 'results', res_path)
# campaign2 = sem.CampaignManager.new(ns_path, script, campaign_dir, overwrite=True, #create a new sem campaign
#                                    check_repo = False, max_parallel_processes=8)

# params = {
#     'enableInstTput': True,
#     'd1': distance,
# }
# runs = 1

# campaign2.run_missing_simulations(params, runs=runs)

# #for sinr values
# ########################################################################################
# ns_path1 = os.path.join(os.path.expanduser('~'), 'repos', 'sem-example', 'ns-3')
# script1 = 'task2' # ns-3 script in ../ns-3/scratch/
# res_path1 = 'results-sinr'+ '-' + str(int(txpower)) + '-' + str(int(radius));
# campaign_dir1 = os.path.join(os.path.expanduser('~'), 'repos', 'sem-example', 'ttm4133-notebooks', 'lab2', 'results', str(apnd),
#                             res_path1)
# campaign1 = sem.CampaignManager.new(ns_path1, script1, campaign_dir1, overwrite=True, #create a new sem campaign
#                                    check_repo = False, max_parallel_processes=8)

# params1 = {
#     'eNBTxPowerDbm': txpower,
#     'enbDist': radius,
#     'enablebuilding': building,
#     'enablesinrenb': True,
# }
# runs1 = 1

# campaign1.run_missing_simulations(params1, runs=runs1)
# ########################################################################################

# #for throughput
# ########################################################################################
# ns_path2 = os.path.join(os.path.expanduser('~'), 'repos', 'sem-example', 'ns-3')
# script2 = 'task2' # ns-3 script in ../ns-3/scratch/
# res_path2 = 'results-flow'+ '-' + str(int(txpower)) + '-' + str(int(radius));
# campaign_dir2= os.path.join(os.path.expanduser('~'), 'repos', 'sem-example', 'ttm4133-notebooks', 'lab2',
#                             'results', str(apnd), res_path2)
# campaign2 = sem.CampaignManager.new(ns_path2, script2, campaign_dir2, overwrite=True, #create a new sem campaign
#                                    check_repo = False, max_parallel_processes=8)

# params2 = {
#     'eNBTxPowerDbm': txpower,
#     'enbDist': radius,
#     'enablebuilding': building,
# #     'enablersrp': True,
#     'enableflowstats': True,
# #     'enablesinrue': True,
# #     'enablesinrenb': True,
# }
# runs2 = 1

# campaign2.run_missing_simulations(params2, runs=runs2)
# ########################################################################################