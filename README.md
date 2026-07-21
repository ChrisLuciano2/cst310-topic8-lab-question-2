# CST-310 · Topic 8 · Lab Question 2

**Question:** Carefully read the description of Project 8. Create an algorithm for generating a sphere. Post screenshots of your algorithm.

Full write-up (algorithm, flowchart, code walkthrough, screenshots) is in
[`Topic 8 Lab Question 2.docx`](./Topic%208%20Lab%20Question%202.docx).

## Summary

The sphere is built procedurally with the latitude/longitude (UV-sphere)
parameterization: rings of latitude ("stacks") from pole to pole, each
subdivided into wedges of longitude ("slices"). Since the sphere is a
unit sphere centered at the origin, each vertex's outward normal equals
its position. Indices connect adjacent (stack, slice) samples into two
triangles per quad and the mesh is drawn with a single indexed
`glDrawElements` call.

## Contents

- `code/sphere_mesh.cpp` — full OpenGL/GLFW program (mesh generation in `build_sphere()`)
- `code/sphere_mesh_shaders/` — vertex/fragment shaders
- `screenshots/sphere_mesh_filled.png` — Blinn-Phong shaded sphere
- `screenshots/sphere_mesh_wireframe.png` — wireframe view showing the lat/long triangulation

## Build requirements

C++17, GLFW, GLAD, GLM, OpenGL 4.1+. Built with CMake + vcpkg in the
original project; this repo contains just the source for reference.
