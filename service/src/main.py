import torch_directml
import pipeline_pb2
import pipeline_pb2_grpc

def main():
    # 1. Test DirectML Hardware Acceleration
    device = torch_directml.device()
    print(f"[AI Backend] Active DirectML Device: {device}")
    
    # 2. Test Protobuf Schema Data Structures
    req = pipeline_pb2.HealthCheckRequest(client_id="qt6_desktop_app")
    resp = pipeline_pb2.HealthCheckResponse(
        is_ready=True,
        cuda_available=False,
        gpu_name=str(device)
    )
    
    print(f"[AI Backend] Protobuf Test Successful!")
    print(f"             Client ID: {req.client_id}")
    print(f"             GPU Status: {resp.gpu_name}")

if __name__ == "__main__":
    main()