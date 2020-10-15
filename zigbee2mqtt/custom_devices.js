/* eslint-disable */
const zigbeeHerdsmanConverters = require('zigbee-herdsman-converters');

const fz = {
    ZigWM_parse: {
        cluster: 'genBasic',
        type: ['attributeReport', 'readResponse'],
        convert: (model, msg, publish, options, meta) => {
            return {
                state: msg.data['41363'] === 1 ? 'ON' : 'OFF',
                counter: msg.data['41364'],
		threshold: msg.data['41365'],
            };
        },
    },
};

const tz = {
    ZigWM_counter: {
        key: ['counter'],
        convertSet: async (entity, key, value, meta) => {
                const opts = {
                        timeout: 30000,
                };

                await entity.write('genBasic', {41364: {value: value, type: 0x23}}, opts);
        },
    },
    ZigWM_threshold: {
        key: ['threshold'],
        convertSet: async (entity, key, value, meta) => {
                const opts = {
                        timeout: 30000,
                };

                await entity.write('genBasic', {41365: {value: value, type: 0x21}}, opts);

        },
    },
};

const hassSwitch = {
	type: 'switch',
	object_id: 'switch',
	discovery_payload: {
	        payload_off: 'OFF',
	        payload_on: 'ON',
	        value_template: '{{ value_json.state }}',
	        command_topic: true,
	},
};

const device = {
	zigbeeModel: ['ZigWM'],
        model: 'ZigWM',
        vendor: 'Open smart home',
	description: '[Simple DIY water meter](https://github.com/pedroke/ZigWaterMeter)',
        supports: 'state, counter, threshold',
        fromZigbee: [fz.ZigWM_parse],
        toZigbee: [tz.ZigWM_counter, tz.ZigWM_threshold],
  	meta: {
        	configureKey: 1,
    	},
    	configure: async (device, coordinatorEndpoint) => {
		// dummy
	},
	homeassistant: [hassSwitch],
};

module.exports = device; 
