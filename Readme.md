Region Quad Trees and Region KD Trees - Implementation
=======================================================

1. Introduction
----------------

This is a c++ implementation of the Region Quad Trees and Region KD Trees for 2D dataset. Code supports Insertion, Point Search, Range Search and Window Search. Deletion is not supported yet. I have performed certain experiments withrespect to time required for different queries, results are described in the report.

2. Running Code
----------------

**Input Format:** First specify the number of initial insertions you want to make to the index structure. In following lines specify x and y coordinates of the points, one pair in each line. Next, specify the number of queries you want to perform. In the following lines specify one query per line in the following format:  
For Insertion: 0 xval yval  
For Point Query: 1 xval yval  
For Range Query: 2 xcenter ycenter range  
For kNN Query: 3 xval yval valueOfK  
For Window Query: 4 xBottomLeft yBottomLeft xTopRight yTopRight  


