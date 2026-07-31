#pragma once

#include <QObject>
#include <QThread>
#include <QImage>
#include <opencv2/opencv.hpp>

class CameraWorker : public QObject {
    Q_OBJECT

public:
    explicit CameraWorker(QObject *parent = nullptr);
    ~CameraWorker() override;

public slots:
    void startCamera(int deviceIndex = 0);
    void stopCamera();

signals:
    void frameReady(const QImage &image, const cv::Mat &rawFrame);
    void cameraError(const QString &errorMsg);

private:
    void captureLoop();

    cv::VideoCapture cap_;
    bool running_{false};
};