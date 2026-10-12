# Validation checkpoint — 12 October 2026

## Build and runtime evidence

Validated through Klick on DESKTOP-GFX1 using U++ 18468 and CLANGx64. The application is native; no browser component is used. This checkpoint covers the circle calculator, Drum staves tab, parallel spacer strips, axial allowances, local files and both themes.

| Gate | Observed result |
| --- | --- |
| Circle/core persistence tests, Debug | 333 passed, 0 failed |
| Stave geometry/export/persistence tests, Debug | 581 passed, 0 failed |
| Circle interface tests, Debug | 27 passed, 0 failed |
| Stave interface tests, Debug | 38 passed, 0 failed |
| Window/theme/cache interface tests, Debug | 11 passed, 0 failed |
| Final non-BLITZ Release build | Successful; `bin/RouterCalculator.exe`, 5,781,504 bytes |

The final Release link completed at **22:22:41 local machine time**, replacing the stable bin output. The old locked `build/RouterCalculator-debug.exe` was left untouched; the fresh runtime checks used `build/RouterCalculator-validation.exe` instead.

Klick job receipts:

- Core test run: `exec-5AAE11E1FB3688E2D36DB54729D56FB5` (exit 0).
- Final GUI test run: `exec-F93F63DFF758DAFFAD3D010C0DEE05AE` (exit 0).
- Final Release build: `exec-B449D460B10A011D8CA36A0ECBFF8FAB` (exit 0).

Core-test executable SHA-256: `4534e1af0282eebf8e13ffffc0c241fc42c2053c8730fd7fc446bf12d856cb60`.

Final Debug GUI executable SHA-256: `422d3ce8379a81a237e7640de87fc53fc4d0ea953a6da1590a5ecc629c4da91c`.

**Release runtime boundary:** the Release executable was compiled and linked directly to bin, but was not launched by the agent. The file-inspection service still returns `PROTECTED_PATH` for bin, preventing the hash retrieval required by its separately approved executable runner. The 990 passing checks above are actual Debug runtime checks, not a claim that the Release executable itself ran them.

## What the tests establish

Stave tests independently sample the required material annulus against the union of the generated part polygons across 108 combinations of counts, radii, strip widths and cadences. They also check seam closure, rectangular strips, one-sided midpoint offsets, wrap-around joints, validation failures, depth/plate separation, numbered CSV/JSON exports and saved-file round trips. Sampled containment is regression evidence, not a machining certification.

Geometry JSON's largest observed decimal round-trip difference was approximately `5.12e-13 mm`; the recursive test requires matching structure and numerical differences no greater than `1e-9 mm`. The displayed decimal places do not specify an achievable cutting tolerance.

The final GUI run made 1,000 unchanged SVG source/layout updates with **zero raster rebuilds**, taking 0.0541 ms in that run. A separate Debug run computed 10,000 48-stave geometries in 1,912.36 ms. These are small local benchmarks, not a complete frame-latency profile or Release performance claim.

Native light/dark captures at 920/1320 logical-pixel page widths and full-window captures are retained under `build/validation-debug/`. The final dark drum screenshot is also at `design/drum_staves_dark.png`. The supplied original `design/screenshot.jpg` is preserved. Preview height remains fixed at 480 logical pixels. Dropdown indicator tint follows the active theme.

## Reproducibility and remaining limits

The local `upp_Ui` dependency was based on `ecadb1294f66b937118f5bda8b974e919ad0b44c` with unrelated local changes already present; those files were not modified by this task. Animation was resolved from the installed sibling nest; its revision was not independently recorded. This is a tested local build, not a clean-clone or cross-platform acceptance certificate.

No actual timber cut, glue-up, robot motion, collision check, tapered-shell calculation or machine calibration was performed. The measured router end gap is a supplied planning allowance. User design libraries, the original screenshot and Klick recovery journals were not deleted during cleanup.
