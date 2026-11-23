# Final print files

This is the one place 3MFs are committed. Everything else is rebuilt by `../build.py` and ignored by git.

These are the files that were actually printed, saved from Bambu Studio with their settings
(walls, infill, brims, filament, vase mode for the skin). One folder per version:

```
releases/
  v1/   Dreamstate 2026: 4-panel modular ring
  v2/   6-panel round drum (in progress, nothing final yet)
```

## v1

| File | Part | Filament | Print |
|---|---|---|---|
| `grid.3mf` | Grid quarter (fit test first) | Black PLA | 4 |
| `core.3mf` | Core | Black PLA | 1 |
| `base.3mf` | Base | Black PLA | 1 |
| `cap.3mf` | Cap | Black PLA | 1 |
| `skin.3mf` | Skin, 0.6 mm spiral vase | White PLA | 1 |
| `cradle537.3mf` | Battery cradle for the Anker 537 | Black PLA | 1 |
| `board.3mf` | Board plate and spacers | Black PLA | 1 |
| `adapter_thread.3mf` | Pole adapter, 3/4" Acme thread | Black PETG, 100% infill | 1 |

Made by `v1/build_modular.py` and tuned in Bambu Studio. The threaded pole adapter's latest
geometry is `v1/build_adapter_thread_v1b.py`. The 0.84 mm fallback skin, the 1 inch slip-on adapter and the thread test nuts
aren't here: `python3 build.py v1` generates them into `out/v1/`.

When a file changes, commit the new one over the old; git keeps the history. If this folder grows
past a few tens of MB, attach the files to a GitHub Release (tag `v1`, `v2`) instead and keep a
note here saying where they are.
