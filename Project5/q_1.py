import csv
import numpy as np
from pylab import *
from scipy.stats import norm
import matplotlib.pyplot as plt
from bisect import bisect_left

# i=0
# Header=[]
# row1=[]
# temp=[]
# NumberOfPackets=[]
# NumberOfBytes=[]
# FlowDuration=[]

# with open('ft-v05.2010-09-29.235501+0000.csv', 'rb') as f:
#     reader = csv.reader(f)
#     for row in reader:
#     	if i==0:
#     		# print "Header\n"
#     		Header=row;
#     		# print row
#     	else:
#     		temp=row
#     		NumberOfPackets.append(temp[4])
#     		NumberOfBytes.append(temp[5])
#     		FlowDuration.append(str(int(temp[7])-int(temp[6])))
#     		# FlowDuration
#     		if i==1:
#     			row1=row
    		
#     	i=i+1

# print i
# # print Header
# # print row1

# 	# for i in range(len(Header)):
# 	# 	# print i
# 	# 	# print Header[i]+ " = "+ row1[i]
# 	# 	# print row1[i]

# print "Number Of data = " + str(len(NumberOfPackets))



# ############################# 1.1 ##############################
# NumberOfBytes = list(map(int, NumberOfBytes))
# NumberOfPackets = list(map(int, NumberOfPackets))
# FlowDuration=list(map(int, FlowDuration))
# # print FlowDuration

# TotalNumPackets=0
# TotalNumBytes=0
# for i in range(len(NumberOfPackets)):
# 	TotalNumPackets=TotalNumPackets + NumberOfPackets[i]
# 	TotalNumBytes=TotalNumBytes + NumberOfBytes[i]

# print TotalNumPackets
# print TotalNumBytes
# print "Q1.1: Avg Packet Size= "+ str((TotalNumBytes*1.0)/TotalNumPackets)
# AvgNumPackets=(TotalNumPackets*1.0)/(len(NumberOfPackets))
# AvgNumBytes=(TotalNumBytes*1.0)/(len(NumberOfBytes))



# print "AvgNumPackets= "+str(AvgNumPackets)
# print "AvgNumBytes= "+str(AvgNumBytes)

# ########################## 1.2 ##################################


# # plt.show()
# a=FlowDuration
# sorted = np.sort(a)
# yvals = np.arange(1,len(sorted)+1)/float(len(sorted))
# plt.plot((sorted), (1-yvals))
# plt.show()
# plt.plot(np.log10(sorted), np.log10(1-yvals))
# plt.show()

# a=NumberOfPackets
# sorted = np.sort(a)
# yvals = np.arange(1,len(sorted)+1)/float(len(sorted))
# plt.plot(sorted, 1-yvals)
# plt.show()
# plt.plot(np.log10(sorted), np.log10(1-yvals))
# plt.show()

# # a=NumberOfBytes
# # sorted = np.sort(a)
# # yvals = np.arange(1,len(sorted)+1)/float(len(sorted))
# # plt.plot(sorted, 1-yvals)
# # plt.show()

# # print sorted(FlowDuration)
# a=NumberOfBytes
# sorted = np.sort(a)
# yvals = np.arange(1,len(sorted)+1)/float(len(sorted)) #http://stackoverflow.com/questions/31147893/logarithmic-plot-of-a-cumulative-distribution-function-in-matplotlib
# plt.plot(sorted, 1-yvals)
# plt.show()
# plt.plot(np.log10(sorted), np.log10(1-yvals))
# plt.show()



# ############################### 1.3 ##############################################################

# DictSenderPort={}
# DictRecvPort={}

# Header=1

# TotalNumBytes=0
# with open('ft-v05.2010-09-29.235501+0000.csv', 'rb') as f:
#     reader = csv.reader(f)
#     for row in reader:
#     	if Header==1:
#     		Header=0
#     	else:
# 			TotalNumBytes=TotalNumBytes+int(row[5])
# 			if row[15] in DictSenderPort:
# 				DictSenderPort[row[15]]=DictSenderPort[row[15]]+int(row[5])
# 			else:	
# 				DictSenderPort[row[15]]=int(row[5])
				
# 			if row[16] in DictRecvPort:
# 				DictRecvPort[row[16]]=DictRecvPort[row[16]]+int(row[5])
# 			else:	
# 				DictRecvPort[row[16]]=int(row[5])
				

# d=DictRecvPort
# i=0

# print "Receiver Port"
# for w in sorted(d, key=d.get, reverse=True):
# 	i=i+1
# 	if i<(11):
# 		print w, d[w],(d[w]*100.0)/TotalNumBytes

# print "Sender Port"
# d=DictSenderPort
# i=0
# for w in sorted(d, key=d.get, reverse=True):
# 	i=i+1
# 	if i<(11):
# 		print w, d[w], (d[w]*100.0)/TotalNumBytes

# print TotalNumBytes


######################################## 1.4  ##########################


import socket, struct

def ip2long(ip):
    """
    Convert an IP string to long
    """
    packedIP = socket.inet_aton(ip)
    return struct.unpack("!L", packedIP)[0]


def Mask(len):
	ans=0
	for x in xrange(1,len+1):
		ans=ans+pow(2,32-x)
	return ans



DictSourceIPTraffic={}

TotalTraffic=0;
TotalPackets=0
Header=1
pricetonTraffic1=0
pricetonTraffic2=0
pricetonTraffic1P=0
pricetonTraffic2P=0
with open('ft-v05.2010-09-29.235501+0000.csv', 'rb') as f:
    reader = csv.reader(f)
    for row in reader:
    	if Header==1:
    		Header=0
    	else: 
			# if int(row[20])==0:
			# 	continue
			TotalTraffic=TotalTraffic+int(row[5])  
			TotalPackets=TotalPackets+ int(row[4]) 		
			src_mask=Mask(int(row[20]))  # source mask length is at 20th index
			row[10]=ip2long(row[10])   # src address at 10th index
			row[10]=row[10]&src_mask   #masking


			des_mask=Mask(int(row[21]))  # source mask length is at 20th index
			row[11]=ip2long(row[11])   # src address at 10th index
			row[11]=row[11]&des_mask   #masking


			# row[10]=socket.inet_ntoa(struct.pack('!L', row[10]))  # again back to ip
			princetonIPPrefix=(ip2long("128.112.0.0")&Mask(16))

			if row[10]==princetonIPPrefix:
				# print "Matched"
				pricetonTraffic1=pricetonTraffic1+int(row[5])
				pricetonTraffic1P=pricetonTraffic1P+int(row[4])

			if row[11]==princetonIPPrefix:
				# print "Matched"
				pricetonTraffic2=pricetonTraffic2+int(row[5])
				pricetonTraffic2P=pricetonTraffic2P+int(row[4])

			

			# if row[10] in DictSourceIPTraffic:
			# 	DictSourceIPTraffic[row[10]]=DictSourceIPTraffic[row[10]]+int(row[5])
			# else:	
			# 	DictSourceIPTraffic[row[10]]=int(row[5])


								


# d=DictSourceIPTraffic

# LenDict=len(d)
# i=0

# Sum=0
# for w in sorted(d, key=d.get, reverse=True):
# 	i=i+1
# 	if i<(0.001*LenDict):
# 		print w, d[w]
# 		Sum=Sum+d[w]

# print TotalTraffic
# print Sum
# print Sum*1.0/TotalTraffic

print pricetonTraffic1, TotalTraffic, pricetonTraffic1*1.0/TotalTraffic

print pricetonTraffic2, TotalTraffic, pricetonTraffic2*1.0/TotalTraffic 

print pricetonTraffic1P, TotalPackets, pricetonTraffic1P*1.0/TotalPackets

print pricetonTraffic2P, TotalPackets, pricetonTraffic2P*1.0/TotalPackets 