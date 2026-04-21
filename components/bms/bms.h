#pragma once
#ifdef USE_ESP32_FRAMEWORK_ESP_IDF

#include "esphome/core/component.h"
#include "esphome/components/ble_client/ble_client.h"
#include "esphome/components/sensor/sensor.h"
#include "esphome/components/binary_sensor/binary_sensor.h"

namespace esphome {
namespace bms {

// BLE Protocol — Topband BMS (Ective / Wattstunde / Startcraft LiFePO4)
//
// Tested with:
//   Ective 200 Ah LiFePO4
//   Wattstunde 200 Ah LiFePO4
//
// Service UUID:           0xFFE0
// Notify Characteristic:  0xFFE4  (handle 0x0016)
// CCCD:                   handle 0x0017
//
// The BMS sends unsolicited notifications every ~800 ms. Each notification
// is a self-contained packet, identified by length and content. All numeric
// values are little-endian.
//
// Packet types:
//
//   len=17, data[0]=0x5E  — Ective current packet
//     data[1..16] = 16 ASCII hex chars → 8 decoded bytes
//     decoded[0..3] = uint32 LE total voltage in mV, rounded to 100 mV (too
//                     coarse to use; cell-sum from len=16 packet preferred)
//     decoded[4..7] = int32 LE, unit 1 mA, offset 600 (600 = 0 A)
//
//   len=17, data[0]=0xE8  — Wattstunde current packet
//     data[1..16] = 16 ASCII hex chars → 8 decoded bytes
//     decoded[0..3] = uint32 LE total voltage in mV, rounded to 100 mV (same
//                     caveat as above)
//     decoded[4..7] = int32 LE signed, unit 1 mA (negative = discharge)
//
//   len=12, all ASCII hex, data[4..11]='0'  — Temperature packet
//     data[0..3] = 4 ASCII hex chars → 2 decoded bytes
//     decoded[0..1] = uint16 LE, Kelvin×10  →  raw/10 - 273.15 = °C
//
//   len=16, all ASCII hex  — Cell voltages or static battery info
//     decoded[0..7] = 4× uint16 LE in mV (range 2500–4200) → cell voltages
//     if not valid cell range: uint32 capacity_mah, uint16 cycles, uint16 soc_%

static constexpr uint16_t NOTIFY_HANDLE = 0x0016;
// Rolling window size for link quality tracking (one slot per BMS update cycle ~1 s).
static constexpr uint8_t LINK_QUALITY_WINDOW = 100;
static_assert(LINK_QUALITY_WINDOW <= 255, "sum of 0/1 values must fit in uint8_t (max 255)");
static_assert(LINK_QUALITY_WINDOW > 0, "LINK_QUALITY_WINDOW must be > 0 to avoid division by zero");
// Minimum discharge current (A) below which runtime is not calculated.
static constexpr float RUNTIME_MIN_CURRENT_A = 0.1f;
// Timeout (ms) without a valid frame before a missed cycle is recorded.
static constexpr uint32_t HEARTBEAT_TIMEOUT_MS = 15000;
// LiFePO4 cell voltage range used to distinguish cell packets from static info packets.
static constexpr uint16_t CELL_MV_MIN = 2500;
static constexpr uint16_t CELL_MV_MAX = 4200;
// Temperature sanity range: Kelvin×10, corresponds to 0°C..80°C.
static constexpr uint16_t TEMP_KELVIN10_MIN = 2731;
static constexpr uint16_t TEMP_KELVIN10_MAX = 3532;
// Ective current packet: raw value 600 = 0 A (offset in mA).
static constexpr int32_t ECTIVE_CURRENT_OFFSET_MA = 600;
// EMA smoothing factor for current used in runtime calculation (α=0.1 ≈ 10-packet window).
static constexpr float CURRENT_EMA_ALPHA = 0.1f;
// Static info packet: valid capacity range in mAh (10 Ah..1000 Ah).
static constexpr uint32_t CAPACITY_MAH_MIN = 10000;
static constexpr uint32_t CAPACITY_MAH_MAX = 1000000;
// Hysteresis thresholds (mA) for the battery_charging binary sensor to avoid
// flipping around the zero-current point. Enter charging state at +200 mA,
// leave it at -200 mA; between the thresholds the previous state is held.
static constexpr int32_t CHARGING_ON_THRESHOLD_MA = 200;
static constexpr int32_t CHARGING_OFF_THRESHOLD_MA = -200;

class BMS : public Component, public ble_client::BLEClientNode {
 public:
  void setup() override;
  void loop() override;
  void dump_config() override;

  void gattc_event_handler(esp_gattc_cb_event_t event, esp_gatt_if_t gattc_if,
                            esp_ble_gattc_cb_param_t *param) override;

  void set_soc_sensor(sensor::Sensor *s) { soc_sensor_ = s; }
  void set_voltage_sensor(sensor::Sensor *s) { voltage_sensor_ = s; }
  void set_current_sensor(sensor::Sensor *s) { current_sensor_ = s; }
  void set_power_sensor(sensor::Sensor *s) { power_sensor_ = s; }
  void set_cell_voltage_sensor_1(sensor::Sensor *s) { cell_voltage_sensor_[0] = s; }
  void set_cell_voltage_sensor_2(sensor::Sensor *s) { cell_voltage_sensor_[1] = s; }
  void set_cell_voltage_sensor_3(sensor::Sensor *s) { cell_voltage_sensor_[2] = s; }
  void set_cell_voltage_sensor_4(sensor::Sensor *s) { cell_voltage_sensor_[3] = s; }
  void set_capacity_sensor(sensor::Sensor *s) { capacity_sensor_ = s; }
  void set_cycles_sensor(sensor::Sensor *s) { cycles_sensor_ = s; }
  void set_temperature_sensor(sensor::Sensor *s) { temperature_sensor_ = s; }
  void set_delta_voltage_sensor(sensor::Sensor *s) { delta_voltage_sensor_ = s; }
  void set_avg_cell_voltage_sensor(sensor::Sensor *s) { avg_cell_voltage_sensor_ = s; }
  void set_min_cell_voltage_sensor(sensor::Sensor *s) { min_cell_voltage_sensor_ = s; }
  void set_max_cell_voltage_sensor(sensor::Sensor *s) { max_cell_voltage_sensor_ = s; }
  void set_runtime_sensor(sensor::Sensor *s) { runtime_sensor_ = s; }
  void set_remaining_wh_sensor(sensor::Sensor *s) { remaining_wh_sensor_ = s; }
  void set_remaining_ah_sensor(sensor::Sensor *s) { remaining_ah_sensor_ = s; }
  void set_charge_power_sensor(sensor::Sensor *s) { charge_power_sensor_ = s; }
  void set_discharge_power_sensor(sensor::Sensor *s) { discharge_power_sensor_ = s; }
  void set_soh_sensor(sensor::Sensor *s) { soh_sensor_ = s; }
  // Nominal (nameplate) capacity in Ah, used to compute State of Health.
  // If 0 (default), SoH is not calculated.
  void set_nominal_capacity_ah(float ah) { nominal_capacity_ah_ = ah; }
  void set_link_quality_sensor(sensor::Sensor *s) { link_quality_sensor_ = s; }
  void set_connected_sensor(binary_sensor::BinarySensor *s) { connected_sensor_ = s; }
  void set_battery_charging_sensor(binary_sensor::BinarySensor *s) { battery_charging_sensor_ = s; }
  void set_cell_imbalance_sensor(binary_sensor::BinarySensor *s) { cell_imbalance_sensor_ = s; }
  void set_cell_imbalance_threshold(uint16_t threshold_mv) { cell_imbalance_threshold_mv_ = threshold_mv; }

 protected:
  void parse_notification_(const uint8_t *data, uint16_t len);
  void publish_derived_();
  void push_link_quality_(bool success);
  static bool decode_ascii_hex_(const uint8_t *ascii, size_t ascii_len, uint8_t *out, size_t out_len);
  static bool is_valid_hex_char_(uint8_t c);
  static inline uint8_t hex_nibble_(uint8_t c);

  sensor::Sensor *soc_sensor_{nullptr};
  sensor::Sensor *voltage_sensor_{nullptr};
  sensor::Sensor *current_sensor_{nullptr};
  sensor::Sensor *power_sensor_{nullptr};
  sensor::Sensor *cell_voltage_sensor_[4]{nullptr, nullptr, nullptr, nullptr};
  sensor::Sensor *capacity_sensor_{nullptr};
  sensor::Sensor *cycles_sensor_{nullptr};
  sensor::Sensor *temperature_sensor_{nullptr};
  sensor::Sensor *avg_cell_voltage_sensor_{nullptr};
  sensor::Sensor *delta_voltage_sensor_{nullptr};
  sensor::Sensor *min_cell_voltage_sensor_{nullptr};
  sensor::Sensor *max_cell_voltage_sensor_{nullptr};
  sensor::Sensor *runtime_sensor_{nullptr};
  sensor::Sensor *remaining_wh_sensor_{nullptr};
  sensor::Sensor *remaining_ah_sensor_{nullptr};
  sensor::Sensor *charge_power_sensor_{nullptr};
  sensor::Sensor *discharge_power_sensor_{nullptr};
  sensor::Sensor *soh_sensor_{nullptr};
  sensor::Sensor *link_quality_sensor_{nullptr};
  float nominal_capacity_ah_{0.0f};
  binary_sensor::BinarySensor *connected_sensor_{nullptr};
  binary_sensor::BinarySensor *battery_charging_sensor_{nullptr};
  binary_sensor::BinarySensor *cell_imbalance_sensor_{nullptr};
  uint16_t cell_imbalance_threshold_mv_{50};

  char tag_[32]{};

  bool connected_{false};
  uint32_t last_voltage_mv_{0};

  // Cached values for heartbeat and capacity derivation
  float last_soc_{NAN};
  float last_capacity_ah_{NAN};
  float last_current_a_{NAN};
  float smoothed_current_a_{NAN};

  // Link quality: rolling window ring buffer
  uint8_t link_quality_buf_[LINK_QUALITY_WINDOW]{};
  uint8_t link_quality_idx_{0};
  uint8_t link_quality_sum_{0};
  uint32_t last_heartbeat_ms_{0};

  // Charging-sensor hysteresis: retained state between updates so that small
  // current fluctuations around 0 A do not flip the binary sensor.
  bool charging_state_{false};

  void update_charging_state_(int32_t current_ma);
};

}  // namespace bms
}  // namespace esphome

#endif  // USE_ESP32_FRAMEWORK_ESP_IDF
