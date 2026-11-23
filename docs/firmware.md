# Firmware notes

Developer notes for the totem firmware: building from the command line, how each feature works, and what was learned on real hardware. If you just want to build a totem, the [build guide](build-guide.md) is all you need.

## Layout

```
firmware/
  totem/                 the totem sketch
    totem.ino            setup, loop, drawing, the show scheduler
    config.h             your strap (via secrets.h), I2C pins, brightness levels, HR zones
    totem_config.h       everything else: panels, power cap, crew, shows, phone, beat drop
    secrets.h.example    copy to secrets.h (gitignored): strap MAC, friends, phone PIN
    crew_ble.*           Bluetooth heart rate client for you + up to 4 friends (NimBLE)
    phone_ble.*          GATT server for the phone remote
    phone_protocol.h     the remote's text commands (host-tested)
    face_layout.h        the 16x16 face: bar, heart, digits (host-tested)
    led_map.h            (x, y) on the ring -> LED index, from the PANEL_* flags (host-tested)
    ring_layout.h        TOTEM_RING 1: the older 3-panel EKG ring (kept for later)
    show_layout.h        spin, phrases, Queen Mary (host-tested)
    show_font.h          the 7 px show font
    audio_drop.*         ES7210 mics -> beat drop detector task
    drop_detect.h        the drop detector itself (host-tested)
    drop_fx.h            the drop rings and the test-mode sound meter
    brightness.*         the thumb wheel, through the PCA9557 expander
    hr_zones.*           BPM -> zone and bar math (host-tested)
    display_ui.h         a small shim so brightness.cpp drives FastLED
  totem_panel_test/      one-panel wiring test: no Bluetooth, no secrets
  find_whoop_mac/        BLE scanner that prints every heart rate strap nearby
```

## Building from the command line

Faster than the IDE for iterating.

```bash
# once
arduino-cli core install esp32:esp32
arduino-cli lib install FastLED NimBLE-Arduino@2.5.1

tools/build.sh                                  # compile the totem
tools/build.sh totem /dev/cu.usbmodem1101       # compile and flash
tools/build.sh totem_panel_test /dev/cu.usbmodem1101
```

Find the port with `arduino-cli board list`. The FQBN in `tools/build.sh` (`esp32:esp32:esp32s3:CDCOnBoot=cdc,FlashSize=16M,PSRAM=opi,PartitionScheme=app3M_fat9M_16MB`) encodes the same settings as the IDE's Tools menu in the build guide; keep the two in sync.

Tested with esp32 core 3.3.x, FastLED 3.10.5 and NimBLE-Arduino 2.5.1.

### Watching serial

```bash
python3 tools/serial_monitor.py                              # Ctrl-C to stop
python3 tools/serial_monitor.py /dev/cu.usbmodem1101 10      # capture 10 s
```

**Neither `arduino-cli monitor` nor `stty ... raw && cat` works on this board.** Both print nothing while the sketch runs normally, which looks exactly like a crashed sketch. The ESP32-S3's native USB-CDC only sends once the host asserts DTR, and neither tool raises it. `serial_monitor.py` opens the port and raises DTR first; that's the whole difference.

**Boot-time output is racy.** The port re-enumerates on reset, so a capture started after the reset has already missed `setup()`. That's why the totem reprints its status (strap, friends, brightness, heap, uptime) from `loop()` every few seconds.

## Host tests

```bash
make -C tests
```

Compiles with a plain `c++`: no Arduino toolchain, no board. Runs in CI on every PR (`.github/workflows/tests.yml`). If `firmware/totem/secrets.h` doesn't exist the Makefile copies the example, so a fresh clone works.

What's covered: the zone math (`hr_zones`), the face layout and every combination of the `PANEL_*`/`RING_*` LED-map flags, the show layouts and font, the beat drop detector against a synthetic trance track, and the phone command parser.

**They're property tests, not tables of expected values.** The zone thresholds are personal and expected to be retuned; a test asserting "110 bpm is zone 3" would fail on every retune. Instead they assert what must hold for any sane threshold set: the slices tile the bar with no gap or overlap, the bar never shortens as BPM rises, zone boundaries land on slice edges, every zone is reachable, and the ends clamp. So after a retune: edit the constants, run the tests.

What's not covered: anything that's I/O against hardware. Every real bug on this project so far (wrong clock phase, tearing, a heart that popped instead of beating, a flickering dim legend) was perceptual or electrical and was found by looking at the LEDs.

## One-panel wiring test (`firmware/totem_panel_test/`)

Lights one 16x16 WS2812B panel on the HUB75 header's R1 pin (GPIO 5), with no Bluetooth, strap or `secrets.h`, so a dark panel can only mean wiring. Flash it before the totem the first time a panel is hooked up.

It loops: the whole panel red, green, blue (one second each; wrong colours mean the panel isn't GRB), then a white dot walking LED 0 to 255. Where the dot starts and whether the rows zigzag tells you the `PANEL_*` flags in `totem_config.h`. It runs at brightness 20 with a 0.5 A cap, so a laptop's USB is enough: plug the board's `USB-C` input into the computer and leave the battery out.

If nothing lights, the data wire is probably on the wrong HUB75 pin. The header carries no power, so moving it to another pin can't damage anything.

## Shows (`show_layout.h`)

Every `SHOW_EVERY_MIN` (5) minutes the heart rate steps aside for one show. Two turns go round, the spin then a message, so each comes round every 10 minutes, and each message turn plays the next phrase in `SHOW_PHRASES`. With `SHOW_SHIP 1` the Queen Mary is a third turn.

- **Spin:** the four faces spin round the ring `SHOW_SPIN_LAPS` (4) times, about 15 seconds, then bump into place and the zone bars flash. `SHOW_TICKER_NUMBERS 1` adds a stock ticker of your and your friend's heart rates racing past between them.
- **Messages:** the phrase scrolls the whole way round the ring three times in thin 7 px capitals, at a steady 6 columns a second (1 to 2 minutes). One panel is the seam: the letters fade in out of one side of it, its middle stays dark so the crowd can see where the line starts, and they fade out into its other side.
- **Queen Mary** (off by default): made for Dreamstate 2026, held right next to the RMS Queen Mary in Long Beach. A night sea with twinkling stars fades in and a three-funnel liner sails round the ring `SHOW_SHIP_LAPS` (3) times, about 53 seconds, her wake behind her, then fades away. `SHOW_SHIP 1` turns her on. The `Ship` struct in `show_layout.h` is the template for an event-specific show of your own.

Settings are section 7 of `totem_config.h`. `SHOW_TEST_MODE 1` plays every show back to back with 4 seconds of heart rate between. Each show runs at one steady brightness, worked out at its start from its brightest frame so it stays under `LED_MAX_MA`; the Queen Mary instead stays at the heart rate's brightness throughout (working hers out took long enough that she visibly jumped mid-lap). All of them need `RING_REVERSE` right: if text or faces jump between panels instead of flowing round, flip it.

## Phone remote

`phone_ble.cpp` on the totem, [`docs/remote/index.html`](remote/index.html) on the phone.

Scroll your own text right away, play the spin or a phrase now, pause the timed shows, set the brightness, turn the lights off and on, and see whether your strap and friends are connected and their heart rates.

**Lights off** blanks every LED but leaves Bluetooth running, so the phone can turn them back on (unplug the battery for truly off). Touching the thumb wheel, picking a brightness, or playing something from the phone turns them back on too. It isn't saved: a reboot always comes back lit.

**How it fits together.** Nothing goes through the internet. The totem advertises itself over Bluetooth as "Totem" and accepts one phone connection. The page is plain HTML served by GitHub Pages; once the phone has loaded it, the browser talks to the totem directly over Web Bluetooth. iPhone Safari has no Web Bluetooth, so use the free **Bluefy** browser.

**Hosting the page.** This repo's copy is at `https://noahsw.github.io/heart-rate-totem/remote/` once GitHub Pages is on (Settings > Pages > Deploy from a branch > `main`, folder `/docs`). The page works with any totem, so builders can just use that address. If you fork and change the protocol, turn Pages on in your fork the same way and use your own address.

The protocol is lines of text written to one characteristic (`PIN`, `SAY`, `PLAY`, `STOP`, `PAUSE`, `BRIGHT`, `POWER`; see `phone_protocol.h`), so a BLE app like LightBlue can drive the totem too. A wrong PIN costs 3 seconds; a phone that hasn't sent the PIN within 20 seconds is dropped, so a stranger can't hold the slot. The PIN goes over the air unencrypted: it keeps casual pranksters out, not someone sniffing Bluetooth.

The phone takes one of NimBLE's 3 connections (your strap, one friend, the phone). With a second friend configured, raise `MYNEWT_VAL_BLE_MAX_CONNECTIONS` as section 6 of `totem_config.h` describes. Settings are section 8; `PHONE_ENABLED 0` turns it off.

## Beat drop (`audio_drop.cpp`, `drop_detect.h`)

The board's two mics listen to the bass (about 40-150 Hz, the kick and bass line). A drop is the bass gone for at least 2 s while the music carries on (a breakdown and build), then back within 0.7 s, 10 dB or more above where the breakdown sat. Or the way house often does it: the build keeps some bass, the bass cuts out for a moment (even half a second), then comes back louder than the groove ever was, 5 dB over usual for a whole second. On a drop two rings in the heart's colour spread out from the heart on every face, one after another (about 1.2 s in all). Only while the heart rate is showing; at most one every 30 s.

It calibrates itself: the "usual" bass level is the 75th percentile of the last minute, learned only while the bass is in, so a long breakdown doesn't become the new normal and the same settings work in a living room and at a festival. Levels are measured net of the mic gain, which steps down 3 dB a second while the mic clips and up after 20 quiet seconds. Silence between sets isn't a breakdown, and neither is music fading up out of silence. The bass has to slam back (at least 8 dB up on a second earlier) and stay in (within 3 dB of usual for 0.2 s), so a sweep or a lone hit isn't a drop. It's tuned to miss a borderline drop rather than fire on something that isn't one.

The mics are on the **ES7210** (I2C 0x40), not the ES8311 (0x18, the speaker side). Audio power comes from a regulator switched by the PCA9557's IO5, off at power-up, so the firmware switches it on (read-modify-write, the wheel's pins untouched). I2S: MCLK IO38, BCLK IO48, LRCK IO21, ES7210 SDOUT1 IO47, 16 kHz, 16-bit stereo. From the board maker's schematic, and tested on the board with house music.

Settings are section 9 of `totem_config.h`. `DROP_ENABLED 0` leaves the mics unpowered. `DROP_TEST_MODE 1` shows a sound meter on all four faces instead of the heart rate, bursts in rainbow over it on a drop, and logs the levels 4 times a second. `DROP_DEMO 1` plays the burst every 2.5 s, music or not, to dial in the look. `DROP_ANIM 1` uses the rainbow burst in normal running too.

## Gotchas found on real hardware

**The Whoop connects, but the core's bundled BLE library says it failed.** The bundled `BLE` library always calls `ble_gattc_exchange_mtu()` as soon as the link comes up. The Whoop does the MTU exchange itself the instant it connects, so that call returns `BLE_HS_EALREADY`, which the library treats as fatal while leaving the link open. Every retry then fails with `Client busy` against its own live connection, so it looks like the Whoop is refusing you when it's actually connected. The fix, and the reason for the NimBLE-Arduino dependency, is `connect(..., exchangeMTU = false)` in `crew_ble.cpp`.

It is not your phone holding the connection: Whoop's HR Broadcast serves several centrals at once. Leave your phone's Bluetooth on.

**`find_whoop_mac/` still uses the bundled BLE library**, which is fine: scanning works, and the two libraries only clash if one sketch includes both. Don't add `NimBLEDevice.h` to it.

**NimBLE's GATT server must be set up before scanning starts.** Otherwise `ble_svc_gap_init` asserts and the board reboots in a loop. That's why `setup()` calls `phoneSetup()` before `bleSetup()`.

**Serial prints can freeze the LEDs.** With a computer plugged in but no monitor open, the USB port is "connected" but nobody reads, and each print blocks for up to ~2 s. `Serial.setTxTimeoutMs(0)` drops output instead.

**LED power while a computer is plugged in.** A laptop's USB port can't feed four panels, so while the board sees a USB host the cap drops to `LED_MAX_MA_USB_HOST` (500 mA). That's why the panels look dim on the bench.

**3.3 V data.** The board's HUB75 header drives the WS2812B data line at 3.3 V. Over the short lead in this build that has been fine; if the first panel flickers or shows random colours, a 74AHCT125 level shifter is the fix.

**The thumb wheel isn't on an ESP32 pin.** It's on the PCA9557 I/O expander on the I2C bus (IO1/IO2), which is why `Wire.begin()` comes before `brightnessSetup()`. Which K is up, down and press is a guess from the usual layout: the serial log names each key, so swap them in `config.h` if rolling up dims.
