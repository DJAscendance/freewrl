# FreeWRL visual tests (macOS)

Renders worlds in the FreeWRL Mac app and in [X_ITE](https://create3000.github.io/x_ite/)
(a maintained X3D/VRML browser, used as the reference), then compares the images.

```sh
brew install imagemagick
tools/visual-test/compare.sh 1.wrl 2.wrl            # worlds from freewrl/tests
tools/visual-test/compare.sh -r ~/worlds a/b.wrl    # any directory
open visual-test-out/index.html
```

Needs: a built `~/Applications/FreeWRL.app` (override with `FREEWRL_APP`), Google
Chrome (override with `CHROME`), ImageMagick, and Xcode command line tools (`swiftc`
compiles the window-id helper on first run). X_ITE loads from a pinned jsdelivr
build; point `XITE` at a local `dist` URL to run offline.

The output directory holds, per world: `freewrl.png`, `xite.png`, `diff.png`,
`side.png` (all three side by side), `freewrl.log`, `xite.log` (browser console);
plus `summary.tsv` and `index.html`. Exit status is 1 on any `REGRESSION` or `CRASH`.

## Scoring

`match` is the normalized cross-correlation of the two renders at half size
(1 = identical). Pass is `match >= 0.90` (`-t` to change). SSIM is reported too,
but it's dominated by empty background: two completely different renders on black
can still score 0.8.

Results:

| result | meaning |
| --- | --- |
| `PASS` | match at or above the threshold |
| `REGRESSION` | below the threshold, and not a known reference limit |
| `REFERENCE_INVALID` | below the threshold; `known.tsv` says X_ITE renders this world wrongly |
| `NONDETERMINISTIC` | below the threshold; `known.tsv` says the world is animated |
| `NO_REFERENCE` | X_ITE produced no image |
| `CRASH` | FreeWRL showed no window |

The score is kept for every result. Add a world to `known.tsv` only after looking at
its `side.png`.

Differences are not automatically FreeWRL bugs. X_ITE has its own gaps (for
example, it draws `tests/2.wrl` without its MultiTexture), so read `side.png`
before acting on a failure.

Known limits:
- Animated worlds (e.g. `tests/6.wrl`) are captured at different moments and won't match.
- Only the initial viewpoint is compared.
- `CROP_TOP` assumes a Retina display. `CROP_BOTTOM` (the HUD) is measured from the
  capture: FreeWRL 6.x's HUD wraps onto more rows in narrow windows, and the 3D view
  stops above it. It falls back to 32 px (master's one-row status bar).

## Pieces

| file | what |
| --- | --- |
| `compare.sh` | the driver: serves the worlds, shoots both, scores, writes the report |
| `shoot_freewrl.sh` | launches FreeWRL in the background (no focus steal), captures its window, crops to the 3D view |
| `shoot_xite.sh` | headless Chrome + `viewer.html`, with a throwaway profile |
| `serve.py` | static server: threaded, large backlog (FreeWRL fetches every texture at once), `/__hold` to delay Chrome's screenshot until X_ITE has rendered |
| `viewer.html` | X_ITE page driven by query params |
| `winid.swift` | prints the FreeWRL window id for `screencapture -l` |

Worlds are served over HTTP rather than loaded as files, so absolute URLs like
Cybertown's `/externprotos/...` resolve against the served root.
