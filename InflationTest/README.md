# Inflation Test

## References

### Marching Cubes
  * https://en.wikipedia.org/wiki/Marching_cubes
  * http://paulbourke.net/geometry/polygonise/

## Parameters

  * `TAB`: Toggles fullscreen.
  * `` ` ``: Toggles the GUI panels.
  * Hit the buttons in the top-right to save/load settings. Note that GUI panels are independent and that save/load only affects their own parameters.

### Marching Cubes
  * `resolution`: Resolution of the space in all dimensions (x, y, z).
  * `scale`: Scale at which to render the space.
  * `threshold`: Cutoff value at which to consider whether a point is above or below the isosurface.
  * `radial clip`: (Broken – Ignore for now)
  * `fill edges`: Whether or not to close the shapes at the edge of the space.
  * `flip normals`: Affects normal shading, purely decorative for now.
  * `smooth`: Smoothes out the shape by using vertex normals instead of face normals.

### Noise Field
  * `num scales`: (Broken – Ignore for now)
  * `curr scale`: (Broken – Ignore for now)
  * `offset multiplier`: (Broken – Ignore for now)
  * `input multiplier`: Multiplier applied when sampling the noise function. Affects the density of the grid.
  * `noise speed in`: Time step speed for values inside the boundary.
  * `noise speed out`: Time step speed for values outside the boundary.
  * `boundary radius`: Radius of the boundary sphere.

### Render
  * `debug`: Renders the noise field as a 3D grid of cubes. Needs optimization, runs really slow.
  * `draw grid`: Draws the grid representing the space.
  * `wireframe`: Whether to draw the mesh as wireframe or filled.
  * `shade normals`: Applies normal shading, purely decorative.

