#pragma once

#include <QObject>
#include <QString>
#include <memory>

#include <grpcpp/grpcpp.h>
#include "pipeline.grpc.pb.h"

class GrpcClient : public QObject {
    Q_OBJECT

public:
    explicit GrpcClient(QObject* parent = nullptr);
    ~GrpcClient() override = default;

    void connectToBackend(const QString& targetUrl = "localhost:50051");

signals:
    void connectionStatusChanged(bool connected, const QString& message);

public slots:
    void requestStatusUpdate();

private:
    std::unique_ptr<objectforge::ObjectForgePipeline::Stub> stub_;
};