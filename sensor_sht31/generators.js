// ********************* SHT31 ********************* //

Blockly.JavaScript['gogo_bright.get_sht31_temp'] = function(block) {
	return [
		'DEV_I2C1.SENSOR_SHT31().readTemp()',
		Blockly.JavaScript.ORDER_ATOMIC
	];
};

Blockly.JavaScript['gogo_bright.get_sht31_humi'] = function(block) {
	return [
		'DEV_I2C1.SENSOR_SHT31().readHumi()',
		Blockly.JavaScript.ORDER_ATOMIC
	];
};
