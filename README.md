# ForgeSlice

A fast, advanced macOS 3D slicer project designed around the FlashForge Adventurer 5M Pro.

## Vision

ForgeSlice aims to combine a streamlined Orca-like workflow with progressive Basic → Advanced → Expert settings, an extensive infill/pattern library, fast non-blocking slicing, Adventurer 5M Pro profiles, and an extensible camera center.

## Planned features

- FlashForge Adventurer 5M Pro first-class printer profile
- STL, 3MF, OBJ and additional mesh/project formats
- Fast C++ slicing backend
- Basic / Advanced / Expert settings modes
- Large infill and solid-pattern library, including structural and experimental patterns
- Random and configurable seam strategies
- Variable layer height and advanced wall/support controls
- Integrated G-code preview and printer workflow
- Extensible camera providers for USB, MJPEG, RTSP and ONVIF sources
- macOS Intel and Apple Silicon support
- Homebrew distribution

## Architecture

The project is intended to use a proven open-source slicing core rather than reinventing the geometry engine. The UI, printer profiles, settings system, infill extensions, camera layer, and macOS packaging will be developed as ForgeSlice-specific components.

## Status

🚧 Early project setup. The production slicer engine and UI are not implemented yet.

## License

The final license will be selected based on the licenses of the upstream slicing components and ForgeSlice-specific code.
