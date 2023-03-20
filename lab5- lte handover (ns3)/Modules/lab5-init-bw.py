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

script = 'lab5'  # NS3 Script to run, (change name here)

listOfSimulationParams = [] # List with all simulations

file_directory = os.path.normpath(os.path.join(os.getcwd(), '../../', 'ttm4133-servers', 'files-upload', 'Labs', 'Lab4'))  # Directory with simulation params

img_directory = os.path.normpath(os.path.join(os.getcwd(), '../../', 'ttm4133-servers', 'files-upload', 'Images', 'Lab4'))  # Directory with simulation params

imgout_dir = os.path.normpath(os.path.join(os.getcwd(), '../../', 'ttm4133-spring23', 'lab5- lte handover (ns3)', 'Figures'))  # Directory with simulation params

txpower = 40   # Use 40 until react gui parsing  (change for lab1)

enableX2 = True
t11 = []
t12 = []

apnd = 'HO'
bandwidth1 = []
bandwidth2 = []
enablediffreq = []

# print("Current working directory: {0}".format(os.getcwd())) # To get cwd

for filename in os.listdir(file_directory):
  f = os.path.join(file_directory, filename)
  if os.path.isfile(f) and filename.endswith('.txt'):
    temp = filename.split('-')
    temp2 = temp[1].split('.')
    print(temp2[0])

print("")


simulations = input("List the simulation to run: ")                                         # String with simulation names
# simulations = 'lab5test'
simulationsArray = simulations.split(',')                                                   # Split by ,
print("")
print("Please enter the following required parameters:")
while True:
    print("Enter the channel bandwidth for 1st eNB (valid values in MHz: 1.4,3,5,10,15,20)")
    t11 = float(input())
    if t11 == 1.4:
        bandwidth1 = 6
        break
    elif t11 == 3:
        bandwidth1 = 15
        break
    elif t11 == 5:
        bandwidth1 = 25
        break
    elif t11 == 10:
        bandwidth1 = 50
        break
    elif t11 == 15:
        bandwidth1 = 75
        break
    elif t11 == 20:
        bandwidth1 = 100
        break
    if bandwidth1 == []:
        continue
while True:
    print("Enter the channel bandwidth for 2nd eNB (valid values in MHz: 1.4,3,5,10,15,20)")
    t12 = float(input())
    if t12 == 1.4:
        bandwidth2 = 6
        break
    elif t12 == 3:
        bandwidth2 = 15
        break
    elif t12 == 5:
        bandwidth2 = 25
        break
    elif t12 == 10:
        bandwidth2 = 50
        break
    elif t12 == 15:
        bandwidth2 = 75
        break
    elif t12 == 20:
        bandwidth2 = 100
        break
    if bandwidth2 == []:
        continue

apnd2 = []
while True:        
    print("Enable different carrier frequencies in eNBs (yes OR y OR no OR n):")    
    t13 = input()
    if t13 == 'y':
        enablediffreq = True
        apnd2 = 'diff'
        break
    if t13 == 'n':
        enablediffreq = False
        apnd2 = 'same'
        break
    if t13 == 'yes':
        enablediffreq = True
        apnd2 = 'diff'
        break
    if t13 == 'no':
        enablediffreq = False
        apnd2 = 'same'
        break
    if enablediffreq == []:
        continue
    

print("Channel bandwidth for the eNBs is (MHz): ", t11, t12)
    
    

showTopos = input("Show a picture of the topology? (y/n)")
# showTopos = 'n'
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

uetemp = []
for ue in json_data["ueList"]:
    uetemp.append('(' + str(ue['x']) + ',' + str(ue['y']) + '),')
    
mobileuetemp = []
for ue in json_data["mobileUeList"]:
    mobileuetemp.append('(' + str(ue['x']) + ',' + str(ue['y']) + '),')

enbtemp = []
for enb in json_data["eNBList"]:
    enbtemp.append('(' + str(enb['x']) + ',' + str(enb['y']) + '),')
    
buildingtemp = []
for building in json_data["buildingList"]:
    buildingtemp.append('(' + str(building['x']) + ',' + str(building['y']) + '),')

txpower = int(json_data["txPower"])


uePos = ''.join(uetemp)
mobileUePos = ''.join(mobileuetemp)
eNBPos = ''.join(enbtemp)
uecomb = uePos + "m" + mobileUePos
buildingPos = ''.join(buildingtemp)






enablebuild = []

if buildingPos == '':
    enablebuild = False
    buildingPos = ''.join('(0,0),')
    
if buildingPos != '':
    enablebuild = True

# print(enablebuild)
# print(buildingPos)


eNBPos = 'es'+ eNBPos + 'ee_'
buildingPos = 'bs'+ buildingPos + 'be_'
uePos = 'us'+ uePos + 'ue_'
mobileUePos = 'ms'+ mobileUePos + 'me_'
    
# print('eNB Pos is:', eNBPos)
# print('Building Pos is:', buildingPos)
# print('sUE Pos is:', uePos)
# print('mUE Pos is:', mobileUePos)

combPos = eNBPos + buildingPos + uePos + mobileUePos

# print(combPos)


res_path = 'results-rsrp' + '-' + ''.join(json_data["name"]) + '-' + str(txpower) + '-' + str(runid) + '-' + str(bandwidth1) + '-' + str(bandwidth2) + '-' + apnd2                               # result path for naming outstreams
campaign_dir = os.path.normpath(os.path.join(os.getcwd(), 'Results', str(apnd), res_path))
campaign = sem.CampaignManager.new(ns_path, script, campaign_dir, overwrite=True,           #create a new sem campaign
                                       check_repo = False, max_parallel_processes=4)
print("RSRP and RSRQ simulation running...")
params = {
    'testPos': combPos,
    'enablersrp': True,
    'eNBTxPowerDbm': txpower,
    'runId': runid,
    'enableX2': enableX2,
    'bw1': bandwidth1,
    'bw2': bandwidth2,
    'enable_diff_freq': enablediffreq,
}
runs = 1
campaign.run_missing_simulations(params, runs=runs)
##########################################################################################
print("Simulation {} finished!".format(counter))
result_signalpower = campaign.db.get_complete_results(params=params) #Results
counter = counter+1


##########################################################################################
campaign_dir = []
campaign = []

print("SINR simulation running...")

res_path = 'results-sinr' + '-' + ''.join(json_data["name"]) + '-' + str(txpower) + '-' + str(runid) + '-' + str(bandwidth1) + '-' + str(bandwidth2) + '-' + apnd2   
campaign_dir = os.path.normpath(os.path.join(os.getcwd(), 'Results', str(apnd), res_path))
campaign = sem.CampaignManager.new(ns_path, script, campaign_dir, overwrite=True,          
                                   check_repo = False, max_parallel_processes=4)
params = {
    'testPos': combPos,
    'enablesinrue': True,
    'eNBTxPowerDbm': txpower,
    'runId': runid,
    'enableX2': enableX2,
    'bw1': bandwidth1,
    'bw2': bandwidth2,
    'enable_diff_freq': enablediffreq,
}
runs = 1
campaign.run_missing_simulations(params, runs=runs)
##########################################################################################
print("Simulation {} finished!".format(counter))
result_sinr = campaign.db.get_complete_results(params=params) #Results
counter = counter+1

##########################################################################################
campaign_dir = []
campaign = []

print("Throughput simulation running...")

res_path = 'results-tput' + '-' + ''.join(json_data["name"]) + '-' + str(txpower) + '-' + str(runid) + '-' + str(bandwidth1) + '-' + str(bandwidth2) + '-' + apnd2   
campaign_dir = os.path.normpath(os.path.join(os.getcwd(), 'Results', str(apnd), res_path))
campaign = sem.CampaignManager.new(ns_path, script, campaign_dir, overwrite=True,          
                                   check_repo = False, max_parallel_processes=4)
params = {
    'testPos': combPos,
    'enableInstTput': True,
    'eNBTxPowerDbm': txpower,
    'runId': runid,
    'enableX2': enableX2,
    'bw1': bandwidth1,
    'bw2': bandwidth2,
    'enable_diff_freq': enablediffreq,
}
runs = 1
campaign.run_missing_simulations(params, runs=runs)
##########################################################################################
print("Simulation {} finished!".format(counter))
result_throughput = campaign.db.get_complete_results(params=params) #Results
counter = counter+1