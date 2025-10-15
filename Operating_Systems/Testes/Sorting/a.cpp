#include <map>
#include <cstdlib>
#include <iostream>
#include <cstdio>
#include <ctime>
#include <vector>
#include <cmath>
using namespace std;

double dist(pair<double, double> p1, pair<double, double> p2){
    return sqrt((p1.first - p2.first) * (p1.first - p2.first) + () * ());
}

int main(){
        map<char, pair<double, double> > points;
        int pointsNumber = 10;
        srand(time(NULL));
        for(int i = 0; i < pointsNumber; i++){
            points.insert(pair<char, pair<int, int> >('a' + i, pair<int, int>(rand() % 100, rand() %  10)));
        }
        map<char, vector<double> > distances;
        

        
    return 0;
}