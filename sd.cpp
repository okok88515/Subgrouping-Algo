#include <iostream>
#include <fstream>
#include <cstdlib>
#include <vector>
#include <cmath>
#include <iomanip>
#include <tuple>
#include <stack>
#include <algorithm>
#include <fstream>
using namespace std;
//staic Variables
int times=100;
string inCQIGroup("CQIGroups75vc.txt");
string inRBCQI("RBCQIs75vc.txt");
string inUserID("UserID75vc.txt");
int NumUe =25 ;
int NumOfSubframe = 10;
int CQI = 15;
double range =0.07;
int QualityLevel=5;
double testBitrate[5];//NumOfQualityLevel
vector<int> RBs[3][50];//NumOfRBs
int NumRB = 50;
vector<double> NonZeroCQIgroups[3][16];//NumOfMMS
int NumOfMMS=3;
vector<int> bestConfig[3];//NumOFMMS
int GroupL=4;
int usercount=0;

int CQIBit[16]={0,16,16,32,56,88,120,144,208,280,328,408,488,552,616,616};
int videoBitrate[5] = {375,750,1750,3000,3850};
double NumOfSlots = 12;
double NumOfGroupCount[16];
double averGroup=0;


//Dymanic Variables
vector<int> GroupConfig;
vector<int> GroupConfigList[4000];
vector<double> SDConfig;
vector<double> SDConfigList[4000];
vector<int> CandidateNo;
vector<double> CandidateSD[4000];
int configsize=0;
int nonZero=1;
int MMSi=0;
int bestCandIndex=0;
vector<int> UserMatch[3];
int preLevel[3][500];
int SwitchCount=0;
vector<int> Switches;
bool first=true;


double avUse=0;
vector<int> bestCand;
double throughputpower=0.0;
vector<double> fairness;
vector<double> ADR;
vector<double> UBI;
double tempUBI;

double CQIDist[16];

double highest=0,middle=0,lowest=0;

bool cmpRBCQI(pair<int,int>a,pair<int,int>b)
{
    return a.second > b.second;
}

double calSD(vector<double> g){
	double total =0;
	for(int i = 0;i<g.size();++i){
		total= total+g[i];
	}
	double mean = total/ (double) g.size();

	double SD =0;
	for(int i=0;i<g.size();++i){
		SD=SD+pow(g[i]-mean,2);	
	}
	SD=SD/(double) g.size();
	SD = sqrt(SD);

	return SD;
}

void SplittingGroup(int Num, int begin, int end){
	if (Num == 1){
		GroupConfig.push_back(end-begin+1);
		vector<double> temp;
		for(int l=begin;l<=end;++l)
			temp.insert(temp.end(),make_move_iterator(NonZeroCQIgroups[MMSi][l].begin()),make_move_iterator(NonZeroCQIgroups[MMSi][l].end()));
		SDConfig.push_back(calSD(temp));
		return;
	}
	else{
		for(int i=1; i<end-begin-Num+3;++i){
			SplittingGroup(1, begin, begin+i-1);
			SplittingGroup(Num-1, begin+i, end);
			if(Num==2){
				GroupConfigList[configsize]=GroupConfig;
				for(int j =0;j<SDConfig.size();++j)
					SDConfigList[configsize].push_back(SDConfig[j]);
				configsize++;
				GroupConfig.pop_back();
				SDConfig.pop_back();
			}
			GroupConfig.pop_back();
			SDConfig.pop_back();
		}
		
	}

}
//for test
void RandomAssignRBCQI(){
	double max, min, x, C;
	for(int i=1;i<=nonZero;++i){
		for(int j=0;j<NonZeroCQIgroups[MMSi][i].size();++j){
			C = NonZeroCQIgroups[MMSi][i][j];
//			cout<<C<<endl;
			for(int k=0;k<NumRB;++k){
				max=C+1;
				if(C<2)
					min=C;
				else
					min=C-1;
        		x = (max - min) * rand() / (RAND_MAX + 1.0) + min;
				RBs[MMSi][k].push_back((int)x);
//				cout<<(int)x<<' ';
			}
//
		}
	}
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
double ResourceAllocation(int mode, int CandNum){
	//mode 0 for 1 MMS, mode 1 for all MMS
	vector<int> GConfig= GroupConfigList[CandNum];
	int NumOfGroups = 0;
	int FinishNum =0;



	if(mode ==0)
		NumOfGroups = GConfig.size();
	else{
		for(int i=0;i<NumOfMMS;++i){
			NumOfGroups += bestConfig[i].size();
		}
	}
	

	vector<double> Group[NumOfGroups];
	int RBGroupCQI[NumOfGroups][NumRB];
	//make groups
	if(mode ==0){
		int start =1;
		for(int i =0;i<NumOfGroups;++i){
			for(int j = start;j<start+GConfig[i];++j){
				Group[i].insert(Group[i].end(),make_move_iterator(NonZeroCQIgroups[MMSi][j].begin()),make_move_iterator(NonZeroCQIgroups[MMSi][j].end()));
			}
			//print Groups
			
			for(int j=0;j<Group[i].size();++j)
				cout<<Group[i][j]<<' ';
			cout<<endl;
			
			start+=GConfig[i];
		}
	}else{

		int start =1;
		int g=0;
		for(int i =0;i<NumOfMMS;++i){
			start=1;
			for(int k=0;k<bestConfig[i].size();++k){
				for(int j = start;j<start+bestConfig[i][k];++j){
					Group[g].insert(Group[g].end(),make_move_iterator(NonZeroCQIgroups[i][j].begin()),make_move_iterator(NonZeroCQIgroups[i][j].end()));
				}

				//print Groups
				cout<<"MMS "<<i<<" Group "<<k<<':';
				for(int j=0;j<Group[g].size();++j)
					cout<<Group[g][j]<<' ';
				cout<<endl;
				g++;
				start+=bestConfig[i][k];

			}
			NumOfGroupCount[bestConfig[i].size()]++;
			averGroup+=bestConfig[i].size();
			cout<<endl;
		}
	}
	
	if(mode == 0){
	//get worst CQI of each group
		int RBUEIndex=0;
		for (int i=0;i<NumOfGroups;++i){
	//		cout<<"Group "<<i<<endl;
			for(int j=0;j<NumRB;++j){
				int tempCQI=100;
				for(int k=RBUEIndex;k<RBUEIndex+Group[i].size();++k){
	//				cout<<RBs[j][k]<<' ';
					if(tempCQI>RBs[MMSi][j][k])
						tempCQI=RBs[MMSi][j][k];
				}
	//			cout<<endl;
				RBGroupCQI[i][j]= tempCQI;
	//			cout<<tempCQI<<' ';

			}
	//		cout<<endl;
			RBUEIndex+=Group[i].size();
		}
	}else{
		int RBUEIndex=0;
		int GroupIndex=0;
		for (int i=0;i<NumOfMMS;++i){
			RBUEIndex=0;
			for(int l=0;l<bestConfig[i].size();++l,GroupIndex++){
		//		cout<<"Group "<<GroupIndex<<endl;
				for(int j=0;j<NumRB;++j){
					int tempCQI=100;
					for(int k=RBUEIndex;k<RBUEIndex+Group[GroupIndex].size();++k){
		//				cout<<RBs[j][k]<<' ';
						if(tempCQI>RBs[i][j][k])
							tempCQI=RBs[i][j][k];
					}
		//			cout<<endl;
					RBGroupCQI[GroupIndex][j]= tempCQI;
		//			cout<<tempCQI<<' ';
				
				}
		//		cout<<endl;
				RBUEIndex+=Group[GroupIndex].size();
			}
		}
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
					//cout<<i<<endl;
					allocatedGroups[i]=1;
					FinishNum++;
					break;
				}

				tempRA[i].push_back(TempRBs[j].first);
				if(worstCQI>TempRBs[j].second){
					worstCQI=TempRBs[j].second;
					tempbitrate=tempRA[i].size() * worstCQI;
				}else{
					tempbitrate=tempbitrate+CQIBit[worstCQI];
				}
			}

			tempthroughput[i]=tempbitrate*Group[i].size();
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
			//cout<<"Base layer "<<allocateGroup<<endl;
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
	//cout<<"Remain RB after Base "<<remainRBs.size()<<endl;
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
	double tempADR = 0.0;
	double fair=0.0;
	double temphighest=0, tempmiddle=0,templowest=0;
	throughputpower=0.0;
	int useRB=0;
	int UE=NumUe*NumOfMMS;
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
					int ti = UserMatch[MMIndex][userIndex];
					preLevel[MMIndex][ti]=-1;
					userIndex++;
				}
			}
			continue;
		}
		//throughput[i] means the throughput of a subframe, it needs to cross a number to match
		result=result+log10(throughput[i]*NumOfSubframe);
		fair=fair+throughput[i]*NumOfSubframe;
		//cout<<"Throughput "<<i<<':'<<throughput[i]<<endl;
		//cout<<"allocate RBs"<<RA[i].size()<<endl;
		useRB+=RA[i].size();


		//for fairness and PMF
		if(mode == 1){
			double temp = throughput[i]*10/Group[i].size();
			//cout<<"throughput per user "<<temp<<endl;
			for(int j=0;j<Group[i].size();++j){
				throughputpower = throughputpower + pow(temp,2.0);
			}
			if(temp<4000){
				templowest=templowest+Group[i].size();
			}else if(temp>17500){
				temphighest=temphighest+Group[i].size();
			}else{
				tempmiddle=tempmiddle+Group[i].size();
			}
			if(!first){
				for(int j=0;j<Group[i].size();++j){

					int ti = UserMatch[MMIndex][userIndex];
					if(preLevel[MMIndex][ti]!=GroupLevel[i]){
						SwitchCount++;
						//cout<<"User "<<t<<endl;
					}
					preLevel[MMIndex][ti]=GroupLevel[i];
					userIndex++;
				}
			}else{
				for(int j=0;j<Group[i].size();++j){

					int ti = UserMatch[MMIndex][userIndex];
					preLevel[MMIndex][ti]=GroupLevel[i];
					userIndex++;
				}
			}

			//For UBI
			int worstCQI=0;
			for(int l=0;l<Group[i].size();++l){
				if(l==0)worstCQI=Group[i][l];
				int bestCQI=Group[i][l];
				tempUBI+=(CQIBit[bestCQI]-CQIBit[worstCQI])*RA[i].size()*10;

				usercount++;
			}
		}
	}

	if(mode==1){
		tempADR = result/(double) UE;
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
/////////////////////////////////////////////////////////////////////////////////////////////////////////////
vector<int> FindProperGroupConfig(){
	double CurrentUtility=-2.0, MaxUtility=-3.0;
	int NumOfSplitGroups = 1;
	double SDThreshold = 0.0;
	int NumOfCand = 0;
	bool FindConfigFlag = false;

	while(CurrentUtility>(MaxUtility+range)){
		bestCand=GroupConfigList[bestCandIndex];
		NumOfSplitGroups++;
		if(nonZero>2 && NumOfSplitGroups==(nonZero-2))break;
		GroupConfig.clear();
		SDConfig.clear();
		CandidateNo.clear();
		NumOfCand=0;
		configsize=0;
		FindConfigFlag = false;
		SDThreshold=0.0;
		for(int i=0;i<4000;++i){
			GroupConfigList[i].clear();
			SDConfigList[i].clear();
			CandidateSD[i].clear();
		}
		SplittingGroup(NumOfSplitGroups, 1, nonZero);

//may cause infinite loop!!
		while(FindConfigFlag != true){
			SDThreshold +=0.5;
			NumOfCand = 0;
			if(SDThreshold>1000){
				for(int i =0; i<configsize;++i){
					for(int j=0;j<SDConfigList[i].size();++j){
						CandidateSD[NumOfCand]=SDConfigList[i];
						CandidateNo.push_back(i);
						FindConfigFlag=true;
						NumOfCand++;

					}
				}
			}

			//cout<<SDThreshold<<endl;

			for(int i =0; i<configsize;++i){
				for(int j=0;j<SDConfigList[i].size();++j){
					if(SDConfigList[i][j]>SDThreshold || SDConfigList[i][j]==0){
						break;
					}

					if(j==SDConfigList[i].size()-1){
						CandidateSD[NumOfCand]=SDConfigList[i];
						CandidateNo.push_back(i);
						FindConfigFlag=true;
						NumOfCand++;
					}
				}
			}
		}
		//cout<<"Num of groups:"<<NumOfSplitGroups<<endl;
		//cout<<"Candidates:"<<endl;
		//for(int i=0;i<CandidateNo.size();++i){
		//	for(int j=0;j<NumOfSplitGroups;++j)
		//		cout<< GroupConfigList[CandidateNo[i]][j]<<' ';
		//	cout<<endl;
		//}


		/*************************************************************
		for(int k=0;k<NumOfCand;++k){
			cout<<"Candidate "<<k<<endl;
			for(int o=0;o<CandidateSD[k].size();++o)
				cout<<CandidateSD[k][o]<<' ';
				cout<<endl;
		}
		*/
		//calculate utility
		double r;
		MaxUtility=CurrentUtility;
		CurrentUtility=0.0;
		for(int i =0;i<CandidateNo.size();++i){
			r=ResourceAllocation(0,CandidateNo[i]);
			//<<"Utility "<<i<<": "<<r<<endl;
			if(r>CurrentUtility){
				CurrentUtility=r;
				bestCandIndex=CandidateNo[i];

			}
		}

	}
	return bestCand; 
}

int main(){
	double SDs[10][10][5];
	testBitrate[0]=10;
	for(int i=0;i<16;++i){
		CQIDist[i]=0;
	}

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
	for(int i=1;i<QualityLevel;++i){
		testBitrate[i]=testBitrate[i-1]+10;
	}
	for (int t=0;t<times;++t){
		if(t!=0)first=false;
		vector<double> CQIgroups[NumOfMMS][CQI+1];
		cout<<"Times "<<t<<endl;
		//srand( time(NULL) );
		//double max =1;
		//double min=2;
		//int numUser= rand()%7+1;
		//double x = (max - min) * rand() / (RAND_MAX + 1.0) + min;
		// produce different CQI Users by random
		/*
		for(int k=0;k<NumOfMMS;++k){
			for (int i=1;i<=CQI;++i){
				//for example test
				//if(i/2==2||i/3==3)
				//	continue;

				numUser= rand()%7+1;
				cout<<"Group CQI "<<i<<endl;
				for(int j=0;j<numUser;++j){
					max=i+1;
					min=i;
					x = (max - min) * rand() / (RAND_MAX + 1.0) + min;
					CQIgroups[k][i].push_back(x);
					NumUe++;
					cout<<x<<' ';
				}
				cout<<endl;
			}
			cout <<endl;
		}
		*/
		int fl=0;
		for(int k=0;k<NumOfMMS;++k){
			int count=0;
			for (int i=1;i<=15;++i){
				cout<<"CQIGroup "<<i<<':';
				int N, tRB,tt;
				double temp;
				Groupfile>>N;
				CQIDist[i]+=N;
				if(N!=0){
					for(int j=0;j<N;++j){
						Groupfile>>temp;
						cout<<temp<<' ';
						CQIgroups[k][i].push_back(temp);
						for(int l=0;l<NumRB;++l){
							RBCQIfile>>tRB;
							RBs[k][l].push_back(tRB);
						}
						UserIDfile>>tt;
						UserMatch[k].push_back(tt);
						//cout<<"tt "<<tt<<endl;
					}
				}else{
					count++;
				}
				cout<<endl;
			}
		}



		//cout<<setw(30)<<"SDs"<<'|'<<"groups"<<endl;
		for(int k=0;k<NumOfMMS;++k){
			nonZero=1;
			MMSi=k;
			cout<<"MMS "<<k<<endl;
			for(int i =1;i<=CQI;++i){
				if(CQIgroups[k][i].size()!=0){
						NonZeroCQIgroups[k][nonZero]=CQIgroups[k][i];
						nonZero++;
				}
			}
			nonZero--;

			//RandomAssignRBCQI();
			bestConfig[k] = FindProperGroupConfig();


		}
		for(int k=0;k<NumOfMMS;++k){
			cout<<"Config of MMS "<<k<<':';
			for(int i=0;i<bestConfig[k].size();++i){
				cout<<bestConfig[k][i]<<' ';
			}
			cout<<endl;
		}
		double r= ResourceAllocation(1,1);
		Switches.push_back(SwitchCount/10);
		
		//tempUBI=log10(tempUBI);
		tempUBI/=(NumOfMMS*NumUe);
		UBI.push_back(tempUBI);
		cout<<r<<endl;
		results.push_back(r);

		for(int i=0;i<NumOfMMS;++i){
			for(int j=0;j<NumRB;++j){
				RBs[i][j].clear();
			}
			for(int j=0;j<16;++j){
				NonZeroCQIgroups[i][j].clear();
			}
			UserMatch[i].clear();
		}
	}


	//each result is the utility of a frame
	double average =0.0;
	int count=0;
	double averagePerSec=0.0;
	for(int i=0;i<results.size();++i){
		//calculate the Utility Per Sec
		/*
		if(i%10==9){
			averagePerSec=averagePerSec/10;
			cout<<averagePerSec<<endl;
			averagePerSec=0.0;
		}else{
			averagePerSec=averagePerSec+results[i];
		}
		*/
		//cout<<"Time "<<i<<' '<<results[i]<<endl;
		//cout<<results[i]<<endl;
		average=average+results[i];
	}	
	average=average/times;

	double aver_fair=0.0;
	for(int i =0;i<fairness.size();++i){
		//cout<<fairness[i]<<endl;
		aver_fair=aver_fair+fairness[i];
	}
    double aveADR=0.0;
	for(int i =0;i<ADR.size();++i){
		cout<<ADR[i]<<endl;
        aveADR=aveADR+ADR[i];
	}
    aveADR=aveADR/times;
	avUse=avUse/times;

	for(int k=0;k<Switches.size();++k){
		//cout<<Switches[k]<<endl;
	}


	cout<<"Average Use RB "<<avUse<<endl;
	avUse=avUse/NumRB;
	cout<<"RB Utilization "<<avUse<<endl;
    cout<<"Average ADR "<<aveADR<<endl;
	cout<<"Average "<<average<<endl;
	aver_fair=aver_fair/times;
	cout<<"Average Fair "<<aver_fair<<endl;

	//PMF
	highest=highest/times;
	lowest=lowest/times;
	middle=middle/times;

	cout<<"Low "<<lowest<<endl;
	cout<<"Middle "<<middle<<endl;
	cout<<"High "<<highest<<endl;

	for(int i=1;i<=15;++i){
		CQIDist[i]/=(NumUe*NumOfMMS*times);
		CQIDist[i]*=100;
		//cout<<"CQI "<<i<<':';
		//cout<<CQIDist[i]<<endl;
	}
	cout<<"GroupCount "<<endl;
	//for group count
	for(int i=1;i<16;++i){
		NumOfGroupCount[i]=NumOfGroupCount[i]/(NumOfMMS*times);
		//NumOfGroupCount[i]=NumOfGroupCount[i]/times;
		//cout<<NumOfGroupCount[i]<<endl;
	}
	averGroup=averGroup/(NumOfMMS*times);
	cout<<"average group "<<averGroup<<endl;
	double aver_UBI=0;
	for(int i=0;i<UBI.size();++i){
		cout<<UBI[i]<<endl;
		if(UBI[i]>0)
			aver_UBI+=UBI[i];
	}
	cout<<"UBI "<<aver_UBI/times<<endl;

	cout<<usercount/times<<endl;


/*
	for(int i=0;i<configsize;++i){
		for(int j=0;j<SDConfigList[i].size();++j){
			cout<< GroupConfigList[i][j]<<' ';
		}
		cout<<endl;
	}
*/
	/*
	//subgrouping into three groups
	for(int i =1;i<=nonZero-2;++i){
		for(int j=1;j<=nonZero-1-i;++j){
				vector<double> temp;
				//cout<<i<<' '<<j<<' '<<8-i-j<<endl;
				for(int l=1;l<=i;++l)
					temp.insert(temp.end(),make_move_iterator(NonZeroCQIgroups[l].begin()),make_move_iterator(NonZeroCQIgroups[l].end()));
				
				SDs[i][j][0]=calSD(temp);
				
				temp.clear();
				for(int l=i+1;l<=j+i;++l)
                    temp.insert(temp.end(),make_move_iterator(NonZeroCQIgroups[l].begin()),make_move_iterator(NonZeroCQIgroups[l].end()));

                SDs[i][j][1]=calSD(temp);

				temp.clear();
				for(int l = i+j+1;l<=nonZero;++l)
					temp.insert(temp.end(),make_move_iterator(NonZeroCQIgroups[l].begin()),make_move_iterator(NonZeroCQIgroups[l].end()));

				SDs[i][j][2]=calSD(temp);


				cout <<setw(10)<< SDs[i][j][0] << setw(10) <<SDs[i][j][1] <<setw(10)<< SDs[i][j][2]<<' ';  
				cout << i<<' '<<j<<' '<<nonZero-i-j<<endl;
			
		}
	}
	*/
}
