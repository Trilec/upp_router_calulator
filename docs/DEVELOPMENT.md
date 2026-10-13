# Development

## Environment and build outputs

The maintainer setup is Windows, U++ **18468**, build method **CLANGx64**, with sibling `upp_Ui` and `upp_animation` nests. Use the installed, tested versions; do not silently upgrade them while fixing this application. `GitHub.var` is a machine-local TheIDE assembly, not a portable dependency installer.

From the repository root in PowerShell, with the maintainer's actual paths:

```powershell
$umk = 'E:/upp-18468/umk.exe'
$nests = 'E:/apps/github/upp_router_calulator,E:/apps/github/upp_Ui,E:/apps/github/upp_animation,E:/upp-18468/uppsrc'
# Debug validation build. Does not replace a copy somebody is currently using.
& $umk $nests RouterCalculater CLANGx64 --out-dir ./build/cache -b +GUI ./build/RouterCalculator-validation.exe
if ($LASTEXITCODE) { throw 'Debug build failed' }
& ./build/RouterCalculator-validation.exe --self-test ./build/validation-debug
if ($LASTEXITCODE) { throw 'GUI validation failed' }
# Core tests, including stave polygons and saved-file round trips.
& $umk $nests Tests CLANGx64 --out-dir ./build/cache -b +CONSOLE ./build/RouterCalcTests-debug.exe
if ($LASTEXITCODE) { throw 'Core build failed' }
& ./build/RouterCalcTests-debug.exe
if ($LASTEXITCODE) { throw 'Core validation failed' }
# Latest Release: deliberately non-BLITZ as an additional include/isolation check.
& $umk $nests RouterCalculater CLANGx64 --out-dir ./build/cache -r +GUI ./bin/RouterCalculator.exe
if ($LASTEXITCODE) { throw 'Release build failed' }
```

On another machine replace the nest and compiler paths, preserving the package names. In TheIDE, use an assembly containing those same nests. `-b` enables BLITZ, `-r` selects Release, and an explicit final output argument controls the executable location. Official UMK documentation: https://www.ultimatepp.org/app$ide$umk$en-us.html

**Every Release executable belongs in `bin/`.** The main app has one stable name: `RouterCalculator.exe`. Debug output, caches, logs and test captures belong in `build/`. Close the particular executable before replacing it; do not kill unrelated running apps. Do not delete the whole bin folder: it may contain real design libraries and backups.

## Source map

| Location | Responsibility |
| --- | --- |
| `RouterCalcCore/RouterCalcCore.*` | Circle equations, units, circle-library validation, staged file writing |
| `RouterCalcCore/Staves.*` | Stave/parallel-strip geometry, depth allowances and stave-library validation |
| `RouterCalcCore/StaveExport.cpp` | Numbered geometry JSON and CSV cut list |
| `RouterCalculater/Window.*` | Application window, calculator tabs, active-page commands, theme preference |
| `RouterCalculater/App.*`, `CircleLayout.cpp` | Circle page, settings and saved designs |
| `RouterCalculater/StavePage.*`, `StaveLayout.cpp` | Drum page, saved shells, settings and export choices |
| `NativeForm.*`, `Graphics.cpp`, `Theme.cpp` | Owned native Ui panels/labels/tables and shared control styling |
| `UnitEdit.*` | UiFloatEdit extension: unit suffixes, incomplete drafts, wheel steps, physical-limit feedback |
| `Diagrams.*`, `CircleDiagram.cpp`, `StaveDiagram.cpp` | Normalised diagram geometry, cached native Painter curves, prepared Draw text/lines |
| `PerformanceAudit.cpp` | Opt-in changed-input to paint-submission measurements |
| `Tests/` and `RouterCalculater/UiTests.cpp` | Core and GUI regression tests |
| `design/` | Original HTML and supplied visual reference; not runtime dependencies |

This is a native application, not an HTML wrapper. The non-GUI core owns the calculations. Controls own input state and project it into native diagrams and tables. Canonical dimensions are double-precision millimetres; only presentation converts units. Never read a formatted result back into the calculation.

## Rendering and themes

The compact diagram cards are 324 logical pixels high (`PREVIEW_HEIGHT`), including native headings and legends; the custom drawing area is 216 pixels high. Widening a window must not make these areas taller. Both collapsed pages use 738 logical pixels, and the default window is 1120 × 850. Diagrams use a 720 × 270 authored space rather than shrinking the former tall composition. Layout applies DPI once. Forms, readouts, tables, dropdowns and dialog captions are native Ui controls. `NativeForm` owns the presentation controls and parents the borrowed inputs inside real UiPanel containers. Declare the form before its borrowed child members so destruction runs in the correct order.

There is no SVG generation or parsing in the application. The diagram builders project canonical dimensions into a bounded design space using ratios, independent of the physical size of the workpiece. `DiagramView::Prepare()` prepares curves into a fitted antialiased buffer and prepares native text/straight-line positions. `Paint()` only blits that buffer and submits prepared Draw operations; it does not calculate, lay out, parse, write files or run a clock. Unit-only label changes retain the curve raster.

The installed Ui rounded-panel fallback allocates and rasterises a complete surface during Paint. Its nine-slice path also stretches subimages during Paint. The app uses the native skin API with an exact-size cached panel surface instead: create it only on size/theme changes, and disable the additional frame pass because the frame is already in the skin. Do not restore full-pane Refresh calls to ordinary numeric edits. Native changed controls invalidate their own areas; parent canvases paint only the background.

Unchanged edits return early because native controls may emit both change and commit notifications. Slider values update without rebuilding styles. Count-driven dropdown items and table-model updates are batched. No idle refresh clock or animation is needed. The original HTML remains a design reference, not a runtime dependency.

Dark and light share semantic colours. Keep theme mapping in `Theme.cpp`; include text selections, dropdowns, checkboxes and scroll areas when checking a new colour. The screenshots and render-count checks in the GUI self-test exercise both themes. They are not a substitute for checking a real display at the user's DPI.

## Input semantics and performance checks

`Version.h` is the single application-version/build/configuration source. Bump the build identifier for every delivered executable, and the version for user-visible releases. The native pale-green badge, window title, `--version` and audit report all use it. Its tooltip includes the actual running executable path. This version is independent of the saved JSON schema.

`StaveControls.cpp` owns the two native slider bindings. Outside-slider values are always physical **outside diameters in mm**; the editor independently presents radius/diameter and mm/cm. The count slider is integral. Typed values extend the slider range, but ordinary drag events must never rescale it under the pointer. Guard programmatic sync and ignore the duplicate commit event. A rejected limit-crossing step restores the prior slider position and keeps the valid preview.

`NativeForm::PrepareCaption` inserts width-dependent line breaks only when caption text or width changes. The installed UiLabel's `nowrap=false` supports explicit lines, not automatic word wrapping. Do not move this preparation into Paint, or switch ordinary labels to rich/vector glyph rendering. Plain labels and tables remain native controls.

Outside diameter/radius is the drum's finished external size. Wall thickness is radial, not another diameter. Outside padding and inside padding are independent milling allowances. Dependent inside dimensions and cut geometry are calculated; edits must not silently rewrite another independent input.

`UnitEdit` accepts numeric entries in the selected unit or explicit `cm`/`mm` suffixes. Use its typed getters through `UnitEdit`, not through a base UiFloatEdit pointer. Converting display units preserves every complete length independently and keeps incomplete text editable. Invalid dimensions cannot be saved/exported, but retain a labelled LAST VALID PREVIEW. Unit and radius/diameter switches remain usable. A wheel step crossing a physical limit is vetoed with an outline and explanation instead of erasing the preview.

Length wheel steps are 0.1 normally, 1 with Alt, 10 with Shift, in the displayed unit. Counts use integral 1/10/100 steps. The mathematical core rejects non-finite values, impossible relationships and numeric overflow/precision loss, rather than imposing an arbitrary 10,000 mm size limit.

Run the actual Release audit from the repository root after building:

```powershell
& ./bin/RouterCalculator.exe --self-test ./build/validation-release
if ($LASTEXITCODE) { throw 'Release GUI validation failed' }
& ./bin/RouterCalculator.exe --audit ./build/performance-release
if ($LASTEXITCODE) { throw 'Performance audit functional failure' }
```

Read `performance-audit.txt`, not just the exit code. The audit reports functional errors separately from timing misses. It exercises changed key, wheel and slider callbacks at two window widths in both themes, with warmup and 60 measurements per workload. The target is p95 at or below 16.67 ms, with 33.33 ms the 30-fps minimum budget. OS paint submission is not monitor scan-out; an unchanged-cache microbenchmark cannot establish responsiveness. Record build configuration, DPI and any concurrent machine activity. See [Performance](PERFORMANCE.md) for observed results and validation boundaries.

## The next tab

Add a separate calculator page owned by `RouterWindow`, following `CirclePage` / `StavePage`. Register it with `tabs.Add(page, title)`, keeping unoccupied tab-strip space rather than inserting dead placeholder tabs. Route units, `Ctrl+S`, `Ctrl+O`, theme changes and unsaved-close checks to the active page. Do not add another unrelated mode switch inside the circle calculator.

Put new equations and validated parameters in the non-GUI core. Give a new saved format an explicit application identifier and schema version; never reinterpret another calculator's fields. Add every new implementation file to its `.upp` package, and add both mathematical and user-action tests before calling it integrated.

## Data and acceptance

Imports validate into a candidate before replacing live state. Saves check the previously read file contents, write a temporary sibling, retain a backup and replace the target. This is optimistic conflict detection, not multi-user collaboration or a transactional database; simultaneous writers are not a supported editing workflow.

The GUI self-test uses isolated files in its supplied evidence directory and does not load the user's adjacent libraries. Its `New(false)` paths are test/internal paths; normal New/Open/Close actions must retain discard prompts. Keep tests that check corrupt imports and external edits.

Before publishing, inspect the complete Git diff, run core and GUI tests, build non-BLITZ Release, inspect light/dark captures, and confirm the final output path. Stage only intended source, documentation and reference images. Keep local paths, binaries, user JSON, logs and Klick recovery journals out of commits. Do not delete recovery journals to achieve a clean status.
