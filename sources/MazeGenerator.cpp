// naya file maze part ko lagi
#include "PathAlgorithm.h"
#include "QtConcurrent/qtconcurrentrun.h"
#include <iostream>
#include <queue>
#include <map>
#include <unistd.h>
#include <chrono>
#include <thread>
#include <QtConcurrent>
#include <QFuture>
#include <stack>
#include <list>
#include <QDebug> // Include for qDebug()
#include <QThread> // Include for QThread::currentThreadId()

// Helper function to add frontier cells (unvisited neighbors 2 steps away)
void PathAlgorithm::addFrontierCells(Node* node, std::vector<Node*>& frontier) {
    const int offsets[4][2] = {{2, 0}, {-2, 0}, {0, 2}, {0, -2}}; // E, W, N, S

    for (const auto& offset : offsets) {
        int newX = node->xCoord + offset[0];
        int newY = node->yCoord + offset[1];

        if (newX >= 1 && newX <= widthGrid && newY >= 1 && newY <= heightGrid) {
            int index = coordToIndex(newX, newY, widthGrid);
            Node* neighbor = &(gridNodes.Nodes[index]);

            if (neighbor->obstacle && !neighbor->visited) {
                neighbor->visited = true; // Mark as frontier
                frontier.push_back(neighbor);
            }
        }
    }
}

// Helper function to get maze neighbors (visited cells 2 steps away)
std::vector<Node*> PathAlgorithm::getMazeNeighbors(Node* node) {
    std::vector<Node*> mazeNeighbors;
    const int offsets[4][2] = {{2, 0}, {-2, 0}, {0, 2}, {0, -2}}; // E, W, N, S

    for (const auto& offset : offsets) {
        int newX = node->xCoord + offset[0];
        int newY = node->yCoord + offset[1];

        if (newX >= 1 && newX <= widthGrid && newY >= 1 && newY <= heightGrid) {
            int index = coordToIndex(newX, newY, widthGrid);
            Node* neighbor = &(gridNodes.Nodes[index]);

            if (!neighbor->obstacle) { // Part of the maze
                mazeNeighbors.push_back(neighbor);
            }
        }
    }

    return mazeNeighbors;
}

// Helper function to connect two nodes by clearing the path between them
void PathAlgorithm::connectNodes(Node* a, Node* b) {
    // Calculate midpoint
    int midX = (a->xCoord + b->xCoord) / 2;
    int midY = (a->yCoord + b->yCoord) / 2;

    // Clear both nodes and the path between them
    a->obstacle = false;
    b->obstacle = false;
    int midIndex = coordToIndex(midX, midY, widthGrid);
    gridNodes.Nodes[midIndex].obstacle = false;

    // Emit signals for visualization
    emit updatedScatterGridView(OBSTACLETOFREE, coordToIndex(a->xCoord, a->yCoord, widthGrid));
    emit updatedScatterGridView(OBSTACLETOFREE, midIndex);
    emit updatedScatterGridView(OBSTACLETOFREE, coordToIndex(b->xCoord, b->yCoord, widthGrid));
}

//maze generation using kruskal algorithm
void PathAlgorithm::performKruskalsMazeAlgorithm(QPromise<int>& promise)
{
    qDebug() << "Maze (Kruskal's): Algorithm started in worker thread:" << QThread::currentThreadId();

    // Cancel check
    promise.suspendIfRequested();
    if (promise.isCanceled()) {
        emit pathfindingSearchCompleted(0, 0);
        return;
    }

    // Step 1: Initialize all nodes as obstacles except start/end
    for (int index = 0; index < widthGrid * heightGrid; ++index) {
        promise.suspendIfRequested();
        if (promise.isCanceled()) {
            emit pathfindingSearchCompleted(0, 0);
            return;
        }

        if (index != gridNodes.startIndex && index != gridNodes.endIndex) {
            gridNodes.Nodes[index].obstacle = true;
            emit updatedScatterGridView(FREETOOBSTACLE, index);
        }
    }

    // Step 2: Initialize Disjoint Set Union (DSU)
    std::vector<int> parent(widthGrid * heightGrid);
    std::vector<int> rank(widthGrid * heightGrid, 0);

    for (int i = 0; i < widthGrid * heightGrid; ++i)
        parent[i] = i; // every cell initially its own parent




    const std::vector<QPair<int, int>> directions = {{2, 0}, {0, 2}}; // right and down
    std::vector<Wall> walls;
    for (int y = 1; y <=heightGrid ; y += 2) {
        for (int x = 1; x <= widthGrid; x += 2) {
            int cellIndex = coordToIndex(x, y, widthGrid);
            for (auto dir : directions) {
                int nx = x + dir.first;
                int ny = y + dir.second;
                if (nx <=widthGrid && ny <=heightGrid) {
                    int neighborIndex = coordToIndex(nx, ny, widthGrid);
                    int wallX = x + dir.first / 2;
                    int wallY = y + dir.second / 2;
                    int wallIndex = coordToIndex(wallX, wallY, widthGrid);
                    walls.push_back({wallIndex, cellIndex, neighborIndex});
                }
            }
        }
    }

    //Shuffle walls
    std::random_device rd;
    std::mt19937 g(rd());
    std::shuffle(walls.begin(), walls.end(), g);

    //Carve paths by removing walls
    for (const Wall& wall : walls) {
        promise.suspendIfRequested();
        if (promise.isCanceled()) {
            emit pathfindingSearchCompleted(0, 0);
            return;
        }

        int set1 = findSet(wall.cell1Index,parent);
        int set2 = findSet(wall.cell2Index,parent);

        if (set1 != set2) {
            unionSet(set1, set2,parent,rank);

            // Carve wall
            gridNodes.Nodes[wall.wallIndex].obstacle = false;
            emit updatedScatterGridView(OBSTACLETOFREE, wall.wallIndex);

            // Carve both cells
            gridNodes.Nodes[wall.cell1Index].obstacle = false;
            emit updatedScatterGridView(OBSTACLETOFREE, wall.cell1Index);

            gridNodes.Nodes[wall.cell2Index].obstacle = false;
            emit updatedScatterGridView(OBSTACLETOFREE, wall.cell2Index);

        }
    }

    //Reset visited/nextUp flags
    for (Node& node : gridNodes.Nodes) {
        node.visited = false;
        node.nextUp = false;
    }

    emit algorithmCompleted();
    emit pathfindingSearchCompleted(0, 0);
    qDebug() << "Maze (Kruskal's): Algorithm completed.";
}
int findSet(int x,std::vector<int>& parent)
{
    if(parent[x]==x) return x;
    return parent[x]=findSet(parent[x],parent);
}

bool unionSet(int a ,int b,std::vector<int> &parent,std::vector<int>& rank)
{
    a=findSet(a,parent);
    b=findSet(b,parent);
    if (a != b) {
        if (rank[a] < rank[b])
            parent[a] = b;
        else if (rank[b] < rank[a])
            parent[b] = a;
        else {
            parent[b] = a;
            rank[a]++;
        }
        return true;
    }
    return false;
}
