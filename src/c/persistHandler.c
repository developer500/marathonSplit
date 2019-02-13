#include <pebble.h>
#include "persistHandler.h"
#include "c/constants.h"

const uint32_t storage_version_key = 200;
const int current_storage_version = 1;

const uint32_t runs_stored_key_offset = 1;
const uint32_t data_stored_key_offset = 2;



int getTimeFromDefault(time_tds * inOutBandPtr,char * inOutRunName) {

	int numBands = 75;

//	int numBands = 72;
//	time_tds countDownBand[MAX_BANDS] = {
//			1199,1386,1222,1396,
//			1211,1223,1224,1148,
//			1182,1369,1263,1201,
//			1200,1334,1172,1336,
//			1216,1255,1255,1236,
//			1320,1240,1241,1172,
//			1161,1160,1190,1175,
//			1167,1184,1212,1189,
//			1174,1180,1162,1201,
//			1222,1201,1213,1217,
//			1163,1162,1204,1230,
//			1228,1202,1179,1180,
//			1203,1235,1203,1220,
//			1206,1205,1208,1219,
//			1167,1210,1208,1178,
//			1200,1200,1200,1200,
//			1200,1200,1200,1200,
//			1200,1200,1200,1200
//							};


	for (int index = 0; index < numBands; index++, inOutBandPtr++) {
		//*inOutBandPtr = countDownBand[index];
		*inOutBandPtr = 1180;
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


	if (!persist_exists(storage_version_key)) {
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

	APP_LOG(APP_LOG_LEVEL_DEBUG, "Saving Run # %d", inRunNumber);
	APP_LOG(APP_LOG_LEVEL_DEBUG, "Number bands # %d", inNumBands);


	for (int index = 0; index < inNumBands; index++) {
		persist_write_int(data_key + 2 + index, inBandPtr[index]);
		APP_LOG(APP_LOG_LEVEL_DEBUG, "Value # %d", inBandPtr[index]);
	}
}
