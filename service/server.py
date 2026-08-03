import concurrent.futures
import threading
import grpc
import cv2
import numpy as np
from ultralytics import YOLO

import pipeline_pb2
import pipeline_pb2_grpc

def create_tracker():
    """Instantiate OpenCV CSRT tracker safely across different cv2 versions."""
    if hasattr(cv2, 'TrackerCSRT_create'):
        return cv2.TrackerCSRT_create()
    elif hasattr(cv2, 'legacy') and hasattr(cv2.legacy, 'TrackerCSRT_create'):
        return cv2.legacy.TrackerCSRT_create()
    elif hasattr(cv2, 'TrackerMIL_create'):
        return cv2.TrackerMIL_create()
    else:
        raise RuntimeError("No compatible OpenCV tracker found.")

class ObjectForgePipelineServicer(pipeline_pb2_grpc.ObjectForgePipelineServicer):
    def __init__(self):
        print("[AI Backend] Initializing Nano model (NO HUMAN allowed mode)...")
        self.model = YOLO("yolov8n.pt")
        
        self.lock = threading.Lock()
        self.tracker = None
        self.tracking_active = False
        self.pending_bbox = None
        
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

        if frame is None:
            return response

        with self.lock:
            # Check if a lock request was recently received
            if self.pending_bbox is not None:
                try:
                    self.tracker = create_tracker()
                    self.tracker.init(frame, self.pending_bbox)
                    self.tracking_active = True
                except Exception as e:
                    print(f"[AI Backend] Tracker initialization failed: {e}")
                    self.tracking_active = False
                    self.tracker = None
                finally:
                    self.pending_bbox = None

            # 1. Continuous Object Tracking Mode
            if self.tracking_active and self.tracker is not None:
                success, bbox = self.tracker.update(frame)
                if success:
                    x, y, w, h = [int(v) for v in bbox]
                    obj = response.detected_objects.add()
                    obj.x = max(0, x)
                    obj.y = max(0, y)
                    obj.width = w
                    obj.height = h
                    obj.label = "TARGET LOCKED"
                    obj.confidence = 1.0
                    return response
                else:
                    # Lost track of target -> Fallback to YOLO scan
                    self.tracking_active = False
                    self.tracker = None

            # 2. Standard Candidate Detection Scan (YOLO)
            results = self.model(frame, conf=0.25, verbose=False)[0]

            for box in results.boxes:
                if int(box.cls[0]) == self.person_class_id:
                    continue

                x1, y1, x2, y2 = map(int, box.xyxy[0])
                conf = float(box.conf[0])

                bbox_msg = response.detected_objects.add()
                bbox_msg.x = x1
                bbox_msg.y = y1
                bbox_msg.width = x2 - x1
                bbox_msg.height = y2 - y1
                bbox_msg.label = "Scan Target"
                bbox_msg.confidence = conf

        return response

    def LockTarget(self, request, context):
        with self.lock:
            if request.width > 0 and request.height > 0:
                self.pending_bbox = (request.x, request.y, request.width, request.height)
                self.tracking_active = False
                return pipeline_pb2.StatusUpdate(message="Target Lock Initiated")
            return pipeline_pb2.StatusUpdate(message="Invalid Bounding Box")

    def GetPipelineStatus(self, request, context):
        yield pipeline_pb2.StatusUpdate(
            state=pipeline_pb2.StatusUpdate.State.IDLE,
            message="ObjectForge Target Locator Ready (localhost:50051)",
            progress_percentage=0.0
        )

def serve():
    server = grpc.server(concurrent.futures.ThreadPoolExecutor(max_workers=8))
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