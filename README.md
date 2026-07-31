# ObjectForge 🛠️

> **Turn real-world physical objects into textured 3D models using just a webcam.**

ObjectForge is a fast, lightweight, and completely local desktop application designed for everyday users to scan physical tabletop objects (cups, tools, toys, shoes, household items) into 3D assets.

Unlike traditional heavy photogrammetry software, ObjectForge hides complex computer vision metrics behind a modern camera-style UI with real-time dynamic coverage guidance.

---

## Key Principles & Architecture

* **100% Local & Offline:** No cloud API keys, no subscriptions, and no heavy server dependencies.
* **Lightweight & Fast:** C++20 Qt6 desktop UI powered by a native C++ classical 3D vision engine and an ONNX/DirectML PyTorch AI backend.
* **Targeted AI Integration:** AI is strictly used where it excels (YOLOv8 object detection, MediaPipe hand gesture selection). Heavy generative models, NeRFs, or LLMs are intentionally omitted.
* **Classical CV Engine:** All 3D photogrammetry—Feature Extraction (ORB/SIFT), Structure-from-Motion (SfM pose estimation), Poisson Mesh Reconstruction, and UV Texture Mapping—runs via optimized native algorithms (`OpenCV`, `Open3D`, `PCL`).
* **Focus & Scope:** Specially optimized for tabletop and handheld objects. *Scope excludes rooms, buildings, or large environments.*

---

## Tech Stack

* **Frontend UI:** C++20 / Qt6 (Widgets + OpenGL Viewport)
* **AI Engine:** Python 3.11 / PyTorch (DirectML GPU Acceleration) / YOLOv8 / MediaPipe
* **IPC Bridge:** gRPC / Protocol Buffers (Zero-copy local socket streaming)
* **3D Vision Pipeline:** OpenCV / Open3D / Poisson Surface Reconstruction
* **Build System:** CMake + `vcpkg` (Manifest Mode)

---

## Development Roadmap & Progress

- [x] **Phase 01: Environment Lockdown**
  - Configured Python DirectML virtual environment & vcpkg C++ manifest.
- [x] **Phase 02: C++ Architecture & IPC Setup**
  - Set up CMake target structure and dependencies (`qtbase`, `opencv4`, `grpc`, `protobuf`).
- [x] **Phase 03: Base Native Desktop Shell**
  - Built dark-themed Qt6 application frame with video preview placeholder and status monitoring.
- [x] **Phase 04: Local gRPC Communication Bridge**
  - Streaming camera feeds and detection bounding boxes between Qt UI and Python AI engine.
- [ ] **Phase 05: Guided Object Selection Pipeline**
  - Double-tap selection, green object overlay highlight, and user confirmation modal.
- [ ] **Phase 06: Dynamic Coverage Guidance**
  - Real-time sparse spatial coverage estimation giving camera position hints to the user.
- [ ] **Phase 07: Classical 3D Reconstruction Pipeline**
  - SfM camera pose estimation, Poisson surface mesh generation, and UV texture mapping.
- [ ] **Phase 08: Interactive Viewport & Export**
  - Integrated 3D mesh previewer with simple 1-click export (`.OBJ`, `.GLTF`, `.STL`) and collapsible Advanced Settings panel.

---

## Building from Source

### Prerequisites
* Visual Studio 2022 (C++ Desktop Development workload)
* CMake 3.25+
* Python 3.11+
* Git

### Setup & Compilation

```powershell
# 1. Clone repository
git clone [https://github.com/YOUR_USERNAME/ObjectForge.git](https://github.com/YOUR_USERNAME/ObjectForge.git)
cd ObjectForge

# 2. Configure & build C++ application
mkdir build
cd build
cmake .. -G "Visual Studio 17 2022" -A x64 -DCMAKE_TOOLCHAIN_FILE="$env:VCPKG_ROOT/scripts/buildsystems/vcpkg.cmake"
cmake --build . --config Release

# 3. Launch desktop app
.\app\Release\ObjectForgeApp.exe
