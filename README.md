# gamecoe - A small C++ Code-Only Game Engine

## Current State - Work in Progress

- **Graphics API**: Currently supports only OpenGL 3.3
- **Rendering**: Basic Shape rendering support (only triangle, rectangle and box at the moment)
- **Full Engine Structure**: Game->Scene->GameObject->Component workflow
- **GameObject/Component Life Cycle**: 
initialize->begin->activate->update+render (repeat once a frame while active)->deactivate
- **Scene Life Cycle**: load->activate->update+render (repeat once a frame while active)->deactivate->unload
- **Multi Active Scene Support**: Want to build a bigger world with more than 1 active Scene at a time? Want to make the pause menu a separated Scene so your code will be more organized? gamecoe supports it!
- **Components**: Currently our built-in Components: Transform, Renderer, ShapeRenderer, Camera
- **logcoe**: Feel free to use our in-house [logcoe](https://github.com/nircoe/logcoe) as your logger during development, logcoe comes built-in with gamecoe!
```cpp
logcoe::debug("Player got 1 coin!");
logcoe::error("Enemy has negative HP!");
```
- **timecoe**: Track the time between frames: 
```cpp
float deltaTime = timecoe::deltaTime();
```
- **colorcoe**: Use one of 140 built-in colors from the colorcoe:: namespace, or a custom color of your own with the gamecoe::Color class!
```cpp
auto green = colorcoe::green();
gamecoe::Color myColor(108, 92, 236, 98);
```
- **inputcoe**: Full basic keyboard and mouse input tracking, want to know if a key/button just got clicked? if it's being held down? what is the position of the mouse? you can find this functionality and more in the inputcoe:: namespace!
```cpp
if (inputcoe::pressed(inputcoe::Key::Space)) { /* do something */ }
auto mousePosition = inputcoe::mousePosition();
if (inputcoe::released(inputcoe::MouseButton::Left)) { /* do something */ }
```

## Build Requirements

- **C++20**
- **CMake 3.20+**
- **Python3**: for GLAD2 generation

## Platform Support

- **Linux**: Developed and tested on Linux (Arch)
- **Windows**: Not tested yet
- **MacOS**: Not tested yet

## Quick Start

### [gencoe](https://github.com/nircoe/gencoe) - Python-based Generator for gamecoe!

gencoe will generate a new ready-to-go project with gamecoe examples for you, and will help you to create new scenes and components faster and easier!

Please look at [gencoe README](https://github.com/nircoe/gencoe) for more details.

First of all, for any help with gencoe, please run:
```bash
# General gencoe help
gencoe --help
# New Project generation help
gencoe init --help
# New Scene generation help
gencoe scene --help
# New Component generation help
gencoe component --help
```

Use [gencoe](https://github.com/nircoe/gencoe) to generate a new project with examples on how to use gamecoe in main.cpp:
```bash
# In order to generate a new project in the current directory:
gencoe init MyGame
# In order to generate a new project in a specific path:
gencoe init MyGame -p /path/to/MyGame
```

Later on use [gencoe](https://github.com/nircoe/gencoe) in order to generate scenes and components for your game!
```bash
# Generate the assets directories for a new Scene and code snippet for it
gencoe scene MainMenu
# Generate .hpp/.cpp files for a new Component
gencoe component PlayerController
# Generate .hpp/.cpp files for a new Component that inherits from another Component
gencoe component CustomRenderer -i Renderer
# Generate .hpp/.cpp files for a new Component inside a namespace
gencoe component Monster -n Enemies
```

## Dependencies

- **[GLFW](https://github.com/glfw/glfw)**: Window and Input handling
- **[GLAD](https://github.com/Dav1dde/glad)**: OpenGL Context
- **[GLM](https://github.com/g-truc/glm)**: Math library
- **[logcoe](https://github.com/nircoe/logcoe)**: In-house logger

## Roadmap

- **Collider**: Collider Component for GameObjects collision detection and handling
- **More Basic Shapes**: Circle, Sphere and more
- **Support for Modern OpenGL**: Introduce support for OpenGL4.6 features
- **Batch Rendering**: Introduce batch rendering system for better performance
- **SpriteRenderer**: New Renderer Component for Sprite rendering
- **soundcoe**: Integrate [soundcoe](https://github.com/nircoe/soundcoe) for sound effects and music management
- **datacoe**: Integrate [datacoe](https://github.com/nircoe/datacoe) for save/load data management system with optional encryption
- **Example Projects**: Add small games created with gamecoe as examples
- **Thread Safety**: Make gamecoe thread-safe and benefit from parallel performance enhancement
- **Text Rendering**: Font loading, text rendering
- **uicoe**: Built-in factory methods for basic UI GameObjects such as Button, Textbox, and more
- **poolcoe**: Built-in Object Pool system
- **GameObject parent-child tracking**: Two-way tracking
- **Better Documentation and Logging**
- **mathcoe**: In-house math library, to replace glm

## MIT License - See LICENSE for details