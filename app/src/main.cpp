#include <QApplication>
#include <QMainWindow>
#include <QPushButton>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QStatusBar>
#include <QStyle>
#include <QThread>
#include <QPixmap>

#include "GrpcClient.h"
#include "CameraWorker.h"

int main(int argc, char *argv[]) {
    QApplication app(argc, argv);

    QMainWindow mainWindow;
    mainWindow.setWindowTitle("ObjectForge - 3D AI Reconstruction Studio");
    mainWindow.resize(1280, 720);

    app.setStyle("Fusion");

    QWidget *centralWidget = new QWidget(&mainWindow);
    QHBoxLayout *mainLayout = new QHBoxLayout(centralWidget);

    // Left Sidebar
    QWidget *sidebar = new QWidget();
    sidebar->setFixedWidth(240);
    QVBoxLayout *sidebarLayout = new QVBoxLayout(sidebar);

    QLabel *titleLabel = new QLabel("ObjectForge Studio");
    QFont titleFont = titleLabel->font();
    titleFont.setBold(true);
    titleFont.setPointSize(12);
    titleLabel->setFont(titleFont);

    QPushButton *connectBtn = new QPushButton("Connect AI Backend");
    QPushButton *startCamBtn = new QPushButton("Start Camera Feed");

    sidebarLayout->addWidget(titleLabel);
    sidebarLayout->addWidget(connectBtn);
    sidebarLayout->addWidget(startCamBtn);
    sidebarLayout->addStretch();

    // Video Viewport
    QLabel *viewportLabel = new QLabel("Camera Viewport Placeholder");
    viewportLabel->setAlignment(Qt::AlignCenter);
    viewportLabel->setStyleSheet("background-color: #1e1e1e; color: #888888; border-radius: 6px;");

    mainLayout->addWidget(sidebar);
    mainLayout->addWidget(viewportLabel, 1);

    mainWindow.setCentralWidget(centralWidget);

    QStatusBar *statusBar = mainWindow.statusBar();
    statusBar->showMessage("ObjectForge Shell Initialized | Backend: Disconnected");

    // Initialize Services
    GrpcClient grpcClient;
    
    QThread *cameraThread = new QThread();
    CameraWorker *cameraWorker = new CameraWorker();
    cameraWorker->moveToThread(cameraThread);

    bool cameraRunning = false;

    // Signal / Slot Connections
    QObject::connect(&grpcClient, &GrpcClient::connectionStatusChanged, [&](bool connected, const QString& message) {
        if (connected) {
            statusBar->showMessage("Backend Connected: " + message);
        } else {
            statusBar->showMessage("Backend Error: " + message);
        }
    });

    QObject::connect(connectBtn, &QPushButton::clicked, [&]() {
        statusBar->showMessage("Connecting to localhost:50051...");
        grpcClient.connectToBackend("localhost:50051");
    });

    QObject::connect(startCamBtn, &QPushButton::clicked, [&]() {
        if (!cameraRunning) {
            cameraThread->start();
            QMetaObject::invokeMethod(cameraWorker, "startCamera", Q_ARG(int, 0));
            startCamBtn->setText("Stop Camera Feed");
            cameraRunning = true;
            statusBar->showMessage("Camera Feed Started");
        } else {
            QMetaObject::invokeMethod(cameraWorker, "stopCamera");
            cameraThread->quit();
            cameraThread->wait();
            startCamBtn->setText("Start Camera Feed");
            viewportLabel->setText("Camera Viewport Placeholder");
            cameraRunning = false;
            statusBar->showMessage("Camera Feed Stopped");
        }
    });

    QObject::connect(cameraWorker, &CameraWorker::frameReady, [&](const QImage &img, const cv::Mat &/*raw*/) {
        QPixmap pix = QPixmap::fromImage(img).scaled(viewportLabel->size(), Qt::KeepAspectRatio, Qt::SmoothTransformation);
        viewportLabel->setPixmap(pix);
    });

    QObject::connect(cameraWorker, &CameraWorker::cameraError, [&](const QString &err) {
        statusBar->showMessage("Camera Error: " + err);
    });

    mainWindow.show();

    int execResult = app.exec();

    // Clean up camera thread on exit
    if (cameraRunning) {
        QMetaObject::invokeMethod(cameraWorker, "stopCamera");
        cameraThread->quit();
        cameraThread->wait();
    }
    delete cameraWorker;
    delete cameraThread;

    return execResult;
}