# flow5
flow5 is a potential flow solver with built-in pre- and post- processing functionalities. Its purpose is to make preliminary designs of wings, planes, hydrofoils and sails reliable, fast and user-friendly.

It is version 7 of the legacy project xflr5.


# Remaining developments 
- Fix bugs and implement minor improvements depending on user requests
- ~~Add fuselage conforming mesh capability to flow5-lib and to the API~~
- ~~Remove the dependency of flow5-lib on Qt6-core~~
- ~~Add plane imports from STL or OBJ meshes to the API~~
- Add fuselage imports from STL or OBJ meshes to the API
- Add fuselage imports from STEP files to the API
- Either complete or remove currently inoperative features, e.g. plane optimization, advanced editors
- Continue to clean the code and to improve the API 
- Clean the documentation

# API 

## Implementation
Please note that the API is still in an experimental state and subject to change. 

The intent is to stabilize both the source code and the API by the end of 2026. Until then, expect major refactoring and breakage of interfaces.

## Examples
1. XFoilRun: Construction of a NACA foil and calculation of a type 1 polar using XFoil.
2. PlaneRun1: Construction of an xfl-type plane with no fuselage and calculation of a type 2 polar with viscous drag evaluated on the fly.
3. PlaneRun2: Construction of an xfl-type plane with a NURBS type fuselage, construction of a conforming mesh, and calculation of a type 2 polar.
4. PlaneRun3: Import of an stl-type plane from a mesh file, configuration of the trailing edge and calculation of a type 1 polar.
