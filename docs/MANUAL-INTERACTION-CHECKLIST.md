# macOS manual interaction checklist

Run by hand before any public Mac binary release.

The first independent QA pass, on `9e78225a08de62fd88ff5d8072ae77f7802cee88`,
could not run these checks because the QA terminal lacked macOS Accessibility
permission. Manual QA then found a Cocoa keyboard-event defect, fixed in
`32caaa36a`, and a targeted QA pass completed the checklist on that head.

Candidate: `macos-arm64-develop-port` @ `32caaa36a845fc668c9fd36cd2cfd8b047c46733`
(merged into `develop` by [pull request #2](https://github.com/DJAscendance/freewrl/pull/2)).

- [x] 1. Launch the Release build.
- [x] 2. Press `q`; confirm a clean exit.
- [x] 3. Relaunch and click several HUD buttons.
- [x] 4. Run test 8 and verify TouchSensor picking.
- [x] 5. Run test 10 and verify PlaneSensor drag/navigation.
- [x] 6. Confirm no crash or stuck FreeWRL process remains.

Tester: independent macOS QA run · Date: 2026-09-25 · Result: **PASS**

QA token: `FREEWRL_6_7_MACOS_ARM64_GL41_KEYBOARD_AND_INTERACTION_QA_PASS`
