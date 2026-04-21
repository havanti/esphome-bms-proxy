import esphome.codegen as cg
import esphome.config_validation as cv
from esphome.components import ble_client, sensor, binary_sensor
from esphome.core import CORE
from esphome.const import (
    CONF_ID,
    DEVICE_CLASS_BATTERY,
    DEVICE_CLASS_BATTERY_CHARGING,
    DEVICE_CLASS_CONNECTIVITY,
    DEVICE_CLASS_CURRENT,
    DEVICE_CLASS_ENERGY,
    DEVICE_CLASS_POWER,
    DEVICE_CLASS_PROBLEM,
    DEVICE_CLASS_TEMPERATURE,
    DEVICE_CLASS_VOLTAGE,
    STATE_CLASS_MEASUREMENT,
    UNIT_AMPERE,
    UNIT_CELSIUS,
    UNIT_PERCENT,
    UNIT_VOLT,
    UNIT_WATT,
    UNIT_WATT_HOURS,
)

DEPENDENCIES = ["ble_client"]
AUTO_LOAD = ["sensor", "binary_sensor"]

bms_ns = cg.esphome_ns.namespace("bms")
BMS = bms_ns.class_("BMS", cg.Component, ble_client.BLEClientNode)

CONF_SOC = "soc"
CONF_VOLTAGE = "voltage"
CONF_CURRENT = "current"
CONF_POWER = "power"
CONF_CELL_VOLTAGE_1 = "cell_voltage_1"
CONF_CELL_VOLTAGE_2 = "cell_voltage_2"
CONF_CELL_VOLTAGE_3 = "cell_voltage_3"
CONF_CELL_VOLTAGE_4 = "cell_voltage_4"
CONF_CAPACITY = "capacity"
CONF_CYCLES = "cycles"
CONF_TEMPERATURE = "temperature"
CONF_DELTA_VOLTAGE = "delta_voltage"
CONF_AVG_CELL_VOLTAGE = "avg_cell_voltage"
CONF_MIN_CELL_VOLTAGE = "min_cell_voltage"
CONF_MAX_CELL_VOLTAGE = "max_cell_voltage"
CONF_CONNECTED = "connected"
CONF_BATTERY_CHARGING = "battery_charging"
CONF_CELL_IMBALANCE = "cell_imbalance"
CONF_CELL_IMBALANCE_THRESHOLD = "cell_imbalance_threshold"
CONF_RUNTIME = "runtime"
CONF_REMAINING_WH = "remaining_wh"
CONF_REMAINING_AH = "remaining_ah"
CONF_CHARGE_POWER = "charge_power"
CONF_DISCHARGE_POWER = "discharge_power"
CONF_SOH = "soh"
CONF_NOMINAL_CAPACITY = "nominal_capacity"
CONF_LINK_QUALITY = "link_quality"

_CELL_VOLTAGE_SCHEMA = sensor.sensor_schema(
    unit_of_measurement=UNIT_VOLT,
    accuracy_decimals=2,
    device_class=DEVICE_CLASS_VOLTAGE,
    state_class=STATE_CLASS_MEASUREMENT,
)

_INSTANCE_SCHEMA = (
    cv.Schema(
        {
            cv.GenerateID(): cv.declare_id(BMS),
            cv.Optional(CONF_SOC): sensor.sensor_schema(
                unit_of_measurement=UNIT_PERCENT,
                accuracy_decimals=0,
                device_class=DEVICE_CLASS_BATTERY,
                state_class=STATE_CLASS_MEASUREMENT,
            ),
            cv.Optional(CONF_VOLTAGE): sensor.sensor_schema(
                unit_of_measurement=UNIT_VOLT,
                accuracy_decimals=1,
                device_class=DEVICE_CLASS_VOLTAGE,
                state_class=STATE_CLASS_MEASUREMENT,
            ),
            cv.Optional(CONF_CURRENT): sensor.sensor_schema(
                unit_of_measurement=UNIT_AMPERE,
                accuracy_decimals=1,
                device_class=DEVICE_CLASS_CURRENT,
                state_class=STATE_CLASS_MEASUREMENT,
            ),
            cv.Optional(CONF_POWER): sensor.sensor_schema(
                unit_of_measurement=UNIT_WATT,
                accuracy_decimals=1,
                device_class=DEVICE_CLASS_POWER,
                state_class=STATE_CLASS_MEASUREMENT,
            ),
            cv.Optional(CONF_CELL_VOLTAGE_1): _CELL_VOLTAGE_SCHEMA,
            cv.Optional(CONF_CELL_VOLTAGE_2): _CELL_VOLTAGE_SCHEMA,
            cv.Optional(CONF_CELL_VOLTAGE_3): _CELL_VOLTAGE_SCHEMA,
            cv.Optional(CONF_CELL_VOLTAGE_4): _CELL_VOLTAGE_SCHEMA,
            cv.Optional(CONF_CAPACITY): sensor.sensor_schema(
                unit_of_measurement="Ah",
                accuracy_decimals=1,
                state_class=STATE_CLASS_MEASUREMENT,
            ),
            cv.Optional(CONF_CYCLES): sensor.sensor_schema(
                accuracy_decimals=0,
                state_class=STATE_CLASS_MEASUREMENT,
            ),
            cv.Optional(CONF_TEMPERATURE): sensor.sensor_schema(
                unit_of_measurement=UNIT_CELSIUS,
                accuracy_decimals=1,
                device_class=DEVICE_CLASS_TEMPERATURE,
                state_class=STATE_CLASS_MEASUREMENT,
            ),
            cv.Optional(CONF_DELTA_VOLTAGE): sensor.sensor_schema(
                unit_of_measurement=UNIT_VOLT,
                accuracy_decimals=3,
                device_class=DEVICE_CLASS_VOLTAGE,
                state_class=STATE_CLASS_MEASUREMENT,
            ),
            cv.Optional(CONF_AVG_CELL_VOLTAGE): sensor.sensor_schema(
                unit_of_measurement=UNIT_VOLT,
                accuracy_decimals=3,
                device_class=DEVICE_CLASS_VOLTAGE,
                state_class=STATE_CLASS_MEASUREMENT,
            ),
            cv.Optional(CONF_MIN_CELL_VOLTAGE): sensor.sensor_schema(
                unit_of_measurement=UNIT_VOLT,
                accuracy_decimals=3,
                device_class=DEVICE_CLASS_VOLTAGE,
                state_class=STATE_CLASS_MEASUREMENT,
            ),
            cv.Optional(CONF_MAX_CELL_VOLTAGE): sensor.sensor_schema(
                unit_of_measurement=UNIT_VOLT,
                accuracy_decimals=3,
                device_class=DEVICE_CLASS_VOLTAGE,
                state_class=STATE_CLASS_MEASUREMENT,
            ),
            cv.Optional(CONF_RUNTIME): sensor.sensor_schema(
                unit_of_measurement="min",
                accuracy_decimals=0,
                state_class=STATE_CLASS_MEASUREMENT,
            ),
            cv.Optional(CONF_REMAINING_WH): sensor.sensor_schema(
                unit_of_measurement=UNIT_WATT_HOURS,
                accuracy_decimals=0,
                device_class=DEVICE_CLASS_ENERGY,
                state_class=STATE_CLASS_MEASUREMENT,
            ),
            cv.Optional(CONF_REMAINING_AH): sensor.sensor_schema(
                unit_of_measurement="Ah",
                accuracy_decimals=1,
                state_class=STATE_CLASS_MEASUREMENT,
            ),
            cv.Optional(CONF_CHARGE_POWER): sensor.sensor_schema(
                unit_of_measurement=UNIT_WATT,
                accuracy_decimals=1,
                device_class=DEVICE_CLASS_POWER,
                state_class=STATE_CLASS_MEASUREMENT,
            ),
            cv.Optional(CONF_DISCHARGE_POWER): sensor.sensor_schema(
                unit_of_measurement=UNIT_WATT,
                accuracy_decimals=1,
                device_class=DEVICE_CLASS_POWER,
                state_class=STATE_CLASS_MEASUREMENT,
            ),
            cv.Optional(CONF_SOH): sensor.sensor_schema(
                unit_of_measurement=UNIT_PERCENT,
                accuracy_decimals=1,
                state_class=STATE_CLASS_MEASUREMENT,
            ),
            cv.Optional(CONF_NOMINAL_CAPACITY, default=0.0): cv.float_range(min=0.0),
            cv.Optional(CONF_LINK_QUALITY): sensor.sensor_schema(
                unit_of_measurement=UNIT_PERCENT,
                accuracy_decimals=0,
                state_class=STATE_CLASS_MEASUREMENT,
            ),
            cv.Optional(CONF_CONNECTED): binary_sensor.binary_sensor_schema(
                device_class=DEVICE_CLASS_CONNECTIVITY,
            ),
            cv.Optional(CONF_BATTERY_CHARGING): binary_sensor.binary_sensor_schema(
                device_class=DEVICE_CLASS_BATTERY_CHARGING,
            ),
            cv.Optional(CONF_CELL_IMBALANCE): binary_sensor.binary_sensor_schema(
                device_class=DEVICE_CLASS_PROBLEM,
            ),
            cv.Optional(CONF_CELL_IMBALANCE_THRESHOLD, default=50): cv.int_range(min=1, max=1000),
        }
    )
    .extend(ble_client.BLE_CLIENT_SCHEMA)
    .extend(cv.COMPONENT_SCHEMA)
)

CONFIG_SCHEMA = cv.ensure_list(_INSTANCE_SCHEMA)


async def to_code(config):
    if not (CORE.is_esp32 and not CORE.using_arduino):
        raise cv.Invalid("bms requires ESP32 with the ESP-IDF framework")

    for conf in config:
        var = cg.new_Pvariable(conf[CONF_ID])
        await cg.register_component(var, conf)
        await ble_client.register_ble_node(var, conf)

        sensor_map = [
            (CONF_SOC, "set_soc_sensor"),
            (CONF_VOLTAGE, "set_voltage_sensor"),
            (CONF_CURRENT, "set_current_sensor"),
            (CONF_POWER, "set_power_sensor"),
            (CONF_CELL_VOLTAGE_1, "set_cell_voltage_sensor_1"),
            (CONF_CELL_VOLTAGE_2, "set_cell_voltage_sensor_2"),
            (CONF_CELL_VOLTAGE_3, "set_cell_voltage_sensor_3"),
            (CONF_CELL_VOLTAGE_4, "set_cell_voltage_sensor_4"),
            (CONF_CAPACITY, "set_capacity_sensor"),
            (CONF_CYCLES, "set_cycles_sensor"),
            (CONF_TEMPERATURE, "set_temperature_sensor"),
            (CONF_DELTA_VOLTAGE, "set_delta_voltage_sensor"),
            (CONF_AVG_CELL_VOLTAGE, "set_avg_cell_voltage_sensor"),
            (CONF_MIN_CELL_VOLTAGE, "set_min_cell_voltage_sensor"),
            (CONF_MAX_CELL_VOLTAGE, "set_max_cell_voltage_sensor"),
            (CONF_RUNTIME, "set_runtime_sensor"),
            (CONF_REMAINING_WH, "set_remaining_wh_sensor"),
            (CONF_REMAINING_AH, "set_remaining_ah_sensor"),
            (CONF_CHARGE_POWER, "set_charge_power_sensor"),
            (CONF_DISCHARGE_POWER, "set_discharge_power_sensor"),
            (CONF_SOH, "set_soh_sensor"),
            (CONF_LINK_QUALITY, "set_link_quality_sensor"),
        ]
        for conf_key, setter in sensor_map:
            if conf_key in conf:
                sens = await sensor.new_sensor(conf[conf_key])
                cg.add(getattr(var, setter)(sens))

        if CONF_CONNECTED in conf:
            sens = await binary_sensor.new_binary_sensor(conf[CONF_CONNECTED])
            cg.add(var.set_connected_sensor(sens))

        if CONF_BATTERY_CHARGING in conf:
            sens = await binary_sensor.new_binary_sensor(conf[CONF_BATTERY_CHARGING])
            cg.add(var.set_battery_charging_sensor(sens))

        cg.add(var.set_cell_imbalance_threshold(conf[CONF_CELL_IMBALANCE_THRESHOLD]))
        cg.add(var.set_nominal_capacity_ah(conf[CONF_NOMINAL_CAPACITY]))
        if CONF_CELL_IMBALANCE in conf:
            sens = await binary_sensor.new_binary_sensor(conf[CONF_CELL_IMBALANCE])
            cg.add(var.set_cell_imbalance_sensor(sens))
