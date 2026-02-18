// mainWindow.h
#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QPushButton>
#include "GridView.h"
#include "PathAlgorithm.h"
#include "qlabel.h"

QT_BEGIN_NAMESPACE

namespace Ui
{
class MainWindow;
}

QT_END_NAMESPACE


class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    // Constructor
    MainWindow(QWidget parent = nullptr);
    // Destructor
    virtual ~MainWindow();

    // Setting up objects
    void setupInteractionComboBox();
    void setupAlgorithmsComboBox();
    void setupGridView(QString gridViewName);

    // Getters
    GridView& getGridView();


public: Q_SIGNALS:
    //void launchedBFS();

public slots:
    // Run the simulation and pause it
    void on_runButton_clicked();

    // Reset the ChartView
    void on_resetButton_clicked();

    // Generate button
    void on_mazeButton_clicked(); // Renamed from on_mazeButton_released() for clarity

    // Handles the different interaction changes
    void on_interactionBox_currentIndexChanged(int index);

    // Handles the different algorithm changes
    void on_algorithmsBox_currentIndexChanged(int index);


    // Action to do when the pathfinding search completes (before visualization)
    void onPathfindingSearchCompleted(int nodesVisited, int pathLength); // This slot will now only populate the list

private slots:
    void on_dialWidth_valueChanged(int value);
    void on_dialHeight_valueChanged(int value);
    void on_sliderMarker_valueChanged(int value);
    void on_sliderMarker_sliderReleased();
    void on_dialWidth_sliderReleased();
    void on_dialHeight_sliderReleased();
    void generateMazeWithAlgorithm(int algorithmEnum);


private:
    Ui::MainWindow ui;
    GridView gridView;
    PathAlgorithm pathAlgorithm;

    bool mazeCurrentlyGenerated; //Flag to track if a maze is generated
};
#endif // MAINWINDOW_H
