#include <iostream>
#include <fstream>
#include <cmath>
#include <vector>
#include <algorithm>

using namespace std;

int NumRB=50;
int NumOfMMS = 3;

int NumOfUEinMMS=15;
string inCQIGroup("CQIGroups45vc.txt");
string inRBCQI("RBCQIs45vc.txt");
string inUserID("UserID45vc.txt");
int NumUe = NumOfUEinMMS;
vector<double> CQIgroups[3];

int times =100;
int QualityLevel=5;
double throughputpower=0.0;
vector<double> fairness;
vector<double> ADR;

double avUse=0;
vector<int> UserMatch[3];
int preLevel[3][500];
int SwitchCount=0;
vector<int> Switches;
bool first=true;
vector<double> UBI;
double tempUBI;

vector<int> RBs[3][50];//NumOfRBs
int CQIBit[16]={0,16,16,32,56,88,120,144,208,280,328,408,488,552,616,616};
int videoBitrate[5] = {100,375,750,1750,3000};

double highest=0,middle=0,lowest=0;

bool cmpRBCQI(pair<int,int>a,pair<int,int>b)
{
    return a.second > b.second;
}

double ResourceAllocation(int mode){
	//mode 0 for 1 MMS, mode 1 for all MMS
	int NumOfGroups = NumOfMMS;
	int FinishNum =0;
    vector<double> Group[NumOfGroups];
	int RBGroupCQI[NumOfGroups][NumRB];
	//make groups
    for(int i=0;i<NumOfGroups;++i){
        for(int j=0;j<NumOfUEinMMS;++j)
            Group[i].push_back(CQIgroups[i][j]);
    }
	
	//get worst CQI of each group
		int RBUEIndex=0;
		for (int i=0;i<NumOfMMS;++i){
	//		cout<<"Group "<<i<<endl;
			for(int j=0;j<NumRB;++j){
				int tempCQI=100;
				for(int k=0;k<Group[i].size();++k){
	//				cout<<RBs[j][k]<<' ';
					if(tempCQI>RBs[i][j][k])
						tempCQI=RBs[i][j][k];
				}
	//			cout<<endl;
				RBGroupCQI[i][j]= tempCQI;
	//			cout<<tempCQI<<' ';

			}
	//		cout<<endl;
			RBUEIndex+=Group[i].size();
		}


	vector<int> remainRBs;
	for(int i =0;i<NumRB;++i){
		remainRBs.push_back(i);
	}
	int allocatedGroups[NumOfGroups];
	double throughput[NumOfGroups];
	for(int p=0;p<NumOfGroups;++p){
		allocatedGroups[p]=0;
		throughput[p]=0;
	}
	vector<int> RA[NumOfGroups];

	while(FinishNum!=NumOfGroups){
		int NumOfRBs[NumOfGroups];
		vector<int> tempRA[NumOfGroups];
		double tempthroughput[NumOfGroups];
		for(int i=0;i<NumOfGroups;++i){
			//skip allocated groups
			if(allocatedGroups[i]==1)
				continue;

			vector<pair<int, int> > TempRBs;
			for(int j=0;j<remainRBs.size();++j){
				pair<int,int> t;
				t.first = remainRBs[j];
				t.second=RBGroupCQI[i][j];
				TempRBs.push_back(t);
			}
			sort(TempRBs.begin(),TempRBs.end(),cmpRBCQI);
//print sort result
/*			
			for(int j=0;j<TempRBs.size();++j){
				cout<<TempRBs[j].second<<' ';
			}
			cout<<endl;
*/
			//allocate RBs
			double tempbitrate=0;
			int worstCQI=100;
			for(int j=0;tempbitrate<videoBitrate[0];++j){
///////////////////////////////////////////////////////////////////////////////////////////還要處理成bitrate
				if(j>=remainRBs.size()){
					//cout<<"Not Enough For Base "<<i<<endl;
					allocatedGroups[i]=1;
					FinishNum++;
					break;
				}

				tempRA[i].push_back(TempRBs[j].first);
                //cout<<"Group "<<i<<" allocate CQI "<<TempRBs[j].second<<endl;
				if(worstCQI>TempRBs[j].second){
					worstCQI=TempRBs[j].second;
					tempbitrate=tempRA[i].size() * worstCQI;
				}else{
					tempbitrate=tempbitrate+CQIBit[worstCQI];
				}
			}
			tempthroughput[i]=tempbitrate*Group[i].size();
            //cout<<"Group "<<i<<' '<<tempRA[i].size()<<endl;
		}
		if(FinishNum==NumOfGroups)
		 break;

		//find the group with least RBs
		int min=1000000000;
		int allocateGroup=0;
		for(int i=0;i<NumOfGroups;++i){
			if(allocatedGroups[i]==1)
			 	continue;
			if(tempRA[i].size()<min){
				min=tempRA[i].size();
				allocateGroup=i;
			}
		}

		//cout<<allocateGroup<<endl;
		//delete RBs and group
		allocatedGroups[allocateGroup]=1;
		if(mode==1){
			cout<<"Base layer "<<allocateGroup<<endl;
		}
		RA[allocateGroup]=tempRA[allocateGroup];
		for(int k=0;k<tempRA[allocateGroup].size();++k){
			for(int j=0;j<remainRBs.size();++j){
				if(remainRBs[j]==tempRA[allocateGroup][k]){
					//cout<<remainRBs[j]<<endl;
					remainRBs.erase(remainRBs.begin()+j);
				}
			}
		}
		throughput[allocateGroup]=tempthroughput[allocateGroup];
		FinishNum++;
	}
	cout<<"Remain RB after Base "<<remainRBs.size()<<endl;
	//calculate utility
	//cout<<"Basic Level Utility:"<<endl;
	double GroupUtility[NumOfGroups];
	for(int i =0;i<NumOfGroups;++i){
		GroupUtility[i]=(double) throughput[i]/(double) RA[i].size();
		//cout<<"Group "<<i<<':'<<GroupUtility[i]<<endl;
	}
	int GReachHighest=0;
	int OutGroups[NumOfGroups];
	int GroupLevel[NumOfGroups];
	for(int i=0;i<NumOfGroups;i++){
		GroupLevel[i]=0;
		OutGroups[i]=0;
	}
	//Allocation for higher level
	while(remainRBs.size()!=0 && GReachHighest<NumOfGroups){
		//cout<<"Remain RBs "<<remainRBs.size()<<endl;
		double max=-999999;
		int bestGroup;
		for(int i=0;i<NumOfGroups;++i){
			if(OutGroups[i]==1)
				continue;
			if(GroupUtility[i]>max){
				max=GroupUtility[i];
				bestGroup=i;
			}
		}
		//cout<<"BestGroup: Group "<<bestGroup<<endl;
		vector<pair<int, int> > TempRBs;
		for(int j=0;j<remainRBs.size();++j){
			pair<int,int> t;
			t.first = remainRBs[j];
			t.second=RBGroupCQI[bestGroup][j];
			TempRBs.push_back(t);
		}
		sort(TempRBs.begin(),TempRBs.end(),cmpRBCQI);

		//allocate RBs
		double tempbitrate=0;
		int worstCQI=100;
		vector<int> tempRA=RA[bestGroup];
		int NotEnough=0;
		for(int j=0;tempbitrate<videoBitrate[GroupLevel[bestGroup]+1];++j){
///////////////////////////////////////////////////////////////////////////////////////////還要處理成bitrate
			tempRA.push_back(TempRBs[j].first);
			if(worstCQI>TempRBs[j].second){
				worstCQI=TempRBs[j].second;
				tempbitrate=tempRA.size() * CQIBit[worstCQI];
			}else{
				tempbitrate=tempbitrate+CQIBit[worstCQI];
			}
			if(j==TempRBs.size()-1 && tempbitrate<videoBitrate[GroupLevel[bestGroup]+1]){
				NotEnough=1;
				break;
			}
		}

		if(NotEnough){
			//cout<<"Not Enough!!"<<endl;
			GReachHighest++;
			OutGroups[bestGroup]=1;
		}else{
			//cout<<"Allocate success!!"<<endl;
			throughput[bestGroup]=tempbitrate*Group[bestGroup].size();
			//cout<<"Best Group "<<bestGroup<<" throughput:"<<throughput[bestGroup]<<endl;
			RA[bestGroup]=tempRA;
			GroupUtility[bestGroup]=tempbitrate/(double) RA[bestGroup].size();
			GroupLevel[bestGroup]++;
			if(GroupLevel[bestGroup]==(QualityLevel-1)){
				GReachHighest++;
				OutGroups[bestGroup]=1;
			}

			for(int k=0;k<tempRA.size();++k){
				for(int j=0;j<remainRBs.size();++j){
					if(remainRBs[j]==tempRA[k]){
						//cout<<remainRBs[j]<<endl;
						remainRBs.erase(remainRBs.begin()+j);
					}
				}
			}
		}
		//if(GReachHighest==NumOfGroups)
		//	cout<<"Can't level up!!"<<endl;
		//if(remainRBs.size()==0)
		//	cout<<"All RBs allocated"<<endl;
	}
	for(int i=0;i<NumOfGroups;++i){
		//cout<<"Group "<<i<<':'<<GroupLevel[i]<<" Groupsize "<<Group[i].size()<<endl;
	}
	double result=0.0;
    double tempADR=0.0;
    int useRB=0;
	double fair=0.0;
	throughputpower=0.0;
    int UE = NumOfMMS*NumUe;
    double temphighest=0, tempmiddle=0,templowest=0;
    int userIndex=0;
	int MMIndex=0;
    tempUBI=0;

	for(int i=0;i<NumOfGroups;++i){
        if(userIndex==NumUe){
			userIndex=0;
			MMIndex++;
		}
	    if(throughput[i]==0){//throughput =0 implies the group was kicked out
			UE=UE-Group[i].size();
            if(mode ==1){
				for(int j=0;j<Group[i].size();++j){
					int t = UserMatch[MMIndex][userIndex];
					preLevel[MMIndex][t]=-1;
					userIndex++;
				}
			}
			continue;
		}
        result=result+log10(throughput[i]*10);
		cout<<"Throughput "<<i<<':'<<throughput[i]<<endl;
		cout<<"allocate RBs"<<RA[i].size()<<endl;
        useRB+=RA[i].size();

                //for fairness

        fair=fair+throughput[i]*10;
		if(mode == 1){
			double temp = throughput[i]*10/Group[i].size();
			cout<<"throughput per user "<<temp<<endl;
			for(int j=0;j<Group[i].size();++j){
				throughputpower = throughputpower + pow(temp,2.0);
			}
            if(temp<4000){
				templowest=templowest+Group[i].size();
			}else if(temp>30000){
				temphighest=temphighest+Group[i].size();
			}else{
				tempmiddle=tempmiddle+Group[i].size();
			}

            if(!first){
				for(int j=0;j<Group[i].size();++j){

					int t = UserMatch[MMIndex][userIndex];
					if(preLevel[MMIndex][t]!=GroupLevel[i]){
						SwitchCount++;
						//cout<<"User "<<t<<endl;
					}
					preLevel[MMIndex][t]=GroupLevel[i];
					userIndex++;
				}
			}else{
				for(int j=0;j<Group[i].size();++j){

					int t = UserMatch[MMIndex][userIndex];
					preLevel[MMIndex][t]=GroupLevel[i];
					userIndex++;
				}
			}
            //For UBI
			int worstCQI=0;
			for(int l=0;l<Group[i].size();++l){
				if(l==0)worstCQI=Group[i][l];
				int bestCQI=Group[i][l];
				tempUBI+=(CQIBit[bestCQI]-CQIBit[worstCQI])*RA[i].size()*10;
			}

		}
	}


    if(mode==1){
        tempADR=result/(double) UE;
        ADR.push_back(tempADR);
		fair=pow(fair,2.0);
		fair=fair/(throughputpower*(NumUe*NumOfMMS));
		fairness.push_back(fair);

        templowest=templowest/(NumUe*NumOfMMS);
		temphighest=temphighest/(NumUe*NumOfMMS);
		tempmiddle=tempmiddle/(NumUe*NumOfMMS);

		highest=highest+temphighest;
		lowest=lowest+templowest;
		middle=middle+tempmiddle;

        avUse=avUse+useRB;
	}
    result=result/(double) useRB;
	return result;

}




int main(){

	ifstream Groupfile(inCQIGroup);
    ifstream RBCQIfile(inRBCQI);
    ifstream UserIDfile(inUserID);
    vector<double> results;


    if(!Groupfile.is_open()){
        cerr<<"Could not open file"<<inCQIGroup<<endl;
        return EXIT_FAILURE;
    }
    if(!RBCQIfile.is_open()){
        cerr<<"Could not open file"<<inRBCQI<<endl;
        return EXIT_FAILURE;
    }
	if(!UserIDfile.is_open()){
        cerr<<"Could not open file"<<inUserID<<endl;
        return EXIT_FAILURE;
    }
    for(int t=0;t<times;++t){
        if(t!=0)first=false;
		for(int k=0;k<NumOfMMS;++k){
			for (int i=1;i<=15;++i){
				cout<<"CQIGroup "<<i<<':';
				int N, tRB,tt;
				double temp;
				Groupfile>>N;
				if(N!=0){
					for(int j=0;j<N;++j){
						Groupfile>>temp;
						cout<<temp<<' ';
                        CQIgroups[k].push_back(temp);
						for(int l=0;l<NumRB;++l){
							RBCQIfile>>tRB;
							RBs[k][l].push_back(tRB);
						}
                        UserIDfile>>tt;
						UserMatch[k].push_back(tt);
						//cout<<"tt "<<tt<<endl;
					}
				}
				cout<<endl;
			}
				
		}
        results.push_back(ResourceAllocation(1));
        Switches.push_back(SwitchCount/10);
        //tempUBI=log(tempUBI);
		tempUBI/=(NumOfMMS*NumUe);
		UBI.push_back(tempUBI);

        for(int i=0;i<NumOfMMS;++i){
			for(int j=0;j<NumRB;++j){
				RBs[i][j].clear();
			}
				CQIgroups[i].clear();
            UserMatch[i].clear();
		}
    }
    double average =0.0;
	for(int i=0;i<results.size();++i){
		//cout<<"Time "<<i<<' '<<results[i]<<endl;
        //cout<<results[i]<<endl;
        average=average+results[i];
	}
    average=average/times;

    //print fairness
    double aver_fair=0.0;
	for(int i =0;i<fairness.size();++i){
		//cout<<fairness[i]<<endl;
		aver_fair=aver_fair+fairness[i];
	}
	aver_fair=aver_fair/times;
    double aveADR=0.0;
	for(int i =0;i<ADR.size();++i){
		//cout<<ADR[i]<<endl;
        aveADR=aveADR+ADR[i];
	}
    aveADR=aveADR/times;
    avUse=avUse/times;
    cout<<"Average ADR "<<aveADR<<endl;

    for(int k=0;k<Switches.size();++k){
		//cout<<Switches[k]<<endl;
	}
    //PMF
	highest=highest/times;
	lowest=lowest/times;
	middle=middle/times;

	cout<<"Average Use RB "<<avUse<<endl;
	avUse=avUse/NumRB;
	cout<<"RB Utilization "<<avUse<<endl;
    cout<<"Average "<<average<<endl;
    cout<<"Average Fairness "<<aver_fair<<endl;

	cout<<"Low "<<lowest<<endl;
	cout<<"Middle "<<middle<<endl;
	cout<<"High "<<highest<<endl;

    double aver_UBI=0;
	for(int i=0;i<UBI.size();++i){
		cout<<UBI[i]<<endl;
		if(UBI[i]>0)
			aver_UBI+=UBI[i];
	}
	cout<<"UBI "<<aver_UBI/times<<endl;

    return 0;
}