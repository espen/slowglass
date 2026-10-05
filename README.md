# Youn Ink Four Color

This is a personal AI assistant project for ESP32-S3 e-paper devices. The current mainline consists of three parts: the ESP32 firmware, a Python backend service, and a management UI for images/todos/devices.

The focus of the project is not a general-purpose npm package but a system that actually runs on e-paper hardware: voice conversations, TTS playback, todo sync, weather/news/calendar/ebook/gallery pages, image transfer over AP mode, OTA firmware management, and a RawDraw UI adapted to four-color panels.

## The 2BP four-color image pipeline

![Youn Ink Four Color 2BP BWRY architecture](README-2bp-architecture.png)

Gallery images enter the server either from the PC/NAS management UI or from the device's AP-mode page, get converted to `2BP BWRY` (black, white, red, yellow), and are pushed to the ESP32-S3 four-color e-paper panel over Wi-Fi. The 2BP four-color pipeline in this repository is maintained independently from the NOTE4's 4BP black/white grayscale gallery: the panel colors, pixel format, and refresh driver all differ.

## Current status

- The backend has been switched to the Python service under `server/`; the old Node `scripts/` in the repo root has been removed.
- The firmware's main UI is rendered with RawDraw, designed for the four-color panel by default while keeping 1bpp black/white panel compatibility.
- Theming currently keeps a single default visual direction: a Nintendo-flavored four-color theme emphasizing semantic use of red, yellow, black, and white.
- Image transfer supports both 1bpp black/white and 2bpp four-color BWRY formats.
- The root `.gitignore` excludes build artifacts, logs, pid files, databases, local config, and key files.

## Directory layout

```text
.
├── firmware/        ESP-IDF firmware: RawDraw UI, page rendering, panel driver, AP image transfer
├── server/          Python backend: WebSocket conversations, TTS, discovery, image push, OTA API
├── frontend/        Management frontend source, with its own package/pnpm workflow
├── docs/            Historical design documents and implementation notes
├── documents/       Project materials
└── package.json     Repo-level helper commands only; no longer the entry point of the old Node service
```

Note: `firmware/scripts/` and `frontend/scripts/` are still in use — they belong to the firmware and frontend tooling respectively. What was removed is the legacy `scripts/` in the repo root.

## Backend service

The backend entry point is `server/llmserve.py`, best managed through `server/start.sh`. Default ports:

| Port | Protocol | Purpose |
| --- | --- | --- |
| `9001` | WebSocket | ESP32 voice, LLM, TTS, sync messages |
| `8766` | UDP | Device discovery |
| `8766` | HTTP | Image push, device image management, OTA API |
| `8090` | HTTP | Standalone management service, optional |

### Install dependencies

```bash
cd server
python3 -m venv .venv
source .venv/bin/activate
pip install -r requirements.txt
```

### Start the service

```bash
export DASHSCOPE_API_KEY=your_Alibaba_Bailian_API_key
cd server
./start.sh start
```

Common commands:

```bash
cd server
./start.sh status
./start.sh logs
./start.sh restart
./start.sh stop
```

They can also be invoked from the repo root:

```bash
npm run server:start
npm run server:status
npm run server:logs
```

### Simulate a device locally

```bash
cd server
python3 mock_client.py --server ws://127.0.0.1:9001
```

## Image and device management

The image HTTP API is served by `server/push_image.py` on port `8766`. It supports:

- Uploading an image file, converting it, and pushing it to the device.
- Choosing the `1bpp` black/white format or the `2bpp` four-color BWRY format.
- Listing the images on a device.
- Deleting images from a device.
- Uploading firmware and serving it for OTA download.

Common endpoints:

```bash
curl http://localhost:8766/api/status
curl http://localhost:8766/api/images
```

Image upload example:

```bash
curl -X POST http://localhost:8766/api/upload_image \
  -F "image=@/path/to/photo.jpg" \
  -F "format=bwry2bpp" \
  -F "title=Photo title"
```

Once the device enters AP image-transfer mode, connect your phone to the device hotspot and open:

```text
http://192.168.4.1
```

## Firmware

The firmware lives in `firmware/` and is based on ESP-IDF. It targets the ZecTrix ESP32-S3 4.2-inch e-paper board by default, supporting the four-color BWRY panel while keeping a 1bpp black/white panel configuration.

### Build

```bash
cd firmware
source ~/Documents/esp/v6.0/esp-idf/export.sh
idf.py build
```

Repo-root helper command:

```bash
npm run firmware:build
```

### Panel configuration

The firmware Kconfig offers a panel type selection:

```text
ZECTRIX_EPD_PANEL_4COLOR_SSD2683  four-color BWRY panel
ZECTRIX_EPD_PANEL_1BPP            black/white 1bpp panel
```

To flash back onto the old black/white panel, switch to `1bpp black/white EPD` in `idf.py menuconfig` first, then rebuild and flash. The RawDraw theme layer degrades the red/yellow semantic colors into readable black/white styles.

## UI notes

The firmware UI is built on the RawDraw component system. Key pages include:

- Chat: shows the user's speech, recognition status, and AI replies.
- Todo: local display, server sync, complete/delete/edit.
- Settings: volume, brightness, theme, network, sync, OTA, and more.
- Gallery: thumbnail list, full-size view, AP image-transfer entry point.
- Weather / weather detail, news, almanac, year progress, calendar, ebook, log.
- Quick-switch overlay: fast navigation between pages.

The four-color theme layer draws components through semantic styles; avoid adding more bare `RED/YELLOW/BLACK/WHITE` in feature pages. When adding UI, prefer RawDraw components and theme tokens.

## Environment variables

Common backend environment variables:

| Variable | Default | Description |
| --- | --- | --- |
| `DASHSCOPE_API_KEY` | none | Alibaba Bailian (DashScope) API key, required to start the backend |
| `LISTEN_HOST` | `0.0.0.0` | WebSocket listen address |
| `LISTEN_PORT` | `9001` | WebSocket port |
| `DISCOVERY_PORT` | `8766` | UDP discovery port |
| `PUSH_IMAGE_PORT` | `8766` | Image/OTA HTTP API port |
| `TTS_WS_CHUNK_BYTES` | `8000` | TTS push chunk size |
| `TTS_WS_CHUNK_GAP_SEC` | `0.01` | Delay between TTS chunks |

Never commit `.env`, databases, logs, pid files, build directories, or firmware artifacts.

## What to commit

Recommended to commit:

- Firmware sources such as `firmware/main/`, `firmware/components/`, `firmware/partitions/`.
- `server/*.py`, `server/static/`, `server/requirements.txt`, `server/DEPLOY.md`.
- Frontend sources such as `frontend/src/`, `frontend/package.json`, `frontend/pnpm-lock.yaml`.
- The root README, documentation, and config templates.

Do not commit:

- `firmware/build/`
- `firmware/managed_components/`
- `firmware/sdkconfig`
- `firmware/releases/`
- `server/.env`
- `server/todo.db`
- `server/*.pid`
- `server/*.log`
- `frontend/.env*`
- `frontend/dist/`
- `node_modules/`
