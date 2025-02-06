import numpy as np
from os.path import join
import matplotlib.pyplot as plt

class Calibrator:
	"""Main class to fit CDM data and generate residual
		units are either pixel of degrees"""
	def __init__(self):
		print("Init of the Calibrator")
		self.px2arcsec = 7.35

		self.center_x =  []
		self.center_y =   []
		self.zd =  []
		self.az =  []

		self.disp_x =  []
		self.disp_y =  []

		self.accep_zone = 1
		self.nfiles = 0

	def readFile(self,filename):
		data = np.loadtxt(filename,unpack=True, dtype=str)

		if len(data)>0:
			self.nfiles +=1

		self.zd =  np.concatenate((self.zd,np.float64(data[2])))
		self.az =  np.concatenate((self.az,np.float64(data[3])))


		self.center_x =  np.concatenate((self.center_x,np.float64(data[4])/self.px2arcsec))
		self.center_y =   np.concatenate((self.center_y,np.float64(data[5])/self.px2arcsec))


		self.disp_x =  np.concatenate((self.disp_x,np.float64(data[10])/3600))
		self.disp_y =  np.concatenate((self.disp_y,np.float64(data[11])/3600))

	def fit_zd_dep(self):
		self.fit_res_y = np.polyfit(self.zd, self.center_y, 2)
		self.fit_res_x = np.polyfit(self.zd, self.center_x, 2)

	def update_zd_dep(self,fit_res_x,fit_res_y):
		self.fit_res_y = fit_res_y
		self.fit_res_x = fit_res_x

	def control_plots(self):
		"""empty for now"""


	def plot_resdual_distribution(self,out_folder,name,write=True):
		if len(self.az)==0:
			print("No Data, aborting")
			return plt.figure()

		fig, axs = plt.subplots(2)

		p_disp_y = np.poly1d(self.fit_res_y)
		p_disp_x = np.poly1d(self.fit_res_x)
		axs[0].hist(self.center_y-p_disp_y(self.zd),bins=100,label='residual center_y')

		axs[0].set_xlabel("Residual LED center Y [pixel]")
		axs[0].legend()
		axs[0].set_xlim(-3,3)

		axs[1].hist(self.center_x-p_disp_x(self.zd),bins=100,label='residual center_x')

		axs[1].set_xlabel("Residual LED center X [pixel]")
		axs[1].legend()
		axs[1].set_xlim(-3,3)
		fig.savefig(join(out_folder,"ResidualDistribution_"+name+".png"), format='png', dpi=300)
		return fig

	def plot_residual(self,out_folder,name,write=True):
		if len(self.az)==0:
			print("No Data, aborting")
			return plt.figure()

		fig, axs = plt.subplots(2)

		p_disp_y = np.poly1d(self.fit_res_y)
		p_disp_x = np.poly1d(self.fit_res_x)
		axs[0].fill_between(np.sort(self.zd), -self.accep_zone,self.accep_zone, color="red",alpha=0.3)
		axs[0].errorbar(self.zd,self.center_y-p_disp_y(self.zd),fmt='.',label='residual center_y')

		axs[0].set_xlabel("Zenith [degree]")
		axs[0].set_ylabel("Residual LED center Y [pixel]")
		axs[0].legend()
		axs[0].set_xlim(0,89)
		axs[0].set_ylim(-3,3)

		axs[1].fill_between(np.sort(self.zd), -self.accep_zone,self.accep_zone, color="red",alpha=0.3)
		axs[1].errorbar(self.zd,self.center_x-p_disp_x(self.zd),fmt='.',label='residual center_x')

		axs[1].set_xlabel("Zenith [degree]")
		axs[1].set_ylabel("Residual LED center X [pixel]")
		axs[1].legend()
		axs[1].set_xlim(0,89)
		axs[1].set_ylim(-3,3)
		fig.savefig(join(out_folder,"Residual_"+name+".png"), format='png', dpi=300)
		return fig

	def plot_y_zd_fit(self,out_folder,name,write=True):
		if len(self.az)==0:
			print("No Data, aborting")
			return plt.figure()

		azimuth = self.az% 360
		maskNE = np.logical_and(azimuth < 90, azimuth > 0)
		maskSE = np.logical_and(azimuth < 180, azimuth > 90)
		maskSW = np.logical_and(azimuth < 270, azimuth > 180)
		maskNW = np.logical_and(azimuth < 360, azimuth > 270)

		# if self.fit_res == None:
			# return
		pol = np.poly1d(self.fit_res_y)
		plt.figure()
		try :
			plt.errorbar(self.zd[maskNE], self.center_y[maskNE], fmt=".", label="disp_y_NE")
			plt.errorbar(self.zd[maskSE], self.center_y[maskSE], fmt=".", label="disp_y_SE")
			plt.errorbar(self.zd[maskSW], self.center_y[maskSW], fmt=".", label="disp_y_SW")
			plt.errorbar(self.zd[maskNW], self.center_y[maskNW], fmt=".", label="disp_y_NE")
		except :
			plt.errorbar(self.zd, self.center_y, fmt=".", label="disp_y_NE")

		plt.xlabel("Zenith [degree]")
		plt.ylabel("LED center [pixel]")
		plt.legend()
		zd_table = np.arange(1,90,1)
		plt.plot(zd_table, pol(zd_table))
		plt.savefig(join(out_folder,"zd_fit_"+name+".png"), format='png', dpi=300)
		return plt

if __name__ == "__main__":
	cal = Calibrator()
	cal.readFile("../../Data/target_data_2024-01-10_07-30-02.0.log")
	cal.readFile("../../Data/target_data_2024-05-15_07-30-02.5.log")
	cal.readFile("../../Data/target_data_2024-05-30_07-30-02.0.log")
	cal.fit_zd_dep()
	plt = cal.plot_y_zd_fit()
	#plt.show()

	fig = cal.plot_residual()
	fig.show()

	fig = cal.plot_resdual_distribution()
	fig.show()