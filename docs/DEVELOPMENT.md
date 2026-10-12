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
| `Graphics.cpp`, `StaveSvg.cpp`, `Theme.cpp` | Authored diagrams, cached native Painter rasterisation, shared palettes |
| `Tests/` and `RouterCalculater/UiTests.cpp` | Core and GUI regression tests |
| `design/` | Original HTML and supplied visual reference; not runtime dependencies |

This is a native application, not an HTML wrapper. The non-GUI core owns the calculations. Controls own input state and project it into native diagrams and tables. Canonical dimensions are double-precision millimetres; only presentation converts units. Never read a formatted result back into the calculation.

## Rendering and themes

`PREVIEW_HEIGHT` fixes the diagram card at 480 logical pixels; widening a window must not make it taller. Layout applies DPI once. `SvgView` caches by source revision, pixel size and theme. `Paint()` draws the cached image: it must not parse SVG, rebuild geometry, write files or start timers.

Unchanged edits return early because numeric controls can emit both change and commit notifications. Slider values are updated without rebuilding their styles; style changes happen on a theme transition. The decorative SVG dot grid was removed instead of redrawing hundreds of irrelevant circles on each edit. There is no repeating preview animation.

Dark and light share semantic colours. Keep theme mapping in `Theme.cpp`; include text selections, dropdowns, checkboxes and scroll areas when checking a new colour. The screenshots and render-count checks in the GUI self-test exercise both themes. They are not a substitute for checking a real display at the user's DPI.

## The next tab

Add a separate calculator page owned by `RouterWindow`, following `CirclePage` / `StavePage`. Register it with `tabs.Add(page, title)`, keeping unoccupied tab-strip space rather than inserting dead placeholder tabs. Route units, `Ctrl+S`, `Ctrl+O`, theme changes and unsaved-close checks to the active page. Do not add another unrelated mode switch inside the circle calculator.

Put new equations and validated parameters in the non-GUI core. Give a new saved format an explicit application identifier and schema version; never reinterpret another calculator's fields. Add every new implementation file to its `.upp` package, and add both mathematical and user-action tests before calling it integrated.

## Data and acceptance

Imports validate into a candidate before replacing live state. Saves check the previously read file contents, write a temporary sibling, retain a backup and replace the target. This is optimistic conflict detection, not multi-user collaboration or a transactional database; simultaneous writers are not a supported editing workflow.

The GUI self-test uses isolated files in its supplied evidence directory and does not load the user's adjacent libraries. Its `New(false)` paths are test/internal paths; normal New/Open/Close actions must retain discard prompts. Keep tests that check corrupt imports and external edits.

Before publishing, inspect the complete Git diff, run core and GUI tests, build non-BLITZ Release, inspect light/dark captures, and confirm the final output path. Stage only intended source, documentation and reference images. Keep local paths, binaries, user JSON, logs and Klick recovery journals out of commits. Do not delete recovery journals to achieve a clean status.
