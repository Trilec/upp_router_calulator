# Validation checkpoint — 12 October 2026

## Current: 0.3.0 / build 2026-10-14.1

The new non-BLITZ **Release compiled and linked directly to `bin/RouterCalculator.exe`**, 5,927,424 bytes. Receipt `exec-28408D844FDB7351FF9F93777496102F` completed 14 October 2026 at 10:30:18 machine-local time, exit 0. The native version badge identifies 0.3.0 and Release; hovering reports the build and path. This supersedes the old bin build described below.

Current Debug runtime evidence: **921 core checks** (335 circle/persistence + 586 stave/exports) and **136 interface checks** (32 circle + 83 stave + 21 window/control), all passed. Core receipt: `exec-9F2DF95E9CB39EF2E9DBCC79AD0B7CEC`. GUI receipt: `exec-8CBCC99B74F36D7ACC7E0A66A8F10391`.

Debug executable SHA-256: `f496484a8a0c2fb14180ce9886de9c66e2b4899eebe8c8305223fc455fa0ea61`. Core-test executable SHA-256: `5fe0fbf6a81040396cb1572a0f9260754c2bc3b9df4c709ea1486444c284d67d`. Native screenshots are retained in `build/validation-030-debug/`; the README screenshot is a Debug capture, not a Release runtime claim.

Added tests exercise outside/count sliders, editor-slider physical unit invariants, expanded ranges, duplicate commit suppression, boundary preservation, three-column mounting rows, saved controls above inputs, compact height, explanation expansion and the central visible version. An initial test fixture retained Radius mode before old Diameter assertions; the fixture now explicitly establishes its mode. The final run above has zero failures.

The full changed-input Debug audit has 32 workloads / 1,920 measured events, zero functional failures and one diagram raster rebuild per changed event. **29 of 32 workloads still miss the 16.67 ms p95 target**; see [Performance](PERFORMANCE.md). Do not interpret successful GUI tests as proof of 60 fps.

**Release runtime boundary:** bin inspection still returns `PROTECTED_PATH` because its project-folder exclusion remains present. Execute busy is no longer the obstacle. The compiled Release has not been launched or hashed by this agent. Its `--self-test`, `--audit` and real-screen DPI/interaction checks remain outstanding; no permission bypass was used, and no saved user JSON was replaced.

---

> Historical 0.1 checkpoint. The native rendering/input refactor below supersedes the old SVG/cache assumptions; the original results remain history, not evidence for the current source.

## 0.2 Native checkpoint — 13 October 2026

The revised Debug application compiled and passed **103 GUI checks** (32 circle, 53 stave, 18 window/control). New cases cover 30/40/50/60 cm sizes, explicit suffixes, unit/radius switches with invalid fields, retained last-valid geometry, modifier-wheel steps and boundary stops. Run: `exec-11F1AFDEFAB860CB8747D380B4CDD147`; executable SHA-256 `fed3a4245732b9cceca43afb18bf5571657ac41ea79c9f8a90a1798014f04fd0`.

The changed-input Debug audit had zero functional failures but **missed the 60-fps p95 target in all 24 workloads**. See [Performance](PERFORMANCE.md) for timings and the Release acceptance boundary. Do not reuse the unchanged-cache timing below as proof of responsiveness.

At this checkpoint, a fresh core-test run and the 0.2 Release build remain pending. Execute reports `EXECUTE_BUSY`, while this project's retained jobs show completion. No other task was cancelled or a busy response treated as a successful build. The earlier bin executable and published commit are not the new refactor.

---

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
