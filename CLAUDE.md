# Heart Rate Totem

Public repo meant to let anyone build their own totem. Keep docs written for non-experts (see `docs/build-guide.md`'s tone: short sentences, one step at a time, no jargon without a definition).

- Firmware: `firmware/totem/` (Arduino, ESP32-S3, FastLED + NimBLE-Arduino). Compile with `tools/build.sh`; host tests with `make -C tests`. Run both before committing firmware changes.
- `firmware/totem/secrets.h` is gitignored; on Noah's machine it's a symlink to `~/.config/biohacker_bro/secrets.h` (the SessionStart hook in `.claude/settings.json` creates it). Never commit a real MAC or PIN.
- 3D parts: `hardware/` Python scripts are the source; generated output goes to `hardware/out/` (ignored). Only slicer-tuned 3MFs in `hardware/releases/` are committed.
- Phone remote: `docs/remote/index.html`, served by GitHub Pages from `/docs`.
- v1 = the 4-panel build in the guide. v2 work is tracked in `docs/v2.md`. When a firmware setting or behaviour changes, update the build guide's settings table and `docs/firmware.md` in the same change.
- Hardware facts that cost time to learn are in `docs/firmware.md` under "Gotchas": read it before touching BLE, serial or the thumb wheel.
