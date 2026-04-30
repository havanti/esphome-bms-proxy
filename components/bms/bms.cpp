#include "bms.h"
#ifdef USE_ESP32_FRAMEWORK_ESP_IDF

#include "esphome/core/log.h"
#include <algorithm>
#include <cmath>
#include <cstdio>
#include <cstring>

namespace esphome {
namespace bms {

void BMS::setup() {
  snprintf(tag_, sizeof(tag_), "bms@%s", this->parent_->address_str());
  ESP_LOGCONFIG(tag_, "BMS component setup");
}

void BMS::loop() {
  if (!connected_.load()) return;
  uint32_t hb = last_heartbeat_ms_.load();
  if (hb == 0) return;
  uint32_t now = millis();
  if (now - hb > HEARTBEAT_TIMEOUT_MS) {
    uint32_t elapsed = now - hb;
    uint32_t missed = std::min(elapsed / BMS_PACKET_INTERVAL_MS, static_cast<uint32_t>(LINK_QUALITY_WINDOW));
    for (uint32_t i = 0; i < missed; i++)
      push_link_quality_(false);
    last_heartbeat_ms_.store(now);
  }
}

void BMS::dump_config() {
  ESP_LOGCONFIG(tag_, "Topband BMS (Ective/Wattstunde):");
  ESP_LOGCONFIG(tag_, "  MAC: %s", this->parent_->address_str());
  ESP_LOGCONFIG(tag_, "  Service UUID: 0xFFE0");
  ESP_LOGCONFIG(tag_, "  Notify Characteristic: 0xFFE4  Handle: 0x%04X", NOTIFY_HANDLE);
  if (nominal_capacity_ah_ > 0.0f)
    ESP_LOGCONFIG(tag_, "  Nominal Capacity: %.1f Ah (SoH enabled)", nominal_capacity_ah_);
  else
    ESP_LOGCONFIG(tag_, "  Nominal Capacity: not set (SoH disabled)");
}

void BMS::gattc_event_handler(esp_gattc_cb_event_t event, esp_gatt_if_t gattc_if,
                               esp_ble_gattc_cb_param_t *param) {
  switch (event) {
    case ESP_GATTC_OPEN_EVT:
      if (param->open.status == ESP_GATT_OK) {
        ESP_LOGI(tag_, "Connected to BMS (%s)", this->parent_->address_str());
        connected_.store(true);
        // Seed the heartbeat clock so the loop timeout is armed even if the BMS
        // never delivers a cell-voltage packet (otherwise link quality would
        // stay silent forever on a half-working link).
        last_heartbeat_ms_.store(millis());
        if (connected_sensor_ != nullptr)
          connected_sensor_->publish_state(true);
      } else {
        ESP_LOGW(tag_, "Connection failed: status=%d", param->open.status);
      }
      break;

    case ESP_GATTC_DISCONNECT_EVT:
      ESP_LOGI(tag_, "Disconnected from BMS (%s)", this->parent_->address_str());
      connected_.store(false);
      last_voltage_mv_ = 0;
      last_soc_ = NAN;
      last_capacity_ah_ = NAN;
      last_current_a_ = NAN;
      smoothed_current_a_ = NAN;
      last_heartbeat_ms_.store(0);
      memset(link_quality_buf_, 0, sizeof(link_quality_buf_));
      link_quality_sum_ = 0;
      link_quality_idx_ = 0;
      charging_state_ = false;
      if (connected_sensor_ != nullptr) connected_sensor_->publish_state(false);
      if (soc_sensor_ != nullptr) soc_sensor_->publish_state(NAN);
      if (voltage_sensor_ != nullptr) voltage_sensor_->publish_state(NAN);
      if (current_sensor_ != nullptr) current_sensor_->publish_state(NAN);
      if (power_sensor_ != nullptr) power_sensor_->publish_state(NAN);
      if (capacity_sensor_ != nullptr) capacity_sensor_->publish_state(NAN);
      if (cycles_sensor_ != nullptr) cycles_sensor_->publish_state(NAN);
      if (temperature_sensor_ != nullptr) temperature_sensor_->publish_state(NAN);
      if (avg_cell_voltage_sensor_ != nullptr) avg_cell_voltage_sensor_->publish_state(NAN);
      if (delta_voltage_sensor_ != nullptr) delta_voltage_sensor_->publish_state(NAN);
      if (min_cell_voltage_sensor_ != nullptr) min_cell_voltage_sensor_->publish_state(NAN);
      if (max_cell_voltage_sensor_ != nullptr) max_cell_voltage_sensor_->publish_state(NAN);
      if (runtime_sensor_ != nullptr) runtime_sensor_->publish_state(NAN);
      if (remaining_wh_sensor_ != nullptr) remaining_wh_sensor_->publish_state(NAN);
      if (remaining_ah_sensor_ != nullptr) remaining_ah_sensor_->publish_state(NAN);
      if (charge_power_sensor_ != nullptr) charge_power_sensor_->publish_state(NAN);
      if (discharge_power_sensor_ != nullptr) discharge_power_sensor_->publish_state(NAN);
      if (soh_sensor_ != nullptr) soh_sensor_->publish_state(NAN);
      if (link_quality_sensor_ != nullptr) link_quality_sensor_->publish_state(NAN);
      if (battery_charging_sensor_ != nullptr) battery_charging_sensor_->publish_state(false);
      if (cell_imbalance_sensor_ != nullptr) cell_imbalance_sensor_->publish_state(false);
      for (auto *s : cell_voltage_sensor_)
        if (s != nullptr) s->publish_state(NAN);
      break;

    case ESP_GATTC_SEARCH_CMPL_EVT: {
      ESP_LOGI(tag_, "Service discovery done — subscribing to notify handle 0x%04X", NOTIFY_HANDLE);
      esp_ble_gattc_register_for_notify(gattc_if, this->parent_->get_remote_bda(), NOTIFY_HANDLE);
      uint8_t notify_en[] = {0x01, 0x00};
      esp_ble_gattc_write_char_descr(gattc_if, this->parent_->get_conn_id(),
                                      CCCD_HANDLE, sizeof(notify_en),
                                      notify_en, ESP_GATT_WRITE_TYPE_RSP,
                                      ESP_GATT_AUTH_REQ_NONE);
      break;
    }

    case ESP_GATTC_REG_FOR_NOTIFY_EVT:
      ESP_LOGD(tag_, "register_for_notify: status=%d handle=0x%04X",
               param->reg_for_notify.status, param->reg_for_notify.handle);
      break;

    case ESP_GATTC_WRITE_DESCR_EVT:
      ESP_LOGD(tag_, "CCCD write: status=%d", param->write.status);
      break;

    case ESP_GATTC_NOTIFY_EVT:
      if (param->notify.handle == NOTIFY_HANDLE)
        parse_notification_(param->notify.value, param->notify.value_len);
      break;

    default:
      break;
  }
}

void BMS::update_charging_state_(int32_t current_ma) {
  if (current_ma > CHARGING_ON_THRESHOLD_MA) {
    charging_state_ = true;
  } else if (current_ma < CHARGING_OFF_THRESHOLD_MA) {
    charging_state_ = false;
  }
  // Between the thresholds the previous state is held — avoids flapping.
  if (battery_charging_sensor_ != nullptr)
    battery_charging_sensor_->publish_state(charging_state_);
}

void BMS::push_link_quality_(bool success) {
  link_quality_sum_ -= link_quality_buf_[link_quality_idx_];
  link_quality_buf_[link_quality_idx_] = success ? 1 : 0;
  link_quality_sum_ += link_quality_buf_[link_quality_idx_];
  link_quality_idx_ = (link_quality_idx_ + 1) % LINK_QUALITY_WINDOW;
  if (link_quality_sensor_ != nullptr)
    link_quality_sensor_->publish_state(link_quality_sum_ * 100 / LINK_QUALITY_WINDOW);
}

void BMS::publish_derived_() {
  // Power, charge power, discharge power: needs current and voltage
  if (!std::isnan(last_current_a_) && last_voltage_mv_ > 0) {
    float voltage_v = last_voltage_mv_ / 1000.0f;
    float power_w = voltage_v * last_current_a_;
    if (power_sensor_ != nullptr)
      power_sensor_->publish_state(power_w);
    if (charge_power_sensor_ != nullptr)
      charge_power_sensor_->publish_state(last_current_a_ > 0.0f ? power_w : 0.0f);
    if (discharge_power_sensor_ != nullptr)
      discharge_power_sensor_->publish_state(last_current_a_ < 0.0f ? -power_w : 0.0f);
  }

  // Remaining Ah: needs SOC and capacity (reused below for Wh and runtime)
  float remaining_ah = NAN;
  if (!std::isnan(last_soc_) && !std::isnan(last_capacity_ah_))
    remaining_ah = (last_soc_ / 100.0f) * last_capacity_ah_;

  if (!std::isnan(remaining_ah)) {
    if (remaining_ah_sensor_ != nullptr)
      remaining_ah_sensor_->publish_state(remaining_ah);
    if (last_voltage_mv_ > 0) {
      float remaining_wh = remaining_ah * (last_voltage_mv_ / 1000.0f);
      if (remaining_wh_sensor_ != nullptr)
        remaining_wh_sensor_->publish_state(remaining_wh);
    }
  }

  // State of Health: needs measured capacity and user-configured nominal capacity
  if (!std::isnan(last_capacity_ah_) && nominal_capacity_ah_ > 0.0f) {
    float soh = std::min((last_capacity_ah_ / nominal_capacity_ah_) * 100.0f, 100.0f);
    if (soh_sensor_ != nullptr)
      soh_sensor_->publish_state(soh);
  }

  // Runtime (min): smoothed current avoids wild swings at low load
  if (!std::isnan(remaining_ah) && !std::isnan(smoothed_current_a_)) {
    if (smoothed_current_a_ < -RUNTIME_MIN_CURRENT_A) {
      float runtime_min = (remaining_ah / (-smoothed_current_a_)) * 60.0f;
      if (runtime_sensor_ != nullptr)
        runtime_sensor_->publish_state(runtime_min);
    } else {
      if (runtime_sensor_ != nullptr)
        runtime_sensor_->publish_state(NAN);
    }
  }
}

bool BMS::is_valid_hex_char_(uint8_t c) {
  return (c >= '0' && c <= '9') || (c >= 'A' && c <= 'F') || (c >= 'a' && c <= 'f');
}

uint8_t BMS::hex_nibble_(uint8_t c) {
  if (c >= '0' && c <= '9') return c - '0';
  if (c >= 'A' && c <= 'F') return c - 'A' + 10;
  return c - 'a' + 10;
}

bool BMS::decode_ascii_hex_(const uint8_t *ascii, size_t ascii_len, uint8_t *out, size_t out_len) {
  if (ascii_len < out_len * 2) return false;
  for (size_t i = 0; i < out_len; i++) {
    if (!is_valid_hex_char_(ascii[i * 2]) || !is_valid_hex_char_(ascii[i * 2 + 1]))
      return false;
    out[i] = static_cast<uint8_t>((hex_nibble_(ascii[i * 2]) << 4) | hex_nibble_(ascii[i * 2 + 1]));
  }
  return true;
}

void BMS::handle_current_packet_(const char *label, int32_t current_ma) {
  float current_a = current_ma / 1000.0f;
  ESP_LOGD(tag_, "%s: Current=%.2f A", label, current_a);
  if (current_sensor_ != nullptr) current_sensor_->publish_state(current_a);
  update_charging_state_(current_ma);
  last_current_a_ = current_a;
  smoothed_current_a_ = std::isnan(smoothed_current_a_)
                            ? current_a
                            : CURRENT_EMA_ALPHA * current_a + (1.0f - CURRENT_EMA_ALPHA) * smoothed_current_a_;
  push_link_quality_(true);
  last_heartbeat_ms_.store(millis());
  publish_derived_();
}

void BMS::parse_notification_(const uint8_t *data, uint16_t len) {
  if (len < 4) return;

  // ── Ective: 0x5E packet — int32 LE current, offset 600, unit 1 mA ───────
  if (data[0] == 0x5E && len == 17) {
    uint8_t decoded[8];
    if (!decode_ascii_hex_(data + 1, 16, decoded, 8)) return;
    int32_t current_raw;
    memcpy(&current_raw, decoded + 4, sizeof(current_raw));
    handle_current_packet_("0x5E", current_raw - ECTIVE_CURRENT_OFFSET_MA);
    return;
  }

  // ── Wattstunde: 0xE8 packet — int32 LE current, signed, unit 1 mA ───────
  if (data[0] == 0xE8 && len == 17) {
    uint8_t decoded[8];
    if (!decode_ascii_hex_(data + 1, 16, decoded, 8)) return;
    int32_t current_raw;
    memcpy(&current_raw, decoded + 4, sizeof(current_raw));
    handle_current_packet_("0xE8", current_raw);
    return;
  }

  // ── Temperature packet: 12 ASCII hex, trailing 8 chars = '0' ────────────
  // Encoding: first 4 chars = uint16 LE, Kelvin×10  →  raw/10 - 273.15 = °C
  // Ective ~9°C:  "050B00000000" → LE=0x0B05=2821 → 8.95°C
  // Wattstunde ~10°C: "0F0B00000000" → LE=0x0B0F=2831 → 9.95°C
  if (len == 12) {
    bool trailing_zeros = true;
    for (int i = 4; i < 12; i++) {
      if (data[i] != '0') { trailing_zeros = false; break; }
    }
    if (trailing_zeros) {
      uint8_t decoded[2];
      if (decode_ascii_hex_(data, 4, decoded, 2)) {
        uint16_t raw_le = decoded[0] | (static_cast<uint16_t>(decoded[1]) << 8);
        if (raw_le >= TEMP_KELVIN10_MIN && raw_le <= TEMP_KELVIN10_MAX) {
          float temp = raw_le / 10.0f - 273.15f;
          ESP_LOGD(tag_, "Temp=%.1f°C (raw_le=%u)", temp, raw_le);
          if (temperature_sensor_ != nullptr)
            temperature_sensor_->publish_state(temp);
          push_link_quality_(true);
          last_heartbeat_ms_.store(millis());
        }
      }
      return;
    }
  }

  // ── 16-char ASCII hex packet: cell voltages OR static battery info ────────
  if (len == 16) {
    // Skip all-zero padding (unused cell slots 5–16)
    bool all_zero = true;
    for (int i = 0; i < 16; i++) {
      if (data[i] != '0') { all_zero = false; break; }
    }
    if (all_zero) return;
    uint8_t decoded[8];
    if (!decode_ascii_hex_(data, 16, decoded, 8)) return;

    // Try cell voltages: 4 × uint16 LE in mV, LiFePO4 range 2500–4200 mV
    uint16_t cells[4];
    for (int i = 0; i < 4; i++)
      cells[i] = decoded[i * 2] | (static_cast<uint16_t>(decoded[i * 2 + 1]) << 8);
    bool valid_cells = true;
    for (int i = 0; i < 4; i++) {
      if (cells[i] < CELL_MV_MIN || cells[i] > CELL_MV_MAX) { valid_cells = false; break; }
    }
    if (valid_cells) {
      uint32_t total_mv = static_cast<uint32_t>(cells[0]) + cells[1] + cells[2] + cells[3];
      last_voltage_mv_ = total_mv;
      uint16_t cell_min = cells[0], cell_max = cells[0];
      for (int i = 1; i < 4; i++) {
        if (cells[i] < cell_min) cell_min = cells[i];
        if (cells[i] > cell_max) cell_max = cells[i];
      }
      uint16_t delta_mv = cell_max - cell_min;
      ESP_LOGD(tag_, "Cells: %u %u %u %u mV → %.3f V  delta=%u mV",
               cells[0], cells[1], cells[2], cells[3], total_mv / 1000.0f, delta_mv);
      if (voltage_sensor_ != nullptr)
        voltage_sensor_->publish_state(total_mv / 1000.0f);
      if (avg_cell_voltage_sensor_ != nullptr)
        avg_cell_voltage_sensor_->publish_state(total_mv / 4000.0f);
      for (int i = 0; i < 4; i++) {
        if (cell_voltage_sensor_[i] != nullptr)
          cell_voltage_sensor_[i]->publish_state(cells[i] / 1000.0f);
      }
      if (delta_voltage_sensor_ != nullptr)
        delta_voltage_sensor_->publish_state(delta_mv / 1000.0f);
      if (min_cell_voltage_sensor_ != nullptr)
        min_cell_voltage_sensor_->publish_state(cell_min / 1000.0f);
      if (max_cell_voltage_sensor_ != nullptr)
        max_cell_voltage_sensor_->publish_state(cell_max / 1000.0f);
      if (cell_imbalance_sensor_ != nullptr)
        cell_imbalance_sensor_->publish_state(delta_mv > cell_imbalance_threshold_mv_);
      // Cell voltage packet = heartbeat: one successful cycle recorded
      push_link_quality_(true);
      last_heartbeat_ms_.store(millis());
      publish_derived_();
      return;
    }

    // Not valid cells → static battery info packet: capacity_mah, cycles, soc%
    uint32_t capacity_mah;
    uint16_t cycles, soc;
    memcpy(&capacity_mah, decoded, 4);
    memcpy(&cycles, decoded + 4, 2);
    memcpy(&soc, decoded + 6, 2);
    if (capacity_mah >= CAPACITY_MAH_MIN && capacity_mah <= CAPACITY_MAH_MAX) {
      float capacity_ah = capacity_mah / 1000.0f;
      ESP_LOGD(tag_, "Static: Capacity=%.3f Ah  Cycles=%u  SOC=%u%%",
               capacity_ah, cycles, soc);
      if (capacity_sensor_ != nullptr) capacity_sensor_->publish_state(capacity_ah);
      if (cycles_sensor_ != nullptr) cycles_sensor_->publish_state(static_cast<float>(cycles));
      if (soc <= 100) {
        if (soc_sensor_ != nullptr) soc_sensor_->publish_state(static_cast<float>(soc));
        last_soc_ = static_cast<float>(soc);
      }
      last_capacity_ah_ = capacity_ah;
      publish_derived_();
    }
  }
}

}  // namespace bms
}  // namespace esphome

#endif  // USE_ESP32_FRAMEWORK_ESP_IDF
