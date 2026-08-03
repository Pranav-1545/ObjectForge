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
#include <QImage>
#include <QMessageBox>
#include <atomic>

#include "GrpcClient.h"
#include "CameraWorker.h"
#include "ClickableLabel.h"

int main(int argc, char *argv[]) {
    QApplication app(argc, argv);

    QMainWindow mainWindow;
    mainWindow.setWindowTitle("ObjectForge - Reconstruction Studio");
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
    QPushButton *unlockBtn = new QPushButton("Unlock Target");
    unlockBtn->setEnabled(false);

    sidebarLayout->addWidget(titleLabel);
    sidebarLayout->addWidget(connectBtn);
    sidebarLayout->addWidget(startCamBtn);
    sidebarLayout->addWidget(unlockBtn);
    sidebarLayout->addStretch();

    // Custom Hover/Key-Responsive Viewport
    ClickableLabel *viewportLabel = new ClickableLabel();

    mainLayout->addWidget(sidebar);
    mainLayout->addWidget(viewportLabel, 1);

    mainWindow.setCentralWidget(centralWidget);

    QStatusBar *statusBar = mainWindow.statusBar();
    statusBar->showMessage("ObjectForge Shell Initialized | Hover over target ROI & press ENTER to lock.");

    // Initialize Services
    GrpcClient grpcClient;
    
    QThread *cameraThread = new QThread();
    CameraWorker *cameraWorker = new CameraWorker();
    cameraWorker->moveToThread(cameraThread);

    bool cameraRunning = false;

    // Shared State
    static std::atomic<bool> isProcessingFrame{false};
    static std::vector<DetectedObject> lastDetections;
    static bool isLocked = false;
    static cv::Rect lockedRect;

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
            viewportLabel->setFocus();
            statusBar->showMessage("Camera Feed Started | Hover target and press ENTER to lock.");
        } else {
            QMetaObject::invokeMethod(cameraWorker, "stopCamera");
            cameraThread->quit();
            cameraThread->wait();
            startCamBtn->setText("Start Camera Feed");
            viewportLabel->setText("Camera Viewport Placeholder");
            cameraRunning = false;
            isLocked = false;
            unlockBtn->setEnabled(false);
            statusBar->showMessage("Camera Feed Stopped");
        }
    });

    // Manual Unlock Button Handler
    QObject::connect(unlockBtn, &QPushButton::clicked, [&]() {
        isLocked = false;
        unlockBtn->setEnabled(false);
        viewportLabel->setFocus();
        statusBar->showMessage("Target Unlocked. Hover over an object and press ENTER.");
    });

    // Target Locking Handler (Triggered on ENTER keypress when hovering)
    QObject::connect(viewportLabel, &ClickableLabel::targetValidated, [&](const cv::Rect& finalRect) {
        if (isLocked) return;

        isLocked = true;
        lockedRect = finalRect;
        unlockBtn->setEnabled(true);

        statusBar->showMessage(QString("TARGET LOCKED [%1, %2] | Size: %3x%4")
                                .arg(lockedRect.x).arg(lockedRect.y)
                                .arg(lockedRect.width).arg(lockedRect.height));
        
        QMessageBox::information(&mainWindow, "Target Locked", 
            QString("Successfully locked target object ROI!\n\nPosition: (%1, %2)\nSize: %3x%4\n\nPress 'Unlock Target' to re-select.")
            .arg(lockedRect.x).arg(lockedRect.y).arg(lockedRect.width).arg(lockedRect.height));
    });

    // Frame Handler: Query gRPC & Pass candidates to Viewport Label
    QObject::connect(cameraWorker, &CameraWorker::frameReady, [&](const QImage &img, const cv::Mat &rawFrame) {
        cv::Mat displayFrame = rawFrame.clone();

        if (!isLocked) {
            // Send frame to backend asynchronously
            if (!isProcessingFrame.load()) {
                isProcessingFrame.store(true);

                std::vector<uchar> jpegBuffer;
                cv::imencode(".jpg", rawFrame, jpegBuffer);

                lastDetections = grpcClient.processFrame(jpegBuffer, rawFrame.cols, rawFrame.rows);
                isProcessingFrame.store(false);
            }

            // Convert detections to candidate rectangles for hover tracking
            std::vector<cv::Rect> candidateRects;
            for (const auto& det : lastDetections) {
                candidateRects.emplace_back(det.x, det.y, det.width, det.height);
            }
            viewportLabel->updateCandidates(candidateRects);

        } else {
            // Locked State: Viewport renders bright cyan crosshairs on locked target
            cv::rectangle(displayFrame, lockedRect, cv::Scalar(255, 255, 0), 3); // Cyan box

            cv::Point center(lockedRect.x + lockedRect.width / 2, lockedRect.y + lockedRect.height / 2);
            cv::drawMarker(displayFrame, center, cv::Scalar(255, 255, 0), cv::MARKER_CROSS, 24, 2);

            cv::putText(displayFrame, "TARGET LOCKED", cv::Point(lockedRect.x, std::max(20, lockedRect.y - 10)),
                        cv::FONT_HERSHEY_SIMPLEX, 0.7, cv::Scalar(255, 255, 0), 2);
        }

        // Convert BGR to RGB and render in Qt Viewport
        cv::cvtColor(displayFrame, displayFrame, cv::COLOR_BGR2RGB);
        QImage qDisplayImg(displayFrame.data, displayFrame.cols, displayFrame.rows, 
                           static_cast<int>(displayFrame.step), QImage::Format_RGB888);

        QPixmap pix = QPixmap::fromImage(qDisplayImg).scaled(viewportLabel->size(), Qt::KeepAspectRatio, Qt::SmoothTransformation);
        viewportLabel->setPixmap(pix);
    });

    mainWindow.show();

    int execResult = app.exec();

    if (cameraRunning) {
        QMetaObject::invokeMethod(cameraWorker, "stopCamera");
        cameraThread->quit();
        cameraThread->wait();
    }
    delete cameraWorker;
    delete cameraThread;

    return execResult;
}