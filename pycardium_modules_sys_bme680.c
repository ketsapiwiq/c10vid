#include "py/obj.h"
#include "py/objlist.h"
#include "py/runtime.h"

#include "epicardium.h"

static mp_obj_t mp_bme680_init()
{
	int ret = epic_bme680_init();

	if (ret < 0) {
		mp_raise_OSError(-ret);
	}

	return mp_const_none;
}
static MP_DEFINE_CONST_FUN_OBJ_0(bme680_init_obj, mp_bme680_init);

static mp_obj_t mp_bme680_deinit()
{
	int ret = epic_bme680_deinit();

	if (ret < 0) {
		mp_raise_OSError(-ret);
	}

	return mp_const_none;
}
static MP_DEFINE_CONST_FUN_OBJ_0(bme680_deinit_obj, mp_bme680_deinit);

static mp_obj_t mp_bme680_get_data()
{
	struct bme680_sensor_data data;
	int ret = epic_bme680_read_sensors(&data);

	if (ret < 0) {
		mp_raise_OSError(-ret);
	}

	mp_obj_t values_list[] = { mp_obj_new_float(data.temperature),
				   mp_obj_new_float(data.humidity),
				   mp_obj_new_float(data.pressure),
				   mp_obj_new_float(data.gas_resistance) };
	return mp_obj_new_tuple(4, values_list);
}
static MP_DEFINE_CONST_FUN_OBJ_0(bme680_get_data_obj, mp_bme680_get_data);

static mp_obj_t mp_bsec_get_data()
{
	struct bsec_sensor_data data;
	int ret = epic_bsec_read_sensors(&data);

	if (ret < 0) {
		mp_raise_OSError(-ret);
	}

	mp_obj_t values_list[] = {
		mp_obj_new_float(data.temperature),
		mp_obj_new_float(data.humidity),
		mp_obj_new_float(data.pressure),
		mp_obj_new_float(data.gas_resistance),
		mp_obj_new_int(data.indoor_air_quality),
		mp_obj_new_int(data.accuracy),
		mp_obj_new_float(data.co2_equivalent),
		mp_obj_new_float(data.breath_voc_equivalent),
	};
	return mp_obj_new_tuple(8, values_list);
}
static MP_DEFINE_CONST_FUN_OBJ_0(bsec_get_data_obj, mp_bsec_get_data);

static const mp_rom_map_elem_t bme680_module_globals_table[] = {
	{ MP_ROM_QSTR(MP_QSTR___name__), MP_ROM_QSTR(MP_QSTR_sys_bme680) },
	{ MP_ROM_QSTR(MP_QSTR_init), MP_ROM_PTR(&bme680_init_obj) },
	{ MP_ROM_QSTR(MP_QSTR_deinit), MP_ROM_PTR(&bme680_deinit_obj) },
	{ MP_ROM_QSTR(MP_QSTR_get_data), MP_ROM_PTR(&bme680_get_data_obj) },
	{ MP_ROM_QSTR(MP_QSTR_bsec_get_data), MP_ROM_PTR(&bsec_get_data_obj) },
};
static MP_DEFINE_CONST_DICT(bme680_module_globals, bme680_module_globals_table);

const mp_obj_module_t bme680_module = {
	.base    = { &mp_type_module },
	.globals = (mp_obj_dict_t *)&bme680_module_globals,
};

/* Register the module to make it available in Python */
MP_REGISTER_MODULE(MP_QSTR_sys_bme680, bme680_module, MODULE_BME680_ENABLED);
