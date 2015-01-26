#include <iostream>
#include <vector>
#include <queue>
#include <cstdio>
#include <stdlib.h>
#include <cmath>
using namespace std;

struct node
{
	node *left,*right;							//Pointers to the left and right node.
	bool isleaf;								//To keep track if the node is a leaf
	bool containsData;							//To check if this node contains some data
	pair<float,float> data;						//The value of the data point contained in this node.
	float coordinate;							//Value of the coordinate according to which it was split
	int splitDimension;							//Dimension of this level 0 or 1
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

class kdtree
{
private:
	node *root;
	priority_queue<pair<node*,float>,
					vector<pair<node*,float> >,
					CompareKNN > myqueue;													//Priority Queue Used for KNN queries.
	int dk;																					//stores the current distance for KNN
	vector<pair<float,float> > answer;														//stores the answer of KNN
public:
	kdtree(){
		root = new node;
		initialize(root,0,make_pair(0.0,0.0),make_pair(1.0,1.0));							//root node represents x level
	}

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

	void initialize(node *anode, int splitDimension, pair<float,float> c1,pair<float,float> c2){
		anode->left = NULL;
		anode->right = NULL;
		anode->isleaf = true;
		anode->containsData = false;
		anode->data = make_pair(-1.0,-1.0);
		anode->splitDimension = splitDimension;
		anode->coordinate = -1;
		anode->c1 = c1;
		anode->c2 = c2;
	}

	void insert(pair<float,float> point){
		int done = 0;
		node *temp = root;
		while(done == 0){
			if (temp->isleaf && !temp->containsData){
				//Is a leaf node and does not contains data.
				temp->data = point;
				temp->containsData = true;
				done = 1;
				pair<float,float> midpoint = findMidPoint(temp);	//Find midpoint of the anode
				//cout<<"Inserted:("<<point.first<<","<<point.second<<") into ("<<midpoint.first<<","<<midpoint.second<<")"<<endl;
			} else if(temp->isleaf){
				if (temp->data == point){
					//Two points cannot have same coordinates.
					return;
				}
				//Is a leaf node and contains a data point
				split(temp);
				//Now anode is not a leaf and hence insert point into its leaf node
				if (takeDecision(temp,point) == 0) temp = temp->left;
				else temp = temp->right;
			} else{
				//temp is an internal node.
				if (takeDecision(temp,point) == 0) temp = temp->left;
				else temp = temp->right;
			}
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
			if (takeDecision(anode,point) == 0){
				return(pointSearch(anode->left,point));
			} else {
				return(pointSearch(anode->right,point));
			}
		}
		return false;
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
				vector<pair<float,float> > answerLeft = windowSearch(anode->left,point1,point2);
				vector<pair<float,float> > answerRight = windowSearch(anode->right,point1,point2);
				answer.insert(answer.end(),answerLeft.begin(),answerLeft.end());
				answer.insert(answer.end(),answerRight.begin(),answerRight.end());
				return answer;
			}
		}
		else {
			//If no intersection then return empty vector.
			return answer;
		}
		return answer;
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
				vector<pair<float,float> > answerLeft = rangeSearch(anode->left,center,range);
				vector<pair<float,float> > answerRight = rangeSearch(anode->right,center,range);
				answer.insert(answer.end(),answerLeft.begin(),answerLeft.end());
				answer.insert(answer.end(),answerRight.begin(),answerRight.end());
				return answer;
			}
		} else{
			//Does not intersetcts
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
					float d1 = findDistanceToBox(anode->left,query);
					float d2 = findDistanceToBox(anode->right,query);
					//cout<<d1<<","<<d2<<","<<d3<<","<<d4<<endl;
					if(dk == -1){
						myqueue.push(make_pair(anode->left,d1));
						myqueue.push(make_pair(anode->right,d2));
					} else{
						if (d1<dk) myqueue.push(make_pair(anode->left,d1));
						if (d2<dk) myqueue.push(make_pair(anode->right,d2));
					}
				}	
			}
		}
		return answer;
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

	bool checkPointInRectangle(pair<float,float> c1,pair<float,float> c2,pair<float,float> point1){
		//If the point is on the boundary of the rectangle then also returns true
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

	void split(node *anode){
		anode->isleaf = false;												//anode is now not a leaf node.
		pair<float,float> point = anode->data;		
		anode->containsData = false;										//anode doesnot contains data.
		anode->data = make_pair(-1.0,-1.0);

		pair<float,float> midpoint = findMidPoint(anode);
		//cout<<"Splitting:("<<midpoint.first<<","<<midpoint.second<<")"<<endl;
		
		if(anode->splitDimension == 0){
			//If level of anode was x level

			//Set split coordinate of anode
			anode->coordinate = ((anode->c1).first+(anode->c2).first)/2.0;

			//Create left and right nodes.
			anode->left = new node;
			anode->right = new node;
			initialize(anode->left,1,anode->c1,make_pair(anode->coordinate,(anode->c2).second));
			initialize(anode->right,1,make_pair(anode->coordinate,(anode->c1).second),anode->c2);

			//Insert point into left or right node
			if (point.first<=anode->coordinate){
				pair<float,float> midpoint = findMidPoint(anode->left);
				//cout<<"Inserted:("<<point.first<<","<<point.second<<") into ("<<midpoint.first<<","<<midpoint.second<<") wrt x"<<endl;
				(anode->left)->data = point;
				(anode->left)->containsData = true;
			} 
			else{
				pair<float,float> midpoint = findMidPoint(anode->right);
				//cout<<"Inserted:("<<point.first<<","<<point.second<<") into ("<<midpoint.first<<","<<midpoint.second<<") wrt y"<<endl;
				(anode->right)->data = point;	
				(anode->right)->containsData = true;
			} 
		} else{
			//If level of anode is y level

			//Set split coordinate for the left node.
			anode->coordinate = ((anode->c1).second+(anode->c2).second)/2.0;

			//Create left and right node
			anode->left = new node;
			anode->right = new node;
			initialize(anode->left,0,anode->c1,make_pair((anode->c2).first,anode->coordinate));
			initialize(anode->right,0,make_pair((anode->c1.first),anode->coordinate),anode->c2);

			//Insert point into left or right node.
			if (point.second<=anode->coordinate) {
				pair<float,float> midpoint = findMidPoint(anode->left);
				//cout<<"Inserted:("<<point.first<<","<<point.second<<") into ("<<midpoint.first<<","<<midpoint.second<<") wrt x"<<endl;
				(anode->left)->data = point;
				(anode->left)->containsData = true;
			}
			else{
				pair<float,float> midpoint = findMidPoint(anode->right);
				//cout<<"Inserted:("<<point.first<<","<<point.second<<") into ("<<midpoint.first<<","<<midpoint.second<<") wrt y"<<endl;
				(anode->right)->data = point;
				(anode->right)->containsData = true;	
			}
		}
		return;
	}

	//Take decision to move left or right
	int takeDecision(node *anode,pair<float,float> point){
		if (anode->splitDimension == 0){
			//x level
			if (point.first <= anode->coordinate) return 0;
			else return 1;
		} else{
			//y level
			if (point.second <= anode->coordinate) return 0;
			else return 1;
		}
	}

	pair<float,float> findMidPoint(node *anode){
		return make_pair(
					((anode->c1).first + (anode->c2).first)/2.0,
					((anode->c1).second+(anode->c2).second)/2.0);
	}

	float euclid(pair<float,float> point1,pair<float,float> point2){
		return(sqrt((point1.first-point2.first)*(point1.first-point2.first)
					+(point1.second-point2.second)*(point1.second-point2.second)));
	}

	float abs(float a){
		if (a>=0) return(a);
		else return(-a);
	}

	float findDistanceToBox(node *anode,pair<float,float> point){
			if (checkPointInRectangle(anode->c1,anode->c2,point)){
				//If point is in the rectangle then return 0.
				//cout<<"Yes"<<endl;
				return 0;
			} else{
				//cout<<"NO"<<endl;
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

	kdtree t;

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
		t.insert(make_pair(x,y));
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
				t.insert(make_pair(x1,y1));
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