#include <iostream>
#include <string>
#include <vector>
#include <algorithm>
#include <queue>
#include <math.h>

#define st first
#define nd second

using namespace std;

const int turnsForward = 10, INF = 1000000000;

enum Owner {
    OPPONENT,
    NEUTRAL,
    ME
};

enum League {
    WOODEN
};

const League league = WOODEN;

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
        int distance = links[fromFactory][i].nd;
        int destination = links[fromFactory][i].st;
        if (destination == fromFactory)
            continue;
        pair <int, int> value = make_pair(-distance, 2);
        vector <int> path = {fromFactory, destination};
        paths.push(make_pair(value, path));
    }
    vector <int> visited(factoryCount, INF);
    
    while(!paths.empty())
    {
        auto best = paths.top();
        paths.pop();
        
        vector <int> path = best.nd;

        int distance = -best.st.st, directFactoryCount = best.st.nd;
        int lastFactory = path.back();
        visited[lastFactory] = min(distance, visited[lastFactory]);
        if (lastFactory == toFactory)
        {
            bestPath = path;
            return distance;
        }
        
        for (int i = 0; i < links[lastFactory].size(); i++)
        {
            int distance_ = distance + links[lastFactory][i].nd;
            int destination_ = links[lastFactory][i].st;
            if (distance_ > visited[destination_] || destination_ == lastFactory)
                continue;
            visited[destination_] = min(visited[destination_], distance_);
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
            // cerr << "FOUND BEST PATH BETWEEN " << factory1 << " AND " << factory2 
            //     << " OF LENGHT " << distance << "\n";       
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

vector <int> computeSpareUnits(gameState& currentState, int factoryCount, int turns=20)
{
    vector <int> spareUnits(factoryCount, INF);
    vector <bool> toDefend(factoryCount, false);
    
    for (int f = 0; f < factoryCount; f++)
        if (currentState.factories[f].owner == ME)
            toDefend[f] = true;

    for (int turn = 1; turn <= turns; turn++)
    {
        gameState defendedState = predictTurn(currentState);    
        for (int f = 0; f < factoryCount; f++)
        {
            spareUnits[f] = min(spareUnits[f], defendedState.factories[f].units);
            if (defendedState.factories[f].owner == OPPONENT && toDefend[f])
            {
                if (spareUnits[f] < 0)
                    spareUnits[f] -= defendedState.factories[f].units;
                else
                    spareUnits[f] = -defendedState.factories[f].units;

                defendedState.factories[f].owner = ME;
                defendedState.factories[f].units = 0;
            }
        }
    }
    return spareUnits;
}

vector <vector <pair <double, int> > > scoreMoves(
    vector <gameState>& nextStates, 
    vector <int>& spareUnits, 
    int factoryCount,
    vector <vector <pair <int, vector <int> > > >& bestPaths
)
{
    vector <vector <double> > factoryValues(nextStates.size());
    for (int t = 0; t < nextStates.size(); t++)
    {
        factoryValues[t].resize(factoryCount);
        computeFactoryValues(nextStates[t].factories, bestPaths, factoryCount, factoryValues[t]);
    }

    vector <vector <pair <double, int> > > scores(factoryCount);
    for (auto& i : scores)
        i.resize(factoryCount);

    for (int fromFactory = 0; fromFactory < factoryCount; fromFactory++)
        for(int toFactory = 0; toFactory < factoryCount; toFactory++)
        {
            if (fromFactory == toFactory 
            || spareUnits[fromFactory] == 0 
            || nextStates[0].factories[fromFactory].owner != ME)
            {
                scores[fromFactory][toFactory].st = -(double)INF;
                continue;
            }
            int turnsAway = bestPaths[fromFactory][toFactory].st;
            // If you cannot predict what will be the state of the game, skip
            if (turnsAway >= nextStates.size())
            {
                scores[fromFactory][toFactory].st = -(double)INF;
                continue;
            }
            double score;
            int unitsNeeded;
            switch(nextStates[turnsAway].factories[toFactory].owner)
            {
                case NEUTRAL:
                    score = factoryValues[turnsAway][toFactory] 
                        / (pow((double)turnsAway, 2) * (double)nextStates[turnsAway].factories[toFactory].units);
                    unitsNeeded = nextStates[turnsAway].factories[toFactory].units;
                case OPPONENT:
                    score = factoryValues[turnsAway][toFactory]
                        / (pow((double)turnsAway, 2) * 8);
                    unitsNeeded = nextStates[turnsAway].factories[toFactory].units;
                case ME:
                    score = factoryValues[turnsAway][toFactory]
                        / (pow((double)turnsAway, 2) * (-(double)spareUnits[toFactory]));
                    unitsNeeded = -spareUnits[toFactory];
            }
            scores[fromFactory][toFactory].st = score;
            scores[fromFactory][toFactory].nd = unitsNeeded;
        }
    return scores;
}

void chooseMoves(
    vector <vector <pair <double, int> > >& scores, 
    vector <int>& spareUnits, 
    gameState& currentState
)
{
    int factoryCount = scores.size();
    switch(league)
    {
        case WOODEN:
        {
            pair <pair <int, int>, int> bestMove;
            double bestScore = -INF;
            for (int f = 0; f < factoryCount; f++)
            {
                if (currentState.factories[f].owner != ME)
                    continue;
                for (int toF = 0; toF < factoryCount; toF++)
                {
                    cerr << "MOVE FROM " << f << " TO " << toF 
                        << " HAS SCORE " << scores[f][toF].st 
                        << " AND REQUIRES " << scores[f][toF].nd << " TROOPS\n";
                    if (scores[f][toF].st > bestScore)
                    {
                        bestScore = scores[f][toF].st;
                        bestMove = make_pair(make_pair(f, toF), scores[f][toF].nd);
                    }
                }
            }
            if (bestScore < 0.0)
                cout << "WAIT\n";
            else
                cout << "MOVE " << bestMove.st.st << " " << bestMove.st.nd << bestMove.nd << "\n";
        }
        default:
            cout << "WAIT\n";
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
        // cerr << "READ LINK:\n";
        // cerr << factory1 << " " << factory2 << " " << distance << "\n";

        
        // May crash if factory ids are not in [0, factoryCount) - check it
        links[factory1].push_back(make_pair(factory2, distance));
        links[factory2].push_back(make_pair(factory1, distance));
    }
    cerr << "FIRST INPUT BATCH READ\n";

    computeBestPaths(bestPaths, links, factoryCount);
    cerr << "BEST PATHS COMPUTED\n";
    int turnsAhead = 0;
    for (int f = 0; f < factoryCount; f++)
        for (int f1 = 0; f1 < factoryCount; f1++)
            turnsAhead = max(turnsAhead, bestPaths[f][f1].st);
    turnsAhead = min(turnsAhead, 20);
    cerr << "TURNS AHEAD: " << turnsAhead << "\n";

    vector <double> factoryValues(factoryCount);

    vector <factoryState> factories(factoryCount);
    vector <troopState> troops;
    
    gameState currentGameState;

    while (1) {
        // cerr << "START TURN\n";
        int entityCount; // the number of entities (e.g. factories and troops)
        cin >> entityCount; cin.ignore();
        currentGameState.troops.clear();
        currentGameState.factories.resize(factoryCount);
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
                // cerr << "READ ENTITY DATA OF FACTORY WITH ID: " << entityId << "\n";
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
        cerr << "TURN DATA READ\n";
        computeFactoryValues(currentGameState.factories, bestPaths, factoryCount, factoryValues);
        auto nextStates = predict(currentGameState, turnsAhead);
        auto spareUnits = computeSpareUnits(currentGameState, factoryCount, turnsAhead);
        vector <vector <pair <double, int> > > moveScores = scoreMoves(nextStates, spareUnits, factoryCount, bestPaths);

        chooseMoves(moveScores, spareUnits, currentGameState);
    }
}