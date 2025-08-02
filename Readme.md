# Overview
This is a minimal dependency, data oriented game engine written in the C99 standard. Most other off the shelf game engines rely heavily on inheritance based design which can result in bloat and sub-optimal performance. In contrast, this engine has a strong emphasis on data oriented design and it's entity component system. This allows for increased code reuse, easily threadable workloads, better hardware utilization, and higher performance. 
The philosophy of this engine is to minimize bloat and overhead incurred through the use of non-required library features and maticulously manages all function calls. For this reason, C is the primary language since it incentivises explicity management of memory and structures unlike C++ which may obscure how memory is allocated. Additionally, this project uses Vulkan as its main graphics api to allow for higher performant graphics and better hardware usage. 

# Features
- Cross platform api
- Vulkan 1.2 rendering backend
- Flexible Entity Component System
- Custom memory system 
- Automatic allocation tracking in debug mode
- Fast startup times

# Building
```
    git clone [repo] --recurse-submodule
    cd spark && mkdir build && cd build
    cmake .. 
    make
```

# Limitations
This is an experimental engine that is not production ready since many features have not yet been implemented. These features will likely not be implemented soon until my current project in Godot is complete. The core systems that are still required are:
- [ ] Editor
- [ ] Scene Loading
- [ ] Sound System

# Todo
- [ ] Custom allocator for memory system
- [ ] String library
- [ ] ECS Refactoring
- [x] Models
- [x] ECS
- [x] Split materials and shaders
- [x] Resource Systems
    - [x] Mesh System
    - [x] Shader system
    - [x] Material system
    - [x] Texture system
- [ ] Renderer
    - [ ] Skybox
    - [ ] Lights
    - [ ] Shadows
    - [ ] UI
        - [x] 2D Geometry
        - [ ] Events (Click, drag, etc.)
        - [ ] Buttons
        - [ ] Text Rendering
        - [ ] Text Input
    - [ ] Vulkan Allocator
    - [ ] Materials
        - [x] Create
        - [ ] Destroy
    - [ ] Post processing
- [ ] Scenes
    - [ ] Serialization
    - [ ] Loading
    - [ ] Deserialization
- [x] Physics
- [ ] Editor
- [x] Dynamically update camera projection matrix on screen resize

