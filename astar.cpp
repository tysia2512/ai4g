#include <iostream>
#include <string>
#include <vector>
#include <algorithm>
#include <queue>

using namespace std;

/**
 * Auto-generated code below aims at helping you parse
 * the standard input according to the problem statement.
 **/

vector<int> hValues;
vector<vector<pair<int, int>>> edges(0);
vector<int> gValues;
vector<int> from;

int main()
{
    int N;
    int E;
    int S;
    int G;
    cin >> N >> E >> S >> G; cin.ignore();
    edges.resize(E);
    gValues.resize(N);
    hValues.resize(N);
    from.resize(N);
    for (int i = 0; i < N; i++) {
        int node;
        cin >> node; cin.ignore();
        hValues[i] = node;
        gValues[i] = -1;
    }
    for (int i = 0; i < E; i++) {
        int x;
        int y;
        int c;
        cin >> x >> y >> c; cin.ignore();
        edges[x].push_back(make_pair(y, c));
        edges[y].push_back(make_pair(x, c));        
    }
    priority_queue<pair<int, int>> pq;
    pq.push(make_pair(-hValues[S], -S));
    gValues[S] = 0;
    vector<bool> visited(E, false);
    
    while(!pq.empty())
    {
        int v = -pq.top().second;     
        int f = -pq.top().first;
        pq.pop();
                
        if (visited[v])
            continue;
        
        cout << v << " " << f << endl;
        if (v == G)
            break;

        int g = gValues[v];

        visited[v] = true;
        for (int i = 0; i < edges[v].size(); i++)
        {
            int u = edges[v][i].first;
            int newG = g + edges[v][i].second;
            if ((gValues[u] == -1 || gValues[u] > newG) && !visited[u])
            {
                gValues[u] = newG;
                //cout << "Adding node: " << u << " with F: " << (newG + hValues[u]) << endl;
                pq.push(make_pair(-(newG + hValues[u]), -u));
                //cout << "From for u: " << u << "set to v: " << v << endl; 
                from[u] = v;
            }
        }
    }
    /*vector<pair<int, int>> ans;
    int v = G;
    ans.push_back(make_pair(v, hValues[v] + gValues[v]));
    while(v != S)
    {
        v = from[v];
        ans.push_back(make_pair(v, hValues[v] + gValues[v]));
    }

    // Write an action using cout. DON'T FORGET THE "<< endl"
    // To debug: cerr << "Debug messages..." << endl;
    for (int i = ans.size() - 1; i >= 0; i--)
        cout << ans[i].first << " " << ans[i].second << endl;*/
    return 0;
}
