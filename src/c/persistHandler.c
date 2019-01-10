#include <pebble.h>
#include "persistHandler.h"
#include "c/constants.h"

const uint32_t storage_version_key = 200;
const int current_storage_version = 1;

const uint32_t runs_stored_key_offset = 1;
const uint32_t data_stored_key_offset = 2;



int getTimeFromDefault(time_tds * inOutBandPtr,char * inOutRunName) {

	int numBands = 7;
	time_tds countDownBand[MAX_BANDS] = {
			1210,
			1879,
			3161,
			3807,
			2617,
			3798,
			2660
	};

	for (int index = 0; index < numBands; index++, inOutBandPtr++) {
		*inOutBandPtr = countDownBand[index];
	}

	strcpy(inOutRunName, "Default");

	return numBands;
}

int getRunsStored() {

	if (!persist_exists(storage_version_key + runs_stored_key_offset)) {
		return 0;
	}
	return persist_read_int(storage_version_key + runs_stored_key_offset);
}

void setRunsStored(int inRunNumber) {

	if (inRunNumber >= getRunsStored()) {
		persist_write_int(storage_version_key + runs_stored_key_offset, inRunNumber+1);
	}
}

int getTimeFromPersist(int inRunNumber, time_tds * inOutBandPtr, char * inOutRunName) {


	if (!persist_exists(storage_version_key) || true) {
		int numBands = getTimeFromDefault(inOutBandPtr, inOutRunName);
		setTimeToPersist(inOutBandPtr, numBands, inOutRunName, 0);
		return numBands;
	}

	int last_storage_version = persist_read_int(storage_version_key);
	if (last_storage_version != current_storage_version) {
		int numBands = getTimeFromDefault(inOutBandPtr, inOutRunName);
		setTimeToPersist(inOutBandPtr, numBands, inOutRunName, 0);
		return numBands;
	}

	if (inRunNumber >= getRunsStored()) {
		int numBands = getTimeFromDefault(inOutBandPtr, inOutRunName);
		setTimeToPersist(inOutBandPtr, numBands, inOutRunName, inRunNumber);
		return numBands;
	}


	int data_key = storage_version_key + data_stored_key_offset + inRunNumber * MAX_BANDS;

	persist_read_string(data_key, inOutRunName, 50);
	int numBands = persist_read_int(data_key + 1);

	for (int index = 0; index < numBands; index++) {
		inOutBandPtr[index] = persist_read_int(data_key + 2 + index);

	}

	return numBands;
}


void setTimeToPersist(time_tds * inBandPtr, int inNumBands,char * inRunName, int inRunNumber) {

	persist_write_int(storage_version_key, current_storage_version);

	setRunsStored(inRunNumber);

	int data_key = storage_version_key + data_stored_key_offset + inRunNumber * MAX_BANDS;

	persist_write_string(data_key, inRunName);
	persist_write_int(data_key + 1, inNumBands);


	for (int index = 0; index < inNumBands; index++) {
		persist_write_int(data_key + 2 + index, inBandPtr[index]);
	}
}
