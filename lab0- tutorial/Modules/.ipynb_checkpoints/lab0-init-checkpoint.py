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
import json

import random
from datetime import datetime

curr_dt = datetime.now()
timestamp = int(round(curr_dt.timestamp()))
random.seed(timestamp)

runid = random.randint(1,100)


#print("Current working directory: {0}".format(os.getcwd()))                                # To get cwd

print("List of created simulations: ")
print(" ")

ns_path = os.path.normpath(os.path.join(os.getcwd(), '../../', 'ns-3'))            # Path for simulation purposes
script = 'lab0'                                                                            # NS3 Script to run, (change name here)

listOfSimulationParams = []                                                                 # List with all simulations
file_directory = os.path.normpath(os.path.join(os.getcwd(), '../../', 'ttm4133-servers', 'files-upload', 'Labs', 'Lab0'))  # Directory with simulation params                                                   # Directory with simulation params

img_directory = os.path.normpath(os.path.join(os.getcwd(), '../../', 'ttm4133-servers', 'files-upload', 'Images', 'Lab0'))  # Directory with simulation params

imgout_dir = os.path.normpath(os.path.join(os.getcwd(), '../../', 'ttm4133-spring23', 'lab0- tutorial', 'Figures'))



txpower = 25                                                                                # Use 40 until react gui parsing  (change for lab1)


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




if showTopos == "y":
    print("Generating topology pictures...")

    for simName in simulationsArray:
        #print('Simulation {}'.format(simName))
        
        p = open(img_directory + "/base64Image-{}.txt".format(simName), "r")
        image_result = open(imgout_dir + '/GeneratedImages/image {}.png'.format(simName), 'wb')
        #image_result = open(imgout_dir + '/GeneratedImages/image/{}.PNG'.format("UEFig"), 'wb')
        
        image = p.read().split(",")
        imageRaw = image[1]
        image_result.write(base64.b64decode(imageRaw))
        markdownDisplay = '<img src="./Figures/GeneratedImages/image {}.png" alt = "test pic" style="background-color: white;"> '.format(simName)
        display(Markdown(markdownDisplay))
    



# for filename in os.listdir(file_directory):                                      # All files from the directory Lab0

#     f = os.path.join(file_directory, filename)                                   # The entire path
    
#     if os.path.isfile(f) and filename.endswith('.txt'):                     # If the file is a txt file.
#         temp = filename.split('-')                                          # Split by - to get the [simName].txt
#         #print(temp[1])                                                      # Print all simualtions
#         for simName in simulationsArray:                                    # If one of the simulations mentioned by the student is that simualtion,
#             if temp[1].startswith(simName):                                 # add the simulation to the listOfSimulationParams which we run later.
#                 tempFile = open(f, 'r')
#                 listOfSimulationParams.append(tempFile.read())


# if len(listOfSimulationParams) == 0:                                        # If no files match, raise an error
#   print("No topologies saved which match the criteria")
#   raise SystemExit                                                          # Exit

# counter = 1                                                                 # Keep track of which simulation we are running

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

enbtemp = []
for enb in json_data["eNBList"]:
    enbtemp.append('(' + str(enb['x']) + ',' + str(enb['y']) + '),')
    
buildingtemp = []
for building in json_data["buildingList"]:
    buildingtemp.append('(' + str(building['x']) + ',' + str(building['y']) + '),')

txpower = int(json_data["txPower"])



uePos = ''.join(uetemp)
eNBPos = ''.join(enbtemp)
buildingPos = ''.join(buildingtemp)

    
# print(uePos)
# print(eNBPos)
# print(buildingPos)
# print(txpower)


if buildingPos == '':
    buildingPos = ''.join('(0,0),')

campaign_dir = []
campaign = []    
res_path = []

res_path = 'results-rsrp' + '-' + ''.join(json_data["name"]) + '-' + str(txpower)                                # result path for naming outstreams
campaign_dir = os.path.normpath(os.path.join(os.getcwd(), 'Results', res_path))
campaign = sem.CampaignManager.new(ns_path, script, campaign_dir, overwrite=True,           #create a new sem campaign
                                       check_repo = False, max_parallel_processes=8)
print("Signal Power Received simualtions...")
params = {
    'arrayPosUEsString': uePos,
    'arrayPosBuildingsString': buildingPos,
    'enablersrp': True,
    'eNBTxPowerDbm': txpower,
    'arrayPoseNBsString' :eNBPos,
    'runId': runid,
}
runs = 1
campaign.run_missing_simulations(params, runs=runs)
##########################################################################################
print("Simulation {} finished!".format(counter))
result_signalpower = campaign.db.get_complete_results(params=params) #Results
counter = counter+1

campaign_dir = []
campaign = []    
res_path = []
##########################################################################################
print("SINR simualtions...")

res_path = 'results-sinr' + '-' + ''.join(json_data["name"]) + '-' + str(txpower)   
campaign_dir = os.path.normpath(os.path.join(os.getcwd(), 'Results', res_path))
campaign = sem.CampaignManager.new(ns_path, script, campaign_dir, overwrite=True,          
                                   check_repo = False, max_parallel_processes=8)
params = {
    'arrayPosUEsString': uePos,
    'arrayPosBuildingsString': buildingPos,
    'enablesinrue': True,
    'eNBTxPowerDbm': txpower,
    'arrayPoseNBsString' : eNBPos,
    'runId': runid,
}
runs = 1
campaign.run_missing_simulations(params, runs=runs)
##########################################################################################
print("Simulation {} finished!".format(counter))
result_sinr = campaign.db.get_complete_results(params=params) #Results
counter = counter+1

campaign_dir = []
campaign = []    
res_path = []
##########################################################################################
print("Throughput simualtions...")

res_path = 'results-flow'+ '-' + ''.join(json_data["name"]) + '-' + str(txpower)                            
campaign_dir = os.path.normpath(os.path.join(os.getcwd(), 'Results', res_path))
campaign = sem.CampaignManager.new(ns_path, script, campaign_dir, overwrite=True,          
                                   check_repo = False, max_parallel_processes=8)

params = {
    'arrayPosUEsString': uePos,
    'arrayPosBuildingsString': buildingPos,
    'enableInstTput': True,
    'eNBTxPowerDbm': txpower,
    'arrayPoseNBsString' : eNBPos,
    'runId': runid,
}
runs = 1
campaign.run_missing_simulations(params, runs=runs)

##########################################################################################
print("Simulation {} finished!".format(counter))
result_throughput = campaign.db.get_complete_results(params=params) #Results
counter = counter+1

# ##########################################################################################
print("Simulations finished")    

raise SystemExit