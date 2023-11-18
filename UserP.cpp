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
int CQI = 15;
int NumUe =30;
double M=NumUe*3,N=0;
string inCQIGroup("CQIGroups90vc.txt");
string inRBCQI("RBCQIs90vc.txt");
string inUserID("UserID90vc.txt");
int QualityLevel=5;
double testBitrate[5];//NumOfQualityLevel
vector<int> RBs[3][50];//NumOfRBs
int NumRB = 50;
vector<double> NonZeroCQIgroups[3][16];//NumOfMMS
int NumOfMMS=3;
vector<int> bestConfig[3];//NumOFMMS
int times=100;
int CQIBit[16]={0,16,16,32,56,88,120,144,208,280,328,408,488,552,616,616};
int videoBitrate[5] = {375,750,1750,3000,3850};
double NumOfSlots = 12;
int T =100;
int GroupL=9;
int bestGroup=0;
double averbestGroup=0;
double DistGroup[16];

double bestthroughput[3]={0,0,0};
double bestthroughputK[3]={0,0,0};
double throughputpow[3]={0,0,0};

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
vector<int> bestCand;
vector<int> UserMatch[3];
double preBitrate[3][500];
double curBitrate[3][500];
int SwitchCount=0;
vector<int> Switches;
bool first=true;
vector<double> UBI;
double tempUBI;

double throughputpower=0.0;
vector<double> fairness;
vector<double> ADR;

double prevhigh=0,prevmiddle=0,prevlow=0;
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

void ResourceAllocation(int GroupConfigIndex){
    vector<int> GConfig = GroupConfigList[GroupConfigIndex]; 
    int NumOfGroups=GConfig.size();
    vector<double> Group[NumOfGroups];

    int start =1;
		for(int i =0;i<NumOfGroups;++i){
			for(int j = start;j<start+GConfig[i];++j){
				Group[i].insert(Group[i].end(),make_move_iterator(NonZeroCQIgroups[MMSi][j].begin()),make_move_iterator(NonZeroCQIgroups[MMSi][j].end()));
			}
			//print Groups
			/**
			for(int j=0;j<Group[i].size();++j)
				cout<<Group[i][j]<<' ';
			cout<<endl;
			**/
			start+=GConfig[i];
		}

    int RBUEIndex=0;
    int MinCQI[NumOfGroups];
	double diviRB = NumRB/NumOfMMS;
	int allocateIndex= diviRB * MMSi;
    double RA[NumOfGroups];
    double throughputK=0;
	double logthroughput=0;
	double fair=0.0;
	throughputpower=0.0;
	double tempADR=0.0;
	int UE = NumOfMMS*NumUe;
	double temphighest=0, tempmiddle=0,templowest=0;
	tempUBI=0;
	double tempGroupBitrate[NumOfGroups];
	double ttempUBI=0;
    for(int i=0;i<NumOfGroups;++i){
        double NumRA = Group[i].size()*NumRB/(M+N);
        RA[i]=NumRA;
	//		cout<<"Group "<<i<<endl;
			for(int j=allocateIndex;j<allocateIndex+NumRA;++j){
				int tempCQI=100;
				for(int k=RBUEIndex;k<RBUEIndex+Group[i].size();++k){
	//				cout<<RBs[j][k]<<' ';
					if(tempCQI>RBs[MMSi][j][k])
						tempCQI=RBs[MMSi][j][k];
				}
	//			cout<<endl;
				MinCQI[i] = tempCQI;
	//			cout<<tempCQI<<' ';

			}
			double temp = NumRA*CQIBit[MinCQI[i]];
			tempGroupBitrate[i]=temp;
            throughputK=throughputK+NumRA*CQIBit[MinCQI[i]];
			fair = throughputK;
			throughputpower = throughputpower+pow(NumRA*CQIBit[MinCQI[i]]*Group[i].size(),2.0);
			logthroughput=logthroughput+log10(NumRA*CQIBit[MinCQI[i]]);
	//		cout<<endl;
			RBUEIndex+=Group[i].size();

			if(temp<400){
				templowest=templowest+Group[i].size();
			}else if(temp>1750){
				temphighest=temphighest+Group[i].size();
			}else{
				tempmiddle=tempmiddle+Group[i].size();
			}
		//For UBI
		int worstCQI=0;
		for(int l=0;l<Group[i].size();++l){
			if(l==0)worstCQI=Group[i][l];
			int bestCQI=Group[i][l];
			ttempUBI+=(CQIBit[bestCQI]-CQIBit[worstCQI])*RA[i]*10;
		}
    }
	if(logthroughput>(bestthroughput[MMSi]+0.5)){
		bestthroughput[MMSi]=logthroughput;
		throughputpow[MMSi]=throughputpower;
		bestthroughputK[MMSi]=throughputK;

		lowest=lowest-prevlow+templowest;
		middle=middle-prevmiddle+tempmiddle;
		highest=highest-prevhigh+temphighest;
		bestGroup=NumOfGroups;
		prevlow=templowest;
		prevmiddle=tempmiddle;
		prevhigh=temphighest;
		int UserIndex=0;
		for(int i=0;i<NumOfGroups;++i){
			for(int j=0;j<Group[i].size();++j){
				int t = UserMatch[MMSi][UserIndex];
				curBitrate[MMSi][t]=tempGroupBitrate[i];
				UserIndex++;
			}
		}
	tempUBI=ttempUBI;


	}
}


//在push_back GroupConfig的地方算simple resource allocation
void SplittingGroup(int Num, int begin, int end){
	if (Num == 1){
		GroupConfig.push_back(end-begin+1);
		return;
	}
	else{
		for(int i=1; i<end-begin-Num+3;++i){
			SplittingGroup(1, begin, begin+i-1);
			SplittingGroup(Num-1, begin+i, end);
			if(Num==2){
				GroupConfigList[configsize]=GroupConfig;
				ResourceAllocation(configsize);
				configsize++;
				GroupConfig.pop_back();
			}
			GroupConfig.pop_back();
		}
		
	}

}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////




int main(){
	double SDs[10][10][5];
	testBitrate[0]=10;


	ifstream Groupfile(inCQIGroup);
    ifstream RBCQIfile(inRBCQI);
	ifstream UserIDfile(inUserID);
	for(int i=0;i<16;++i){
		DistGroup[i]=0;
	}
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
						CQIgroups[k][i].push_back(temp);
						for(int l=0;l<NumRB;++l){
							RBCQIfile>>tRB;
							RBs[k][l].push_back(tRB);
						}
						UserIDfile>>tt;
						UserMatch[k].push_back(tt);
					}
				}
				cout<<endl;
			}
				
		}


		//cout<<setw(30)<<"SDs"<<'|'<<"groups"<<endl;
		for(int k=0;k<NumOfMMS;++k){
			prevhigh=0;
			prevmiddle=0;
			prevlow=0;
			nonZero=1;
			MMSi=k;
			configsize=0;
			for(int i=0;i<4000;++i){
				GroupConfigList[i].clear();
				SDConfigList[i].clear();
				CandidateSD[i].clear();
			}
			cout<<"MMS "<<k<<endl;
			for(int i =1;i<=CQI;++i){
				if(CQIgroups[k][i].size()!=0){
						NonZeroCQIgroups[k][nonZero]=CQIgroups[k][i];
						nonZero++;
				}
			}
			nonZero--;
			for(int i=2;i<(nonZero-2);++i){
			//for(int i=2;i<7;++i){
				SplittingGroup(i,1,nonZero);

			}
			averbestGroup+=bestGroup;
			DistGroup[bestGroup]++;
			//for count quality switching
			for(int i=0;i<NumUe;++i){
				if(!first){
						int ti = UserMatch[MMSi][i];
						int mi=abs((int) preBitrate[MMSi][ti] - (int) curBitrate[MMSi][ti]);

						if(mi>1){
							SwitchCount++;
							//cout<<"User "<<t<<endl;
						}
						preBitrate[MMSi][ti]=curBitrate[MMSi][ti];
				}else{
						int ti = UserMatch[MMSi][i];
						preBitrate[MMSi][ti]=curBitrate[MMSi][ti];
				}
				
			}
			//RandomAssignRBCQI();
			//bestConfig[k] = FindProperGroupConfig();


		}
		double result=0.0;
		//for ADR
		double tempADR=0.0;


		int UE = NumOfMMS*NumUe;
		for(int k=0;k<NumOfMMS;++k){
			result=result+bestthroughput[k];
		}
		tempADR=result/(double) UE;
		result=result/NumRB;
		results.push_back(result);
		ADR.push_back(tempADR);

		for(int i=0;i<NumOfMMS;++i){
			for(int j=0;j<NumRB;++j){
				RBs[i][j].clear();
			}
			bestthroughput[i]=0;
			for(int j=0;j<16;++j){
				NonZeroCQIgroups[i][j].clear();
			}
			UserMatch[i].clear();
		}
		//for fairness
		double fair=0.0;
		throughputpower=0.0;

		//for fairness
		for (int i=0;i<NumOfMMS;++i){
			fair=fair+bestthroughputK[i];
			throughputpower=throughputpower+throughputpow[i];
		}
		fair=pow(fair,2.0);
		fair=fair/(throughputpower);
		fairness.push_back(fair);

		Switches.push_back(SwitchCount/10);

		//tempUBI=log(tempUBI);
		tempUBI/=(NumOfMMS*NumUe);
		UBI.push_back(tempUBI);

	}
	double average =0.0;
	for(int i=0;i<results.size();++i){
		//cout<<"Time "<<i<<' '<<results[i]<<endl;
		if(results[i]>1 || results[i]<0){
			//cout<<"0.5"<<endl;
			average=average+0.5;
		}else{
			//cout<<results[i]<<endl;
			average=average+results[i];
		}
	}
	average=average/times;

	//print fairness
    double aver_fair=0.0;
	for(int i =0;i<fairness.size();++i){
		//cout<<fairness[i]<<endl;
		aver_fair=aver_fair+fairness[i];
	}
	aver_fair=aver_fair/times;
	cout<<"Average Fair "<<aver_fair<<endl;
    double aveADR=0.0;
	for(int i =0;i<ADR.size();++i){
		//cout<<ADR[i]<<endl;
        aveADR=aveADR+ADR[i];
	}
    aveADR=aveADR/times;


	//Switches
	for(int k=0;k<Switches.size();++k){
		//cout<<Switches[k]<<endl;
	}


	//PMF
	highest=highest/(NumUe*NumOfMMS);
	lowest=lowest/(NumUe*NumOfMMS);
	middle=middle/(NumUe*NumOfMMS);

	highest=highest/times;
	lowest=lowest/times;
	middle=middle/times;

    cout<<"Average ADR "<<aveADR<<endl;
	cout<<"Average "<<average<<endl;

	cout<<"Low "<<lowest<<endl;
	cout<<"Middle "<<middle<<endl;
	cout<<"High "<<highest<<endl;

	cout<<"average group "<<averbestGroup/(times*NumOfMMS)<<endl;

	double aver_UBI=0;
	for(int i=0;i<UBI.size();++i){
		cout<<UBI[i]<<endl;
		if(UBI[i]>0)
			aver_UBI+=UBI[i];
	}
	cout<<"UBI "<<aver_UBI/times<<endl;

	for(int i=1;i<16;++i){
		double d= DistGroup[i]/(times*NumOfMMS);
		cout<<d<<endl;
	}
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
