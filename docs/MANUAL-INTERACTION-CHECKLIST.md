# macOS manual interaction checklist

Run by hand before any public Mac binary release. These items were not
verified by independent QA (the QA terminal lacked macOS Accessibility
permission). Candidate: `macos-arm64-develop-port` @ `9e78225a08de62fd88ff5d8072ae77f7802cee88`.

- [ ] 1. Launch the Release build.
- [ ] 2. Press `q`; confirm a clean exit.
- [ ] 3. Relaunch and click several HUD buttons.
- [ ] 4. Run test 8 and verify TouchSensor picking.
- [ ] 5. Run test 10 and verify PlaneSensor drag/navigation.
- [ ] 6. Confirm no crash or stuck FreeWRL process remains.

Tester: ________ Date: ________ Result: ________
