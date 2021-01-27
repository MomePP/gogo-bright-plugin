// ********************* DHTxx ********************* //

Blockly.Blocks["gogo_bright.get_dust_level"] = {
	init: function() {
		this.appendDummyInput()
            .appendField(Blockly.Msg.SENSOR_HM3301_PREFIX)
            .appendField(new Blockly.FieldDropdown([["PM 1.0","0"], ["PM 2.5","1"], ["PM 10","2"]]), "pmSize");
		this.setOutput(true, 'Number');
		this.setInputsInline(true);
		this.setPreviousStatement(false);
		this.setNextStatement(false);
		this.setColour(Blockly.Msg.COLOR_SENSOR_HM3301)
	}
};
