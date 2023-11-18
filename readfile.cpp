#include <iostream>
#include <fstream>
#include <vector>

using namespace std;

int main(){
    string filename("test90large.txt");
    int NumOfUEs = 30;
    string outCQIGroup("CQIGroups90l.txt");
    string outRBCQI("RBCQIs90l.txt");
    string outUserID("UserID90l.txt");
    int NumOfMMS = 3;
    int NumOfRBs = 50;

    int times=1800;
    vector<int> RBs[NumOfMMS][NumOfRBs];
    vector<int> sortRBs[NumOfMMS][NumOfRBs];
    vector<pair<double,int> > CQIGroups[NumOfMMS][16];
    int CQI;
    vector<int> UserID;

    ifstream file(filename);
    ofstream Groupfile(outCQIGroup);
    ofstream RBCQIfile(outRBCQI);
    ofstream UserIDfile(outUserID);

    if(!file.is_open()){
        cerr<<"Could not open file"<<filename<<endl;
        return EXIT_FAILURE;
    }

    for(int l=0;l<times;++l){
        for(int i =0;i<NumOfMMS;++i){
            for(int j=0;j<NumOfUEs;++j){
                for(int k=0;k<NumOfRBs;++k){
                    file>>CQI;
                    cout<<CQI<<';';
                    RBs[i][k].push_back(CQI);
                }
                cout<<endl;
            }
                
        }

        double C =0.0;
        for(int i=0;i<NumOfMMS;++i){
            for(int j=0;j<NumOfUEs;++j){
                C=0.0;
                for(int k=0;k<NumOfRBs;++k){
                    C=C+RBs[i][k][j];
                }
                C=C/(double) NumOfRBs;
                int c =(int) C;
                pair<double, int >temp ;
                temp.first = C;
                temp.second=j;
                CQIGroups[i][c].push_back(temp);
            }
        }

        for(int i=0;i<NumOfMMS;++i){
            for(int j=1;j<=15;++j){
                Groupfile<<CQIGroups[i][j].size()<<' ';
                for(int l =0;l<CQIGroups[i][j].size();++l){
                    int index = CQIGroups[i][j][l].second;
                    for(int k=0;k<NumOfRBs;++k){
                        //sortRBs[i][k].push_back(RBs[i][k][index]);
                        cout<<RBs[i][k][index]<<' ';
                        RBCQIfile<<RBs[i][k][index]<<' ';
                    }

                    RBCQIfile<<endl;
                    Groupfile<<CQIGroups[i][j][l].first<<' ';
                    UserID.push_back(index);
                    UserIDfile<<index<<' ';
                }
                Groupfile<<endl;
                cout<<endl;
            }
            UserIDfile<<endl;
        }


        for(int i=0;i<NumOfMMS;++i){
            for(int j=0;j<NumOfRBs;++j){
                RBs[i][j].clear();
            }
            for(int j=0;j<16;++j){
                CQIGroups[i][j].clear();
            }
        }
    }

    Groupfile.close();
    RBCQIfile.close();
    UserIDfile.close();
    file.close();


    return 0;
}