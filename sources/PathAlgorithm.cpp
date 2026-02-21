// path ko lagi naya file
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

//Constructor
PathAlgorithm::PathAlgorithm(QObject* parent): QObject (parent)
{
    // The algorithm is not running at startup
    running = false;
    simulationOnGoing = false;
    endReached = false;

    speedVisualization = 250;
    qDebug() << "PathAlgorithm: Constructor called. Main thread ID:" << QThread::currentThreadId();
}

//Destructor
PathAlgorithm::~PathAlgorithm()
{

}

//Getters/Setters: current Algorithm from gridView
ALGOS PathAlgorithm::getCurrentAlgorithm() const
{
    return currentAlgorithm;
}

//Getters/Setters: current Algorithm from gridView
void PathAlgorithm::setCurrentAlgorithm(ALGOS algorithm)
{
    this->currentAlgorithm = algorithm;
}

void PathAlgorithm::setSpeedVizualization(int speed)
{
    this->speedVisualization = speed;
}

// Getters/Setters: Simulation on going
void PathAlgorithm::setSimulationOnGoing(bool onGoing)
{
    this->simulationOnGoing = onGoing;
}
// Implement the new method:
void PathAlgorithm::setGridNodes(const grid& newGridNodes, int width, int height)
{
    this->gridNodes = newGridNodes; 
    this->widthGrid = width;       
    this->heightGrid = height;      
    qDebug() << "PathAlgorithm: Internal grid dimensions updated to " << this->widthGrid << "x" << this->heightGrid;
    qDebug() << "PathAlgorithm: Internal gridNodes.Nodes.size() is now: " << this->gridNodes.Nodes.size();
    qDebug() << "PathAlgorithm: Internal startIndex: " << this->gridNodes.startIndex << ", endIndex: " << this->gridNodes.endIndex;

    for(Node& node: this->gridNodes.Nodes) {
        node.visited = false;
        node.nextUp = false;
    }
}

std::vector<Node> PathAlgorithm::retrieveNeighborsGrid(const grid& gridNodes, const Node& currentNode, int widthGrid, int heightGrid)
{

    std::vector<Node> neighbors;

    // right: adding +1 to x:
    if (currentNode.xCoord + 1 <= widthGrid)
    {
        int rightIndex = coordToIndex(currentNode.xCoord + 1, currentNode.yCoord, widthGrid);
        neighbors.push_back(gridNodes.Nodes[rightIndex]);

    }

    // down: adding -1 to y:
    if (currentNode.yCoord - 1 >= 1)
    {
        int downIndex = coordToIndex(currentNode.xCoord, currentNode.yCoord -1, widthGrid);
        neighbors.push_back(gridNodes.Nodes[downIndex]);
    }

    // left: adding -1 to x:
    if (currentNode.xCoord - 1 >= 1)
    {
        int leftIndex = coordToIndex(currentNode.xCoord - 1, currentNode.yCoord, widthGrid);
        neighbors.push_back(gridNodes.Nodes[leftIndex]);
    }

    // up: adding +1 to y:
    if (currentNode.yCoord + 1 <= heightGrid)
    {
        int upIndex = coordToIndex(currentNode.xCoord, currentNode.yCoord + 1, widthGrid);
        neighbors.push_back(gridNodes.Nodes[upIndex]);
    }

    return neighbors;
}

QString PathAlgorithm::algorithmToString(ALGOS algo) {
    switch (algo) {
    case BFS: return "BFS";
    case DFS: return "DFS";
    case NOALGO: return "NOALGO";
    default: return "UNKNOWN_ALGO";
    }
}
void PathAlgorithm::checkGridNode(grid gridNodes, int heightGrid, int widthGrid)
{
    // Display grid
    std::cerr << "State of grid node \n";
    int countVisited = 0; int countObstacle = 0; int countFree = 0;

    for (Node node: gridNodes.Nodes)
    {
        std::cerr << "(" << node.xCoord << ", " <<  node.yCoord << "): ";

        if (node.visited){std::cerr << ": V"; countVisited++;}

        if (node.obstacle){std::cerr << ": O"; countObstacle++;}
        else{std::cerr << ": F"; countFree++;}

        if (node.xCoord == widthGrid){std::cerr << " \n";}
        else{std::cerr << " | ";}

    }
    std::cerr << "Totals: " << "Visited: " << countVisited
              << " - Obstacles: " << countObstacle
              << " - Free:" << countFree << "\n";

    // Check size of vector
    if (static_cast<int>(gridNodes.Nodes.size()) != static_cast<int>(heightGrid * widthGrid))
    {std::cerr << "Number of nodes in gridNodes: " << gridNodes.Nodes.size() << " vs " << heightGrid * widthGrid << " [ISSUE] \n";}
    else{std::cerr << "Number of nodes in gridNodes: " << gridNodes.Nodes.size() << "\n";}

}

void PathAlgorithm::runAlgorithm(ALGOS algorithm)
{
    simulationOnGoing=true;
    running=true;
    qDebug() << "PathAlgorithm: runAlgorithm called. Thread ID:" << QThread::currentThreadId();

    switch (algorithm) {
    case BFS:
        futureOutput = QtConcurrent::run(&pool, &PathAlgorithm::performBFSAlgorithm, this);
        break;
    case DFS:
        futureOutput = QtConcurrent::run(&pool, &PathAlgorithm::performDFSAlgorithm, this);
        break;
    case KRUSKAL:
        futureOutput = QtConcurrent::run(&pool, &PathAlgorithm::performKruskalsMazeAlgorithm, this);
        break;
    case NOALGO:
        std::cerr <<"NO ALGO \n";
    default:
        break;

    }

}

void PathAlgorithm::resumeAlgorithm()
{
    running = true;
    futureOutput.resume();
    qDebug() << "PathAlgorithm: futureOutput.resume() called.";
}

void PathAlgorithm::pauseAlgorithm()
{
    running = false;
    futureOutput.suspend();
    qDebug() << "PathAlgorithm: futureOutput.suspend() called.";
}

void PathAlgorithm::stopAlgorithm()
{
    running = false;
    futureOutput.cancel();
    qDebug() << "PathAlgorithm: futureOutput.cancel() called.";
}

// BFS Algorithm

void PathAlgorithm::performBFSAlgorithm(QPromise<int>& promise)
{
    qDebug() << "BFS: Algorithm started in worker thread:" << QThread::currentThreadId();

    promise.suspendIfRequested();
    if (promise.isCanceled()) {
        // If cancelled before starting, emit with 0, 0
        emit pathfindingSearchCompleted(0, 0);
        return;
    }

    bool reachEnd = false;
    std::queue<Node*> nextNodesQueue; // Use Node* to avoid copying large Node objects
    std::map<int, int> parentMap; // Map child index to parent index for path reconstruction

    // Starting point
    Node* startNode = &(gridNodes.Nodes[gridNodes.startIndex]);
    nextNodesQueue.push(startNode);
    gridNodes.Nodes[gridNodes.startIndex].visited = true; // Mark start as visited immediately
    parentMap[gridNodes.startIndex] = -1; // No parent for start node

    int nodesVisitedCount = 0; // Counter for visited nodes

    while(!nextNodesQueue.empty())
    {
        promise.suspendIfRequested();
        if (promise.isCanceled()) {
            qDebug() << "BFS: Algorithm cancelled during loop.";
            // Emit with current stats on cancel
            emit pathfindingSearchCompleted(nodesVisitedCount, 0);
            return;
        }

        Node* currentNode = nextNodesQueue.front();
        nextNodesQueue.pop();
        nodesVisitedCount++; // Increment when a node is dequeued and processed

        int currentIndex = coordToIndex(currentNode->xCoord, currentNode->yCoord, widthGrid);
        if (currentIndex != gridNodes.startIndex && currentIndex != gridNodes.endIndex) {
            emit updatedScatterGridView(VISIT, currentIndex);
        }

        // Check if goal is reached
        if (currentIndex == gridNodes.endIndex)
        {
            std::cerr << "Reached end \n";
            reachEnd = true;
            break; // Path found, exit search loop
        }
        // Note: retrieveNeighborsGrid returns copies...

        // Manual neighbor retrieval for BFS/DFS (more typical)
        int dx[] = {0, 0, 1, -1}; // Up, Down, Right, Left
        int dy[] = {1, -1, 0, 0};

        for (int i = 0; i < 4; ++i) {
            int neighborX = currentNode->xCoord + dx[i];
            int neighborY = currentNode->yCoord + dy[i];

            if (neighborX >= 1 && neighborX <= widthGrid &&
                neighborY >= 1 && neighborY <= heightGrid)
            {
                int neighborIndex = coordToIndex(neighborX, neighborY, widthGrid);
                Node* neighborNode = &(gridNodes.Nodes[neighborIndex]);

                if (!neighborNode->visited && !neighborNode->obstacle) {
                    neighborNode->visited = true; // Mark as visited when added to queue
                    nextNodesQueue.push(neighborNode);
                    parentMap[neighborIndex] = currentIndex; // Store parent

                    // Update this node as 'next' in the gridView
                    if (neighborIndex != gridNodes.endIndex) { // Don't mark end as 'next' if found
                        emit updatedScatterGridView(NEXT, neighborIndex);
                    }
                }
            }
        }

    }

    int pathLength = 0;
    // If the end is reached, we output the path and calculate pathLength
    if (reachEnd){
        endReached = true;

        // Path reconstruction and pathLength calculation
        std::vector<int> pathIndices;
        int currentPathIndex = gridNodes.endIndex;
        while (currentPathIndex != -1) {
            pathIndices.insert(pathIndices.begin(), currentPathIndex); // Insert at beginning to reverse
            if (parentMap.count(currentPathIndex)) {
                currentPathIndex = parentMap[currentPathIndex];
            } else {
                currentPathIndex = -1; // Should not happen if path is valid, but good for safety
            }
        }
        pathLength = pathIndices.size() > 0 ? pathIndices.size() - 1 : 0; // Path length is edges (nodes - 1)

        emit pathfindingSearchCompleted(nodesVisitedCount, pathLength); // Emit signal when search is complete
        qDebug() << "BFS: Pathfinding search completed. Emitting pathfindingSearchCompleted()";

        // Visualization of the path
        emit updatedLineGridView(QPointF(gridNodes.Nodes[gridNodes.endIndex].xCoord, gridNodes.Nodes[gridNodes.endIndex].yCoord), true, true); // Clear and start line from end

        for (size_t i = pathIndices.size() - 1; i > 0; --i) { // Iterate from goal back to start (excluding start)
            promise.suspendIfRequested();
            if (promise.isCanceled()) {
                qDebug() << "BFS: Algorithm cancelled during visualization.";
                break;
            }
            int pathNodeIndex = pathIndices[i];
            emit updatedScatterGridView(PATH, pathNodeIndex);
            emit updatedLineGridView(QPointF(gridNodes.Nodes[pathNodeIndex].xCoord, gridNodes.Nodes[pathNodeIndex].yCoord), true, false);
        }
        // Ensure start node is also part of the line if not already
        emit updatedLineGridView(QPointF(gridNodes.Nodes[gridNodes.startIndex].xCoord, gridNodes.Nodes[gridNodes.startIndex].yCoord), true, false);

    }else{
        endReached = false;
        emit pathfindingSearchCompleted(nodesVisitedCount, 0); // Emit signal even if path not found
        qDebug() << "BFS: Pathfinding search not found. Emitting pathfindingSearchCompleted()";
    }

    // Reset visited flags for next run
    for(Node& node: gridNodes.Nodes) {
        node.visited = false;
        node.nextUp = false; // Also reset nextUp for BFS/DFS
    }

    qDebug() << "DEBUG: " << algorithmToString(currentAlgorithm) << ": About to emit algorithmCompleted()."; // Add this line
    emit algorithmCompleted();

    qDebug() << "DEBUG: " << algorithmToString(currentAlgorithm) << ": Emitted algorithmCompleted() and pathfindingSearchCompleted()."; // Add this line
} // mero part ko sakiyooo

// dfs aad

void PathAlgorithm::performDFSAlgorithm(QPromise<int>& promise)
{
    qDebug() << "DFS: Algorithm started in worker thread:" << QThread::currentThreadId();
    promise.suspendIfRequested();
    if (promise.isCanceled())
    {
        emit pathfindingSearchCompleted(0, 0); // Emittin with current stats on cancel
        return;
    }
    bool reachEnd = false;
    std::stack<Node*> nextNodesStack; 
    std::map<int, int> parentMap; // Mappin child index to parent index

    Node* startNode = &(gridNodes.Nodes[gridNodes.startIndex]);
    nextNodesStack.push(startNode);
    gridNodes.Nodes[gridNodes.startIndex].visited = true; // Mark start as visited
    parentMap[gridNodes.startIndex] = -1; // No parent for start node

    int nodesVisitedCount = 0; // Counter for visited nodes

    while(!nextNodesStack.empty())
    {
        promise.suspendIfRequested();
        if (promise.isCanceled()) {
            qDebug() << "DFS: Algorithm cancelled during loop.";
            emit pathfindingSearchCompleted(nodesVisitedCount, 0); // Emit with current stats on cancel
            return;
        }

        Node* currentNode = nextNodesStack.top();
        nextNodesStack.pop();
        nodesVisitedCount++; // Increment when a node is popped and processed

        int currentIndex = coordToIndex(currentNode->xCoord, currentNode->yCoord, widthGrid);

        if (currentIndex != gridNodes.startIndex && currentIndex != gridNodes.endIndex) {
            emit updatedScatterGridView(VISIT, currentIndex);
        }

        if (currentIndex == gridNodes.endIndex)
        {
            std::cerr << "Reached end \n";
            reachEnd = true;
            break;
        }

        // Manual neighbor retrieval for DFS
        int dx[] = {0, 0, 1, -1}; // Up, Down, Right, Left
        int dy[] = {1, -1, 0, 0};

        for (int i = 0; i < 4; ++i) {
            int neighborX = currentNode->xCoord + dx[i];
            int neighborY = currentNode->yCoord + dy[i];

            if (neighborX >= 1 && neighborX <= widthGrid &&
                neighborY >= 1 && neighborY <= heightGrid)
            {
                int neighborIndex = coordToIndex(neighborX, neighborY, widthGrid);
                Node* neighborNode = &(gridNodes.Nodes[neighborIndex]);

                if (!neighborNode->visited && !neighborNode->obstacle) {
                    neighborNode->visited = true; // Marking as 'visited' when added to stack
                    nextNodesStack.push(neighborNode);
                    parentMap[neighborIndex] = currentIndex; // Store parent

                    if (neighborIndex != gridNodes.endIndex) {
                        emit updatedScatterGridView(NEXT, neighborIndex);
                    }
                }
            }
        }
    }

    int pathLength = 0;
    if (reachEnd){
        endReached = true;

        // Path reconstruction and pathLength calculation
        std::vector<int> pathIndices;
        int currentPathIndex = gridNodes.endIndex;
        while (currentPathIndex != -1) {
            pathIndices.insert(pathIndices.begin(), currentPathIndex);
            if (parentMap.count(currentPathIndex)) {
                currentPathIndex = parentMap[currentPathIndex];
            } else {
                currentPathIndex = -1;
            }
        }
        pathLength = pathIndices.size() > 0 ? pathIndices.size() - 1 : 0;

        emit pathfindingSearchCompleted(nodesVisitedCount, pathLength);
        qDebug() << "DFS: Pathfinding search completed. Emitting pathfindingSearchCompleted()";

        // Visualizing the path
        emit updatedLineGridView(QPointF(gridNodes.Nodes[gridNodes.endIndex].xCoord, gridNodes.Nodes[gridNodes.endIndex].yCoord), true, true);

        for (size_t i = pathIndices.size() - 1; i > 0; --i) {
            promise.suspendIfRequested();
            if (promise.isCanceled()) {
                qDebug() << "DFS: Algorithm cancelled during visualization.";
                break;
            }
            int pathNodeIndex = pathIndices[i];
            emit updatedScatterGridView(PATH, pathNodeIndex);
            emit updatedLineGridView(QPointF(gridNodes.Nodes[pathNodeIndex].xCoord, gridNodes.Nodes[pathNodeIndex].yCoord), true, false);
        }
        emit updatedLineGridView(QPointF(gridNodes.Nodes[gridNodes.startIndex].xCoord, gridNodes.Nodes[gridNodes.startIndex].yCoord), true, false);

    }else{
        endReached = false;
        emit pathfindingSearchCompleted(nodesVisitedCount, 0);
        qDebug() << "DFS: Pathfinding search not found. Emitting pathfindingSearchCompleted()";
    }

    // Resetting visited flags for next run
    for(Node& node: gridNodes.Nodes) {
        node.visited = false;
        node.nextUp = false; 
    }

    emit algorithmCompleted();
    qDebug() << "DFS: Algorithm completed (visualization sakiyo). Emitting algorithmCompleted()";
}
