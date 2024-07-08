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


cdmDB = "CDM_LST1_fit.csv"

dates = pd.date_range("20240101", periods=1,freq="M")

try :
	CDM_Period_DB = pd.read_csv(cdmDB, index_col=0, parse_dates=True)
	a = CDM_Period_DB.index[-1]+ timedelta(days=1)
	lign = pd.DataFrame([], index=pd.date_range(str(a.year)+str(a.month).zfill(2)+str(a.day).zfill(2), periods=1,freq="M"), columns=['Nfiles','p0_x','p1_x','p2_x','p0_y','p1_y','p2_y'])
	CDM_Period_DB = pd.concat([CDM_Period_DB, lign])
except :
	CDM_Period_DB = pd.DataFrame([], index=dates, columns=['Nfiles','p0_x','p1_x','p2_x','p0_y','p1_y','p2_y'])

if datetime.now()<CDM_Period_DB.index[-1] :
	print("Month is not finished. Aborting")
	raise RuntimeError


y = CDM_Period_DB.index[-1].year
m = CDM_Period_DB.index[-1].month

cal = Cal.Calibrator()

file_list = glob.glob(log_folder+str(y)+"-"+str(m).zfill(2)+"*.log")

for f in file_list:
	os.system("rsync -prv "+f+" "+work_folder)


file_list = glob.glob(work_folder+"/target_data_"+str(y)+"-"+str(m).zfill(2)+"*.log")

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

cal.fit_zd_dep()
CDM_Period_DB.iloc[-1] = np.concatenate([[cal.nfiles],cal.fit_res_x,cal.fit_res_y])
CDM_Period_DB.to_csv(cdmDB)

os.system("rm "+work_folder+"/*")
# plt = cal.plot_y_zd_fit()
# plt.show()

# fig = cal.plot_residual()
# fig.show()

# fig = cal.plot_resdual_distribution()
# fig.show()