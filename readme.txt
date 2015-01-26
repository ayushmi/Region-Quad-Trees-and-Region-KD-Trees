Run the following commands: 
(replace 
	num1 by number of data points in the datafile
	num2 by number of queries in the query file)

$ g++ quadtree.cpp
$ (echo 'num1'; cat datafile; echo 'num2'; cat queryfile;) | ./a.out > answerfile