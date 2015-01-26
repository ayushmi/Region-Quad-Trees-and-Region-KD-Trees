import csv
import sys
import numpy

filenames = ["insertionTimes","knnSearchTimes","pointSearchTimes","rangeSearchTimes2","windowSearchTimes"]
columnNames = ["Insertion","k-NN Search","Point Seach","Range Search","Window Search"]
print "\\hline"
print "Query & Minimum & Maximum & Average & Standard Deviation \\\\"
print "\\hline"
for i in range(0,5):
	mylist = numpy.genfromtxt(filenames[i])
	print columnNames[i]," & ", min(mylist)," & ",max(mylist), " & ", numpy.mean(mylist)," & ",numpy.std(mylist),"\\\\"
	print "\\hline"
