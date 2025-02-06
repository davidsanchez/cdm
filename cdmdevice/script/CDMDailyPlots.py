import numpy as np
import os
import matplotlib.pyplot as plt
import pandas as pd
import glob
import Calibrator as Cal
from datetime import datetime, timedelta
# from astropy.time import Time

log_folder = "/home/cdmmgr/log/saved/"
work_folder = "/home/cdmmgr/output/work/"
out_folder = "/home/cdmmgr/output/plots/"
cdmDB = "CDM_LST1_fit.csv"

CDM_Period_DB = pd.read_csv(cdmDB, index_col=0, parse_dates=True)

y = datetime.now().year
m = datetime.now().month
d = datetime.now().day

name = str(y)+"-"+str(m).zfill(2)+"-"+str(d).zfill(2)
cal = Cal.Calibrator()

file_list = glob.glob(log_folder+str(y)+"-"+str(m).zfill(2)+"-"+str(d).zfill(2)+"*.log")

for f in file_list:
	os.system("rsync -prv "+f+" "+work_folder)

file_list = glob.glob(work_folder+"/target_data_"+str(y)+"-"+str(m).zfill(2)+"-"+str(d).zfill(2)+"*.log")

for f in file_list:
	try :
		os.system("sed -i \'\' \"/ -7.35 -7.35/d\" "+f)
		os.system("sed -i \'\' \"/ 1 1/d\" "+f)
		os.system("sed -i \'\' \"/ 07:30:0/d\" "+f) 
		os.system("sed -i \'\' \"/^$/d\" "+f)
		cal.readFile(f)
	except:
		print("error reading the files")
		pass
	
print("Reading CSV DB file")
cal.fit_res_x = [CDM_Period_DB["p0_x"][-1],CDM_Period_DB["p1_x"][-1],CDM_Period_DB["p2_x"][-1]]
cal.fit_res_y = [CDM_Period_DB["p0_y"][-1],CDM_Period_DB["p1_y"][-1],CDM_Period_DB["p2_y"][-1]]
plt = cal.plot_y_zd_fit(out_folder,name,write=True)
#plt.show()

fig = cal.plot_residual(out_folder,name,write=True)
#fig.show()

fig = cal.plot_resdual_distribution(out_folder,name,write=True)
#fig.show()
os.system("rm "+work_folder+"/*")
