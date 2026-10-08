# Long-Range Wireless Telemetry Node — PCB Design Package

**Designer:** Youssif Ahmed Zayed — Business Technology, CIC
**Project:** ESP32 + nRF24L01+PA/LNA long-range telemetry node (2-layer PCB)
**Rev:** A — 2026-10-08

A battery-powered wireless sensor node. An **ESP32-WROOM-32** reads a sensor and
transmits the data over an **nRF24L01+PA/LNA** 2.4 GHz radio to a matching
receiver, up to ~1 km line-of-sight. Powered by a 3.7 V Li-ion cell charged over
USB-C through a **TP4056** module, with a **12 mm latching ON/OFF switch**.

---

## 1. Files in this package

| File | What it is |
|------|------------|
| `schematic.png` / `schematic.svg` | Full schematic (print this for the report) |
| `telemetry_node.net` | KiCad netlist — import into KiCad to get all parts + connections |
| `connections.csv` | Human-readable pin-to-pin connection table |
| `firmware/telemetry_TX_node.ino` | Transmitter sketch (this board) |
| `firmware/telemetry_RX_base.ino` | Receiver sketch (second ESP32+nRF24 for the demo) |
| `README.md` | This document |

---

## 2. Bill of Materials (BOM)

| Ref | Part | Value / Model | Mounting | Footprint (KiCad lib) |
|-----|------|---------------|----------|-----------------------|
| U1 | ESP32-WROOM-32 DevKit, 30-pin | module | **PTH** on 2×(1×15) female headers | `Connector_PinHeader_2.54mm:PinHeader_1x15_P2.54mm_Vertical` ×2 (J1, J2) |
| M2 | nRF24L01+ PA/LNA | module | **PTH** on 2×4 header | `Connector_PinHeader_2.54mm:PinHeader_2x04_P2.54mm_Vertical` (J3) |
| M1 | TP4056 Type-C charger (DW01+8205A) | module | SMD module *(exception)* | 1×4 pad row — wire B+/B−/OUT+/OUT− |
| SW1 | ON/OFF switch | 12 mm latching, metal | **PTH** (wired) | `PinHeader_1x02_P2.54mm_Vertical` |
| BT1 / J5 | Li-ion cell | 3.7 V 650 mAh | battery *(exception)* | `PinHeader_1x02_P2.54mm_Vertical` |
| C1 | Electrolytic cap | 10 µF / 16 V | **PTH** radial | `Capacitor_THT:CP_Radial_D5.0mm_P2.00mm` |
| C2 | Ceramic cap | 100 nF | **PTH** disc | `Capacitor_THT:C_Disc_D5.0mm_W2.5mm_P5.00mm` |
| C3 | Electrolytic cap | 10 µF / 16 V | **PTH** radial | `Capacitor_THT:CP_Radial_D5.0mm_P2.00mm` |

> Every soldered part is **plated-through-hole (PTH)** as required, except the
> **TP4056 module** and the **battery**.

---

## 3. Connections (summary)

**Power chain:** `BATT+/− → TP4056 B+/B− → OUT+ → SW1 → VSYS → ESP32 VIN`.
`TP4056 OUT− = system GND`. `ESP32 3V3 (regulator output) → nRF24 VCC`.

**SPI bus (VSPI) + control:**

| Signal | ESP32 pin | nRF24 pin |
|--------|-----------|-----------|
| SCK  | GPIO18 (D18) | SCK |
| MISO | GPIO19 (D19) | MISO |
| MOSI | GPIO23 (D23) | MOSI |
| CE   | GPIO4  (D4)  | CE  |
| CSN  | GPIO5  (D5)  | CSN |
| IRQ  | GPIO16 (D16) | IRQ (optional, unused in firmware) |
| VCC  | 3V3 | VCC |
| GND  | GND | GND |

Full node list is in `connections.csv`.

---

## 4. ⚠️ One design decision to confirm (power rail)

The 30-pin ESP32 DevKit uses an **AMS1117-3.3** regulator with ~1.1 V dropout.
Fed from a single Li-ion cell (3.0–4.2 V), the 3V3 rail can **sag when the
battery is low**. Three ways to handle it, pick one for your report:

1. **Simplest (as drawn):** battery → VIN. Works well when the cell is charged
   (≈3.9–4.2 V); the 3V3 rail may droop below ~3.6 V cell voltage. Fine for a
   demo. *(This is what the netlist/schematic implement.)*
2. **Most robust:** add a small **5 V boost module** between SW1 and VIN. Rail
   stays solid the whole discharge curve. Add one 1×2 footprint for it.
3. **Clean low-power:** replace the DevKit regulator path with a dedicated
   **3.3 V LDO** (e.g. AMS1117-3.3 or HT7333) fed from VSYS, output to the ESP32
   3V3 pin and nRF24 — only if you drive the bare module, not the DevKit.

Recommendation for a fast, reliable submission: **option 1**, and mention
option 2 in your report as the production improvement. Either way, keep **C1
(10 µF) + C2 (100 nF) right at the nRF24 socket** — the PA/LNA module browns-out
without it; this is the #1 cause of "nRF24 not responding".

---

## 5. Build it in KiCad (fast path)

1. New KiCad project → open **PCB Editor (Pcbnew)**.
2. `File → Import → Netlist…` → choose `telemetry_node.net` → **Update PCB**.
   All 9 parts appear with footprints already assigned and a ratsnest showing
   every connection.
3. If any footprint shows as missing, it's only a library-path issue — click the
   part and pick the same name from the standard KiCad `Connector_PinHeader_2.54mm`
   / `Capacitor_THT` libraries (they ship with KiCad).
4. Set board to **2 layers** (`File → Board Setup → Physical Stackup`).
5. Place and route per section 6, run **DRC**, then `File → Fabrication Outputs →
   Gerbers` for the factory.

> Prefer a drawn schematic in KiCad? Open **Schematic Editor**, drop the symbols
> (`Conn_01x15` ×2, `Conn_02x04_Odd_Even`, `Conn_01x02` ×2, `Conn_01x04`, `CP`,
> `C`), wire them per `connections.csv`, assign the footprints above, then
> `Tools → Update PCB from Schematic`. The netlist is the shortcut; this is the
> "full schematic" route if the assignment requires an eeschema file.

---

## 6. 2-layer layout & routing guide

**Board size:** ~70 × 45 mm is comfortable. Keep the nRF24 antenna end hanging
off one edge.

**Placement**
- ESP32 centered; its two header rows are the backbone. **Measure your board's
  row spacing with calipers** (common 30-pin boards are 25.4 mm or 28 mm between
  rows) and set the J1↔J2 gap to match before routing.
- nRF24 (J3) near the ESP32 SPI pins (right side), **antenna pointing off the
  PCB edge**.
- TP4056 near the board edge so the USB-C port is reachable from outside the case.
- SW1 and battery leads at the opposite edge from the antenna.
- C1/C2 **as close as physically possible** to J3 pins 1 (GND) and 2 (VCC).

**Layers & copper**
- **Top = signal + power**, **Bottom = ground pour** (filled zone tied to GND).
- Pour a GND copper zone on **both** layers; stitch them with several vias.
- Keep the ground pour **cleared back under and around the nRF24 antenna**
  (a "keep-out" with no copper) so it doesn't detune the antenna.

**Trace widths**
- Power (`VSYS`, `+3V3`, `GND`, `VBAT_SW`, battery nets): **0.6–0.8 mm**.
- SPI / control signals: **0.3 mm** is plenty.

**Good practice**
- Keep SPI traces short and roughly equal; avoid running them under the radio.
- Add 2–4 **M3 mounting holes** at the corners.
- Silkscreen: label J1/J2 pin 1, the nRF24 orientation, polarity of C1/C3, and
  B+/B−/OUT+/OUT− on the TP4056 pads so assembly is foolproof.
- Run **DRC** (clearance ≥ 0.2 mm, via ≥ 0.3 mm) before exporting Gerbers.

---

## 7. Firmware / live demo

Two ESP32+nRF24 boards make the best demo: flash one with
`telemetry_TX_node.ino` and the other with `telemetry_RX_base.ino` (Arduino IDE,
**RF24 by TMRh20** library, board = "ESP32 Dev Module", 115200 baud). The
receiver's Serial Monitor prints each packet (sequence, uptime, sensor value) —
clear proof the link works. Both use channel 100 and `RF24_250KBPS` for maximum
range; change the channel if the air is congested.

---

## 8. What to hand in

- `schematic.png` (schematic sheet)
- KiCad PCB with 2 layers routed + DRC clean, and exported Gerbers
- This README as the design report (BOM, connections, layout rationale)
- Optional: a short video of the TX→RX Serial output for the demo
