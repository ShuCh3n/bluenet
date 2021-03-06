/**
 * Author: Dominik Egger
 * Copyright: Distributed Organisms B.V. (DoBots)
 * Date: Nov 4, 2014
 * License: LGPLv3+, Apache License, or MIT, your choice
 */
#pragma once

#include "ble_gap.h"
#include "ble_gatts.h"

#include "cs_BluetoothLE.h"
#include "characteristics/cs_Serializable.h"

using namespace BLEpp;

/* Structure used to store peripheral devices detected during a scan.
 *
 * We store the bluetooth address of the device, the number of times that the
 * device was seen during scanning (occurrences) and the rssi (received signal
 * strength indication) value of the last received advertisement.
 * **note** the bluetooth address is stored in little-endian (LSB-first) so when
 * displaying it to the user we need to start at the back of the array
 * **note** struct has to be packed in order to avoid word alignment.
 */
struct __attribute__((__packed__)) peripheral_device_t {
	uint8_t addr[BLE_GAP_ADDR_LEN];
	uint16_t occurrences;
	int8_t rssi;
};

/* The size of the header used in the scan list message
 *
 * Only one byte is used as header which defines the number of elements
 * in the list
 */
#define HEADER_SIZE 1

/* The size in bytes needed to store the device structure after serializing
 *
 * **Note** this only works if struct ist packed
 */
#define SERIALIZED_DEVICE_SIZE sizeof(peripheral_device_t)

/* The maximum number of devices stored during a scan and returned as a list
 *
 * If the maximum number is exceeded, the devices with the lowest occurrence
 * will be replaced
 */
#define MAX_NR_DEVICES 10

/* Result of a scan device operation
 *
 * The ScanResult class stores a list of devices which were detected during
 * a scan operation. It keeps track of how often a device was seen and with
 * which RSSI value.
 */
class ScanResult : public Serializable {

private:
	/* A pointer to the list of detected devices
	 */
	peripheral_device_t* _list;

	/* The index in the list of the next free slot
	 */
	uint8_t _freeIdx;

public:
	/* Default constructor
	 */
	ScanResult();

	/* Default destructor
	 *
	 * Free list on destruction
	 */
	~ScanResult() {
		if (_list) {
			free(_list);
		}
	}

	/* Allocate and initialize an empty list
	 *
	 * If there was already a list created earlier, the
	 * old list is freed and a new list is allocated
	 */
	void init();

	/* Basic not equal operator
	 *
	 * @val ScanResult instance which should be compared
	 *   with this instance
	 *
	 * Checks and compares the fields of the two instances
	 * to determine if they are equal or not
	 *
	 * @return true if not equal,
	 *         false otherwise
	 */
	bool operator!=(const ScanResult& val);

	/* Print the list of devices for debug purposes to the UART
	 */
	void print() const;

	/* Update the list with the given device
	 *
	 * @adrs_ptr pointer to an array of length <BLE_GAP_ADDR_LEN>
	 *   which contains the bluetooth address of the device in little-endian
	 * @rssi rssi value of the advertisement package
	 *
	 * If the bluetooth address is already in the list increase the occurrence by one
	 * and update the rssi value; if it is not yet in the list, add it to the list with
	 * occurrence=1; if the list is full, remove the device with lowest occurrence and
	 * add the new device to the list with occurence=1
	 */
	void update(uint8_t * adrs_ptr, int8_t rssi);

	/* Return the number of devices already stored in the list
	 *
	 * @return number of stored devices
	 */
	uint16_t getSize() const;

	/* Clear the list of devices
	 */
	void reset();

	//////////// serializable ////////////////////////////

    /* Return length of buffer required to store the serialized form of this object.
     *
     * The size is determined by multiplying the number of devices with
     * <SERIALIZED_DEVICE_SIZE> and adding the header size <HEADER_SIZE>
     *
     * @return number of bytes required
     */
    uint16_t getSerializedLength() const;

	/* Return the maximum possible length of the serialized object
	 *
	 * This is defined by:
	 * <HEADER_SIZE> + <MAX_NR_DEVICES> * <SERIALIZED_DEVICE_SIZE>
	 *
	 * @return the maximum possible length
	 */
    uint16_t getMaxLength() const {
    	return HEADER_SIZE + MAX_NR_DEVICES * SERIALIZED_DEVICE_SIZE;
    }

    /* Serialize the data into a byte array
     *
     * @buffer buffer in which the data should be copied. **note** the buffer
     *   has to be preallocated with at least <getSerializedLength> size
     * @length length of the buffer in bytes
     *
     * Copy data representing this object into the given buffer. The buffer
     * has to be preallocated with at least <getSerializedLength> bytes. The
     * resulting buffer will have data stored in the form:
     *
     * NR_OF_DEVICES,DEVICE1_BT_ADDR,DEVICE1_RSSI,DEVICE1_OCCURRENCES,DEVICE2_BT_ADDR,
     *   DEVICE_2_RSSI,...
     *
     * one byte for the number of devices, <BLE_GAP_ADDR_LEN> bytes for the
     * bluetooth address of device 1 (in big-endian (MSB first)), one byte for the rssi
     * of device 1, 2 bytes for the occurrences of device 1, <BLE_GAP_ADDR_LEN> bytes for
     * the bluetooth address of device 2, one byte for the rssi of device 2, etc.
     *
     */
    void serialize(uint8_t* buffer, uint16_t length) const;

    /* De-serialize the data from the byte array into this object
     *
     * @buffer buffer containing the data of the object. for the form of the data, see
     * the function <serialize>
     * @length length of the buffer in bytes
     *
     * Copy the data from the given buffer into this object.
     */
    void deserialize(uint8_t* buffer, uint16_t length);

};

//namespace BLEpp { // template has to be in the same namespace as the other CharacteristicT templates
//
///* Scan Result characteristic
// *
// * This template implements the functions specific for the Scan Result Characteristic.
// * It takes care of setting and reading the list of scanned devices and handles
// * notify requests for the characteristic, in particular making sure that the list
// * is sent over the air.
// */
//template<> class CharacteristicT<ScanResult> : public Characteristic<ScanResult> {
//
//private:
//	/* Pointer to buffer used to store the serialized ScanResult object
//	 */
//	uint8_t* _buffer;
//
//	/* Flag to indicate if notification is pending to be sent once currently waiting
//	 * tx operations are completed
//	 */
//	bool _notificationPending;
//
//public:
//	/* Default asign operator
//	 *
//	 * @val ScanResult object which should be assigned
//	 *
//	 * Assigns the ScanResult object to this characteristic
//	 */
//	CharacteristicT& operator=(const ScanResult& val) {
//		Characteristic<ScanResult>::operator=(val);
//		return *this;
//	}
//
//	/* Returns the ScanResult object currently assigned to the characteristic
//	 *
//	 * Serialized the ScanResult object into a byte buffer and returns it as a
//	 * <CharacteristicValue>
//	 *
//	 * @return the serialized ScanResult object in a <CharacteristicValue> object
//	 */
//	CharacteristicValue getCharacteristicValue() {
//		CharacteristicValue value;
//		const ScanResult& t = this->getValue();
//		uint32_t len = t.getSerializedLength();
//		if (_buffer) {
//			free(_buffer);
//		}
//		_buffer = (uint8_t*)calloc(len, sizeof(uint8_t));
//		t.serialize(_buffer, len);
//		return CharacteristicValue(len, _buffer);
//	}
//
//	/* Assign the given <CharacteristicValue> to this characteristic
//	 *
//	 * @value the <CharacteristicValue> object which should be assigned
//	 *
//	 * Deserializes the byte buffer obtained from the <CharacteristicValue>
//	 * into a ScanResult object and assigns that to the charachteristic
//	 */
//	void setCharacteristicValue(const CharacteristicValue& value) {
//		ScanResult t;
//		t.deserialize(value.data, value.length);
//		this->setValue(t);
//	}
//
//	/* Return the maximum possible length of the buffer
//	 *
//	 * This is defined by:
//	 * <HEADER_SIZE> + <MAX_NR_DEVICES> * <SERIALIZED_DEVICE_SIZE>
//	 *
//	 * @return the maximum possible length
//	 */
//	uint16_t getValueMaxLength() {
//		return HEADER_SIZE + MAX_NR_DEVICES * SERIALIZED_DEVICE_SIZE;
//	}
//
//	/* Callback function if a notify tx error occurs
//	 *
//	 * This is called when the notify operation fails with a tx error. This
//	 * can occur when too many tx operations are taking place at the same time.
//	 *
//	 * A <BLEpp::CharacteristicBase::notify> is called when the master device
//	 * connected to the Crownstone requests automatic notifications whenever
//	 * the value changes.
//	 */
//	void onNotifyTxError() {
////		LOGw("[%s] no tx buffers, waiting for BLE_EVT_TX_COMPLETE!", _name.c_str());
//		_notificationPending = true;
//	}
//
//	/* Callback function once tx operations complete
//	 *
//	 * @p_ble_evt the event object which triggered the onTxComplete callback
//	 *
//	 * This is called whenever tx operations complete. If a notification is pending
//	 * <BLEpp::CharacteristicBase::notify> is called again and the notification
//	 * is cleared if the call eas successful. If not successful, it will be tried
//	 * again during the next callback call
//	 */
//	void onTxComplete(ble_common_evt_t * p_ble_evt) {
//		// if we have a notification pending, try to send it
//		if (_notificationPending) {
//			uint32_t err_code = notify();
//			if (err_code != NRF_SUCCESS) {
////				LOGw("[%s] failed to resend notification!, err_code: %d", _name.c_str(), err_code);
//			} else {
////				LOGi("[%s] successfully resent notification", _name.c_str());
//				_notificationPending = false;
//			}
//		}
//	}
//
//};
//
//}
