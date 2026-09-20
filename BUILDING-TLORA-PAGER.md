# Building `tlora-pager-tft` (Meshtastic UI + offline maps)

This branch builds Meshtastic for the LilyGo T-LoRa Pager **with the MUI
interface and the on-device offline map**, which mainline does not yet ship.

## What this branch is

- Firmware: PR [meshtastic/firmware#11888](https://github.com/meshtastic/firmware/pull/11888)
  (`ericlewis:codex/pager-mui-integration`), which enables the `tlora-pager-tft`
  environment.
- UI: [marshallumsted/device-ui@`tlora-pager-maps`](https://github.com/marshallumsted/device-ui/tree/tlora-pager-maps),
  which merges the open pager PR stack on top of `upstream/pager`:

  | PR | Content |
  |----|---------|
  | #405 | physical keyboard, encoder, conversation controls |
  | #406 | unified dashboard, node and channel rows |
  | #409 | live local GPS receiver status |
  | #408 | map controls, tile caching, tile status (branch tip) |

  The stack is cumulative, so #408 contains the rest. PR #404 is closed and
  superseded by #405.

All of the above is unmerged draft work. The map path in particular was marked
by its author as not hardware validated.

## PlatformIO version: must be 6.1.19

**Do not build this with PlatformIO 6.2.0.** The build will fail with a
misleading error:

    ModuleNotFoundError: No module named 'SCons.Tool.FortranCommon'

### Why

The espressif32 build runs in two phases. After compiling the ESP-IDF
libraries, `builder/frameworks/espidf.py` spawns a *nested* `pio run` for the
Arduino phase, resolving the executable relative to `$PYTHONEXE`:

```python
PYTHON_EXE = env.subst("$PYTHONEXE")
pio_exe_path = str(Path(os.path.dirname(PYTHON_EXE)) / "pio")
```

That resolves to PlatformIO's internal `penv`, not the interpreter you invoked.
If the outer and nested PlatformIO versions differ, they disagree about
`tool-scons`:

- the espressif32 platform pins SCons **4.8.1** (`platform.json`)
- PlatformIO **6.1.19** core wants `~4.40801.0` (4.8.1), which agrees
- PlatformIO **6.2.0** core wants `~4.41101.0` (4.11.1), which does not

The mismatched run reinstalls `packages/tool-scons`, deleting the
`scons-local-4.8.1` directory the other process already has on `sys.path`. The
next lazy `SCons.Tool.*` submodule import then fails. Fortran is incidental; it
is simply the first submodule imported after the directory disappears.

### Setup

Both the build interpreter and PlatformIO's `penv` must be 6.1.19:

```bash
python -m venv .venv
./.venv/bin/pip install 'platformio==6.1.19'
~/.platformio/penv/bin/pip install 'platformio==6.1.19'
rm -rf ~/.platformio/packages/tool-scons      # clear any stale mixed install
./.venv/bin/pio run -e tlora-pager-tft
```

## Output

Artifacts land in `.pio/build/tlora-pager-tft/`:

| File | Use |
|------|-----|
| `firmware-*.factory.bin` | full image including bootloader and partition table, flash to `0x0` |
| `firmware-*.bin` | app image only, for OTA into `app0` |
| `littlefs-*.bin` | filesystem image for the `spiffs` partition |

Build footprint: Flash 61.4% (4,022,699 of 6,553,600), RAM 35.7%.

Note the Meshtastic partition scheme gives the app 6.25 MB, unlike LilyGo's
stock 3 MB APP / 9.9 MB FATFS layout. Flashing `factory.bin` to `0x0` rewrites
the partition table accordingly.

## Map tiles

Tiles are not included. The map layer reads, in order of preference:

1. a single `.pmtiles` raster archive on the SD card (much easier to manage)
2. loose `/{z}/{x}/{y}.png` tiles under `/map`, or `/maps/{STYLE}/` for
   multiple styles
3. network tile URLs

Max zoom is 19 for OpenStreetMap, 20 otherwise.
