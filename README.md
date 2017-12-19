# UrchinEngine
Game engine library: 3D, Physics, Sound, IA, Map Editor

Video: https://www.youtube.com/watch?v=lSbCx7u6RtE

- 3D Engine:
  - Dynamic shadow (cached parallel split shadow maps, variance shadow map, blur)
  - Dynamic lighting (deferred shading): omni-directional & directional
  - Terrain multi-layered
  - Animate 3d models reader
  - Normal mapping
  - Ambient occlusion (HBAO)
  - Anti-aliasing (FXAA)
  - Space partitioning (octree)
  - UI 2D: button, window, text (TTF reader), text input, bitmap (TGA reader), slider
  - Skybox

- Physics Engine:
  - Supported shapes: sphere, box, cylinder, capsule, cone, convex hull, compound shape, terrain
  - Supported body: rigid body
  - Algorithms:
    - Broadphase: AABB Tree
    - Narrowphase: sphere-sphere, sphere-box, GJK/EPA, concave-convex
    - Constraint solver (iterative): external force, friction, restitution, inertia
  - Continuous collision detection (CCD)
  - Island
  - Ray cast

- Sound Engine:
  - Ambience sound and 3D sound
  - Streaming
  - Sound trigger in function of character position

- IA Engine:
  - Automatic navigation mesh generator
  - Pathfinding A* with funnel algorithm
  
- Map Editor:
  - Add > 3d model, rigid body and sound on scene
  - Debug > view light scope, hitbox, sound trigger, navigation mesh
