#include <QChartView>
#include<QPushButton>
#include <QMessageBox>
#include <QLabel>         // Include QLabel
#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "GridView.h"


MainWindow::MainWindow(QWidget *parent): QMainWindow(parent), ui(new Ui::MainWindow), gridView(30, 30, 19), pathAlgorithm()
{
    // Setup of the window
    ui->setupUi(this);


    // Customize chart background
    QLinearGradient backgroundGradient;
    backgroundGradient.setStart(QPointF(0, 0));
    backgroundGradient.setFinalStop(QPointF(0, 1));
    backgroundGradient.setColorAt(0.0, QRgb(0xd2d0d1));
    backgroundGradient.setColorAt(1.0, QRgb(0x4c4547));
    backgroundGradient.setCoordinateMode(QGradient::ObjectBoundingMode);

    QBrush brush(backgroundGradient);

    QPalette palette;
    palette.setBrush(QPalette::Window, brush);
    setPalette(palette);

    // Setup
    ui->dialWidth   ->setValue  (gridView.widthGrid);
    ui->dialWidth   ->setMinimum(5);
    ui->dialWidth   ->setMaximum(35);
    ui->lcdWidth    ->display   (gridView.widthGrid);

    ui->dialHeight  ->setValue  (gridView.heightGrid);
    ui->dialHeight  ->setMinimum(5);
    ui->dialHeight  ->setMaximum(35);
    ui->lcdHeight   ->display   (gridView.heightGrid);

    ui->sliderMarker->setValue  (gridView.markerSize);
    ui->lcdMarker   ->display   (gridView.markerSize);

    // Initial Simulation speed
    ui->speedSpinBox->setMaximum(100);
    int speed = ui->speedSpinBox->maximum() / 5;
   
    // Initial state for the run button
    ui->runButton->setChecked(false); // Ensure it starts in the "play" state
    ui->runButton->setText(QString("Start PathFinding")); // Initial text


    // Setting up the chart view
    setupGridView("gridView");

    // Setting up the Interaction Combo Box
    setupInteractionComboBox();

    // Setting up the Algorithms Combo Box
    setupAlgorithmsComboBox();

    // A change in the grid view create a change in the chartview
    connect(&pathAlgorithm, &PathAlgorithm::updatedScatterGridView, &gridView, &GridView::handleUpdatedScatterGridView);
    connect(&pathAlgorithm, &PathAlgorithm::updatedLineGridView,    &gridView, &GridView::handleUpdatedLineGridView);

    // Connecting the end signal of path planning to the window
    connect(&pathAlgorithm, &PathAlgorithm::algorithmCompleted,  &MainWindow::onAlgorithmCompleted);
    connect(&pathAlgorithm, &PathAlgorithm::pathfindingSearchCompleted,  &MainWindow::onPathfindingSearchCompleted);
  
}


MainWindow::~MainWindow() //destructor
{
    delete ui;
    
}

void MainWindow::onPathfindingSearchCompleted(int nodesVisited, int pathLength)
{

        data.algorithmName = ui->algorithmsBox->currentText();
    data.gridSize = QString("%1x%2").arg(gridView.widthGrid).arg(gridView.heightGrid); // Initialize with current grid size
    data.numDeadEnds = 0;   // Placeholder
}


void MainWindow::setupInteractionComboBox()
{
    // Default text
    ui->interactionBox->setPlaceholderText(QStringLiteral("--Select Interaction--"));
    ui->interactionBox->setCurrentIndex(-1);

    // Adding first interation: Add starting point
    ui->interactionBox->addItem("Add Start");

    // Adding second interaction: Add end point
    ui->interactionBox->addItem("Add Goal");

}

void MainWindow::setupAlgorithmsComboBox()
{
    ui->algorithmsBox->setPlaceholderText(QStringLiteral("--Select Algorithm--"));
    ui->algorithmsBox->setCurrentIndex(-1);

    //algos used
    ui->algorithmsBox->addItem("BFS Algorithm");
    ui->algorithmsBox->addItem("DFS Algorithm");
}

void MainWindow::setupGridView(QString gridViewName)
{

    // Setting up chartview
    ui->gridView->setObjectName(gridViewName);
    ui->gridView->setMinimumWidth(qreal(700));
    ui->gridView->setMinimumHeight(qreal(700));
    // Setup nodes in GridView
    gridView.setupNodes(); // This creates and populates gridView.gridNodes.Nodes

    // passing the updated grid data and dimensions to PathAlgorithm
    pathAlgorithm.setGridNodes(gridView.gridNodes, gridView.widthGrid, gridView.heightGrid);
    // This is crucial for a clean visualization when grid changes
    gridView.chart->removeAllSeries();
    gridView.chart->addSeries(gridView.freeElements);
    gridView.chart->addSeries(gridView.obstacleElements);
    gridView.chart->addSeries(gridView.visitedElements);
    gridView.chart->addSeries(gridView.nextElements);
    gridView.chart->addSeries(gridView.pathElements);
    gridView.chart->addSeries(gridView.pathLine);
    gridView.chart->addSeries(gridView.startElement);
    gridView.chart->addSeries(gridView.endElement);
    // Create Chart in chartview
    QChart* chart = gridView.createChart();
    ui->gridView->setChart(chart);

}

GridView& MainWindow::getGridView()
{
    return gridView;
}

