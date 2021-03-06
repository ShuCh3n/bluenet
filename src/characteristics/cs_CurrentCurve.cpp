/**
 * Author: Bart van Vliet
 * Copyright: Distributed Organisms B.V. (DoBots)
 * Date: Mar. 02, 2015
 * License: LGPLv3+, Apache License, or MIT, your choice
 */

#include "characteristics/cs_CurrentCurve.h"


CurrentCurve::CurrentCurve() : _plength(0), _payload(NULL) {
}

CurrentCurve::~CurrentCurve() {
	free(_payload);
}

void CurrentCurve::init() {
	_payload = (uint16_t*)calloc(ADC_BUFFER_SIZE, sizeof(uint16_t));
}

bool CurrentCurve::operator!=(const CurrentCurve &other) {
	if (_plength != other._plength) return true;
	if (memcmp(_payload, other._payload, CURRENT_CURVE_SERIALIZED_SIZE) != 0) return true;
	return false;
}

uint16_t CurrentCurve::getSerializedLength() const {
	return CURRENT_CURVE_SERIALIZED_SIZE + CURRENT_CURVE_HEADER_SIZE;
}

void CurrentCurve::setPayload(uint16_t *payload, uint16_t plength) {
	_plength = ((plength > ADC_BUFFER_SIZE) ? ADC_BUFFER_SIZE : plength);
	memcpy(_payload, payload, _plength*sizeof(uint16_t));
}

uint8_t CurrentCurve::add(uint16_t value) {
	if (_plength >= ADC_BUFFER_SIZE) return 1;
	_payload[_plength++] = value;
	return 0;
}

void CurrentCurve::clear() {
	memset(_payload, 0, ADC_BUFFER_SIZE * sizeof(uint16_t));
	_plength = 0;
}

void CurrentCurve::serialize(uint8_t* buffer, uint16_t length) const {
	if (length < 3) return; // throw error
	
	uint8_t *ptr = buffer;
	*ptr++ = (_plength >> 8) & 0xFF;
	*ptr++ = _plength & 0xFF;
	// Can't memcpy, since then it's LSB
//	if (_plength) memcpy(ptr, _payload, _plength*sizeof(uint16_t));
	for (int i=0; i<_plength; ++i) {
		*ptr++ = (_payload[i] >> 8) & 0xFF;
		*ptr++ = _payload[i] & 0xFF;
	}
}

void CurrentCurve::deserialize(uint8_t* buffer, uint16_t length) {
	if (length < 3) return;

	uint8_t *ptr = buffer;
	_plength = (*ptr++ << 8) & 0xFF00;
	_plength |= *ptr++;
	memset(_payload, 0, CURRENT_CURVE_SERIALIZED_SIZE);
	_plength = ((_plength > ADC_BUFFER_SIZE) ? ADC_BUFFER_SIZE : _plength);
	// Can't memcpy, since then it's LSB
//	memcpy(_payload, ptr, _plength*sizeof(uint16_t));
	for (int i=0; i<_plength; ++i) {
		_payload[i] = (*ptr++ << 8) & 0xFF00;
		_payload[i] |= *ptr++;
	}
}

