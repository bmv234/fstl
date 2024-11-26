# Changelog

## Qt6 Migration Changes

### Build System Changes
1. CMakeLists.txt updates:
   - Increased minimum CMake version to 3.16 (required for Qt6)
   - Changed Qt5 find_package to Qt6
   - Updated Qt5::OpenGL to Qt6::OpenGLWidgets
   - Added Qt6-specific install paths for Windows
   - Made XKB optional on Linux systems
   - Updated C++ standard to C++17

### Code Changes
1. Signal/Slot Syntax:
   - Updated old-style SIGNAL/SLOT macros to new pointer-to-member syntax in:
     * shaderlightprefs.cpp
     * window.cpp

2. Container Classes:
   - Changed QVector to QList in loader.cpp (Qt6 deprecates QVector in favor of QList)
   - Updated container-related method calls to use QList

3. Keyboard Shortcuts:
   - Updated keyboard shortcut syntax in window.cpp:
     * Old: `Qt::CTRL + Qt::SHIFT + Qt::Key_C`
     * New: `QKeySequence(Qt::CTRL | Qt::SHIFT | Qt::Key_C)`

4. OpenGL Integration:
   - Updated OpenGL includes to use Qt6 paths
   - Updated OpenGL widget base class includes
   - Updated OpenGL function calls to Qt6 equivalents

5. Include Paths:
   - Updated Qt include paths to match Qt6 module organization
   - Removed deprecated QtOpenGL includes
   - Added specific widget includes where needed

### Header File Updates
1. canvas.h:
   - Updated OpenGL includes for Qt6
   - Added missing Qt widget includes

2. shaderlightprefs.h:
   - Added explicit widget includes
   - Updated forward declarations

3. window.h:
   - Updated Qt includes to match Qt6 module organization
   - Added missing widget includes

### Documentation
1. README.md:
   - Added note about Qt6 update
   - Updated build requirements
   - Updated build instructions for Qt6
   - Added Qt6-specific dependency information

### Dependencies
1. Added:
   - Qt6::OpenGLWidgets (replaces Qt5::OpenGL)
   - CMake 3.16 or higher
   - C++17 compiler support

2. Removed:
   - Qt5 dependencies
   - Older CMake support

### Known Issues
- XKB warning appears during build on Linux systems (can be safely ignored)
- Some Qt6 features might require additional system libraries on Linux

### Testing
All core functionality has been verified to work with Qt6:
- STL file loading and rendering
- All view modes (perspective/orthographic)
- Shader preferences
- File drag-and-drop
- Auto-reload capability
- Screenshot functionality
