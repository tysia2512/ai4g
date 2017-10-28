#include <iostream>
#include <string>
#include <vector>
#include <algorithm>
#include <sstream>
#include <queue>
#include <iterator>
#include <array>

#define st first
#define nd second

using namespace std;
const int INF = 100000;
const int maxW = 9, maxH = 9, directions = 4, orientations = 2, players = 2;

enum moveType {
    MOVE = 0,
    WALL =1
};

enum moveDirection {
    UP = 0,
    DOWN = 1,
    LEFT = 2,
    RIGHT = 3
};

string directionToString(moveDirection d)
{
    switch(d)
    {
        case UP:
            return "UP";
        case DOWN:
            return "DOWN";
        case LEFT:
            return "LEFT";
        case RIGHT:
            return "RIGHT";
    }
    return "WRONG DIRECTION";
}

enum wallOrientation {
    VERTICAL = 0,
    HORIZONTAL = 1
};

string wallToString(int x, int y, wallOrientation o)
{
    stringstream s;
    s << x << " " << y << " ";
    switch(o)
    {
        case VERTICAL:
            s << "V";
            break;
        case HORIZONTAL:
            s << "H";
    }
    return s.str();
}

int w; // width of the board
int h; // height of the board
int playerCount; // number of players (2 or 3)
int myId; // id of my player (0 = 1st player, 1 = 2nd player, ...)
moveDirection targets[] = {RIGHT, LEFT, DOWN, UP};
int myWallsLeft;

pair <int, int> positions[players];
array<array<array<bool, directions>, maxH>, maxW> possibleMoves;
array<array<array<bool, directions>, maxH>, maxW> possibleWalls;

void initGrid()
{
    for (int x = 0; x < w; x++)
        for (int y = 0; y < h; y++)
            for (int d = 0; d < directions; d++)
                possibleMoves[x][y][d] = true;
    
    for (int x = 0; x < w; x++)
        for (int y = 0; y < h; y++)
            for(int o = 0; o < orientations; o++)
                possibleWalls[x][y][o] = true;

    for (int x = 0; x < w; x++)
    {
        possibleMoves[x][0][UP] = false;
        possibleMoves[x][h-1][DOWN] = false;
    }
    for (int y = 0; y < h; y++)
    {
        possibleMoves[0][y][LEFT] = false;
        possibleMoves[w-1][y][RIGHT] = false;
    }

    for (int x = 1; x < w; x++)
        for (int y = 0; y < h - 1; y++)
            possibleWalls[x][y][VERTICAL] = true;
    
    for (int x = 0; x < w - 1; x++)
        for (int y = 1; y < h; y++)
            possibleWalls[x][y][HORIZONTAL] = true;
}

void markWall(int x, int y, wallOrientation o)
{
    possibleWalls[x][y][o] = false;
    if (o == HORIZONTAL)
    {
        if (y > 0)
        {
            possibleWalls[x + 1][y - 1][VERTICAL] = false;
            possibleMoves[x][y - 1][DOWN] = false;
            possibleMoves[x + 1][y - 1][DOWN] = false;
        }
        possibleMoves[x][y][UP] = false;
        possibleMoves[x + 1][y][UP] = false;        
    }
    else
    {
        if (x > 0)
        {
            possibleWalls[x - 1][y + 1][HORIZONTAL] = false;
            possibleMoves[x - 1][y][RIGHT] = false;
            possibleMoves[x - 1][y + 1][RIGHT] = false;
        }
        possibleMoves[x][y][LEFT] = false;
        possibleMoves[x][y + 1][LEFT] = false;
    }
}

bool directionIsPossible(moveDirection d, int x, int y)
{
    if (x == 0)
        cerr << "IS POSSIBLE: " << x << " " << y << " " << directionToString(d) << " " << int(possibleMoves[x][y][d]) << endl;
    return possibleMoves[x][y][d];
}

pair<int, int> makeMove(int x, int y, moveDirection d)
{
    switch (d)
    {
        case RIGHT:
            return make_pair(x + 1, y);
        case LEFT:
            return make_pair(x - 1, y);
        case UP:
            return make_pair(x, y - 1);
        case DOWN:
            return make_pair(x, y + 1);
    }
    return make_pair(-1, -1);
}

int dist(int X, int Y, moveDirection t)
{
    if (X == -1 && Y == -1)
        return -1;
    bool visited[w][h];
    for (int x = 0; x < w; x++)
        for(int y = 0; y < h; y++)
            visited[x][y] = false;

    queue <pair <int, pair <int, int> > > q;
    
    // Add starting points
    switch(t)
    {
        case RIGHT:
            for (int y = 0; y < h; y++)
            {
                q.push(make_pair(0, make_pair(w - 1, y)));
                visited[w - 1][y] = true;
            }
            break;
        case LEFT:
            for (int y = 0; y < h; y++)
            {
                q.push(make_pair(0, make_pair(0, y)));
                visited[0][y];
            }
            break;
        case UP:
            for (int x = 0; x < w; x++)
            {
                q.push(make_pair(0, make_pair(x, 0)));
                visited[x][h - 1] = true;
            }
            break;
        case DOWN:
            for (int x = 0; x < w; x++)
            {
                q.push(make_pair(0, make_pair(x, h - 1)));
                visited[x][0];
            }
            break;
    }

    // Run BFS
    while (!q.empty())
    {
        int dist = q.front().st;
        int x = q.front().nd.st;
        int y = q.front().nd.nd;
        q.pop();
        if (x == X && y == Y)
            return dist;

        for (int d = 0; d < directions; d++)
        {
            if (possibleMoves[x][y][d])
            {
                pair<int, int> newCoor = makeMove(x, y, (moveDirection)d);
                if (visited[newCoor.st][newCoor.nd])
                    continue;
                q.push(make_pair(dist + 1, newCoor));
                visited[newCoor.st][newCoor.nd] = true;
            }
        }
    }
    return -1;
}

pair<int, string> bestMove()
{
    int bestOppDist = INF;
    for (int id = 0; id < players; id++)
    {
        if (id == myId)
            continue;
        int mod = 0;
        if (id < myId)
            mod = -1;
        bestOppDist = min(bestOppDist, dist(positions[id].st, positions[id].nd, targets[id]) + mod);
    }
    moveDirection bestDir;
    int bestDist = INF;
    int x = positions[myId].st, y = positions[myId].nd;
    for (int d = 0; d < directions; d++)
    {
        if (!directionIsPossible((moveDirection)d, x, y))
            continue;
        pair<int, int> newCoor = makeMove(x, y, (moveDirection)d);
        int newDist = dist(newCoor.st, newCoor.nd, targets[myId]);
        if (newDist < bestDist)
        {
            bestDir = (moveDirection)d;
            bestDist = newDist;
        }
    }
    return make_pair(bestOppDist - bestDist, directionToString(bestDir));
}

pair<int, string> bestWall()
{
    array<array<array<bool, directions>, maxH>, maxW> pmC;
    array<array<array<bool, directions>, maxH>, maxW> pwC;
    copy(begin(possibleWalls), end(possibleWalls), begin(pwC));    
    copy(begin(possibleMoves), end(possibleMoves), begin(pmC));    

    string bestWall = "";
    int bestEval = -INF;

    for (int x = 0; x < w; x++)
        for (int y = 0; y < h; y++)
            for (int o = 0; o < orientations; o++)
            {
                if (!possibleWalls[x][y][o])
                    continue;
                // cerr << "TRY WALL: " << wallToString(x, y, (wallOrientation)o) << endl;
                    
                markWall(x, y, (wallOrientation)o);
                int bestOppDist = INF;
                for (int id = 0; id < players; id++)
                {
                    if (id == myId)
                        continue;
                    int mod = 0;
                    if (id < myId)
                        mod = -1;
                    bestOppDist = min(bestOppDist, dist(positions[id].st, positions[id].nd, targets[id]) + mod);
                }
                // cerr << "bestOppDist: " << bestOppDist << "\n";
                // cerr << "myStatus: " << positions[myId].st << " " << positions[myId].nd << " " << directionToString(targets[myId]) << endl;
                int newDist = dist(positions[myId].st, positions[myId].nd, targets[myId]);
                // cerr << "myDist: " << newDist << "\n";
                int eval = bestOppDist - newDist;
                if (eval > bestEval)
                {
                    bestEval = eval;
                    bestWall = wallToString(x, y, (wallOrientation)o);
                }
                // cerr << "current best wall: " << bestWall << "\n";

                // Get positions to original state
                copy(begin(pwC), end(pwC), begin(possibleWalls));    
                copy(begin(pmC), end(pmC), begin(possibleMoves));  
            }
    return make_pair(bestEval, bestWall);
}

int main()
{

    cin >> w >> h >> playerCount >> myId; cin.ignore();

    // game loop
    while (true) 
    {
        initGrid();
        for (int i = 0; i < playerCount; i++) 
        {
            int x; // x-coordinate of the player
            int y; // y-coordinate of the player
            int wallsLeft; // number of walls available for the player
            cin >> x >> y >> wallsLeft; cin.ignore();
            positions[i] = make_pair(x, y);
            if (i == myId)
                myWallsLeft = wallsLeft;
        }

        int wallCount; // number of walls on the board
        cin >> wallCount; cin.ignore();
        for (int i = 0; i < wallCount; i++) 
        {
            int wallX; // x-coordinate of the wall
            int wallY; // y-coordinate of the wall
            string wallO; // wall orientation ('H' or 'V')
            cin >> wallX >> wallY >> wallO; cin.ignore();
            wallOrientation wO;
            if (wallO == "H")
                wO = HORIZONTAL;
            else
                wO = VERTICAL;
            markWall(wallX, wallY, wO);
        }

        cerr << "MY POSITION: " << positions[myId].st << " " << positions[myId].nd << endl;
        if(positions[myId].st == -1 && positions[myId].nd == -1)
            continue;
        auto move = bestMove();
        cerr << "BEST MOVE: " << move.st << " " << move.nd << endl;
        
        auto wall = bestWall();
        cerr << "BEST WALL: " << wall.st << " " << wall.nd << endl;
        
        cerr << "RUN" << endl;
        if (move.st >= wall.st || myWallsLeft == 0)
            cout << move.nd << endl;
        else 
            cout << wall.nd << endl;
    }
}