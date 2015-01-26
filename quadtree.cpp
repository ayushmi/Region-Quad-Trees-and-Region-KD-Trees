#include <iostream>
#include <vector>
#include <queue>
#include <math.h>
#include <time.h>
#include <cstdio>
using namespace std;

//A quadtree node
struct node
{
	node *q1,*q2,*q3,*q4;						//Pointers to the four quandrants. Usual sense
	bool isleaf;								//To keep track if the node is a leaf
	bool containsData;							//To check if this node contains some data
	pair<float,float> data;						//The value of the data point contained in this node.
	pair<float,float> c1;						//Bottom left coordinates of the box
	pair<float,float> c2;						//Top right coordinates of the box
};

//Struct required for overloading comparision for priority queue used in the KNN search query.
struct CompareKNN {
	//Second element of the pair is the distance of the node center from the query point.
	bool operator()(pair<node*,float> const &p1,pair<node*,float> const &p2){
		//return true if p1 is ordered before p2 in terms of distance to the query point.
		return p1.second>p2.second;
	}
};

class quadtree
{
private:
	node* root;
	priority_queue<pair<node*,float>,
					vector<pair<node*,float> >,
					CompareKNN > myqueue;
	int dk;										//stores the current distance for KNN
	vector<pair<float,float> > answer;			//stores the answer of KNN
public:
	quadtree(){
		root = new node;
		initialize(root,make_pair(0.0,0.0),make_pair(1.0,1.0),NULL);
	};

	node * getRootNode(){
		return root;
	}	

	void print(vector<pair<float,float> >answer){
		if(answer.size() == 0){
			//cout<<"No results found"<<endl;
		} else{
			for (int i = 0; i < answer.size(); ++i){
				printf("%0.6f\t%0.6f\n",answer[i].first,answer[i].second);
			}
		}
	}

	void insert_iterative(node* anode,pair<float,float> point){
	
		int done = 0;
		node *temp = anode;
		while(done == 0){
			if (temp->isleaf == true && temp->containsData==false){
				//If this node is leaf and has no data then insert data in it
				temp->containsData = true;							//Now this node contains data.
				temp->data = point;									//Update the data of anode.
				done =1;
				pair<float,float> midpoint = findMidPoint(temp);	//Find midpoint of the anode
			} else if(temp->isleaf == true){
				if (temp->data == point){
					//Two points cannot have same coordinates.
					return;
				}
				//If this node is a leaf but it already contains some data.
				split(temp);										//Split the node. Puts the old data into new leaf

				//Now anode is not a leaf and hence insert point into its leaf node
				pair<float,float> midpoint = findMidPoint(temp);	//Find midpoint of the anode
				int quadrant = checkQuadrant(point,midpoint);		//1|2|3|4 based on the quadrant the node is in.
				switch(quadrant){
					case 1: temp = temp->q1; break;
					case 2: temp = temp->q2; break;
					case 3: temp = temp->q3; break;
					case 4: temp = temp->q4; break;
				}
			} else{
				//Not a leaf and hence cannot contain data therefore insert to one of its child
				pair<float,float> midpoint = findMidPoint(temp);	//Find midpoint of the anode
				int quadrant = checkQuadrant(point,midpoint);		//1|2|3|4 based on the quadrant the node is in.
				switch(quadrant){
					case 1: temp = temp->q1; break;
					case 2: temp = temp->q2; break;
					case 3: temp = temp->q3; break;
					case 4: temp = temp->q4; break;
				}
			}
		}
	}

	void insert(node* anode,pair<float,float> point){
	
		if (anode->isleaf == true && anode->containsData==false){
			//If this node is leaf and has no data then insert data in it
			anode->containsData = true;							//Now this node contains data.
			anode->data = point;								//Update the data of anode.
			return;
		} else if(anode->isleaf == true){
			if (anode->data == point){
				return;
			}
			//If this node is a leaf but it already contains some data.
			split(anode);										//Split the node. Puts the old data into new leaf

			//Now anode is not a leaf and hence insert point into its leaf node
			pair<float,float> midpoint = findMidPoint(anode);	//Find midpoint of the anode
			int quadrant = checkQuadrant(point,midpoint);		//1|2|3|4 based on the quadrant the node is in.
			switch(quadrant){
				case 1: insert(anode->q1,point); break;
				case 2: insert(anode->q2,point); break;
				case 3: insert(anode->q3,point); break;
				case 4: insert(anode->q4,point); break;
			}
			return;

		} else{
			//Not a leaf and hence cannot contain data therefore insert to one of its child
			pair<float,float> midpoint = findMidPoint(anode);	//Find midpoint of the anode
			int quadrant = checkQuadrant(point,midpoint);		//1|2|3|4 based on the quadrant the node is in.
			switch(quadrant){
				case 1: insert(anode->q1,point); break;
				case 2: insert(anode->q2,point); break;
				case 3: insert(anode->q3,point); break;
				case 4: insert(anode->q4,point); break;
			}
			return;
		}
	}

	//Returns true or false based on whether the pointis present in the dataset or not.
	bool pointSearch(node *anode,pair<float,float> point){
		if (anode->isleaf == true && anode->containsData == true){
			//anode is leaf and contains data then match data with the query
			if (anode->data == point){
				return true;
			} else{
				return false;
			}
		} else if(anode->isleaf==true){
			//anode doesnot contains any data and is leaf
			return false;
		} else {
			//anode is an internal node.
			pair<float,float> midpoint = findMidPoint(anode);	//Find midpoint of the anode
			int quadrant = checkQuadrant(point,midpoint);		//1|2|3|4 based on the quadrant the node is in.
			switch(quadrant){
				case 1: return(pointSearch(anode->q1,point)); break;
				case 2: return(pointSearch(anode->q2,point)); break;
				case 3: return(pointSearch(anode->q3,point)); break;
				case 4: return(pointSearch(anode->q4,point)); break;
			}
		}
		return false;
	}

	vector<pair<float,float> > rangeSearch(node *anode,pair<float,float> center,float range){
		vector<pair<float,float> > answer;
		if (Intersects(anode,center,range)){
			//Intersects
			if (anode->isleaf == true && anode->containsData == true){
				//If anode is a leaf node and contains data.
				if(euclid(center,anode->data)<=range) answer.push_back(anode->data);
				return answer;
			} else if(anode->isleaf == true){
				//If anode is a leaf node but does not contain any data point
				return answer;
			} else{
				//If anode is an internal node, then search all its children and written the answer.
				vector<pair<float,float> > answerQ1 = rangeSearch(anode->q1,center,range);
				vector<pair<float,float> > answerQ2 = rangeSearch(anode->q2,center,range);
				vector<pair<float,float> > answerQ3 = rangeSearch(anode->q3,center,range);
				vector<pair<float,float> > answerQ4 = rangeSearch(anode->q4,center,range);
				answer.insert(answer.end(),answerQ1.begin(),answerQ1.end());
				answer.insert(answer.end(),answerQ2.begin(),answerQ2.end());
				answer.insert(answer.end(),answerQ3.begin(),answerQ3.end());
				answer.insert(answer.end(),answerQ4.begin(),answerQ4.end());
				return answer;
			}
		} else{
			//Does not intersetcts
			return answer;
		}
		return answer;
	}

	vector<pair<float,float> > windowSearch(node *anode,pair<float,float> point1, pair<float,float> point2){
		vector<pair<float,float> > answer;

		if (
				//Check if the any point of query window lies within anode box.
			 (checkPointInRectangle(anode->c1,anode->c2,point1) 
				|| checkPointInRectangle(anode->c1,anode->c2,point2)
				|| checkPointInRectangle(anode->c1,anode->c2,make_pair(point1.first,point2.second))
				|| checkPointInRectangle(anode->c1,anode->c2,make_pair(point2.first,point1.second)))
				//Or Check if the whole anode box lies within the query window.
			||(checkPointInRectangle(point1,point2,anode->c1)
				&&checkPointInRectangle(point1,point2,anode->c2)
				&&checkPointInRectangle(point1,point2,make_pair((anode->c1).first,(anode->c2).second))
				&&checkPointInRectangle(point1,point2,make_pair((anode->c2).first,(anode->c1).second))
				)
			){
			//Intersects
			if(anode->isleaf == true && anode->containsData == true){
				//anode is a leaf node and contains data
				if (checkPointInRectangle(point1,point2,anode->data)){
					//If data point lies in the query window then add it to answer set
					answer.push_back(anode->data);
				}
				return answer;
			} else if(anode->isleaf == true){
				//anode is a leaf node but doesnot contains data
				return answer;
			} else{
				vector<pair<float,float> > answerQ1 = windowSearch(anode->q1,point1,point2);
				vector<pair<float,float> > answerQ2 = windowSearch(anode->q2,point1,point2);
				vector<pair<float,float> > answerQ3 = windowSearch(anode->q3,point1,point2);
				vector<pair<float,float> > answerQ4 = windowSearch(anode->q4,point1,point2);
				answer.insert(answer.end(),answerQ1.begin(),answerQ1.end());
				answer.insert(answer.end(),answerQ2.begin(),answerQ2.end());
				answer.insert(answer.end(),answerQ3.begin(),answerQ3.end());
				answer.insert(answer.end(),answerQ4.begin(),answerQ4.end());
				return answer;
			}
		}
		else {
			//If no intersection then return empty vector.
			return answer;
		}
		return answer;
	}

	vector<pair<float,float> > knnSearch(node *anode,pair<float,float> query,int k){
		//Intialisations necessary for the search function
		myqueue = priority_queue<pair<node*,float>,vector<pair<node*,float> >,CompareKNN>();	//Initialize the priority queue, containing pair of float pointer, 
		dk= -1;																					//stores the current distance.
		answer.clear();
		answer = vector<pair<float,float> >();																			//stores the answer
		if (k==0) return answer;
		if(anode->isleaf == true && anode->containsData == true){
			//There is only one data point in the tree
			answer.push_back(anode->data);
			return answer;
		} else if(anode->isleaf == true){
			//There is no data point in the tree.
			return answer;
		} else {
			//anode is non leaf root node.
			myqueue.push(make_pair(anode,findDistanceToBox(anode,query)));						//Insert root
			//Since this node does not contains any data point therefore do not need to update the Answer or dk
			while(!myqueue.empty()){

				//Extract the first element of the queue
				pair<node*,float> extract = myqueue.top();
				myqueue.pop();
				node *anode = extract.first;
				float nodeDistance = extract.second;
				//cout<<nodeDistance<<endl;
				if(anode->isleaf == true && anode->containsData == true){
					if (answer.size() == k && euclid(anode->data,query)<dk){
						answer.pop_back();
						//cout<<"removed last"<<endl;
						answer.push_back(anode->data);
						//cout<<"inserted:"<<((anode->data).first)<<","<<((anode->data).second)<<endl;
						dk = euclid(anode->data,query);
					}
					else if(answer.size() == k){
						continue;
					} else{
						answer.push_back(anode->data);
						//cout<<"inserted:"<<((anode->data).first)<<","<<((anode->data).second)<<endl;
						if(k==answer.size()) dk = euclid(anode->data,query);
					}
				} else if(anode->isleaf == true){
					//anode is leaf but does not contains data.
					continue;
				} else{
					//anode is a non-leaf node. Therefore add its children to the queue.
					float d1 = findDistanceToBox(anode->q1,query);
					float d2 = findDistanceToBox(anode->q2,query);
					float d3 = findDistanceToBox(anode->q3,query);
					float d4 = findDistanceToBox(anode->q4,query);
					//cout<<d1<<","<<d2<<","<<d3<<","<<d4<<endl;
					if(dk == -1){
						myqueue.push(make_pair(anode->q1,d1));
						myqueue.push(make_pair(anode->q2,d2));
						myqueue.push(make_pair(anode->q3,d3));
						myqueue.push(make_pair(anode->q4,d4));	
					} else{
						if (d1<dk) myqueue.push(make_pair(anode->q1,d1));
						if (d2<dk) myqueue.push(make_pair(anode->q2,d2));
						if (d3<dk) myqueue.push(make_pair(anode->q3,d3));
						if (d4<dk) myqueue.push(make_pair(anode->q4,d4));
					}
				}	
			}
		}
		return answer;
	}

private: 
		void initialize(node* anode,pair<float,float> c1,pair<float,float> c2, node *parent){
			anode->q1 = NULL;
			anode->q2 = NULL;
			anode->q3 = NULL;
			anode->q4 = NULL;
			anode->isleaf = true;				//Initially every node is a leaf node.
			anode->containsData = false;		//Initially a node does not contains data.
			anode->data = make_pair(-1,-1);
			anode->c1 = c1;						//LowerLeft coordinates
			anode->c2 = c2;						//TopRight coordinates
		}

		//Splits the box into four boxes and inserts the data point into those four boxes
		void split(node *anode){
			
			//Set anode as the inner node
			anode->isleaf = false;
			anode->containsData = false;
			pair<float,float> data = anode->data;
			anode->data = make_pair(-1,-1);
			
			//Make four new children nodes for anode
			pair<float,float> midpoint = findMidPoint(anode);
			anode->q1 = new node;
			initialize(anode->q1,midpoint,anode->c2,anode);
			anode->q2 = new node;
			initialize(anode->q2,make_pair((anode->c1).first,midpoint.second),make_pair(midpoint.first,(anode->c2).second),anode);
			anode->q3 = new node;
			initialize(anode->q3,anode->c1,midpoint,anode);
			anode->q4 = new node;
			initialize(anode->q4,make_pair(midpoint.first,(anode->c1).second),make_pair((anode->c2).first,midpoint.second),anode);

			//Insert data of anode into its children.
			int quadrant = checkQuadrant(data,midpoint);
			switch(quadrant){
				case 1: 
					(anode->q1)->data = data;
					(anode->q1)->containsData = true;
					break;
				case 2: 
					(anode->q2)->data = data;
					(anode->q2)->containsData = true;
					break;
				case 3: 
					(anode->q3)->data = data;
					(anode->q3)->containsData = true;
					break;
				case 4: 
					(anode->q4)->data = data;
					(anode->q4)->containsData = true;
					break;
			}

			return;
		}

		pair<float,float> findMidPoint(node *anode){
			return make_pair(
						((anode->c1).first + (anode->c2).first)/2.0,
						((anode->c1).second+(anode->c2).second)/2.0);
		}

		int checkQuadrant(pair<float,float> point,pair<float,float> midpoint){
			if (point.first>=midpoint.first && point.second>=midpoint.second){
				return 1;
			} else if(point.first<midpoint.first && point.second>=midpoint.second){
				return 2;
			} else if(point.first<midpoint.first){
				return 3;
			} else return 4;
		}

		bool checkPointInRectangle(pair<float,float> c1,pair<float,float> c2,pair<float,float> point1){
			if(
				(c1.first<=point1.first)
				&& (c2.first>=point1.first)
				&& (c1.second<=point1.second)
				&& (c2.second>=point1.second)){ 
				return true;
			} else{
				return false;
			}
		}

		float euclid(pair<float,float> point1,pair<float,float> point2){
			return(sqrt((point1.first-point2.first)*(point1.first-point2.first)
						+(point1.second-point2.second)*(point1.second-point2.second)));
		}

		bool Intersects(node *anode,pair<float,float> center,float range){
			pair<float,float> midpoint = findMidPoint(anode);									//Find midpoint of the anode
			float height = (anode->c2).second - (anode->c1).second;								//Width of the rectangle
			float width = (anode->c2).first - (anode->c1).first;								//Height of the rectangle

			float xcenterDistance = abs(midpoint.first-center.first); 							//Distance along x axis between centers of rectangle and cicle.
			float ycenterDistance = abs(midpoint.second-center.second);							//Distance along y axis between centers of rectangle and cicle.
			float distanceToCorner = (xcenterDistance-width/2.0)*(xcenterDistance-width/2.0)	//Distance from center of the circle to the corner
									+(ycenterDistance-height/2.0)*(ycenterDistance-height/2.0);

			if(xcenterDistance>(width/2.0 + range))												//If distance along x axis is greater than the width/2+radius
				return false;
			if(ycenterDistance>(height/2.0 + range)) 											//If distance along y axis is greater than the height/2+radius
				return false;
			if(xcenterDistance<=(width/2) || ycenterDistance<=(height/2)) 						//Center lies within the rectangle.
				return true;
			if (distanceToCorner <= range*range)												//Circle intersects the corner of the rectangle.
				return true;
			return false;
		}

		float abs(float a){
			if (a>=0) return(a);
			else return(-a);
		}

		float findDistanceToBox(node *anode,pair<float,float> point){
			if (checkPointInRectangle(anode->c1,anode->c2,point)){
				//If point is in the rectangle then return 0.
				return 0;
			} else{
				//If point is not in the rectangle find minimum distance to the box.
				pair<float,float> p1 = anode->c1;
				pair<float,float> p2 = make_pair((anode->c1).first,(anode->c2).second);
				pair<float,float> p3 = anode->c2;
				pair<float,float> p4 = make_pair((anode->c2).first,(anode->c1).second);
				if (point.second<p1.second)
				{
					if(point.first>=p1.first && point.first<=p4.first){
						return(distanceToLine(p1,p4,point));
					} else if(point.first<p1.first){
						return(euclid(p1,point));
					} else{
						return(euclid(p4,point));
					}
				} else if(point.second>p2.second) {
					if(point.first>=p2.first && point.first<=p3.first){
						return(distanceToLine(p2,p3,point));
					} else if(point.first<p2.first){
						return(euclid(p2,point));
					} else{
						return(euclid(p3,point));
					}
				} else if(point.first<p1.first){
					return(distanceToLine(p1,p2,point));	
				} else{
					return(distanceToLine(p3,p4,point));
				}
				return 0;
			}
		}

		float distanceToLine(pair<float,float> p1,pair<float,float> p2,pair<float,float> point){
			float x1 = p1.first;
			float x2 = p2.first;
			float y1 = p1.second;
			float y2 = p2.second;
			float x0 = point.first;
			float y0 = point.second;
			//(y2-y1)x-y(x2-x1)+y1x2-x1y2
			return abs(((y2-y1)*x0-(x2-x1)*y0+x2*y1-y2*x1)/(euclid(p1,p2)));
		}
};

int main(){
	quadtree t;
	
	clock_t start_t, end_t, total_t=0;

	int insertionCount=0;
	int rangeCount=0;
	int pointCount=0;
	int windowCount=0;
	int knnCount=0;

	//Insert inital data to the result.
	int nums;
	cin>>nums;
	for (int i = 0; i < nums; ++i)
	{
		float x,y;
		scanf("%f %f",&x,&y);
		//start_t = clock();
		t.insert(t.getRootNode(),make_pair(x,y));
		//end_t = clock();
		//cout<<((double)(end_t - start_t) / CLOCKS_PER_SEC)<<endl;
	}

	//insertionCount+=nums;

	int Queries;
	cin>>Queries;
	int count1=0,count2=0;
	for (int i = 0; i < Queries; ++i)
	{
		int code;
		scanf("%d",&code);
		float x1,y1;
		float x2,y2;
		float range;
		int k;
		bool resultPointSearch = false;
		switch(code){
			case 0: 
				//Insertion
				scanf("%f %f",&x1,&y1);
				printf("0\t%0.6f\t%0.6f\n",x1,y1);
				//start_t = clock();
				t.insert(t.getRootNode(),make_pair(x1,y1));
				//end_t = clock();
				//cout<<((double)(end_t - start_t) / CLOCKS_PER_SEC)<<endl;
				//insertionCount++;
				printf("\n");
				break;
			case 1:
				//Point Query
				scanf("%f %f",&x1,&y1);
				printf("1\t%0.6f\t%0.6f\n",x1,y1);
				//start_t = clock();
				resultPointSearch = t.pointSearch(t.getRootNode(),make_pair(x1,y1));
				//end_t = clock();
				//cout<<((double)(end_t - start_t) / CLOCKS_PER_SEC)<<endl;
				//pointCount++;
				if(resultPointSearch){
					printf("True\n");
				} else{
					printf("False\n");
				}
				printf("\n");
				break;
			case 2:
				//Range Query
				scanf("%f %f %f",&x1,&y1,&range);
				printf("2\t%0.6f\t%0.6f\t%0.6f\n",x1,y1,range);
				//start_t = clock();
				t.print(t.rangeSearch(t.getRootNode(),make_pair(x1,y1),range));
				//end_t = clock();
				//cout<<((double)(end_t - start_t) / CLOCKS_PER_SEC)<<endl;
				//rangeCount++;
				printf("\n");
				break;
			case 3:
				//kNN Query
				scanf("%f %f %d",&x1,&y1,&k);
				printf("3\t%0.6f\t%0.6f\t%d\n",x1,y1,k);
				//start_t = clock();
				t.print(t.knnSearch(t.getRootNode(),make_pair(x1,y1),k));
				//end_t = clock();
				//cout<<((double)(end_t - start_t) / CLOCKS_PER_SEC)<<endl;
				//knnCount++;
				printf("\n");
				break;
			case 4:
				//Window Query
				scanf("%f %f %f %f",&x1,&y1,&x2,&y2);
				printf("4\t%0.6f\t%0.6f\t%0.6f\t%0.6f\n",x1,y1,x2,y2);
				//start_t = clock();
				t.windowSearch(t.getRootNode(),make_pair(x1,y1),make_pair(x2,y2));
				//end_t = clock();
				//windowCount++;
				//cout<<((double)(end_t - start_t) / CLOCKS_PER_SEC)<<endl;
				if(x1>x2){
					float tempx = x1;
					x1 = x2;
					x2 = tempx;
				}
				if (y1>y2){
					float tempy = y1;
					y1 = y2;
					y2 = tempy;
				}
				t.print(t.windowSearch(t.getRootNode(),make_pair(x1,y1),make_pair(x2,y2)));
				printf("\n");
				break;
		}
	}
	return 0;
}