# gamecoe - A small C++ Code-Only Game Engine

## Current State - Work in Progress

### gamecoe has been recently tested with OpenGL 3.0-4.6 on Linux (Arch) and worked fine.
### If you find any bugs, please open an issue or contact games@nircoe.com

- **Graphics API**: OpenGL 3.0-4.6 with version-gated features, default version is 4.6.
    - **Set specific version**: Add the following in your CMake files before fetching gamecoe (example for version 3.3):
        ```cmake
        set(GAMECOE_GRAPHICS_VERSION_MAJOR 3)
        set(GAMECOE_GRAPHICS_VERSION_MINOR 3)
        ```
    - **DSA (Direct State Access)** for OpenGL 4.5+ - Stateless buffer operations (`#if GAMECOE_HAS_DSA`)
    - **UBO (Uniform Buffer Object)** for OpenGL 3.1+ - Efficient uniform data sharing (`#if GAMECOE_HAS_UBO`)
    - **Shader Preprocessing**: gamecoe auto-injects `#version` and `#define` macros into your shaders
- **Rendering**: Basic Shape rendering support (only triangle, rectangle and box at the moment)
- **Multi Active Scene Support**: Want to build a bigger world with more than 1 active Scene at a time? 
                                  Want to make the pause menu a separated Scene so your code will be more organized? 
                                  gamecoe supports it!
- **gamecoe structure**: gamecoe holds several namespaces
    - **gamecoe**: The main namespace, built out of a few modules -
        - **gamecoe/core**: `Game`, `Scene`, `Window` classes
        - **gamecoe/entity**: `GameObject`, `Component`, `Transform`, `Camera`, `Renderer`, `ShapeRenderer` classes
        - **gamecoe/graphics**: Mostly internal for gamecoe or for power user usage -
                                `Shader`, `GraphicsBuffer`, `VertexArray` classes
                                (`Texture` class implemented but not yet integrated into a Component)
        - **gamecoe/utils**: Mostly internal for gamecoe
    - **timecoe**: Track the time between frames
        ```cpp
        #include <timecoe.hpp>
        float deltaTime = timecoe::deltaTime();
        ```
    - **colorcoe**: Use one of 140 built-in colors from the colorcoe:: namespace, 
                    or a custom color of your own with the gamecoe::Color class!
        ```cpp
        #include <colorcoe.hpp>
        auto green = colorcoe::green();
        gamecoe::Color myColor(108, 92, 236, 98);
        ```
    - **inputcoe**: Full basic keyboard and mouse input tracking, want to know if a key/button just got clicked? 
                    if it's being held down? what is the position of the mouse? 
                    you can find this functionality and more in the inputcoe:: namespace!
        ```cpp
        #include <inputcoe.hpp>
        if (inputcoe::pressed(inputcoe::Key::Space)) { /* do something */ }
        auto mousePosition = inputcoe::mousePosition();
        if (inputcoe::released(inputcoe::MouseButton::Left)) { /* do something */ }
        ```
- **gamecoe toolkit**: Use a variety of our in-house tools for better game development experience with gamecoe. 
                       All are optional, to avoid bloatware.
    - **logcoe**: Feel free to use our in-house tool [logcoe](https://github.com/nircoe/logcoe) as your logger during development, 
                  logcoe comes built-in with gamecoe!
        Just add `set(GAMECOE_USE_LOGCOE ON)` in your CMake files before fetching gamecoe, and you are good to go!
        ```cpp
        logcoe::debug("Player got 1 coin!");
        logcoe::error("Enemy has negative HP!");
        ```
    - **soundcoe**: Feel free to use our in-house tool [soundcoe](https://github.com/nircoe/soundcoe) for your game sound management, 
                    soundcoe comes built-in with gamecoe!
        Just add `set(GAMECOE_USE_SOUNDCOE ON)` in your CMake files before fetching gamecoe, and you are good to go!
        ```cpp
        soundcoe::playMusic("boss_fight.wav");
        soundcoe::playSound("hit.mp3");
        ```

## gamecoe Workflow

- **Engine Structure**: `Game` → `Scene` → `GameObject` → `Component` classes workflow

### GameObject/Component Lifecycle
```
    initialize()              Called when GameObject/Component is created
        ↓
    begin()                   Called at the beginning of Scene activation
        ↓
    activate() ←────────╮     Called when GameObject/Component is activated
        ↓               │
        ├─→ update() ←╮ │     Called every frame while active
        ├─→ render() ─╯ │     Called every frame if GameObject has Renderer, or Component is a Renderer
        ↓               │
    deactivate() ───────╯     Called when GameObject/Component is deactivated
```

### Scene Lifecycle
```
    load() ←───────────────╮  Called when Scene is loaded
        ↓                  │
    activate() ←────────╮  │  Called when Scene is activated
        ↓               │  │
        ├─→ update() ←╮ │  │  Called every frame while active
        ├─→ render() ─╯ │  │  Called every frame while active
        ↓               │  │
    deactivate() ───────╯  │  Called when Scene is deactivated
        ↓                  │
    unload() ──────────────╯  Called when Scene is unloaded
```

## Build Requirements

- **C++20**
- **CMake 3.20+**
- **Python3**: for GLAD2 generation

## Platform Support

- **Linux**: Developed and tested on Linux (Arch)
- **Windows**: Not tested yet
- **MacOS**: Not tested yet

## OpenGL Version Support

- **Officially Supported**: OpenGL 3.0+
- **Tested Compatibility**: May work on OpenGL 2.0+, but not officially supported

The engine is designed for OpenGL 3.0+ core profile. Older versions may work but are not guaranteed.

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
- **[logcoe](https://github.com/nircoe/logcoe)**: In-house logger (Optional)
- **[soundcoe](https://github.com/nircoe/soundcoe)**: In-house sound manager (Optional)

## Roadmap

- **Collider**: Collider Component for GameObjects collision detection and handling
- **More Basic Shapes**: Circle, Sphere and more
- **Batch Rendering**: Introduce batch rendering system for better performance
- **SpriteRenderer**: New Renderer Component for Sprite rendering
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