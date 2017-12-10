#include <iostream>
#include <string>
#include <vector>
#include <algorithm>
#include <queue>

#define st first
#define nd second

using namespace std;

const int turnsForward = 10;

enum Owner {
    OPPONENT,
    NEUTRAL,
    ME
};

struct factoryState {
    int production;
    int units;
    Owner owner;
};

struct troopState {
    Owner owner;
    int fromFactory, toFactory;
    int units;
    int arrivingIn;
};

struct gameState {
    vector <factoryState> factories;
    vector <troopState> troops;
};

gameState predictTurn(gameState& currentGameState)
{
    int factoryCount = currentGameState.factories.size();
    gameState nextState;
    // net number of my units reaching destination
    vector <int> arrivingUnits(factoryCount, 0);
    
    // Move troops
    for (auto& troop : currentGameState.troops)
    {
        auto troop_ = troop;
        if (troop_.arrivingIn > 1)
        {
            troop_.arrivingIn--;
            nextState.troops.push_back(troop_);
        }
        else
        {
            int allyMultiplier = (int)troop_.owner - 1;
            arrivingUnits[troop_.toFactory] += allyMultiplier * troop_.units;
        }
    }

    // Produce new units
    nextState.factories = currentGameState.factories;    
    for (auto& factory : nextState.factories)
        if (factory.owner != NEUTRAL)
            factory.units += factory.production;

    // Solve battles
    for (int f = 0; f < factoryCount; f++)
    {
        if (arrivingUnits[f] > 0)
            switch(nextState.factories[f].owner)
            {
                case ME:
                    nextState.factories[f].units += arrivingUnits[f];
                    break;
                case NEUTRAL:
                case OPPONENT:
                    if (arrivingUnits[f] > nextState.factories[f].units)
                    {
                        nextState.factories[f].units = arrivingUnits[f] - nextState.factories[f].units;
                        nextState.factories[f].owner = ME;
                    }
                    else
                        nextState.factories[f].units = nextState.factories[f].units - arrivingUnits[f];
                    break;
            }
        if (arrivingUnits[f] < 0)
            switch(nextState.factories[f].owner)
            {
                case OPPONENT:
                    nextState.factories[f].units += -arrivingUnits[f];
                    break;
                case ME:
                case NEUTRAL:
                    if (arrivingUnits[f] > nextState.factories[f].units)
                    {
                        nextState.factories[f].units = -arrivingUnits[f] - nextState.factories[f].units;
                        nextState.factories[f].owner = OPPONENT;
                    }
                    else
                        nextState.factories[f].units = nextState.factories[f].units - (-arrivingUnits[f]);
                    break;
            }
    }
    return nextState;

}

vector<gameState> predict(gameState& currentGameState, int turns = 5)
{
    vector <gameState> nextStates(turns+1);
    nextStates[0] = currentGameState;
    for (int t = 1; t <= turns; t++)
        nextStates[t] = predictTurn(nextStates[t-1]);
    return nextStates;
}

int computeBestPathBetween(
    int fromFactory, 
    int toFactory, 
    vector <vector <pair <int, int> > >& links, 
    int factoryCount, 
    vector <int>& bestPath)
{
    priority_queue <pair <pair <int, int>, vector <int> > > paths;

    for (int i = 0; i < links[fromFactory].size(); i++)
    {
        int distance = links[fromFactory][i].st;
        int destination = links[fromFactory][i].nd;
        pair <int, int> value = make_pair(-distance, 2);
        vector <int> path = {fromFactory, destination};
        paths.push(make_pair(value, path));
    }
    
    while(!paths.empty())
    {
        auto best = paths.top();
        paths.pop();
        
        vector <int> path = best.nd;
        int distance = -best.st.st, directFactoryCount = best.st.nd;
        int lastFactory = path.back();
        if (lastFactory == toFactory)
        {
            bestPath = path;
            return best.st.st;
        }
        
        for (int i = 0; i < links[lastFactory].size(); i++)
        {
            int distance_ = distance + links[lastFactory][i].st;
            int destination_ = links[lastFactory][i].nd;
            pair <int, int> value = make_pair(-distance_, directFactoryCount + 1);
            vector <int> path_ = path;
            path_.push_back(destination_);
            paths.push(make_pair(value, path_));
        }

    }

}

void computeBestPaths(
    vector <vector <pair <int, vector <int> > > >& bestPaths, 
    vector <vector <pair <int, int> > >& links, 
    int factoryCount)
{
    for (int factory1 = 0; factory1 < factoryCount; factory1++)
        for (int factory2 = factory1 + 1; factory2 < factoryCount; factory2++)
        {
            vector <int> path;
            int distance = computeBestPathBetween(factory1, factory2, links, factoryCount, path);
            bestPaths[factory1][factory2] = make_pair(distance, path);
            reverse(path.begin(), path.end());      
            bestPaths[factory2][factory1] = make_pair(distance, path);           
        }
}

void computeFactoryValues(
    vector <factoryState>& factoryStates,
    vector <vector <pair <int, vector <int> > > >& bestPaths, 
    int factoryCount,
    vector <double>& factoryValue    
)
{
    vector <int> pathsThrough(factoryCount, 0);
    for (int f1 = 0; f1 < factoryCount; f1++)
        for (int f2 = f1 + 1; f2 < factoryCount; f2++)
            for (int i = 0; i < bestPaths[f1][f2].nd.size(); i++)
                pathsThrough[bestPaths[f1][f2].nd[i]]++;

    for (int f = 0; f < factoryCount; f++)
    {
        factoryValue[f] = (double)factoryStates[f].production
            + 0.01 * pow(pathsThrough[f], 0.5) + 0.1;
    }    
}

int main()
{
    int factoryCount; 
    cin >> factoryCount; cin.ignore();
    int linkCount; 
    cin >> linkCount; cin.ignore();
    
    // Prepare matrix for best paths
    vector <vector <pair <int, vector <int> > > > bestPaths;
    bestPaths.resize(factoryCount);
    for (int i = 0; i < factoryCount; i++)
        bestPaths[i].resize(factoryCount);

    vector <vector <pair <int, int> > > links;
    links.resize(factoryCount);
    for (int i = 0; i < factoryCount; i++)
        links[i].resize(factoryCount);

    for (int i = 0; i < linkCount; i++) {
        int factory1;
        int factory2;
        int distance;
        cin >> factory1 >> factory2 >> distance; cin.ignore();
        
        // May crash if factory ids are not in [0, factoryCount) - check it
        links[factory1].push_back(make_pair(factory2, distance));
        links[factory2].push_back(make_pair(factory1, distance));
    }

    computeBestPaths(bestPaths, links, factoryCount);

    vector <double> factoryValues(factoryCount);

    vector <factoryState> factories(factoryCount);
    vector <troopState> troops;
    
    gameState currentGameState;

    while (1) {
        int entityCount; // the number of entities (e.g. factories and troops)
        cin >> entityCount; cin.ignore();
        currentGameState.troops.clear();
        for (int i = 0; i < entityCount; i++) {
            int entityId;
            string entityType;
            int arg1;
            int arg2;
            int arg3;
            int arg4;
            int arg5;
            cin >> entityId >> entityType >> arg1 >> arg2 >> arg3 >> arg4 >> arg5; cin.ignore();
            if (entityType == "FACTORY")
            {
                currentGameState.factories[entityId].production = arg3;
                currentGameState.factories[entityId].units = arg2;
                currentGameState.factories[entityId].owner = (Owner)(arg1 + 1);
            }
            if (entityType == "TROOP")
            {
                troopState troop{(Owner)(arg1 + 1), arg2, arg3, arg4, arg5};
                currentGameState.troops.push_back(troop);
            }
        }
        computeFactoryValues(currentGameState.factories, bestPaths, factoryCount, factoryValues);
        

        // Any valid action, such as "WAIT" or "MOVE source destination cyborgs"
        cout << "WAIT" << endl;
    }
}