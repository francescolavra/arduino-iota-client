/*
 * MIT License
 *
 * Copyright (c) 2019 Francesco Lavra <francescolavra.fl@gmail.com>
 * and Contributors
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */

#include "IotaClient.h"

#ifdef __cplusplus
extern "C"
{
#endif

#include "iota-c-library/src/iota/conversion.h"

#ifdef __cplusplus
}
#endif

#ifdef IOTACLIENT_DEBUG
#define DPRINTF	Serial.printf
#else
#define DPRINTF(fmt, ...)	do {} while(0)
#endif

IotaClient::IotaClient(Client &networkClient, const char *host, int port)
: _client(networkClient, host, port) {
}

bool IotaClient::getNodeInfo(struct iotaNodeInfo *info) {
	DynamicJsonBuffer jsonBuf;
	JsonObject &jsonReq = jsonBuf.createObject();
	int respStatus;

	jsonReq["command"] = "getNodeInfo";
	respStatus = sendRequest(jsonReq);
	if (respStatus != 200) {
		DPRINTF("%s: response status code %d\n", __FUNCTION__, respStatus);
		return false;
	}
	jsonBuf.clear();

	JsonObject& jsonResp = jsonBuf.parseObject(_client.responseBody());
	if (jsonResp["appName"].is<char *>()) {
		info->appName = jsonResp["appName"].as<String>();
	}
	if (jsonResp["appVersion"].is<char *>()) {
		info->appVersion = jsonResp["appVersion"].as<String>();
	}
	if (jsonResp["jreVersion"].is<char *>()) {
		info->jreVersion = jsonResp["jreVersion"].as<String>();
	}
	if (jsonResp["jreAvailableProcessors"].is<int>()) {
		info->jreAvailableProcessors = jsonResp["jreAvailableProcessors"];
	}
	if (jsonResp["jreFreeMemory"].is<unsigned long>()) {
		info->jreFreeMemory = jsonResp["jreFreeMemory"];
	}
	if (jsonResp["jreMaxMemory"].is<unsigned long>()) {
		info->jreMaxMemory = jsonResp["jreMaxMemory"];
	}
	if (jsonResp["jreTotalMemory"].is<unsigned long>()) {
		info->jreTotalMemory = jsonResp["jreTotalMemory"];
	}
	if (jsonResp["latestMilestone"].is<char *>()) {
		info->latestMilestone = jsonResp["latestMilestone"].as<String>();
	}
	if (jsonResp["latestMilestoneIndex"].is<int>()) {
		info->latestMilestoneIndex = jsonResp["latestMilestoneIndex"];
	}
	if (jsonResp["latestSolidSubtangleMilestone"].is<char *>()) {
		info->latestSolidSubtangleMilestone =
				jsonResp["latestSolidSubtangleMilestone"].as<String>();
	}
	if (jsonResp["latestSolidSubtangleMilestoneIndex"].is<int>()) {
		info->latestSolidSubtangleMilestoneIndex =
				jsonResp["latestSolidSubtangleMilestoneIndex"];
	}
	if (jsonResp["neighbors"].is<int>()) {
		info->neighbors = jsonResp["neighbors"];
	}
	if (jsonResp["packetsQueueSize"].is<int>()) {
		info->packetsQueueSize = jsonResp["packetsQueueSize"];
	}
	if (jsonResp["tips"].is<int>()) {
		info->tips = jsonResp["tips"];
	}
	if (jsonResp["transactionsToRequest"].is<int>()) {
		info->transactionsToRequest = jsonResp["transactionsToRequest"];
	}
	if (jsonResp["coordinatorAddress"].is<char *>()) {
		info->coordinatorAddress = jsonResp["coordinatorAddress"].as<String>();
	}
	if (jsonResp["features"].is<JsonArray>()) {
		JsonArray &featureArray = jsonResp["features"].as<JsonArray&>();
		info->features.clear();
		for (int i = 0; i < featureArray.size(); i++) {
			info->features.push_back(featureArray.get<String>(i));
		}
	}
	return true;
}

bool IotaClient::getBalances(std::vector<String> &addrs,
		std::vector<uint64_t> &balances) {
	DynamicJsonBuffer jsonBuf;
	JsonObject &jsonReq = jsonBuf.createObject();
	int respStatus;

	jsonReq["command"] = "getBalances";
	JsonArray& addrArray = jsonBuf.createArray();
	for (auto it = addrs.cbegin(); it != addrs.cend(); it++) {
		addrArray.add(*it);
	}
	jsonReq["addresses"] = addrArray;
	jsonReq["threshold"] = 100;
	respStatus = sendRequest(jsonReq);
	jsonBuf.clear();
	if (respStatus == 200) {
		JsonObject& jsonResp = jsonBuf.parseObject(_client.responseBody());

		if (jsonResp["balances"].is<JsonArray>()) {
			balances.clear();
			JsonArray &balanceArray = jsonResp["balances"].as<JsonArray&>();
			for (int i = 0; i < balanceArray.size(); i++) {
				balances.push_back(balanceArray.get<uint64_t>(i));
			}
			return true;
		}
	}
	return false;
}

bool IotaClient::findTransactions(std::vector<String> &txs,
		std::vector<String> bundles, std::vector<String> addrs,
		std::vector<String> tags, std::vector<String> approvees) {
	DynamicJsonBuffer jsonBuf;
	JsonObject &jsonReq = jsonBuf.createObject();
	int respStatus;

	jsonReq["command"] = "findTransactions";
	if (bundles.size() != 0) {
		JsonArray& bundleArray = jsonBuf.createArray();
		for (auto it = bundles.cbegin(); it != bundles.cend(); it++) {
			bundleArray.add(*it);
		}
		jsonReq["bundles"] = bundleArray;
	}
	if (addrs.size() != 0) {
		JsonArray& addrArray = jsonBuf.createArray();
		for (auto it = addrs.cbegin(); it != addrs.cend(); it++) {
			addrArray.add(*it);
		}
		jsonReq["addresses"] = addrArray;
	}
	if (tags.size() != 0) {
		JsonArray& tagArray = jsonBuf.createArray();
		for (auto it = tags.cbegin(); it != tags.cend(); it++) {
			tagArray.add(*it);
		}
		jsonReq["tags"] = tagArray;
	}
	if (approvees.size() != 0) {
		JsonArray& approveeArray = jsonBuf.createArray();
		for (auto it = approvees.cbegin(); it != approvees.cend(); it++) {
			approveeArray.add(*it);
		}
		jsonReq["approvees"] = approveeArray;
	}
	respStatus = sendRequest(jsonReq);
	if (respStatus != 200) {
		DPRINTF("%s: response status code %d\n", __FUNCTION__, respStatus);
		return false;
	}
	jsonBuf.clear();
	txs.clear();
	JsonObject& jsonResp = jsonBuf.parseObject(_client.responseBody());
	if (jsonResp["hashes"].is<JsonArray>()) {
		JsonArray &hashArray = jsonResp["hashes"].as<JsonArray&>();
		for (int i = 0; i < hashArray.size(); i++) {
			txs.push_back(hashArray.get<String>(i));
		}
	}
	return true;
}

bool IotaClient::getTransaction(String &hash, struct IotaTx *tx) {
	DynamicJsonBuffer jsonBuf;
	JsonObject &jsonReq = jsonBuf.createObject();
	JsonArray& hashArray = jsonBuf.createArray();
	int respStatus;

	jsonReq["command"] = "getTrytes";
	hashArray.add(hash);
	jsonReq["hashes"] = hashArray;
	respStatus = sendRequest(jsonReq);
	if (respStatus != 200) {
		DPRINTF("%s: response status code %d\n", __FUNCTION__, respStatus);
		return false;
	}
	jsonBuf.clear();
	JsonObject& jsonResp = jsonBuf.parseObject(_client.responseBody());
	if (!jsonResp["trytes"].is<JsonArray>()) {
		return false;
	}
	JsonArray &txArray = jsonResp["trytes"].as<JsonArray&>();
	if (txArray.size() != 1) {
		return false;
	}
	String txChars = txArray.get<String>(0);
	tx->signatureMessage = txChars.substring(0, 2187);
	tx->address = txChars.substring(2187, 2268);
	chars_to_int64(txChars.c_str() + 2268, &tx->value, 27);
	tx->obsoleteTag = txChars.substring(2295, 2322);
	chars_to_int64(txChars.c_str() + 2322, &tx->timestamp, 9);
	chars_to_int64(txChars.c_str() + 2331, &tx->currentIndex, 9);
	chars_to_int64(txChars.c_str() + 2340, &tx->lastIndex, 9);
	tx->bundle = txChars.substring(2349, 2430);
	tx->trunk = txChars.substring(2430, 2511);
	tx->branch = txChars.substring(2511, 2592);
	tx->tag = txChars.substring(2592, 2619);
	chars_to_int64(txChars.c_str() + 2619, &tx->attachmentTimestamp, 9);
	chars_to_int64(txChars.c_str() + 2628, &tx->attachmentTimestampLowerBound,
			9);
	chars_to_int64(txChars.c_str() + 2637, &tx->attachmentTimestampUpperBound,
			9);
	tx->nonce = txChars.substring(2646, NUM_TRANSACTION_TRYTES);
	return true;
}

bool IotaClient::getTransactionsToApprove(int depth, String &trunk,
		String &branch) {
	DynamicJsonBuffer jsonBuf;
	JsonObject &jsonReq = jsonBuf.createObject();
	int respStatus;

	jsonReq["command"] = "getTransactionsToApprove";
	jsonReq["depth"] = depth;
	respStatus = sendRequest(jsonReq);
	if (respStatus != 200) {
		DPRINTF("%s: response status code %d\n", __FUNCTION__, respStatus);
		return false;
	}
	jsonBuf.clear();
	JsonObject& jsonResp = jsonBuf.parseObject(_client.responseBody());
	if (!jsonResp["trunkTransaction"].is<char *>() ||
			!jsonResp["branchTransaction"].is<char *>()) {
		return false;
	}
	trunk = jsonResp["trunkTransaction"].as<String>();
	branch = jsonResp["branchTransaction"].as<String>();
	return true;
}

bool IotaClient::attachToTangle(String &trunk, String &branch, int mwm,
		std::vector<String> &txs, std::vector<String> &txsWithPoW) {
	DynamicJsonBuffer jsonBuf;
	JsonObject &jsonReq = jsonBuf.createObject();
	JsonArray& txArray = jsonBuf.createArray();
	int respStatus;

	jsonReq["command"] = "attachToTangle";
	jsonReq["trunkTransaction"] = trunk;
	jsonReq["branchTransaction"] = branch;
	jsonReq["minWeightMagnitude"] = mwm;
	for (auto it = txs.cbegin(); it != txs.cend(); it++) {
		txArray.add(*it);
	}
	jsonReq["trytes"] = txArray;
	respStatus = sendRequest(jsonReq);
	if (respStatus != 200) {
		DPRINTF("%s: response status code %d\n", __FUNCTION__, respStatus);
		return false;
	}
	jsonBuf.clear();
	JsonObject& jsonResp = jsonBuf.parseObject(_client.responseBody());
	if (!jsonResp["trytes"].is<JsonArray>()) {
		return false;
	}
	JsonArray& txWithPoWArray = jsonResp["trytes"].as<JsonArray&>();
	if (txWithPoWArray.size() != txs.size()) {
		return false;
	}
	txsWithPoW.clear();
	for (int i = 0; i < txWithPoWArray.size(); i++) {
		txsWithPoW.push_back(txWithPoWArray.get<String>(i));
	}
	return true;
}

bool IotaClient::storeTransactions(std::vector<String> &txs) {
	DynamicJsonBuffer jsonReqBuf;
	JsonObject &jsonReq = jsonReqBuf.createObject();
	JsonArray& txArray = jsonReqBuf.createArray();

	jsonReq["command"] = "storeTransactions";
	for (auto it = txs.cbegin(); it != txs.cend(); it++) {
		txArray.add(*it);
	}
	jsonReq["trytes"] = txArray;
	return (sendRequest(jsonReq) == 200);
}

bool IotaClient::broadcastTransactions(std::vector<String> &txs) {
	DynamicJsonBuffer jsonReqBuf;
	JsonObject &jsonReq = jsonReqBuf.createObject();
	JsonArray& txArray = jsonReqBuf.createArray();

	jsonReq["command"] = "broadcastTransactions";
	for (auto it = txs.cbegin(); it != txs.cend(); it++) {
		txArray.add(*it);
	}
	jsonReq["trytes"] = txArray;
	return (sendRequest(jsonReq) == 200);
}

bool IotaClient::wereAddressesSpentFrom(std::vector<String> &addrs,
		std::vector<bool> &spent) {
	DynamicJsonBuffer jsonBuf;
	JsonObject &jsonReq = jsonBuf.createObject();
	int respStatus;

	jsonReq["command"] = "wereAddressesSpentFrom";
	JsonArray& addrArray = jsonBuf.createArray();
	for (auto it = addrs.cbegin(); it != addrs.cend(); it++) {
		addrArray.add(*it);
	}
	jsonReq["addresses"] = addrArray;
	respStatus = sendRequest(jsonReq);
	jsonBuf.clear();
	if (respStatus == 200) {
		JsonObject& jsonResp = jsonBuf.parseObject(_client.responseBody());

		if (jsonResp["states"].is<JsonArray>()) {
			spent.clear();
			JsonArray &spentArray = jsonResp["states"].as<JsonArray&>();
			for (int i = 0; i < spentArray.size(); i++) {
				spent.push_back(spentArray.get<bool>(i));
			}
			return true;
		}
	}
	return false;
}

int IotaClient::sendRequest(JsonObject &jsonReq) {
	_client.beginRequest();
	_client.post("/");
	_client.sendHeader("Content-Type", "application/json");
	_client.sendHeader("X-IOTA-API-Version", "1");
	_client.sendHeader("Content-Length", jsonReq.measureLength());
	_client.beginBody();
	jsonReq.printTo(_client);
	_client.endRequest();
	return _client.responseStatusCode();
}
