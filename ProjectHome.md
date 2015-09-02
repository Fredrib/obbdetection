**_A physics and rendering engine to showcase a practical implementation of the SIGGRAPH paper: "OBBTree: A Hierarchical Structure for Rapid Interference Detection" (1996)._**

**CODE HIGHLIGHTS:**

- Performs offline covariance-based OBB fitting of each meshes' convex hull to form the full OBB Tree.  Code is "mildly" optimized for reasonable runtime.

- A mixture of different convex hull algorithms was used to tradeoff speed Vs. numerical robustness. CGAL, qhull and StanHull were all used to varying degrees.  These packages dominate running time of OBB generation code.

- Performs runtime collision detection of OBB nodes using Gottschalk's algorithm based on the separating axis theorem.

- Triangle - Triangle detection is implemented using Tomas Moller's "A Fast Triangle-Triangle Intersection Test".

- A simple RK4 integrator with quaternion based rotations is implemented to test collision detection system.

- A simple rendering engine in DirectX, with a simple shadow-map is implemented.

**COMPILE NOTES:**

- Program was compiled under Visual Studio 2008, C++ and using DirectX SDK (June 2010).

- All required dependencies should be linked in via visual studio project properties, with the exception of DirectX.  You will need to go to VS Tools Menu -> Options -> Projects and Solutions -> VC++ Directories and include a link to the DirectX SDK include and library directories to compile.  See "\obbdetection\obbdetection\COMPILE\_NOTES.txt" for more details.

- For a list of known bugs and future work see: "\obbdetection\obbdetection\TO\_BE\_DONE.txt"