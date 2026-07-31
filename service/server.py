import concurrent.futures
import time
import grpc
import cv2
import numpy as np
from ultralytics import YOLO

import pipeline_pb2
import pipeline_pb2_grpc

class ObjectForgePipelineServicer(pipeline_pb2_grpc.ObjectForgePipelineServicer):
    def __init__(self):
        print("[AI Service] Initializing ObjectForge AI Engine with YOLOv8...")
        self.model = YOLO("yolov8n.pt")  # Auto-downloads lightweight YOLO weights on first run
        print("[AI Service] YOLOv8 Engine Ready!")

    def ProcessFrame(self, request, context):
        # Decode raw frame bytes
        frame_bytes = np.frombuffer(request.image_data, dtype=np.uint8)
        frame = cv2.imdecode(frame_bytes, cv2.IMREAD_COLOR)

        response = pipeline_pb2.FrameResponse(timestamp_ms=request.timestamp_ms)

        if frame is not None:
            results = self.model(frame, verbose=False)[0]
            for box in results.boxes:
                x1, y1, x2, y2 = map(int, box.xyxy[0])
                conf = float(box.conf[0])
                cls_id = int(box.cls[0])
                label = self.model.names[cls_id]

                bbox = response.detected_objects.add()
                bbox.x = x1
                bbox.y = y1
                bbox.width = x2 - x1
                bbox.height = y2 - y1
                bbox.label = label
                bbox.confidence = conf

        return response

    def GetPipelineStatus(self, request, context):
        yield pipeline_pb2.StatusUpdate(
            state=pipeline_pb2.StatusUpdate.State.IDLE,
            message="YOLOv8 AI Engine Ready over gRPC (localhost:50051)",
            progress_percentage=0.0
        )

def serve():
    server = grpc.server(concurrent.futures.ThreadPoolExecutor(max_workers=4))
    pipeline_pb2_grpc.add_ObjectForgePipelineServicer_to_server(
        ObjectForgePipelineServicer(), server
    )
    server.add_insecure_port('[::]:50051')
    print("[AI Service] ObjectForge gRPC server running on localhost:50051...")
    server.start()
    try:
        server.wait_for_termination()
    except KeyboardInterrupt:
        server.stop(0)

if __name__ == '__main__':
    serve()