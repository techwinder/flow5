# flow5
flow5 is a potential flow solver with built-in pre- and post- processing functionalities. Its purpose is to make preliminary designs of wings, planes, hydrofoils and sails reliable, fast and user-friendly.

It is version 7 of the legacy project xflr5.


# Remaining developments 
- Fix bugs and implement minor improvements depending on user requests.
- ~~Remove the dependency of flow5-lib on Qt6-core.~~ _[done (https://github.com/techwinder/flow5/commit/0f7d7d28f78e2f881f8390ec250c300d897fc965)]_
- Add fuselage conforming mesh capability to <tt>flow5-lib</tt> and to the API.
- Either complete or remove currently inoperative features, e.g. plane optimization, advanced editors.
- Continue to clean the code and to improve the API.
- Clean the documentation.

**Note:** 

The API is still in an experimental state and subject to change. The intent is to stabilize both the source code and the API by the end of 2026. Until then, expect major refactoring and breakage of interfaces.
