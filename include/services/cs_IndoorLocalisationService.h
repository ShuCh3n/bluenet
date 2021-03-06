/**
 * Author: Dominik Egger
 * Copyright: Distributed Organisms B.V. (DoBots)
 * Date: Oct 21, 2014
 * License: LGPLv3+, Apache License, or MIT, your choice
 */
#pragma once

#include <third/std/function.h>
#include "characteristics/cs_ScanResult.h"
#include "characteristics/cs_TrackDevices.h"
#include "characteristics/cs_UuidConfig.h"
#include "cs_BluetoothLE.h"

#include "common/cs_Storage.h"

#define INDOORLOCALISATION_UUID "7e170000-429c-41aa-83d7-d91220abeb33"

/* Struct used by the <IndoorLocalisationService> to store elements
 */
struct ps_indoorlocalisation_service_t : ps_storage_base_t {
	struct {
		uint8_t list[TRACKDEVICES_HEADER_SIZE + TRACKDEVICES_MAX_NR_DEVICES * TRACKDEVICES_SERIALIZED_SIZE];
	} trackedDevices;
	uint32_t nearbyTimeout;
};

class IndoorLocalizationService : public BLEpp::GenericService {

public:
	typedef function<int8_t()> func_t;

protected:
	// TODO -oDE: are really all of these characteristics part of the
	//   indoor localisation?
	void addSignalStrengthCharacteristic();
	void addScanControlCharacteristic();
	void addPeripheralListCharacteristic();
	void addTrackedDeviceListCharacteristic();
	void addTrackedDeviceCharacteristic();

	void addDeviceTypeCharactersitic();
	void addRoomCharacteristic();

	/* Get a handle to the persistent storage struct and load it from FLASH.
	 *
	 * Persistent storage is implemented in FLASH. Just as with SSDs, it is important to realize that
	 * writing less than a minimal block strains the memory just as much as flashing the entire block.
	 * Hence, there is an entire struct that can be filled and flashed at once.
	 */
	void loadPersistentStorage();

	/* Save to FLASH.
	*/
	void savePersistentStorage();

	void writeTrackedDevices();
	void readTrackedDevices();

	void startTracking();
	void stopTracking();

public:
	IndoorLocalizationService(BLEpp::Nrf51822BluetoothStack& stack);

	void tick();
//	void addSpecificCharacteristics();

	/** Sets the number of ticks the rssi of a device is not above threshold before a device is considered not nearby. */
	void setNearbyTimeout(uint16_t counts);

	/** Returns the number of ticks the rssi of a device is not above threshold before a device is considered not nearby. */
	uint16_t getNearbyTimeout();

	void on_ble_event(ble_evt_t * p_ble_evt);

	void onRSSIChanged(int8_t rssi);
	void setRSSILevel(int8_t RSSILevel);
	void setRSSILevelHandler(func_t func);

#if(SOFTDEVICE_SERIES != 110)
	void onAdvertisement(ble_gap_evt_adv_report_t* p_adv_report);
#endif
	static IndoorLocalizationService& createService(BLEpp::Nrf51822BluetoothStack& stack);
private:
	BLEpp::Nrf51822BluetoothStack* _stack;

	BLEpp::CharacteristicT<int8_t>* _rssiCharac;
	BLEpp::Characteristic<ScanResult>* _peripheralCharac;
	BLEpp::Characteristic<TrackedDeviceList>* _trackedDeviceListCharac;
	BLEpp::Characteristic<TrackedDevice>* _trackedDeviceCharac;
	
	func_t _rssiHandler;

	bool _trackMode;
	bool _trackIsNearby;
	
	bool _initialized;
	bool _scanMode;

	ScanResult* _scanResult;
	TrackedDeviceList* _trackedDeviceList;

	pstorage_handle_t _storageHandle;
	ps_indoorlocalisation_service_t _storageStruct;
};
