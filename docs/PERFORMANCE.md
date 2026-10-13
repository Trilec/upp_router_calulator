# Native UI performance audit — 13 October 2026

## Current 0.3.0 measurement — 14 October 2026

The current code uses a compact native layout, stable slider ranges and direct 720 × 270 diagram preparation into a fixed-height view. It does not generate SVG. `NativeForm` also prepares word wrapping outside Paint instead of clipping long explanation labels. Exact-size panel skins and duplicate-input guards remain in place.

The current **Debug** audit is `exec-C613531E8BC8695EF592EA8ACDEEB064`, completed 10:29:38 machine-local time, exit 0. It logs version 0.3.0, build 2026-10-14.1, executable `build/RouterCalculator-native-test.exe`, DPI(100)=100. Raw report: `build/performance-030-debug/performance-audit.txt`. Two themes × two widths × eight interaction types = 32 workloads, six warmup plus 60 measured changed events each.

| Interaction | Mean range (ms) | p95 range (ms) |
| --- | ---: | ---: |
| Circle key | 15.596–18.478 | 21.423–31.126 |
| Circle wheel | 14.947–18.646 | 17.616–34.436 |
| Circle slider | 15.046–19.130 | 17.411–29.436 |
| Drum outside wheel | 13.290–16.035 | 16.580–27.252 |
| Drum wall wheel | 12.989–13.740 | 17.918–20.821 |
| Drum count wheel | 15.017–17.083 | 23.012–33.366 |
| Drum outside slider | 12.671–15.631 | 15.462–26.826 |
| Drum count slider | 12.460–16.245 | 13.477–26.958 |

Each workload made 60 raster rebuilds for 60 changed events. Functional failures: zero. Largest measured event: **40.828 ms**. No one-second update was reproduced. The 60-fps p95 target was missed by **29/32** workloads; two also missed the 33.33 ms p95 budget. These are input-handler-to-OS-paint-submission measurements, not paced display presentation or hardware mouse latency. Concurrent machine load was not isolated.

The **0.3.0 Release build succeeded** at `bin/RouterCalculator.exe` (receipt `exec-28408D844FDB7351FF9F93777496102F`). Release runtime and timings remain unverified: the bin file exclusion prevents the hash/read step required by the separately approved output runner. Do not extrapolate Debug timings into a Release result or advertise a guaranteed 60 fps. The command sequence in Development is ready for an owner-authorised Release run.

The 13 October measurements below are retained as historical evidence, not the current result.

---

## Status

The live SVG path has been removed. Forms, readouts, tables and dialog captions are native Ui controls. Diagram curves use prepared Painter buffers; text and straight lines use prepared Draw operations. Functional GUI checks pass, but **60 fps has not been established**. The measured Debug build still misses the 16.67 ms p95 target. Do not turn a successful audit exit code into a performance PASS.

Release timing is outstanding. The project file-access exclusion still includes bin, preventing retrieval of the hash required by its independently approved executable runner. This is separate from permission to compile a Release executable into bin. Do not bypass either policy with a helper executable or another folder identity.

## Changed-input measurements

Debug, CLANGx64, U++ 18468, DESKTOP-GFX1, DPI(100)=100. The visible-window harness invokes real key/wheel/slider handlers, then window.Sync and ProcessEvents. It measures paint submission, not physical mouse latency or monitor scan-out. Concurrent machine activity was not isolated.

| Workload | Original SVG mean, ms | Native mean range, ms | Native p95 range, ms |
| --- | ---: | ---: | ---: |
| Circle key | 56.34–59.23 | 21.11–22.87 | 33.72–35.59 |
| Circle wheel | Not recorded separately | 21.17–22.13 | 26.66–34.01 |
| Circle slider | Not recorded separately | 20.01–22.59 | 24.18–35.71 |
| Stave outside | 82.93–92.42, key | 16.75–20.32, wheel | 26.74–37.72 |
| Stave wall wheel | Not recorded separately | 16.30–18.14 | 24.08–27.50 |
| Stave count wheel | Not recorded separately | 18.10–22.95 | 26.82–35.49 |

Ranges cover light/dark themes and widths 980/1320 logical pixels. The baseline used 16 samples per key workload. The final native run used six warmup events followed by 60 measured events per workload: 24 workloads, 1,440 measured events. Each recorded 60 geometry raster rebuilds for 60 changed events, not two or three rebuilds per event. Functional failures: zero. Workloads missing the 16.67 ms p95 target: **24 of 24**. Some also exceeded the 33.33 ms p95 minimum budget. Largest observed event: 44.877 ms. No one-second update was reproduced.

This is a reduction in the tested Debug path, not proof of Release responsiveness. The original unchanged-cache benchmark did not exercise changed geometry, control updates or paint submission and was insufficient for the user's report.

## Findings

The old circle path spent about 42 ms rebuilding its SVG raster in Debug; the mathematical calculation took a fraction of a millisecond. Native text was unnecessarily entering vector-glyph rendering. SVG authoring/parsing and the unused implementation files have now been removed.

The installed Ui rounded-panel fallback allocates and rasterises a complete surface during Paint. Trying its nine-slice skin also regressed measurements because it stretches subimages during Paint. The app now supplies an exact-size native skin prepared only when panel size or theme changes. The additional frame pass is disabled because the frame is already present in the skin. Native controls retain painting and parenting.

A whole-panel invalidation/coalescing experiment was removed after measurement. Changed controls invalidate themselves; page Paint functions draw only the background. Diagram buffers fit the normalised design extent, not the physical dimensions or entire window. Straight dimension lines use prepared Draw operations; curves retain antialiasing. Fonts, positions and text colours are prepared before Paint.

Unchanged-input guards suppress redundant change/commit updates. Count-driven dropdown changes and readout model writes are batched. Unit-only label changes do not rebuild curves. No recurring redraw timer is used.

## Evidence

Baseline: `exec-29FBFDE90C65BF79CA6FBB6DD682F5FE`.

Final native audit: `exec-BF39488317893A30FD7D2E8A5D70925D`, exit 0 at 01:08:42 machine-local time. Report: `build/performance-native-final-direct/performance-audit.txt`.

Debug executable: `build/RouterCalculator-native-test.exe`, SHA-256 `fed3a4245732b9cceca43afb18bf5571657ac41ea79c9f8a90a1798014f04fd0`, 11,591,680 bytes. Build: `exec-C4CB3A58FDE9DEBFB243728D2C0AF24A`.

GUI test: `exec-11F1AFDEFAB860CB8747D380B4CDD147`, exit 0. Circle 32 passed, staves 53 passed, window/control 18 passed: **103 checks, zero failures**. Captures: `build/validation-native-final/`.

## Remaining acceptance

Run current core tests, build non-BLITZ Release to `bin/RouterCalculator.exe`, then run Release `--self-test` and `--audit` with read/execute authority correctly resolved. Commands are in [Development](DEVELOPMENT.md). Record actual Release p95/max and DPI before accepting the 60-fps goal. Retain this Debug evidence rather than replacing it with speculative Release estimates.
