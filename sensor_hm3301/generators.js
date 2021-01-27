// ********************* DHT12 ********************* //

Blockly.JavaScript['gogo_bright.get_dust_level'] = function(block) {
    var code = `DEV_I2C1.SENSOR_HM3301().readDust(${block.getFieldValue('pmSize')})`
	return [
		code,
		Blockly.JavaScript.ORDER_ATOMIC
	];
};
