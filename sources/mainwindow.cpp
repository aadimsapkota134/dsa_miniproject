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
    connect(&pathAlgorithm, &PathAlgorithm::algorithmCompleted,this,  &MainWindow::onAlgorithmCompleted);
    connect(&pathAlgorithm, &PathAlgorithm::pathfindingSearchCompleted,this,  &MainWindow::onPathfindingSearchCompleted); //aayusha ramrari code lekha ta yr
  
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
//yaha samma aayusha le, aaba sarbesh
void MainWindow::on_runButton_clicked()
{
    qDebug() << "DEBUG: on_runButton_clicked() entered.";
    if (ui->algorithmsBox->currentIndex() == -1){
        QMessageBox::information(this, "Information", "Kripaya pathfinding algorithm select garnuhos!");
        // Reset button state if no algorithm is selected
        ui->runButton->setChecked(false);
        ui->runButton->setText(QString("Start PathFinding")); // Consistent initial text
    } else if (pathAlgorithm.simulationOnGoing){ // If simulation has been started before (running or paused)
        if (pathAlgorithm.running){
            qDebug() << "DEBUG: Run button: Simulation is ongoing, handling pause/resume.";
            // Algorithm is currently running, so pause it
            pathAlgorithm.pauseAlgorithm();
            gridView.setSimulationRunning(false);
            pathAlgorithm.running = false; // Explicitly set running to false
            ui->runButton->setChecked(false);
            ui->runButton->setText(QString("Resume PathFinding")); 
        } else {
            // Algorithm is paused, so resume it
            pathAlgorithm.resumeAlgorithm();
            gridView.setSimulationRunning(true);
            pathAlgorithm.running = true; // Explicitly set running to true
            ui->runButton->setChecked(false);
            ui->runButton->setText(QString("Pause PathFinding")); 
        }
    } else {
        // This is the initial start of the pathfinding algorithm
        pathAlgorithm.running = true;
        pathAlgorithm.simulationOnGoing = true; //Set this to true on initial start

        // set the grid node of the path algorithm object;
        pathAlgorithm.gridNodes = gridView.gridNodes;
        pathAlgorithm.heightGrid = gridView.heightGrid;
        pathAlgorithm.widthGrid = gridView.widthGrid;

        // Setting the run button as checkable and checked (setCheckable should ideally be in UI XML or constructor)
        ui->runButton->setCheckable();
        ui->runButton->setChecked(true); 
        ui->runButton->setText(QString("Pause PathFinding")); 

        // Blocking the interaction with the gridView
        gridView.setSimulationRunning(true);

        // Enabling the current QScatter series point as visible
        gridView.AlgorithmView(true);
        // Call path finding
        pathAlgorithm.runAlgorithm(pathAlgorithm.getCurrentAlgorithm());
    }
}

void MainWindow::on_mazeButton_clicked()
{
    QMessageBox msgBox(this);
    msgBox.setWindowTitle("Select Maze generation algorithm");
    QPushButton *easyButton = msgBox.addButton(tr("Kruskal's"), QMessageBox::AcceptRole);

    QPushButton *cancelButton = msgBox.addButton(QMessageBox::Cancel);

    msgBox.exec();

    if (msgBox.clickedButton() == easyButton) {
        generateMazeWithAlgorithm(KRUSKAL); 
    }
    else if(msgBox.clickedButton()==cancelButton) {
        // Cancel clicked — do nothing
        return;
    }
    else{
        //yettikai haldeko    
    }
}
void MainWindow::generateMazeWithAlgorithm(int algorithmEnum)
{
    pathAlgorithm.setCurrentAlgorithm(static_cast<ALGOS>(algorithmEnum)); // Set for PathAlgorithm
    gridView.setCurrentAlgorithm(algorithmEnum); // Set for GridView (for internal logic if needed)

    pathAlgorithm.running = true;
    pathAlgorithm.simulationOnGoing = true;

    // Update the pathAlgorithm's grid info
    pathAlgorithm.gridNodes = gridView.gridNodes;
    pathAlgorithm.heightGrid = gridView.heightGrid;
    pathAlgorithm.widthGrid = gridView.widthGrid;

    gridView.setSimulationRunning(true);
    gridView.AlgorithmView(true);
    pathAlgorithm.runAlgorithm(static_cast<ALGOS>(algorithmEnum));
    //Reset the algorithm so pathfinding doesn't rerun maze generation.This reset should ideally happen AFTER onPathfindingSearchCompleted has processed the signal
     pathAlgorithm.setCurrentAlgorithm(NOALGO);
     gridView.setCurrentAlgorithm(NOALGO);
}


void MainWindow::on_resetButton_clicked()
{
    // Calling populate grid with same previous arrangement
    gridView.populateGridMap(gridView.getCurrentArrangement());

    // Reset pathfinding flags and button state on reset
    pathAlgorithm.running = false;
    pathAlgorithm.simulationOnGoing = false;
    ui->runButton->setChecked(false);
    ui->runButton->setText(QString("Start PathFinding"));
    gridView.setSimulationRunning(false); // Ensure grid interaction is re-enabled
    // Reset interaction to NOINTERACTION
    gridView.setCurrentInteraction(NOINTERACTION);
    ui->interactionBox->setCurrentIndex(-1);
    // Reset algorithm selection to NOALGO internally
    pathAlgorithm.setCurrentAlgorithm(NOALGO);


    ui->algorithmsBox->setCurrentIndex(-1); // Deselects any algorithm in the combobox

    // Reset button text for run button
    ui->runButton->setText("Start PathFinding");

    


   
}

void MainWindow::on_interactionBox_currentIndexChanged(int index)
{
    // Updating the current interaction chosen by the user
    gridView.setCurrentInteraction(index);
   
}

void MainWindow::on_algorithmsBox_currentIndexChanged(int index)
{
    // Changing the current Algorithm
    gridView.setCurrentAlgorithm(index);
    pathAlgorithm.setCurrentAlgorithm(static_cast<ALGOS>(index));
}
//yaha samma mero aaba dada koooo
