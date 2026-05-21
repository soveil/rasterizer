# Software Rasterizer
<img width="800" height="600" alt="Egg flipping animation" src="https://github.com/user-attachments/assets/0bd09b44-1684-4c99-a546-c58b4076d35b" title="test"/>

## Overview
This is a CPU-driven rasterizer built in C++.
It supports rendering basic shapes like spheres and arbitrary polygons, Phong shading, and polygon clipping in clip space.
The images are generated from text files with object and scene descriptions that are parsed with a custom library from my professor.
There are no external dependencies beyond the library for scene parsing.

## Features
- Complete graphics transformation pipeline with homogeneous coordinates (object ⟶ world ⟶ camera ⟶ clip ⟶ device)
- Phong shading with point, directional, and ambient lighting
- Sutherland-Hodgman polygon clipping
- Blinn's line clipping
- Polygon object function (draw object from vertices and faces)
- Sphere, cone, disk, cylinder, and cube functions
- Diffuse, metallic, and plastic shaders
- Scanline rendering via edge tables
- Custom matrix and vector classes

## Examples

### Point lights inside shapes with inverted normals

<img width="640" height="480" alt="pedestals" src="https://github.com/user-attachments/assets/b1e7effe-2fc0-4543-9476-7bb6d466d838" />

### Spheres with increasing specular coefficients

<img width="640" height="480" alt="spheres" src="https://github.com/user-attachments/assets/4dbb995a-a8ff-44cd-8351-a7fa6690a4c4" />

## Future Work
The code in this repository depends on a library from my professor to convert a custom text scene definition file into instructions for the renderer.
The scene files are similar to an extended OBJ file that supports multiple objects and lighting.
To get around this dependency and allow the code here to stand on its own,
I'm planning to replace the library with an OBJ parser and my own custom scene description language with support for including OBJ files, camera movement, and lighting.

Due to the scene parsing dependency, the project can't be built as it's stored in this repository yet.
