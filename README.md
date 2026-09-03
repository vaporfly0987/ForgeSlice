# ForgeSlice

A fast, advanced macOS 3D slicer focused on the FlashForge Adventurer 5M Pro.

## Current MVP

ForgeSlice has a dependency-free C++ slicing backend that can load ASCII or binary STL meshes, create Z slices, reconstruct closed contours, generate multiple perimeter passes, generate solid top/bottom regions, calculate filament extrusion, and emit test G-code.

### 26 built-in infill choices

`rectilinear` · `grid` · `lines` · `zigzag` · `cross` · `cross3d` · `triangles` · `trihexagon` · `honeycomb` · `hexagon` · `cubic` · `cubic-subdivision` · `octet` · `gyroid` · `concentric` · `hilbert` · `archimedean-chords` · `stars` · `scaffolding` · `lightning` · `adaptive-lines` · `adaptive-cubic` · `voronoi` · `random` · `hilbert3d` · `sierpinski`

Patterns are selectable with `--pattern`; `--list-patterns` prints the library.

## CLI

```text
ForgeSlice model.stl [options]
  -o <file>                 Output G-code path
  --layer-height <mm>       Layer height
  --first-layer <mm>        First layer height
  --walls <n>               Wall count
  --infill <percent>        Infill density
  --pattern <name>          Infill pattern
  --bottom-layers <n>       Solid bottom layers
  --top-layers <n>          Solid top layers
  --retraction <mm>         Retraction setting
  --speed <mm/s>            Print speed
  --travel <mm/s>           Travel speed
  --list-patterns           List all patterns
```

Example: `ForgeSlice duck.stl -o duck.gcode --walls 5 --infill 35 --pattern gyroid`

## Important status

The backend is functional as an **MVP/test slicer**, not yet a production replacement for OrcaSlicer/FlashPrint. Exact polygon offsets, robust polygon clipping, supports, true retraction moves, 3MF/OBJ import, printer upload, preview rendering, and camera integration still need production-grade implementations. Generated G-code should be previewed/simulated before being sent to a real printer.

## Full application roadmap

- Adventurer 5M Pro first-class profile
- Basic → Advanced → Expert settings
- Native macOS UI
- 3MF / OBJ / additional formats
- Production geometry core with exact offsets and clipping
- Large structural/experimental infill library
- Random/configurable seam strategies
- Variable layer height
- Advanced walls, supports and bridges
- G-code preview and estimates
- USB / Wi-Fi / Ethernet printer workflow
- USB, MJPEG, RTSP and ONVIF camera providers
- Intel x86_64 + Apple Silicon
- Homebrew distribution

## Architecture

The long-term design is to use a proven production slicing core for geometry-critical operations rather than reinventing an industrial-grade geometry engine. ForgeSlice-specific layers provide the UI, printer profiles, settings system, pattern selection, camera center, workflow, and packaging.

## License

The final license will be selected based on the licenses of any upstream slicing components and ForgeSlice-specific code.
