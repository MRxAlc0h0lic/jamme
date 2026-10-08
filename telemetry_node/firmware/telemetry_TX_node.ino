/*  Long-Range Telemetry Node  —  TRANSMITTER (sensor node)
 *  Board: ESP32-WROOM-32 (30-pin) + nRF24L01+PA/LNA  (this PCB)
 *
 *  Wiring on this board (VSPI):
 *     nRF24 SCK  -> GPIO18     nRF24 CE   -> GPIO4
 *     nRF24 MISO -> GPIO19     nRF24 CSN  -> GPIO5
 *     nRF24 MOSI -> GPIO23     nRF24 IRQ  -> GPIO16 (unused here)
 *     nRF24 VCC  -> 3V3        nRF24 GND  -> GND   (+ 10uF & 100nF at socket)
 *
 *  Library: "RF24" by TMRh20  (Arduino Library Manager)
 *  It sends a small telemetry packet (uptime + a sensor reading) every second.
 */

#include <SPI.h>
#include <RF24.h>

#define CE_PIN   4
#define CSN_PIN  5

RF24 radio(CE_PIN, CSN_PIN);

// Shared 5-byte pipe address (must match the receiver)
const uint8_t address[6] = "NODE1";

struct TelemetryPacket {
  uint32_t uptime_ms;   // milliseconds since boot
  uint16_t seq;         // packet counter
  int16_t  sensor;      // e.g. temperature*100, light, ADC, etc.
};

TelemetryPacket pkt;
uint16_t counter = 0;

void setup() {
  Serial.begin(115200);
  delay(300);
  Serial.println(F("Telemetry TX node starting..."));

  if (!radio.begin()) {
    Serial.println(F("nRF24 not responding - check wiring / 10uF cap!"));
    while (1) { delay(1000); }
  }

  // Long-range friendly settings
  radio.setPALevel(RF24_PA_MAX);      // PA/LNA module -> full power
  radio.setDataRate(RF24_250KBPS);    // lowest rate = best range/sensitivity
  radio.setChannel(100);              // pick a quiet channel (0-125)
  radio.setRetries(5, 15);            // delay, count
  radio.enableDynamicPayloads();
  radio.openWritingPipe(address);
  radio.stopListening();

  Serial.println(F("Radio ready (TX)."));
}

void loop() {
  // --- replace this block with a real sensor reading if you have one ---
  int raw = analogRead(34);           // example: analog sensor on GPIO34 (ADC1)
  pkt.sensor    = (int16_t)raw;
  pkt.uptime_ms = millis();
  pkt.seq       = counter++;
  // --------------------------------------------------------------------

  bool ok = radio.write(&pkt, sizeof(pkt));
  Serial.printf("seq=%u sensor=%d -> %s\n", pkt.seq, pkt.sensor,
                ok ? "ACK" : "no ACK");

  delay(1000);
}
