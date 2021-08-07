using namespace std;
#include <bits/stdc++.h>


string buyBond(int id, int price, int size){
    string s = "ADD " + to_string(id)  + " BOND BUY " + to_string(price) + " " + to_string(size) + "\n";
    return s;

}

string buyBond(int id, int price, int size){
    string s = "ADD " + to_string(id)  + " BOND SELL " + to_string(price) + " " + to_string(size) + "\n";
    return s;

}
