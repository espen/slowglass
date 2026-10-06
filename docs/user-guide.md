# Slowglass user guide

How to use the device day to day: buttons, pages, settings, the web UI, and
what happens on battery. This guide covers a default build (all widgets
enabled) on the ZecTrix ESP32-S3 4.2" e-paper board.

The device has three physical buttons: **UP**, **DOWN**, and **BOOT**.
A "long press" is holding the button for about one second.

> **E-paper is slow.** A full refresh takes several seconds, and button
> clicks pressed *while the panel is refreshing* are ignored, not queued.
> If nothing happens, wait for the flicker to finish and press again.

## Buttons at a glance

| Gesture | What it does |
| --- | --- |
| **UP / DOWN click** | Page-specific: browse photos, change month, scroll, move selection |
| **UP double-click** | Open/close the **Apps menu** (quick-switch between pages) |
| **DOWN long press** | Open **Settings** (works from any page) |
| **UP long press** | Leave Settings |
| **UP + DOWN held together** | Start the **WiFi setup hotspot** |
| **BOOT click** | Page-specific: select / confirm / open details |
| **BOOT long press** | Page-specific: on Gallery, start photo-transfer mode; on Weather/Door Sign, fetch data now; in any hotspot mode, exit it |
| **BOOT press (device asleep)** | **Wake the device** — BOOT is the only wake button |

Notes:

- DOWN doubles as the power key: if the device is fully off on battery,
  hold DOWN to power it on.
- A long press never also triggers a click — release and press again for
  a separate click.
- There are no double-click gestures on DOWN or BOOT.

## First-time setup (WiFi)

1. On first boot (or when no WiFi network is saved), the device opens an
   **open hotspot named `ZecTrix-XXXX`** and shows a "WiFi Setup" screen.
2. Join that hotspot from your phone or laptop. A setup page should open
   automatically (captive portal); if not, browse to `http://192.168.4.1`.
3. Pick your WiFi network, enter the password, and submit. The device
   leaves setup mode and connects on its own.

To redo WiFi setup later (new network, moved house): **hold UP and DOWN
together for a second**. To cancel setup mode without changing anything,
**long-press BOOT**.

Once connected, the clock sets itself via NTP (times are UTC unless the
firmware is configured otherwise) and data sources start fetching.

## Switching pages: the Apps menu

**Double-click UP** anywhere to open the Apps menu — a small window listing
every enabled page (home page first).

- **UP / DOWN** — move the selection (wraps around; five rows visible at a
  time, scrollbar appears when there are more)
- **BOOT** — open the selected page
- **UP double-click** again — close the menu without switching

Which pages appear here, and which page the device boots into, is runtime
configuration — see [Choosing pages](#choosing-pages-home-page-and-enabled-pages)
below. The defaults are Weather (home) and Settings.

## The status bar

Most pages show a status bar across the top:

- **WiFi bars** (left) — solid bars when connected; hollow bars with a
  slash when not.
- **Server marker** — a small starburst when the backend server is
  connected; a hollow square when WiFi is up but the server is not.
- **Date**, the **page title** (center), the **time**, and a **battery
  icon** (right). The battery shows 3/2/1 bars and turns **red at ≤15%** —
  that red icon is the only low-battery warning, so charge when you see it.

Full-bleed pages (Weather, Door Sign, fullscreen photos, the ebook reader)
hide the status bar.

## Pages

### Weather

A full-screen forecast dashboard (MET Norway data). It refreshes on its
own schedule (default: every 60 minutes).

- **BOOT long press** — fetch the forecast right now.

The location is auto-detected from your IP. If that's wrong (VPNs often
cause this), set it manually on the web UI's `/weather` page — see below.

### Door Sign (MakePlans)

Shows today's bookings and occupancy for a MakePlans room. Pair it from
the web UI's `/makeplans` page with a pairing code (see below). Once
paired it refreshes every 5 minutes, and on battery it wakes exactly at
booking boundaries so the sign flips on time.

- **BOOT long press** — fetch the schedule right now.

### Gallery (photos)

Browse uploaded photos. Two views: a "memory card" view (list + preview)
and fullscreen.

- **UP / DOWN** — previous / next photo
- **BOOT click** — toggle fullscreen
- **BOOT long press** — start **photo-transfer mode** (see below)

A slideshow can cycle photos automatically — set the interval in Settings
or in the web UI. Adding, deleting, captioning, and reordering photos is
done from the web UI, not on the device.

#### Transferring photos (hotspot mode)

From the Gallery, **long-press BOOT**. The device opens a hotspot:

- Network: **`InkScreen-AP`**, password **`12345678`**
- Browse to **`http://192.168.4.1`**

Upload any JPEG/PNG — the page letterboxes it to 400×300 and dithers it to
the panel's four colors before sending. **Long-press BOOT** again to exit.

If the device is on your home network with the LAN server running (see
[The web UI](#the-web-ui)), you can skip hotspot mode entirely and manage
photos at the device's LAN address.

### Calendar

A monthly grid.

- **UP / DOWN** — previous / next month
- **BOOT click** — enter day-selection mode, then **UP / DOWN** moves the
  cursor and **BOOT** confirms a day

### News

A headline list.

- **UP / DOWN** — move between headlines
- **BOOT click** — open the article preview. Inside the preview:
  **UP** scrolls up, **DOWN** switches between the footer buttons
  (close / read aloud), **BOOT** activates the focused button.

### Almanac

- **UP / DOWN** — previous / next month
- **BOOT long press** — jump back to today

### Year Progress

- **UP / DOWN** — step through the months (wraps around)
- **BOOT click** — back to the year overview

### Life Progress

Informational only — no buttons (the global gestures still work).

### Ebook

A plain-text reader. In the book list, **UP / DOWN** selects a file; in
the reader, **UP / DOWN** turn pages and **BOOT** returns to the list. The
page counter shows in the status bar.

> Note: opening a book from the list with BOOT is not wired up in the
> current firmware — the reader is only reachable if a book is already
> open.

### Chat

A conversation view for the voice-assistant backend (if you run one).

- **UP / DOWN** — scroll
- **BOOT click** — open the volume dialog (**UP/DOWN** adjust, **BOOT**
  saves)

### Log

The device's recent log lines, for troubleshooting.

- **UP / DOWN** — scroll
- **BOOT long press** — reload the log

## Settings

**Long-press DOWN** on any page to open Settings; **long-press UP** to
leave. The left pane lists categories, the right pane the options:

- **UP / DOWN** — move between options (wraps around)
- **BOOT** — activate / toggle the selected option

| Category | Option | What it does |
| --- | --- | --- |
| System | **Restart** | Reboots the device immediately |
| Gallery | **Slideshow** | Cycles the photo slideshow interval: Off → 5min → 10min → 30min → Off. While a slideshow is on, the device won't go to sleep on USB power |
| Network | **Wi-Fi** | Connect / disconnect from the saved WiFi network |
| Network | **LAN Server** | Start / stop the configuration web server on your home network. Needs WiFi. While it runs, the device stays awake |
| Network | **LAN IP** | Shows the device's address on your network (read-only) |
| Power Saving | **Sleep now** | Puts the device into deep sleep immediately. Press **BOOT** to wake |
| About | | Device name, firmware version, hardware, serial number, and project origin (a fork of youn-ink-fourcolor) |

## The web UI

When the device is on WiFi **and powered over USB**, it automatically
starts a web server on your home network. The address is shown in
Settings → Network → LAN IP; open `http://<that address>/` in a browser.
(On battery the server doesn't auto-start — it would drain the battery —
but you can switch it on manually in Settings.)

| Page | What you can do |
| --- | --- |
| `/` | Upload photos (auto-converted to the panel's colors), view/delete/reorder the gallery, edit photo captions, show a specific photo on the panel, set the slideshow interval, stop the service, or put the device to sleep |
| `/weather` | Set the weather location manually (city label, latitude, longitude, UTC offset), or switch back to automatic IP-based location |
| `/makeplans` | Pair the door sign: enter your MakePlans account name, the room's resource ID, and a pairing code (generated in MakePlans admin under the resource's "Room display" — codes are valid for 10 minutes). You can also unpair, trigger a fetch, or set the NFC booking link |

The same pages are also served in photo-transfer hotspot mode at
`http://192.168.4.1`.

### Choosing pages (home page and enabled pages)

Which page the device boots into, which pages the Apps menu lists, and how
often data sources refresh are runtime settings, changed over HTTP:

```bash
# See the current config
curl http://<device>/api/pages

# Set the home page and the Apps-menu list
curl -X POST http://<device>/api/pages \
  -d '{"home":"doorsign","enabled":["doorsign","weather","gallery","settings"]}'

# Change fetch intervals (minutes; takes effect after the next restart/wake)
curl -X POST http://<device>/api/pages -d '{"intervals":{"weather":30}}'
```

Valid page names: `weather`, `doorsign`, `gallery`, `calendar`, `news`,
`almanac`, `lifebar`, `yearprogress`, `ebook`, `chat`, `log`, `settings`
(only pages compiled into your build are accepted). The home page is
always included in the Apps menu automatically. Home/enabled changes apply
immediately — no reboot needed.

Interval keys are data-source names: `weather` (default 60 min) and
`makeplans` (default 5 min, listed once the door sign is paired). An
interval of `0` disables that source.

## Power and sleep

The device behaves differently on USB power and on battery:

**On USB power** it generally stays awake: the LAN web server auto-starts
and keeps it up, and an active slideshow also prevents sleep. With the
server stopped and the slideshow off, it deep-sleeps after an idle period
(default 30 minutes) and wakes only when you press **BOOT**.

**On battery** it duty-cycles: it stays awake for about 4 minutes after
waking, then deep-sleeps until the next scheduled data fetch (at most 60
minutes; the door sign wakes earlier around booking boundaries). The image
on the panel stays visible while asleep — that's the point of e-paper.
Press **BOOT** at any time to wake it early.

**Maximum battery life:** pick a photo (or page) you want displayed, turn
the slideshow off, then use Settings → Power Saving → **Sleep now** (or
"Stop & sleep" in the web UI). The panel keeps showing the last image
indefinitely; press **BOOT** when you want the device back.

## Quick troubleshooting

- **Button presses do nothing** — the panel is probably mid-refresh;
  presses during a refresh are dropped. Wait for the flicker to stop.
- **Device seems off** — it's most likely deep-sleeping; press **BOOT**.
  If it's truly powered off on battery, hold **DOWN** to power on.
- **Can't reach the web UI** — the LAN server only auto-starts on USB
  power. Check Settings → Network: WiFi must say Connected, and LAN
  Server shows the address when running (toggle it on with BOOT if not).
- **Weather shows the wrong place** — set the location manually at
  `http://<device>/weather`.
- **WiFi changed** — hold **UP + DOWN** together to reopen the setup
  hotspot.
