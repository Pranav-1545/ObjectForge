#include "CameraWorker.h"
#include <QDebug>

CameraWorker::CameraWorker(QObject *parent) : QObject(parent) {}

CameraWorker::~CameraWorker() {
    stopCamera();
}

void CameraWorker::startCamera(int deviceIndex) {
    if (running_) return;

    if (!cap_.open(deviceIndex, cv::CAP_ANY)) {
        emit cameraError("Failed to open camera device index " + QString::number(deviceIndex));
        return;
    }

    // Set resolution to 1280x720
    cap_.set(cv::CAP_PROP_FRAME_WIDTH, 1280);
    cap_.set(cv::CAP_PROP_FRAME_HEIGHT, 720);

    running_ = true;
    captureLoop();
}

void CameraWorker::stopCamera() {
    running_ = false;
    if (cap_.isOpened()) {
        cap_.release();
    }
}

void CameraWorker::captureLoop() {
    cv::Mat frame;
    while (running_) {
        if (!cap_.read(frame) || frame.empty()) {
            QThread::msleep(10);
            continue;
        }

        // Convert BGR (OpenCV format) to RGB (Qt QImage format)
        cv::Mat rgbFrame;
        cv::cvtColor(frame, rgbFrame, cv::COLOR_BGR2RGB);

        QImage img(rgbFrame.data, rgbFrame.cols, rgbFrame.rows, static_cast<int>(rgbFrame.step), QImage::Format_RGB888);

        emit frameReady(img.copy(), frame.clone());
        QThread::msleep(30); // ~30 FPS frame rate lock
    }
}