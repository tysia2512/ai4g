#include <iostream>
#include <string>
#include <vector>
#include <algorithm>
#include <math.h>
#include <memory>

using namespace std;

const int charNum = 30;

int N;
double C;

/**
 * Auto-generated code below aims at helping you parse
 * the standard input according to the problem statement.
 **/

struct node {
    vector<node*> children;
    int visits;
    double score;
    node(vector<node*> to_delete) {
        children.resize(charNum);
        to_delete.push_back(this);
    }
};
vector<node*> to_delete;

void addPlayout(string& playout, double score, node * v) {
    // cerr << "PLAYOUT: " << playout << "\n";
    int c = playout[0] - 'a';
    v->visits++;
    v->score += score;        
    
    if (v->children[c] != nullptr)
    {
        playout.erase(0, 1);
        // cerr << "NODE EXISTS, passing playout: " << playout << "\n";
        addPlayout(playout, score, v->children[c]);
    }
    else
    {
        // cerr << ('a' + c) << "\n";
        v->children[c] = new node(to_delete);
        v->children[c]->score = score;
        v->children[c]->visits = 1;
    }
}

void findBest(node * v, vector<char>& result) {
    double bestUCT = 0.0;
    int bestI = -1;

    for(int i = 0; i < v->children.size(); i++)
    {
        node * child = v->children[i];
        if(!child)
            continue;
        else
        {
            double val = child->score / (double)child->visits + C * sqrt(log((double)v->visits)/(double)child->visits);
            cerr << val << " " << bestUCT << "\n";
            if (val > bestUCT)
            {
                bestUCT = val;
                bestI = i;
            }
        }
    }
    if (bestI == -1)
        return;
    result.push_back('a' + bestI);
    findBest(v->children[bestI], result);
}

int main()
{
    cin >> N >> C; cin.ignore();
    node * root = new node(to_delete);
    for (int i = 0; i < N; i++) {
        string playout;
        double score;
        cin >> playout >> score; cin.ignore();
        addPlayout(playout, score, root);
    }
    vector<char> result;
    findBest(root, result);
    for (int i = 0; i < result.size(); i++)
        cout << result[i];
    cout << "\n";

    // Write an action using cout. DON'T FORGET THE "<< endl"
    // To debug: cerr << "Debug messages..." << endl;

    // cout << "aaaaaaaa" << endl;
}