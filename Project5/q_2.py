
DictIP={}

def DictInit(FileName):
	global DictIP
	fileHandle=open(FileName, 'r')
	for line in fileHandle:
		fields=line.split('|')
		fields[5]="".join(fields[5].split())
		# fields[5].replace(" ", "")
		if '.' in fields[5]:
			if fields[5] not in DictIP:
				DictIP[fields[5]]=0
	fileHandle.close()


def DictIPUpdate(FileName):
	fileHandle=open(FileName, 'r')
	i=0
	for line in fileHandle:
		i=i+1		
		fields=line.split('|')
		# fields[5].replace(" ", "")
		fields[5]="".join(fields[5].split())
		if '.' in fields[5]:
			if fields[5] in DictIP:
				DictIP[fields[5]]=DictIP[fields[5]]+1
			# continue
		
			# if i%100==0:
			# 	print str(i)
			# 	# print DictIP[fields[5]]
			# 	print DictIP[fields[5]]
		
		# for fields[5] in DictIP:
		# 	DictIP[fields[5]]=DictIP[fields[5]]+1

			
	fileHandle.close()


############################### 2.1 ###########################################
def UpdateCount(FileName):
	fileHandle = open(FileName, 'r')
	i=0
	for line in fileHandle:
		fields = line.split('|')
		fields[5]="".join(fields[5].split())
		if '.' in fields[5]:			#IPv4
			i=i+1
		
	fileHandle.close()
	return i

CountJan=UpdateCount("updates.20140103.1200.txt")+UpdateCount("updates.20140103.1215.txt")+UpdateCount("updates.20140103.1230.txt")+UpdateCount("updates.20140103.1245.txt")+UpdateCount("updates.20140103.1300.txt")+UpdateCount("updates.20140103.1315.txt")+UpdateCount("updates.20140103.1330.txt")+UpdateCount("updates.20140103.1345.txt")
CountFeb=UpdateCount("updates.20140203.1200.txt")+UpdateCount("updates.20140203.1215.txt")+UpdateCount("updates.20140203.1230.txt")+UpdateCount("updates.20140203.1245.txt")+UpdateCount("updates.20140203.1300.txt")+UpdateCount("updates.20140203.1315.txt")+UpdateCount("updates.20140203.1330.txt")+UpdateCount("updates.20140203.1345.txt")
CountMar=UpdateCount("updates.20140303.1200.txt")+UpdateCount("updates.20140303.1215.txt")+UpdateCount("updates.20140303.1230.txt")+UpdateCount("updates.20140303.1245.txt")+UpdateCount("updates.20140303.1300.txt")+UpdateCount("updates.20140303.1315.txt")+UpdateCount("updates.20140303.1330.txt")+UpdateCount("updates.20140303.1345.txt")

print CountJan		
print CountFeb
print CountMar

print "Total Count: " + str(CountJan+CountFeb+CountMar)
AvgNumUpdateJan=(CountJan)/120.0
print "AvgNumUpdateJan: "+str(AvgNumUpdateJan)
AvgNumUpdateFeb=(CountFeb)/120.0
print "AvgNumUpdateFeb: "+str(AvgNumUpdateFeb)
AvgNumUpdateMar=(CountMar)/120.0
print "AvgNumUpdateMar: "+str(AvgNumUpdateMar)


################################### 2.2 #############################################
DictInit("../rib/rib.20140103.1200.txt")
print "Dictionary Initialised"
DictIPUpdate("updates.20140103.1200.txt")
DictIPUpdate("updates.20140103.1215.txt")
DictIPUpdate("updates.20140103.1230.txt")
DictIPUpdate("updates.20140103.1245.txt")
DictIPUpdate("updates.20140103.1300.txt")
DictIPUpdate("updates.20140103.1315.txt")
DictIPUpdate("updates.20140103.1330.txt")
DictIPUpdate("updates.20140103.1345.txt")

d= DictIP
i=0
LenDict=len(DictIP)

TotalUpdateCount=0
NumOfUpdates=0
NumOfZeroUpdate=0
UpdateCount=0
for w in sorted(d, key=d.get, reverse=True):
	i=i+1
	NumOfUpdates=NumOfUpdates+1
	TotalUpdateCount=TotalUpdateCount+d[w]
	if d[w]==0:
		NumOfZeroUpdate=NumOfZeroUpdate+1
	if i<(10):		
		print w, d[w]
	if i<(LenDict*0.001):		
		UpdateCount=UpdateCount+d[w]

print "NumOfUpdates: " ,NumOfUpdates
print "NumOfZeroUpdate:",NumOfZeroUpdate
print "Percentage of Zero Update: ", NumOfZeroUpdate*1.0/NumOfUpdates

print "MostFrequentUpdate: ",UpdateCount*1.0/TotalUpdateCount


DictInit("../rib/rib.20140203.1200.txt")
print "Dictionary Initialised"
DictIPUpdate("updates.20140203.1200.txt")
DictIPUpdate("updates.20140203.1215.txt")
DictIPUpdate("updates.20140203.1230.txt")
DictIPUpdate("updates.20140203.1245.txt")
DictIPUpdate("updates.20140203.1300.txt")
DictIPUpdate("updates.20140203.1315.txt")
DictIPUpdate("updates.20140203.1330.txt")
DictIPUpdate("updates.20140203.1345.txt")


TotalUpdateCount=0
NumOfUpdates=0
NumOfZeroUpdate=0
d= DictIP
i=0
UpdateCount=0
LenDict=len(DictIP)
for w in sorted(d, key=d.get, reverse=True):
	i=i+1
	TotalUpdateCount=TotalUpdateCount+d[w]
	NumOfUpdates=NumOfUpdates+1
	if d[w]==0:
		NumOfZeroUpdate=NumOfZeroUpdate+1
	if i<(10):		
		print w, d[w]
	if i<(LenDict*0.001):		
		UpdateCount=UpdateCount+d[w]

print "NumOfUpdates: " ,NumOfUpdates
print "NumOfZeroUpdate:",NumOfZeroUpdate
print "Percentage of Zero Update: ", NumOfZeroUpdate*1.0/NumOfUpdates
print "MostFrequentUpdate: ",UpdateCount*1.0/TotalUpdateCount

DictInit("../rib/rib.20140303.1200.txt")
print "Dictionary Initialised"
DictIPUpdate("updates.20140303.1200.txt")
DictIPUpdate("updates.20140303.1215.txt")
DictIPUpdate("updates.20140303.1230.txt")
DictIPUpdate("updates.20140303.1245.txt")
DictIPUpdate("updates.20140303.1300.txt")
DictIPUpdate("updates.20140303.1315.txt")
DictIPUpdate("updates.20140303.1330.txt")
DictIPUpdate("updates.20140303.1345.txt")




TotalUpdateCount=0
NumOfUpdates=0
NumOfZeroUpdate=0
d= DictIP
i=0
UpdateCount=0
for w in sorted(d, key=d.get, reverse=True):
	i=i+1
	NumOfUpdates=NumOfUpdates+1
	TotalUpdateCount=TotalUpdateCount+d[w]
	if d[w]==0:
		NumOfZeroUpdate=NumOfZeroUpdate+1
	if i<(10):		
		print w, d[w]

	if i<(LenDict*0.001):		
		UpdateCount=UpdateCount+d[w]
print "NumOfUpdates: " ,NumOfUpdates
print "NumOfZeroUpdate:",NumOfZeroUpdate
print "Percentage of Zero Update: ", NumOfZeroUpdate*1.0/NumOfUpdates

print "MostFrequentUpdate: ",UpdateCount*1.0/TotalUpdateCount