// aajha ko lagi naya header file
#ifndef PATHALGORITHM_H
#define PATHALGORITHM_H
#include <QObject>
#include <QDebug>

#include <QtConcurrent>
#include <QFuture>
#include "GridView.h"
#include <queue> // for std::priority_queue
#include <cmath> // For fabsf

//Build walls list between adjacent odd-indexed cells
struct Wall {
    int wallIndex;
    int cell1Index;
    int cell2Index;
};

bool unionSet(int a ,int b,std::vector<int> &parent,std::vector<int>& rank);
int findSet(int x,std::vector<int>& parent);
class PathAlgorithm : public QObject
{

    Q_OBJECT
public:

    //Constructor
    explicit PathAlgorithm(QObject* parent = nullptr);

    //Destructor
    virtual ~PathAlgorithm();

    //Getters/Setters: current Algorithm from gridView
    ALGOS getCurrentAlgorithm() const;
    void setCurrentAlgorithm(ALGOS algorithm);

    // Getters/Setters: Simulation on going
    void setSimulationOnGoing(bool onGoing);

    // Running pausing and canceling algorithms
    void runAlgorithm(ALGOS algorithm);
    void pauseAlgorithm();
    void resumeAlgorithm();
    void stopAlgorithm();

    // Path planning Algorithms
    void performBFSAlgorithm(QPromise<int>& promise);
    void performDFSAlgorithm(QPromise<int>& promise);
    //maze generation using kruskal algorithm
    void performKruskalsMazeAlgorithm(QPromise<int>& promise);

    // Retrieving the neighbors of a point in a grid
    std::vector<Node> retrieveNeighborsGrid(const grid& gridNodes, const Node& currentNode, int widthGrid, int heightGrid);
    void FillNeighboursNode(Node& node);

    void checkGridNode(grid gridNodes, int heightGrid, int widthGrid);

public: Q_SIGNALS:
    void updatedScatterGridView (UPDATETYPES VISIT,     int currentIndex);
    void updatedLineGridView    (QPointF currentPoint,  bool addingPoint,   bool clearPriorToUpdate=false);

    void algorithmCompleted(); // Emitted after visualization 
    void pathfindingSearchCompleted(int nodesVisited, int pathLength);
private:
    // Helper functions
    void addFrontierCells(Node* node, std::vector<Node*>& frontier);
    std::vector<Node*> getMazeNeighbors(Node* node);
    void connectNodes(Node* a, Node* b);
public:

    ALGOS currentAlgorithm;
    bool running;
    bool simulationOnGoing;
    bool endReached;
    // grid nodes manipulated by the path planning object
    grid gridNodes;
    int heightGrid;
    int widthGrid;

    // Multithreading
    QThreadPool pool;
    QFuture<int> futureOutput;
public:
    QString algorithmToString(ALGOS algo);
public:
    //  new method to update to grids:
    void setGridNodes(const grid& newGridNodes, int width, int height);
};
// sakiyooo , aba sutxuu
