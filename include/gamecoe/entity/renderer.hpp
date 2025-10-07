/*
 * IMMEDIATE TODO LIST - Renderer System Implementation

**** FIRST OF ALL - 
    I want to implement basic renderer for primitive shapes first before complex meshes, 
    for the basic main.cpp opengl scene with triangles, rectangles and boxes.

 * 
 * Phase 1: Base Renderer Class Design
 * 1. Create abstract Renderer base class that inherits from ComponentBase
 * 2. Define pure virtual render() method for polymorphic rendering
 * 3. Consider whether Renderer should be a Component or separate system
 * 4. Design integration with existing graphics classes (Shader, Mesh, Material, Texture)
 * 
 * Phase 2: MeshRenderer Implementation  
 * 5. Implement MeshRenderer class that inherits from Renderer
 * 6. Integrate with Transform component for model matrix
 * 7. Support for multiple meshes/materials per renderer
 * 8. Handle shader uniform setting (model, view, projection matrices)
 * 
 * DESIGN QUESTIONS TO RESOLVE:
 * - Should Renderer be a Component or a separate rendering system?
 * - How to handle multiple materials per mesh renderer?
 * - Camera/view matrix passing - parameter vs global state?
 * - Render queue/sorting for transparency and performance?
 */

#pragma once
