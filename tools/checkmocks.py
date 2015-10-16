"""

Reading Cullan Howlett & Angela Burden's mock catalogues.
KMarkovic, 10/04/2015
"""

from glob import glob

datapath = "/Volumes/DataStorage/Gadget/cullan/"
asciidmf = "PICOLA_L1280_N1536_R9500_z0p150_reform.*"
asciigalf = "HODFast_L1280_N1536_R9500_z0p150.dat"

# Make figure
from matplotlib import pyplot
import pylab as plt
from mpl_toolkits.mplot3d import Axes3D
import random
fig = plt.figure()
ax = Axes3D(fig)

# Gals
f = open(datapath+asciigalf, 'r')
count = 0
xg=[]
yg=[]
zg=[]
try:
	for line in f:
		linevec = line.split()
		xg.append(float(linevec[0]))
		yg.append(float(linevec[1]))
		zg.append(float(linevec[2]))
		count+=1
		if count==10000: break
finally:
	print str(count)+" datapoints read in."
	print min(xg), min(yg), min(zg)
	print max(xg), max(yg), max(zg)
ax.scatter(xg,yg,zg,color='red',label="galaxies")

# DM
labeldone=True
for filename in glob(datapath+asciidmf):
	f = open(filename, 'r')
	count = 0
	xdm=[]
	ydm=[]
	zdm=[]
	try:
		for line in f:
			linevec = line.split()
			xdm.append(float(linevec[0]))
			ydm.append(float(linevec[1]))
			zdm.append(float(linevec[2]))
			count+=1
			if count==1000: break
	finally:
		print str(count)+" datapoints read in."
		print min(xdm), min(ydm), min(zdm)
		print max(xdm), max(ydm), max(zdm)

	ax.scatter(xdm,ydm,zdm,color='black',label="DM" if labeldone else "")
	if labeldone: labeldone=False

plt.legend()
plt.show()

