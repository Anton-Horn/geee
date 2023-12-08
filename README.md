# My Game Application

This repository contains an application utilizing the `enginecore` as its game engine.

## Overview

The application integrates the `enginecore` as its underlying game engine to handle core functionalities.

### Features

- Engine-based rendering and logic handling.
- Integration of entt for Entity Component System (ECS).
- Native C++ scripting for logic implementation.

## Getting Started

### Prerequisites

- **Vulkan SDK:** Ensure the Vulkan SDK is installed on your system.

### Setup

1. **Clone the Repository:** 
    ```bash
    git clone https://github.com/your/repository.git
    ```

2. **Building the Application:**
   - Ensure `enginecore` is properly linked and included in the project.

3. **Project Structure:**
   ```
   MyApplication/
   |-- src/
   |   |-- main.cpp
   |   |-- YourOtherSourceFiles.cpp
   |-- enginecore/
   |   |-- (EngineCoreSourceFiles)
   |-- engineeditor/
   |   |-- (EditorSourceFiles)
   |-- assets/
   |   |-- (EngineAssets)
   |-- other_folders/
   |   |-- (OtherFolders)
   |-- generate.bat
   |-- README.md
   ```

4. **Building Steps:**
   - Integrate `enginecore` into your build system (Visual Studio, Premake, etc.).
   - Link necessary include paths and library dependencies for `enginecore` in your project.

## Usage

### Initializing the Engine

```cpp
#include "enginecore/Application.h"

// ...

ec::ApplicationCreateInfo createInfo;
createInfo.createCallback = create;
createInfo.updateCallback = update;
createInfo.terminateCallback = terminateApplication;
createInfo.windowCreateInfo = { 1280, 720, "editor" };

ec::Application app;
app.create(createInfo);
```

### Utilizing the Engine

- Once initialized, use engine functionalities to load scenes, manage assets, and implement game logic.
- Utilize entt for Entity Component System (ECS) operations within your application.

### Native C++ Scripting

- Implement game logic and behavior using native C++ scripting within your application.

## Contributing

Contributions are welcome! If you'd like to contribute, please follow these steps:

1. Fork the repository.
2. Create a new branch for your feature or bug fix.
3. Commit your changes.
4. Push your changes to your fork.
5. Submit a pull request.

## License

This project is licensed under the MIT License - see the [LICENSE](LICENSE) file for details.
