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
        print("[AI Backend] Initializing Nano model (NO HUMAN allowed mode)...")
        self.model = YOLO("yolov8n.pt")
        # Pre-cache class names to easily find 'person' ID
        self.person_class_id = None
        for id, name in self.model.names.items():
            if name.lower() == 'person':
                self.person_class_id = id
                break
        print(f"[AI Backend] Person exclusion active (Class ID: {self.person_class_id})")

    def ProcessFrame(self, request, context):
        np_arr = np.frombuffer(request.image_data, np.uint8)
        frame = cv2.imdecode(np_arr, cv2.IMREAD_COLOR)

        response = pipeline_pb2.FrameResponse(timestamp_ms=request.timestamp_ms)

        if frame is not None:
            # Drop confidence filter very low to detect ANY shape, verbose=False
            results = self.model(frame, conf=0.20, verbose=False)[0]

            for box in results.boxes:
                # CRITICAL: If the detected object is classified as a person, 
                # discard it immediately!
                if int(box.cls[0]) == self.person_class_id:
                    continue

                x1, y1, x2, y2 = map(int, box.xyxy[0])
                conf = float(box.conf[0])

                bbox = response.detected_objects.add()
                bbox.x = x1
                bbox.y = y1
                bbox.width = x2 - x1
                bbox.height = y2 - y1
                # Return agnostic label
                bbox.label = "Scan Target" 
                bbox.confidence = conf

        return response

    def GetPipelineStatus(self, request, context):
        yield pipeline_pb2.StatusUpdate(
            state=pipeline_pb2.StatusUpdate.State.IDLE,
            message="ObjectForge Target Locator Ready (localhost:50051)",
            progress_percentage=0.0
        )

def serve():
    server = grpc.server(concurrent.futures.ThreadPoolExecutor(max_workers=4))
    pipeline_pb2_grpc.add_ObjectForgePipelineServicer_to_server(
        ObjectForgePipelineServicer(), server
    )
    server.add_insecure_port('[::]:50051')
    print("[AI Service] Server listening on localhost:50051")
    server.start()
    try:
        server.wait_for_termination()
    except KeyboardInterrupt:
        server.stop(0)

if __name__ == '__main__':
    serve()