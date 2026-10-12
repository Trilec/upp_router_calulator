# UPP Router Calculator: working rules

Read `docs/DEVELOPMENT.md` and `docs/GEOMETRY.md` before editing. Use Klick for this machine's source/build workflow. Use the actual approved workspace, U++ installation and CLANGx64 method; do not install or upgrade tools silently.

Keep existing package/nest names, including `RouterCalculater`, unless a rename is explicitly requested. Put new calculator features on a separate owned tab. Core geometry and schemas must remain independent of CtrlLib/Ui.

Release output is always `bin/RouterCalculator.exe`; overwrite that latest app, never the adjacent user JSON libraries. All Release executables belong in bin. Debug builds, caches and validation captures belong in build. Do not stop unrelated running executables to replace a locked debug file.

Keep the preview fixed-height. No calculation or SVG parsing in Paint; no idle redraw clock. Preserve radius/diameter and mm/cm invariants, incomplete-input handling, unsaved-change prompts and both themes. Run the core and GUI tests, plus a non-BLITZ build, after changes.

Do not describe a successful compilation as a test run. Geometry JSON is not machine-ready. Keep limitations explicit.

Preserve concurrent user work. Stage only intended files. Fetch and inspect remote state before an authorised publication; do not force-push. Do not commit binaries, machine-local .var files, saved user designs, logs or Klick journals. Never delete recovery evidence merely to clean Git status.
