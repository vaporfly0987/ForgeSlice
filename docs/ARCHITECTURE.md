# ForgeSlice architecture

## Layers

```text
macOS App
  ├── UI / Viewport
  ├── Settings Model
  ├── Printer Workflow
  └── Camera Center
          │
          ▼
     ForgeSlice Core
  ├── Project / Mesh Model
  ├── Printer Profiles
  ├── Slicing Orchestrator
  ├── Infill Registry
  ├── G-code Pipeline
  └── Caches / Background Jobs
          │
          ▼
   Proven Slicing Engine
```

## Design principles

- Keep the UI responsive by running slicing and G-code generation off the main thread.
- Treat printer profiles as data-first resources so new printers can be added without recompiling the UI.
- Keep infill algorithms behind a registry so standard, structural, and experimental patterns can coexist.
- Make camera providers modular: USB, MJPEG, RTSP, and ONVIF can share one camera abstraction.
- Prefer an established slicing core rather than rebuilding mesh intersection, perimeter generation, support generation, and other geometry-critical systems from scratch.
- Preserve a clean boundary between ForgeSlice-specific code and any upstream slicing component so licensing obligations remain easy to audit.

## Planned slicing pipeline

1. Import STL/3MF/OBJ/project.
2. Validate and repair mesh when necessary.
3. Resolve printer, material, quality, strength, support, seam, and cooling settings.
4. Slice layers in background worker tasks.
5. Generate perimeters, top/bottom surfaces, infill, supports, and travel paths.
6. Optimize and emit printer-specific G-code.
7. Render an interactive G-code preview.
8. Send or export the job through the selected printer workflow.
