#include <iostream>
#include <QtCore/QtMath>
#include <QtCore/QDebug>
#include <cmath>
#include <QMessageBox>
#include <QEventLoop>
#include "GridView.h"
#include <QCategoryAxis>
#include <queue>
#include <unistd.h>
#include <QTest>

// Constructor
GridView::GridView(int widthGrid, int heightGrid, int markerSize, QChartView* parent): QChartView(parent)
{
    // Default dimensions
    this->widthGrid = widthGrid;
    this->heightGrid = heightGrid;
    this->markerSize = markerSize;

    //Initialize QChart
    chart = new QChart();
    chart->setBackgroundVisible(true);

    // Series of scatter elements
    freeElements        = new QScatterSeries();
    obstacleElements    = new QScatterSeries();
    visitedElements     = new QScatterSeries();
    nextElements        = new QScatterSeries();
    pathElements        = new QScatterSeries();
    startElement        = new QScatterSeries();
    endElement          = new QScatterSeries();

    // Start, goal elements
    startElement    ->append(QPoint());
    endElement      ->append(QPoint());

    // Line series
    pathLine      = new QLineSeries();

    // Setting up current objects
    currentInteraction  = NOINTERACTION;
    currentArrangement  = EMPTY;
    currentAlgorithm    = NOALGO;
    simulationRunning   = false;

    // Initializing gridNode
    grid gridNodes;

}

// Destructor
GridView :: ~ GridView()
{
    std::cerr << "Destroying Grid View \n";
    std::cerr << "Backend Grid: \n" <<
        "Start: (" << gridNodes.Nodes[gridNodes.startIndex].xCoord << ", " << gridNodes.Nodes[gridNodes.startIndex].yCoord << "): "
              <<gridNodes.startIndex << "\n" <<
        "End: (" << gridNodes.Nodes[gridNodes.endIndex].xCoord << ", " << gridNodes.Nodes[gridNodes.endIndex].yCoord << "): "
              <<gridNodes.endIndex << "\n";

    delete freeElements;
    delete obstacleElements;
    delete visitedElements;
    delete nextElements;
    delete startElement;
    delete endElement;
}

// Setter: currentInteraction set up with index
void GridView::setCurrentInteraction(int index)
{
    currentInteraction = static_cast<INTERACTIONS>(index);
}

// Setter: currentInteraction set up with enum
void GridView::setCurrentInteraction(INTERACTIONS interaction)
{
    currentInteraction = interaction;
}

// Setter: current state
void GridView::setSimulationRunning(bool state)
{
    simulationRunning = state;
}

// Setter: currentAlgorithm
void GridView::setCurrentAlgorithm(int index)
{
    currentAlgorithm = static_cast<ALGOS>(index);
}


INTERACTIONS GridView::getCurrentInteraction() const
{
    return currentInteraction;
}

// Getter: currentArrangement
ARRANGEMENTS GridView::getCurrentArrangement() const
{
    return currentArrangement;
}

// Getter: grid of nodes
grid& GridView::getGrid()
{
    return gridNodes;
}

// Getter: current Algorithm for the main window
ALGOS GridView::getCurrentAlgorithm() const
{
    return currentAlgorithm;
}

// Getter current state
bool GridView::getSimulationRunning() const
{
    return simulationRunning;
}

// Getter: height grid
int GridView::getHeightGrid() const
{
    return heightGrid;

}

void GridView::populateGridMap(ARRANGEMENTS arrangement, bool reset)
{

    if (simulationRunning)
    {
        QMessageBox::information(this, "Information", "Simulation Banda Garnu Hoss!");
        return;
    }

    if (reset){
        // Removing all points
        freeElements    ->removePoints(0, freeElements      ->points().size());
        obstacleElements->removePoints(0, obstacleElements  ->points().size());
        visitedElements ->removePoints(0, visitedElements   ->points().size());
        nextElements    ->removePoints(0, nextElements      ->points().size());
        pathElements    ->removePoints(0, pathElements      ->points().size());
        pathLine        ->removePoints(0, pathLine          ->points().size());

        // Remove nodes from gridNodes
        gridNodes.Nodes.clear();

        // Modifying
        chart->axes(Qt::Horizontal).first() ->setRange(qreal(0.4), qreal(this->widthGrid  + 0.5));
        chart->axes(Qt::Vertical).first()   ->setRange(qreal(0.4), qreal(this->heightGrid + 0.5));

    }

    // Inserting points in the series
    for (int i = 0; i < heightGrid * widthGrid; i++)
    {
        freeElements->append(QPoint());
        obstacleElements->append(QPoint());
        visitedElements->append(QPoint());
        nextElements->append(QPoint());
        pathElements->append(QPoint());
    }


    //right number of nodes in the grid
    Node gridNodeBackend;
    for (int i{}; i < widthGrid * heightGrid; i++){gridNodes.Nodes.push_back(gridNodeBackend);}

    if (arrangement == EMPTY)
    {
        // Setting default start and end points
        startElement    ->replace(0, QPointF(1, heightGrid));
        endElement      ->replace(0, QPointF(widthGrid, 1));

        // Index for gridPoint Vector
        int indexGrid{};

        qreal y{1};
        // Populating the grid with elements
        for (int j=1;  j <= this->heightGrid; j++)
        {
            qreal x{1};
            for (int i=1;  i <= this->widthGrid; i++)
            {
                if (i == startElement->points()[0].x() && j == startElement->points()[0].y()){

                    // Updating the backend grid: starting Element
                    gridNodes.startIndex = indexGrid;

                }else if (i == endElement->points()[0].x() && j == endElement->points()[0].y()){

                    // Updating the backend grid: ending Element
                    gridNodes.endIndex = indexGrid;

                }else{

                    // Populating the QScatter Series of free Elements
                    freeElements->replace(indexGrid, QPointF(x, y));
                }

                // Deleting the obstacle if present (useful for reset)
                if (gridNodes.Nodes[indexGrid].obstacle == true)
                {
                    obstacleElements->replace(indexGrid, QPointF());
                }

                // Grid Point: updating coordinates
                Node gridNodeBackend;
                gridNodeBackend.xCoord = int(x);
                gridNodeBackend.yCoord = int(y);

                //populating the backend grid with the point along with the starting and ending
                gridNodes.Nodes[indexGrid] = gridNodeBackend;

                // Reset of properties
                gridNodes.Nodes[indexGrid].obstacle = false;
                gridNodes.Nodes[indexGrid].visited = false;
                gridNodes.Nodes[indexGrid].nextUp = false;

                // Reset of Elements
                visitedElements ->replace(indexGrid, QPointF());
                nextElements    ->replace(indexGrid, QPointF());
                pathElements    ->replace(indexGrid, QPointF());

                x++;
                indexGrid++;

            }
            y++;
        }
    }else{

        std::cerr << "BAD ARRANGEMENT \n";
    }

    // Setting up the current index
    gridNodes.currentIndex = gridNodes.startIndex;

    // Check size of vector
    std::cerr << "\nNumber of nodes in gridNodes: " << gridNodes.Nodes.size() << " vs " << this->heightGrid * this->widthGrid << "\n";

}

//lekhdaa lekhdaa pagal bhaye ma
//ab tumhare hawaale watan saathiyon
