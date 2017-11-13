#include <iostream>
#include <string>
#include <vector>
#include <algorithm>

using namespace std;

/**
 * Auto-generated code below aims at helping you parse
 * the standard input according to the problem statement.
 **/

const int INF = 1000000;

int visited = 0;
vector<int> leaves;
int B, D;

int minmax(int p, int q, int alpha, int beta, bool isMax)
{
    // cout << p << " " << q << " " << alpha << " " << beta << " " << isMax << "\n";
    int v;
    visited++;
    if (p == q)
        return leaves[p];
    int len = (q - p + 1) / B;
    if (isMax)
        v = -INF;
    else
        v = INF;
    for(int pp = p; pp < (q + 1); pp += len)
    {
        if (beta <= alpha)
            break;
        if (isMax)
        {
            v = max(v, minmax(pp, pp + len - 1, alpha, beta, false));
            alpha = max(alpha, v);
        }
        else
        {
            v = min(v, minmax(pp, pp + len - 1, alpha, beta, true));
            beta = min(beta, v);
        }
    }
    return v;
}
 
int main()
{
    cin >> D >> B; cin.ignore();
    int leavesNum = 1;
    for(int i = 1; i <= D; i++) 
        leavesNum *= B;
    
    leaves.resize(leavesNum);
    for(int i = 0; i < leaves.size(); i++)
        cin >> leaves[i];
    
    cout << minmax(0, leaves.size() -1, -INF, INF, true) << " ";
    cout << visited;

    // Write an action using cout. DON'T FORGET THE "<< endl"
    // To debug: cerr << "Debug messages..." << endl;
}
