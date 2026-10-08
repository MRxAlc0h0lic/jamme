/*  Long-Range Telemetry Node  —  RECEIVER (base station)
 *  Run this on a SECOND ESP32 + nRF24 (same wiring) so you can demo a live link.
 *  It prints every received packet to the Serial Monitor @115200.
 *
 *  Library: "RF24" by TMRh20
 */

#include <SPI.h>
#include <RF24.h>

#define CE_PIN   4
#define CSN_PIN  5

RF24 radio(CE_PIN, CSN_PIN);
const uint8_t address[6] = "NODE1";   // must match the transmitter

struct TelemetryPacket {
  uint32_t uptime_ms;
  uint16_t seq;
  int16_t  sensor;
};

TelemetryPacket pkt;

void setup() {
  Serial.begin(115200);
  delay(300);
  Serial.println(F("Telemetry RX base starting..."));

  if (!radio.begin()) {
    Serial.println(F("nRF24 not responding - check wiring / 10uF cap!"));
    while (1) { delay(1000); }
  }

  radio.setPALevel(RF24_PA_MAX);
  radio.setDataRate(RF24_250KBPS);
  radio.setChannel(100);              // MUST match the transmitter
  radio.enableDynamicPayloads();
  radio.openReadingPipe(1, address);
  radio.startListening();

  Serial.println(F("Radio ready (RX). Waiting for packets..."));
}

void loop() {
  if (radio.available()) {
    uint8_t len = radio.getDynamicPayloadSize();
    radio.read(&pkt, sizeof(pkt));
    Serial.printf("RX  seq=%u  uptime=%lums  sensor=%d  (len=%u)\n",
                  pkt.seq, pkt.uptime_ms, pkt.sensor, len);
  }
}
