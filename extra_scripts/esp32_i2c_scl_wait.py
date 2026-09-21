"""
Give the ESP32 I2C bus a real clock-stretch timeout.

Arduino-ESP32's I2C HAL (cores/esp32/esp32-hal-i2c-ng.c) creates every device
with `dev_config.scl_wait_us = 0`. ESP-IDF documents 0 as "use the default reg
value", but i2c_hal_master_set_scl_timeout_val() never special-cases it. On the
ESP32-S3, i2c_ll_calculate_timeout_us_to_reg_val() evaluates

    32 - __builtin_clz(clk_cycle_num_per_us * timeout_us)

and __builtin_clz(0) is undefined behaviour. On Xtensa it yields 32, so the SCL
timeout register is written with 0: a 2^0 = 1 clock-cycle tolerance for clock
stretching.

Any slave that stretches SCL therefore aborts the transaction, and the failure
surfaces as ESP_ERR_INVALID_STATE out of i2c_master_transmit() (the synchronous
path returns that whenever the final bus status is not I2C_STATUS_DONE). Short
register reads rarely stretch and work; bulk writes do stretch and fail. On the
T-LoRa Pager this kills the BHI260AP firmware upload and makes BQ27220
data-memory writes intermittent.

Patch the hardcoded 0 to a real timeout before the core is compiled. The edit
is idempotent, and the platform already mutates this framework package during
the hybrid IDF compile.
"""

Import("env")  # noqa: F821  (provided by PlatformIO/SCons)

import os

# 20 ms requested -> register value 20 -> ~26 ms of clock-stretch tolerance on a
# 40 MHz source clock. Well under the 5-bit field maximum, generous for a sensor
# hub ingesting firmware, short enough that a genuinely wedged bus still fails.
SCL_WAIT_US = 20000
MARKER = "patched by extra_scripts/esp32_i2c_scl_wait.py"
OLD = "dev_config.scl_wait_us = 0;"
NEW = "dev_config.scl_wait_us = %d;  /* %s */" % (SCL_WAIT_US, MARKER)

fw = env.PioPlatform().get_package_dir("framework-arduinoespressif32")  # noqa: F821
path = os.path.join(fw or "", "cores", "esp32", "esp32-hal-i2c-ng.c")

if not fw or not os.path.isfile(path):
    print("*** esp32_i2c_scl_wait: esp32-hal-i2c-ng.c not found, skipping")
else:
    with open(path, encoding="utf-8") as fh:
        src = fh.read()
    if MARKER in src:
        print("*** esp32_i2c_scl_wait: already patched (scl_wait_us=%d)" % SCL_WAIT_US)
    elif OLD in src:
        with open(path, "w", encoding="utf-8") as fh:
            fh.write(src.replace(OLD, NEW))
        print("*** esp32_i2c_scl_wait: set scl_wait_us=%d in %s" % (SCL_WAIT_US, path))
    else:
        print("*** esp32_i2c_scl_wait: WARNING anchor not found; core layout changed?")
