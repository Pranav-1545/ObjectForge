#include "GrpcClient.h"
#include <QDebug>
#include <chrono>

GrpcClient::GrpcClient(QObject *parent) : QObject(parent) {}

bool GrpcClient::connectToBackend(const QString& targetUrl) {
    auto channel = grpc::CreateChannel(targetUrl.toStdString(), grpc::InsecureChannelCredentials());
    stub_ = objectforge::ObjectForgePipeline::NewStub(channel);

    // Simple test RPC call to check backend connection
    objectforge::StatusRequest request;
    request.set_client_id("ObjectForge_Qt_Client");

    grpc::ClientContext context;
    context.set_deadline(std::chrono::system_clock::now() + std::chrono::seconds(2));

    auto reader = stub_->GetPipelineStatus(&context, request);
    objectforge::StatusUpdate status;

    if (reader->Read(&status)) {
        isConnected_ = true;
        emit connectionStatusChanged(true, QString::fromStdString(status.message()));
        return true;
    } else {
        isConnected_ = false;
        emit connectionStatusChanged(false, "Failed to connect to backend service.");
        return false;
    }
}

std::vector<DetectedObject> GrpcClient::processFrame(const std::vector<uchar>& imageBytes, int width, int height) {
    std::vector<DetectedObject> detections;
    if (!isConnected_ || !stub_) return detections;

    objectforge::FrameRequest request;
    request.set_timestamp_ms(std::chrono::duration_cast<std::chrono::milliseconds>(
        std::chrono::system_clock::now().time_since_epoch()).count());
    request.set_image_data(imageBytes.data(), imageBytes.size());
    request.set_width(width);
    request.set_height(height);

    objectforge::FrameResponse response;
    grpc::ClientContext context;
    context.set_deadline(std::chrono::system_clock::now() + std::chrono::milliseconds(2000));

    grpc::Status status = stub_->ProcessFrame(&context, request, &response);

    if (status.ok()) {
        for (int i = 0; i < response.detected_objects_size(); ++i) {
            const auto& box = response.detected_objects(i);
            DetectedObject obj;
            obj.x = box.x();
            obj.y = box.y();
            obj.width = box.width();
            obj.height = box.height();
            obj.label = QString::fromStdString(box.label());
            obj.confidence = box.confidence();
            detections.push_back(obj);
        }
    }
    return detections;
}