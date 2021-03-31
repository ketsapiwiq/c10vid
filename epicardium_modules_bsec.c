/* Adapted from  bsec_iot_example.c and bsec_iot_ulp_plus_example.c */

#include "card10.h"
#include "bosch.h"
#include "bsec_integration.h"

#include "ble/ess.h"

#include "epicardium.h"
#include "modules.h"
#include "config.h"
#include "modules/log.h"

#include "FreeRTOS.h"
#include "task.h"

#include "max32665.h"
#include "gcr_regs.h"

#include <string.h>
#include <stdio.h>

TaskHandle_t bsec_task_id;
static int64_t last_bme680_timestamp;
static bool bsec_task_active;
static bool debug;
static struct bsec_sensor_data last_bsec_data;
#define ULP 0

// From generic_18v_3s_4d/bsec_serialized_configurations_iaq.c
static const uint8_t bsec_config_generic_18v_3s_4d[454] = {
	0,   8,   4,   1,   61,  0,   0,   0,   0,   0,   0,   0,   174, 1,
	0,   0,   48,  0,   1,   0,   0,   192, 168, 71,  64,  49,  119, 76,
	0,   0,   225, 68,  137, 65,  0,   191, 205, 204, 204, 190, 0,   0,
	64,  191, 225, 122, 148, 190, 0,   0,   0,   0,   216, 85,  0,   100,
	0,   0,   0,   0,   0,   0,   0,   0,   28,  0,   2,   0,   0,   244,
	1,   225, 0,   25,  0,   0,   128, 64,  0,   0,   32,  65,  144, 1,
	0,   0,   112, 65,  0,   0,   0,   63,  16,  0,   3,   0,   10,  215,
	163, 60,  10,  215, 35,  59,  10,  215, 35,  59,  9,   0,   5,   0,
	0,   0,   0,   0,   1,   88,  0,   9,   0,   7,   240, 150, 61,  0,
	0,   0,   0,   0,   0,   0,   0,   28,  124, 225, 61,  52,  128, 215,
	63,  0,   0,   160, 64,  0,   0,   0,   0,   0,   0,   0,   0,   205,
	204, 12,  62,  103, 213, 39,  62,  230, 63,  76,  192, 0,   0,   0,
	0,   0,   0,   0,   0,   145, 237, 60,  191, 251, 58,  64,  63,  177,
	80,  131, 64,  0,   0,   0,   0,   0,   0,   0,   0,   93,  254, 227,
	62,  54,  60,  133, 191, 0,   0,   64,  64,  12,  0,   10,  0,   0,
	0,   0,   0,   0,   0,   0,   0,   229, 0,   254, 0,   2,   1,   5,
	48,  117, 100, 0,   44,  1,   112, 23,  151, 7,   132, 3,   197, 0,
	92,  4,   144, 1,   64,  1,   64,  1,   144, 1,   48,  117, 48,  117,
	48,  117, 48,  117, 100, 0,   100, 0,   100, 0,   48,  117, 48,  117,
	48,  117, 100, 0,   100, 0,   48,  117, 48,  117, 100, 0,   100, 0,
	100, 0,   100, 0,   48,  117, 48,  117, 48,  117, 100, 0,   100, 0,
	100, 0,   48,  117, 48,  117, 100, 0,   100, 0,   44,  1,   44,  1,
	44,  1,   44,  1,   44,  1,   44,  1,   44,  1,   44,  1,   44,  1,
	44,  1,   44,  1,   44,  1,   44,  1,   44,  1,   8,   7,   8,   7,
	8,   7,   8,   7,   8,   7,   8,   7,   8,   7,   8,   7,   8,   7,
	8,   7,   8,   7,   8,   7,   8,   7,   8,   7,   112, 23,  112, 23,
	112, 23,  112, 23,  112, 23,  112, 23,  112, 23,  112, 23,  112, 23,
	112, 23,  112, 23,  112, 23,  112, 23,  112, 23,  255, 255, 255, 255,
	255, 255, 255, 255, 220, 5,   220, 5,   220, 5,   255, 255, 255, 255,
	255, 255, 220, 5,   220, 5,   255, 255, 255, 255, 255, 255, 255, 255,
	255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255,
	255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 44,  1,   0,   0,
	0,   0,   83,  141, 0,   0
};

/*!
 * @brief           Capture the system time in microseconds
 *
 * @return          system_current_time    current system timestamp in microseconds
 */
static int64_t get_timestamp_us()
{
	int64_t tick = xTaskGetTickCount();
	return tick * 1000;
}

/*!
 * @brief           Handling of the ready outputs
 *
 * @param[in]       timestamp       time in nanoseconds
 * @param[in]       iaq             IAQ signal
 * @param[in]       iaq_accuracy    accuracy of IAQ signal
 * @param[in]       temperature     temperature signal
 * @param[in]       humidity        humidity signal
 * @param[in]       pressure        pressure signal
 * @param[in]       raw_temperature raw temperature signal
 * @param[in]       raw_humidity    raw humidity signal
 * @param[in]       gas             raw gas sensor signal
 * @param[in]       bsec_status     value returned by the bsec_do_steps() call
 *
 * @return          none
 */
static void output_ready(
	int64_t timestamp,
	float iaq,
	uint8_t iaq_accuracy,
	float temperature,
	float humidity,
	float pressure,
	float raw_temperature,
	float raw_humidity,
	float gas,
	bsec_library_return_t bsec_status,
	float static_iaq,
	float co2_equivalent,
	float breath_voc_equivalent
) {
	last_bsec_data.temperature               = temperature;
	last_bsec_data.humidity                  = humidity;
	last_bsec_data.pressure                  = pressure / 100.;
	last_bsec_data.gas_resistance            = gas;
	last_bsec_data.timestamp                 = timestamp;
	last_bsec_data.accuracy                  = iaq_accuracy;
	last_bsec_data.indoor_air_quality        = iaq;
	last_bsec_data.static_indoor_air_quality = static_iaq;
	last_bsec_data.co2_equivalent            = co2_equivalent;
	last_bsec_data.breath_voc_equivalent     = breath_voc_equivalent;

	__sync_synchronize();

	last_bme680_timestamp = timestamp;

	bleESS_update_from_bsec_data(&last_bsec_data);

	if (debug) {
		LOG_INFO(
			"bsec",
			"time[ms]: %u, IAQ: %u, IAQ ACC[0-3]: %u, T[.1C]: %u, Hum[.1%%]: %u, P[Pa]: %u, Raw T[.1C]: %u, Raw Hum[.1%%]: %u, Gas[Ohm]: %u, Static IAQ: %u, CO2[ppm]: %u, Breath VOC[ppb]: %u",
			(unsigned int)(timestamp / 1e6),
			(unsigned int)(iaq),
			(unsigned int)(iaq_accuracy),
			(unsigned int)(temperature * 10),
			(unsigned int)(humidity * 10),
			(unsigned int)(pressure),
			(unsigned int)(raw_temperature * 10),
			(unsigned int)(raw_humidity * 10),
			(unsigned int)(gas),
			(unsigned int)(static_iaq),
			(unsigned int)(co2_equivalent),
			(unsigned int)(breath_voc_equivalent * 1e3)
		);
	}
}

int epic_bsec_read_sensors(struct bsec_sensor_data *data)
{
	if (data == NULL) {
		return -EFAULT;
	}
	if (!bsec_task_active) {
		return -ENODEV;
	}

	/* TODO: could also return -EINVAL */
	while (last_bme680_timestamp == 0)
		vTaskDelay(pdMS_TO_TICKS(10));

	*data = last_bsec_data;
	return 0;
}

static uint32_t bsec_load(char *path, uint8_t *buffer, uint32_t n_buffer)
{
	uint32_t len = 0;
	int fd, res;

	LOG_DEBUG("bsec", "load %s %lu", path, n_buffer);

	if ((fd = epic_file_open(path, "r")) < 0) {
		LOG_DEBUG("bsec", "Open failed");
		return 0;
	}

	uint32_t header;
	if ((res = epic_file_read(fd, &header, sizeof(header))) !=
	    sizeof(header)) {
		LOG_WARN("bsec", "Header failed");
		goto done;
	}

	if (header > n_buffer) {
		LOG_WARN("bsec", "Too large");
		goto done;
	}

	if (epic_file_read(fd, buffer, header) != (int)header) {
		LOG_WARN("bsec", "Read failed");
		goto done;
	}

	len = header;

	LOG_DEBUG("bsec", "Success");
done:
	epic_file_close(fd);
	return len;
}
/*!
 * @brief           Load previous library state from non-volatile memory
 *
 * @param[in,out]   state_buffer    buffer to hold the loaded state string
 * @param[in]       n_buffer        size of the allocated state buffer
 *
 * @return          number of bytes copied to state_buffer
 */
static uint32_t state_load(uint8_t *state_buffer, uint32_t n_buffer)
{
	return bsec_load("bsec_iaq.state", state_buffer, n_buffer);
}

/*!
 * @brief           Save library state to non-volatile memory
 *
 * @param[in]       state_buffer    buffer holding the state to be stored
 * @param[in]       length          length of the state string to be stored
 *
 * @return          none
 */
static void state_save(const uint8_t *state_buffer, uint32_t length)
{
	int fd, res;

	LOG_DEBUG("bsec", "state_save %d", (int)length);

	if ((fd = epic_file_open("bsec_iaq.state", "w")) < 0) {
		LOG_WARN("bsec", "Open failed");
		return;
	}

	uint32_t header = length;
	if ((res = epic_file_write(fd, &header, sizeof(header))) !=
	    sizeof(header)) {
		LOG_WARN("bsec", "Header failed");
		goto done;
	}

	if (epic_file_write(fd, state_buffer, header) != (int)header) {
		LOG_WARN("bsec", "Write failed");
		goto done;
	}

	LOG_DEBUG("bsec", "stack high: %lu", uxTaskGetStackHighWaterMark(NULL));
done:
	epic_file_close(fd);
}
/*!
 * @brief           Delete the library state from non-volatile memory
 *
 * @return          none
 */
static void state_delete(void)
{
	LOG_DEBUG("bsec", "state_delete");

	epic_file_unlink("bsec_iaq.state");
}

static int8_t
i2c_write(uint8_t addr, uint8_t reg, uint8_t *p_buf, uint16_t size)
{
	int8_t ret;
	hwlock_acquire(HWLOCK_I2C);
	ret = card10_bosch_i2c_write(addr, reg, p_buf, size);
	hwlock_release(HWLOCK_I2C);
	return ret;
}

static int8_t i2c_read(uint8_t addr, uint8_t reg, uint8_t *p_buf, uint16_t size)
{
	int8_t ret;
	hwlock_acquire(HWLOCK_I2C);
	ret = card10_bosch_i2c_read(addr, reg, p_buf, size);
	hwlock_release(HWLOCK_I2C);
	return ret;
}

static void delay(uint32_t msec)
{
	if (xTaskGetSchedulerState() == taskSCHEDULER_NOT_STARTED) {
		/* We need to fall back to hardware waits if not running
		 * in a task context */
		card10_bosch_delay(msec);
	} else {
		vTaskDelay(pdMS_TO_TICKS(msec));
	}
}

/*!
 * @brief           Load library config from non-volatile memory
 *
 * @param[in,out]   config_buffer    buffer to hold the loaded state string
 * @param[in]       n_buffer        size of the allocated state buffer
 *
 * @return          number of bytes copied to config_buffer
 */
static uint32_t config_load(uint8_t *config_buffer, uint32_t n_buffer)
{
	uint32_t len = bsec_load("bsec_iaq.config", config_buffer, n_buffer);

	if (len == 0) {
		LOG_INFO("bsec", "Using default bsec_config_generic_18v_3s_4d");
		len = sizeof(bsec_config_generic_18v_3s_4d);
		memcpy(config_buffer, bsec_config_generic_18v_3s_4d, len);
	}

	return len;
}

#if ULP
void ulp_plus_trigger_iaq()
{
	/* We call bsec_update_subscription() in order to instruct BSEC to perform an extra measurement at the next
     * possible time slot
     */

	bsec_sensor_configuration_t requested_virtual_sensors[1];
	uint8_t n_requested_virtual_sensors = 1;
	bsec_sensor_configuration_t
		required_sensor_settings[BSEC_MAX_PHYSICAL_SENSOR];
	uint8_t n_required_sensor_settings = BSEC_MAX_PHYSICAL_SENSOR;
	bsec_library_return_t status       = BSEC_OK;

	/* To trigger a ULP plus, we request the IAQ virtual sensor with a specific sample rate code */
	requested_virtual_sensors[0].sensor_id = BSEC_OUTPUT_IAQ;
	requested_virtual_sensors[0].sample_rate =
		BSEC_SAMPLE_RATE_ULP_MEASUREMENT_ON_DEMAND;

	/* Call bsec_update_subscription() to enable/disable the requested virtual sensors */
	status = bsec_update_subscription(
		requested_virtual_sensors,
		n_requested_virtual_sensors,
		required_sensor_settings,
		&n_required_sensor_settings
	);

	/* The status code would tell is if the request was accepted. It will be rejected if the sensor is not already in
     * ULP mode, or if the time difference between requests is too short, for example. */
}
#endif

bool bsec_active(void)
{
	return bsec_task_active;
}

int bsec_read_bme680(struct bme680_sensor_data *data)
{
	if (!bsec_task_active) {
		return BME680_E_COM_FAIL;
	}

	if (data == NULL) {
		return -EFAULT;
	}

	while (last_bme680_timestamp == 0)
		vTaskDelay(pdMS_TO_TICKS(10));

	data->temperature    = last_bsec_data.temperature;
	data->humidity       = last_bsec_data.humidity;
	data->pressure       = last_bsec_data.pressure;
	data->gas_resistance = last_bsec_data.gas_resistance;

	return BME680_OK;
}

/**
 * Checks config and activates the BSEC libary if requested.
 *
 * Initializes the BSEC library before starting the task to
 * reduce the stack size needed for the task by at least 250 bytes
 */
int bsec_activate(void)
{
	return_values_init ret;
#if ULP
	float sample_rate = BSEC_SAMPLE_RATE_ULP;
#else
	float sample_rate = BSEC_SAMPLE_RATE_LP;
#endif

	if (!config_get_boolean_with_default("bsec_enable", false)) {
		return -1;
	}

	debug = config_get_boolean_with_default("bsec_debug", false);

	float temperature_offset =
		config_get_integer_with_default("bsec_offset", -22) / 10.;
	if (temperature_offset != 0.0) {
		LOG_INFO(
			"besec",
			"BSEC Temp offset %d/10 K",
			(int)(temperature_offset * 10)
		);
	}

	/* Puts AT LEAST 2 * #BSEC_MAX_PROPERTY_BLOB_SIZE = 2 * 454 = 908 bytes onto the stack */
	ret = bsec_iot_init(
		sample_rate,
		-temperature_offset,
		i2c_write,
		i2c_read,
		delay,
		state_load,
		config_load
	);

	if (ret.bsec_status == BSEC_E_CONFIG_VERSIONMISMATCH) {
		/* BSEC version changed and old state is not compatible anymore */
		/* If the config is also not valid anymore, the user will have
		 * to fix that. */
		state_delete();
		ret = bsec_iot_init(
			sample_rate,
			-temperature_offset,
			i2c_write,
			i2c_read,
			delay,
			state_load,
			config_load
		);
	}

	if (ret.bme680_status) {
		LOG_WARN("bsec", "bme680 init failed: %d", ret.bme680_status);
		/* Could not initialize BME680 */
		return -1;
	} else if (ret.bsec_status) {
		LOG_WARN("bsec", "bsec init failed: %d", ret.bsec_status);
		/* Could not initialize BSEC library */
		return -1;
	}
	return 0;
}

void vBSECTask(void *pvParameters)
{
	bsec_task_active = true;
	bsec_task_id     = xTaskGetCurrentTaskHandle();

#if ULP
	/* State is saved every 100 samples, which means every 100 * 300 secs = 500 minutes  */
	const int stat_save_interval = 100;
#else
	/* State is saved every 10.000 samples, which means every 10.000 * 3 secs = 500 minutes  */
	const int stat_save_interval = 10000;
#endif

	/* Call to endless loop function which reads and processes data based on sensor settings */
	/* Puts AT LEAST 2 * BSEC_MAX_STATE_BLOB_SIZE + 8 * sizeof(bsec_input_t) =
	 * 2 * 139 + 8 * 20 = 438 bytes onto the stack */
	bsec_iot_loop(
		delay,
		get_timestamp_us,
		output_ready,
		state_save,
		stat_save_interval
	);
}
