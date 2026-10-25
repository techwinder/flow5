# flow5
flow5 is a potential flow solver with built-in pre- and post- processing functionalities. Its purpose is to make the preliminary designs of wings, planes, hydrofoils and sails reliable, fast and user-friendly.

It is version 7 of the legacy project xflr5.


# Remaining developments 
- Fix bugs and implement minor improvements depending on user requests
- ~~Consider adding force location to extra-drag input~~
- Continue to clean the code and to improve the API 
- Clean the documentation

# API 

## Implementation
There are no more major changes expected to the source code and the API. Occasional deprecation and changes to the interfaces may still occur if required to fix issues.

## Examples
- XFoilRun: Construction of a NACA foil and calculation of a type 1 polar using XFoil.
- XFoilBatchRun: Construction of a NACA foil and multi-threaded calculation of a 2d polar mesh.
- PlaneRun1: Construction of an xfl-type plane with no fuselage and calculation of a type 2 polar with viscous drag evaluated on the fly. Shows how to use custom units.
- PlaneRun2: Construction of an xfl-type plane with a NURBS type fuselage, construction of a conforming mesh, and calculation of a type 2 polar.
- PlaneRun3: Import of an stl-type plane from a mesh file, configuration of the trailing edge and calculation of a type 1 polar.
- PlaneRun4: Import of a plane and a polar from xml files, construction of a conforming mesh, and calculation of a type 1 polar.
- PlaneRun5: Import of an STL-type fuselage into an xfl-type plane.
- PlaneRun6: Import of a fuselage from a STEP file into an xfl-type plane.
