#include "GrpcClient.h"
#include <QDebug>

GrpcClient::GrpcClient(QObject* parent)
    : QObject(parent) {}

void GrpcClient::connectToBackend(const QString& targetUrl) {
    auto channel = grpc::CreateChannel(targetUrl.toStdString(), grpc::InsecureChannelCredentials());
    stub_ = objectforge::ObjectForgePipeline::NewStub(channel);
    
    // Quick probe to verify connection
    requestStatusUpdate();
}

void GrpcClient::requestStatusUpdate() {
    if (!stub_) {
        emit connectionStatusChanged(false, "Client not initialized");
        return;
    }

    objectforge::StatusRequest request;
    request.set_client_id("ObjectForge_Qt_UI");

    grpc::ClientContext context;
    objectforge::StatusUpdate update;

    auto stream = stub_->GetPipelineStatus(&context, request);
    if (stream->Read(&update)) {
        QString msg = QString::fromStdString(update.message());
        bool connected = (update.state() != objectforge::StatusUpdate_State_DISCONNECTED &&
                          update.state() != objectforge::StatusUpdate_State_ERROR);
        emit connectionStatusChanged(connected, msg);
    } else {
        emit connectionStatusChanged(false, "Failed to connect to Python gRPC backend");
    }
}