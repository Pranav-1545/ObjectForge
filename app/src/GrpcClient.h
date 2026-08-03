#pragma once

#include <QObject>
#include <QString>
#include <vector>
#include <memory>
#include <grpcpp/grpcpp.h>
#include "pipeline.pb.h"
#include "pipeline.grpc.pb.h"

struct DetectedObject {
    int x;
    int y;
    int width;
    int height;
    QString label;
    float confidence;
};

class GrpcClient : public QObject {
    Q_OBJECT

public:
    explicit GrpcClient(QObject *parent = nullptr);
    ~GrpcClient() override = default;

    bool connectToBackend(const QString& targetUrl = "localhost:50051");
    std::vector<DetectedObject> processFrame(const std::vector<uchar>& imageBytes, int width, int height);

signals:
    void connectionStatusChanged(bool connected, const QString& message);

private:
    std::unique_ptr<objectforge::ObjectForgePipeline::Stub> stub_;
    bool isConnected_{false};
};