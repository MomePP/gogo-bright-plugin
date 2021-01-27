Blockly.Blocks["gogo_bright.get_sht31_temp"] = {
	init: function() {
		this.appendDummyInput()
			.appendField(Blockly.Msg.SENSOR_SHT31_TEMP);
		this.setOutput(true, 'Number');
		this.setInputsInline(true);
		this.setPreviousStatement(false);
		this.setNextStatement(false);
		this.setColour(Blockly.Msg.COLOR_SENSOR_SHT31);
	}
};

Blockly.Blocks["gogo_bright.get_sht31_humi"] = {
	init: function() {
		this.appendDummyInput()
			.appendField(Blockly.Msg.SENSOR_SHT31_HUMI);
		this.setOutput(true, 'Number');
		this.setInputsInline(true);
		this.setPreviousStatement(false);
		this.setNextStatement(false);
		this.setColour(Blockly.Msg.COLOR_SENSOR_SHT31);
	}
};
