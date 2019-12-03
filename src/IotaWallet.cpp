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

#include <time.h>

#include "IotaWallet.h"

#ifdef __cplusplus
extern "C"
{
#endif

#include "iota-c-library/src/iota/addresses.h"
#include "iota-c-library/src/iota/bundle.h"
#include "iota-c-library/src/iota/common.h"
#include "iota-c-library/src/iota/conversion.h"
#include "iota-c-library/src/iota/transfers.h"

#if !defined(ESP32) && !defined(ESP8266)

/* Needed for the time(time_t *) function to work. */
int _gettimeofday(struct timeval *__p, void *__tz)
{
	if (__p) {
		uint32_t now = millis();

		__p->tv_sec = now / 1000;
		__p->tv_usec = now - __p->tv_sec * 1000;
	}
	return 0;
}

#endif

#ifdef __cplusplus
}
#endif

#define IOTAWALLET_RANDOMWALK_DEPTH	10

#ifdef IOTAWALLET_DEBUG
#define DPRINTF	printf
#else
#define DPRINTF(fmt, ...)	do {} while(0)
#endif

struct iotaWalletBundle {
	iota_wallet_bundle_description_t descr;
	char bundleHash[NUM_HASH_TRYTES];
	iota_wallet_tx_output_t outTx;
	BUNDLE_CTX bundle_ctx;
};

static std::vector<String> *iotaWalletTxPtr;
static char *iotaWalletBundleHashPtr;

static int iotaWalletBundleHashReceiver(char *hash)
{
	if (iotaWalletBundleHashPtr) {
		memcpy(iotaWalletBundleHashPtr, hash, NUM_HASH_TRYTES);
		return 1;
	}
	else {
		return 0;
	}
}

static int iotaWalletTxReceiver(iota_wallet_tx_object_t *tx_object)
{
	if (iotaWalletTxPtr && iotaWalletBundleHashPtr) {
		String &tx = *(iotaWalletTxPtr->insert(iotaWalletTxPtr->begin(),
				String()));

		if (tx.reserve(NUM_TRANSACTION_TRYTES) == 0) {
			iotaWalletTxPtr->erase(iotaWalletTxPtr->begin());
			return 0;
		}
		for (unsigned int i = 0; i < NUM_TRANSACTION_TRYTES; i++) {
			tx += '9';
		}
		iota_wallet_construct_raw_transaction_chars((char *) tx.c_str(),
				iotaWalletBundleHashPtr, tx_object);
		yield();
		return 1;
	}
	else {
		return 0;
	}
}

IotaWallet::IotaWallet(IotaClient &iotaClient) : _iotaClient(iotaClient) {
	_security = 2;
	_mwm = 14;
	_PoWClient = NULL;
	_firstUnspentAddr = _lastSpentAddr = -1;
}

bool IotaWallet::begin(String seed) {
	if (seed.length() != NUM_HASH_TRYTES) {
		return false;
	}
	iota_wallet_init();
	chars_to_bytes(seed.c_str(), _seedBytes, NUM_HASH_TRYTES);
	return true;
}

unsigned int IotaWallet::getSecurityLevel() {
	return _security;
}

bool IotaWallet::setSecurityLevel(unsigned int security) {
	if (!in_range(security, MIN_SECURITY_LEVEL, MAX_SECURITY_LEVEL)) {
		return false;
	}
	_security = security;
	return true;
}

unsigned int IotaWallet::getMinWeightMagnitude() {
	return _mwm;
}

void IotaWallet::setMinWeightMagnitude(unsigned int mwm) {
	_mwm = mwm;
}

void IotaWallet::setPoWClient(PoWClient &client) {
	_PoWClient = &client;
}

bool IotaWallet::getBalance(uint64_t *balance, unsigned int startAddrIdx,
		unsigned int *nextAddrIdx) {
	return getAddrsWithBalance(NULL, 0, balance, 0, startAddrIdx, nextAddrIdx);
}

bool IotaWallet::getReceiveAddress(String &addr, bool withChecksum,
		unsigned int startIdx, unsigned int *addrIdx) {
	std::vector<String> addrs;
	int idx;

	if ((startIdx == (unsigned int)-1) && (_firstUnspentAddr >= 0)) {
		addr = getAddress(_firstUnspentAddr, withChecksum);
		if (addrIdx) {
			*addrIdx = _firstUnspentAddr;
		}
		return true;
	}
	idx = ((startIdx != (unsigned int)-1) ? startIdx : (_lastSpentAddr + 1));
	while (true) {
		addrs.clear();
		for (int i = 0; i < 8; i++) {
			addrs.push_back(getAddress(idx, false));
			idx++;
		}
		std::vector<bool> spent;
		if (!_iotaClient.wereAddressesSpentFrom(addrs, spent)) {
			DPRINTF("%s: couldn't get spent addresses\n", __FUNCTION__);
			return false;
		}
		for (int i = 0; i < spent.size(); i++) {
			if (!spent[i]) {
				if (withChecksum) {
					unsigned char addrBytes[NUM_HASH_BYTES];
					char fullAddr[NUM_HASH_TRYTES + NUM_ADDR_CKSUM_TRYTES + 1];

					chars_to_bytes(addrs[i].c_str(), addrBytes,
							NUM_HASH_TRYTES);
					get_address_with_checksum(addrBytes, fullAddr);
					fullAddr[NUM_HASH_TRYTES + NUM_ADDR_CKSUM_TRYTES] = '\0';
					addr = String(fullAddr);
				}
				else {
					addr = addrs[i];
				}
				if (addrIdx) {
					*addrIdx = idx - addrs.size() + i;
				}
				else if (_firstUnspentAddr < 0) {
					_firstUnspentAddr = idx - addrs.size() + i;
				}
				return true;
			}
			else if (addrIdx == NULL) {
				_lastSpentAddr = idx - addrs.size() + i;
			}
		}
	}
}

bool IotaWallet::attachAddress(String addr) {
	String trunk, branch;
	iota_wallet_tx_output_t outTx;
	iota_wallet_bundle_description_t bundleDescr;
	char bundleHash[NUM_HASH_TRYTES];
	std::vector<String> txs;

	if (!_iotaClient.getTransactionsToApprove(IOTAWALLET_RANDOMWALK_DEPTH,
			trunk, branch)) {
		return false;
	}
	memcpy(outTx.address, addr.c_str(), sizeof(outTx.address));
	outTx.value = 0;
	memset(outTx.tag, '9', sizeof(outTx.tag));
	memset(&bundleDescr, 0, sizeof(&bundleDescr));
	bundleDescr.output_txs = &outTx;
	bundleDescr.output_txs_length = 1;
	bundleDescr.timestamp = time(NULL);
	iotaWalletBundleHashPtr = bundleHash;
	iotaWalletTxPtr = &txs;
	iota_wallet_create_tx_bundle(iotaWalletBundleHashReceiver,
			iotaWalletTxReceiver, &bundleDescr);
	if (_PoWClient) {
		DPRINTF("%s: using external PoW client\n", __FUNCTION__);
		if (!_PoWClient->pow(trunk, branch, _mwm, txs)) {
			return false;
		}
	}
	else {
		if (!_iotaClient.attachToTangle(trunk, branch, _mwm, txs)) {
			return false;
		}
	}
	return (_iotaClient.storeTransactions(txs) &&
			_iotaClient.broadcastTransactions(txs));
}

bool IotaWallet::addrVerifyCksum(String addr) {
	if (addr.length() != NUM_HASH_TRYTES + NUM_ADDR_CKSUM_TRYTES) {
		return false;
	}
	return (address_verify_checksum(addr.c_str()) == 0);
}

int IotaWallet::sendTransfer(uint64_t value, String recipient, String tag,
		unsigned int inputStartIdx, unsigned int *inputAddrIdx,
		unsigned int changeStartIdx, unsigned int *changeAddrIdx) {
	std::vector<struct iotaAddrWithBalance> inputAddrs;
	uint64_t availableBalance;
	struct iotaWalletBundle *bundle;
	String trunk, branch;
	std::vector<String> txList;
	int ret = IOTA_OK;

	if (!addrVerifyCksum(recipient)) {
		return IOTA_ERR_INV_ADDR;
	}
	if ((tag.length() > NUM_TAG_TRYTES) ||
			(tryte_chars_validate(tag.c_str(), tag.length()) < 0)) {
		return IOTA_ERR_INV_TAG;
	}
	if (value != 0) {
		if (!getAddrsWithBalance(&inputAddrs,
				(MAX_BUNDLE_INDEX_SZ - 2) / _security,
				&availableBalance, value, inputStartIdx, inputAddrIdx)) {
			DPRINTF("%s: couldn't get addresses with balance\n", __FUNCTION__);
			return IOTA_ERR_NETWORK;
		}
		DPRINTF("%s: found %d input address(es), with total balance %llu\n",
				__FUNCTION__, inputAddrs.size(), availableBalance);
		if (availableBalance < value) {
			if (inputAddrs.size() == (MAX_BUNDLE_INDEX_SZ - 2) / _security) {
				return IOTA_ERR_FRAGM_BALANCE;
			}
			else {
				return IOTA_ERR_INSUFF_BALANCE;
			}
		}
	}
	bundle = (struct iotaWalletBundle *) allocBundle(inputAddrs.size(),
			(value != 0) && (availableBalance > value));
	if (!bundle) {
		DPRINTF("%s: couldn't allocate memory for bundle\n", __FUNCTION__);
		return IOTA_ERR_NO_MEM;
	}
	memcpy(bundle->descr.output_txs[0].address, recipient.c_str(),
			sizeof(bundle->descr.output_txs[0].address));
	bundle->descr.output_txs[0].value = (int64_t)value;
	memcpy(bundle->descr.output_txs[0].tag, tag.c_str(), tag.length());
	if (value != 0) {
		for (int i = 0; i < inputAddrs.size(); i++) {
			String addr = getAddress(inputAddrs[i].addrIdx, false);

			memcpy(bundle->descr.input_txs[i].address, addr.c_str(),
					sizeof(bundle->descr.input_txs[i].address));
			bundle->descr.input_txs[i].key_index = inputAddrs[i].addrIdx;
			bundle->descr.input_txs[i].value = inputAddrs[i].balance;
			DPRINTF("%s: input %d: key index %d, address %s, value %llu\n",
					__FUNCTION__, i, bundle->descr.input_txs[i].key_index,
					addr.c_str(), bundle->descr.input_txs[i].value);
		}
		availableBalance -= value;

		if (availableBalance != 0) {
			String changeAddr;
			unsigned int idx;

			while (true) {
				if (!getReceiveAddress(changeAddr, false, changeStartIdx, &idx))
				{
					DPRINTF("%s: couldn't get change address\n", __FUNCTION__);
					ret = IOTA_ERR_NETWORK;
					goto exit;
				}
				for (int i = 0; i < inputAddrs.size(); i++) {
					if (inputAddrs[i].addrIdx == idx) {
						changeStartIdx = idx + 1;
						break;
					}
				}
				if (changeStartIdx == idx + 1) {
					DPRINTF("%s: address index %u found in input list, "
							"searching for another change address\n",
							__FUNCTION__, idx);
					continue;
				}
				else {
					break;
				}
			}
			if (changeAddrIdx != NULL) {
				*changeAddrIdx = idx;
			}
			memcpy(bundle->descr.change_tx->address, changeAddr.c_str(),
					sizeof(bundle->descr.change_tx->address));
			bundle->descr.change_tx->value = (int64_t)availableBalance;
			memcpy(bundle->descr.change_tx->tag, tag.c_str(), tag.length());
			DPRINTF("%s: change transaction: address %s, value %llu\n",
					__FUNCTION__, changeAddr.c_str(),
					bundle->descr.change_tx->value);
		}
	}
	if (!_iotaClient.getTransactionsToApprove(IOTAWALLET_RANDOMWALK_DEPTH,
			trunk, branch)) {
		DPRINTF("%s: couldn't get transactions to approve\n", __FUNCTION__);
		ret = IOTA_ERR_NETWORK;
		goto exit;
	}
	bundle->descr.timestamp = time(NULL);
	iotaWalletBundleHashPtr = bundle->bundleHash;

	iotaWalletTxPtr = &txList;
	DPRINTF("%s: creating bundle with %d output transaction(s), %d input "
			"transaction(s) and %s change transaction\n", __FUNCTION__,
			bundle->descr.output_txs_length, bundle->descr.input_txs_length,
			bundle->descr.change_tx ? "1" : "no");
	iota_wallet_create_tx_bundle_mem(iotaWalletBundleHashReceiver,
			iotaWalletTxReceiver, &bundle->descr, &bundle->bundle_ctx, yield);
	freeBundle(bundle);
	if (_PoWClient) {
		DPRINTF("%s: using external PoW client\n", __FUNCTION__);
		if (!_PoWClient->pow(trunk, branch, _mwm, txList)) {
			return IOTA_ERR_POW;
		}
	}
	else {
		if (!_iotaClient.attachToTangle(trunk, branch, _mwm, txList)) {
			DPRINTF("%s: couldn't attach to tangle\n", __FUNCTION__);
			return IOTA_ERR_NETWORK;
		}
	}
	if (!_iotaClient.storeTransactions(txList)) {
		DPRINTF("%s: couldn't store transactions\n", __FUNCTION__);
		return IOTA_ERR_NETWORK;
	}
	if (value != 0) {
		_firstUnspentAddr = -1;
		if (inputAddrIdx == NULL) {
			_lastSpentAddr = inputAddrs[inputAddrs.size() - 1].addrIdx;
		}
	}
	return (_iotaClient.broadcastTransactions(txList) ? IOTA_OK :
			IOTA_ERR_NETWORK);
exit:
	freeBundle(bundle);
	return ret;
}

String IotaWallet::getAddress(unsigned int index, bool withChecksum) {
	unsigned char addrBytes[NUM_HASH_BYTES];

	get_public_addr(_seedBytes, index, _security, addrBytes);
	yield();
	if (withChecksum) {
		char fullAddr[NUM_HASH_TRYTES + NUM_ADDR_CKSUM_TRYTES + 1];

		get_address_with_checksum(addrBytes, fullAddr);
		fullAddr[NUM_HASH_TRYTES + NUM_ADDR_CKSUM_TRYTES] = '\0';
		return String(fullAddr);
	}
	else {
		char addrChars[NUM_HASH_TRYTES + 1];

		bytes_to_chars(addrBytes, addrChars, NUM_HASH_BYTES);
		addrChars[NUM_HASH_TRYTES] = '\0';
		return String(addrChars);
	}
}

bool IotaWallet::getAddrsWithBalance(
		std::vector<struct iotaAddrWithBalance> *list, int listMaxSize,
		uint64_t *totalBalance, uint64_t neededBalance,
		unsigned int startAddrIdx, unsigned int *nextAddrIdx) {
	std::vector<String> addrs;
	unsigned int addrIdx = startAddrIdx;
	uint64_t balance = 0;

	if (addrIdx == (unsigned int)-1) {
		addrIdx = 0;
	}
	while (true) {
		addrs.clear();
		for (int i = 0; i < 8; i++) {
			addrs.push_back(getAddress(addrIdx, false));
			addrIdx++;
		}
		std::vector<uint64_t> balances;
		if (!_iotaClient.getBalances(addrs, balances)) {
			DPRINTF("%s: couldn't get balances\n", __FUNCTION__);
			return false;
		}
		unsigned long partialBalance = 0;
		for (int i = 0; i < balances.size(); i++) {
			if (balances[i] != 0) {
				partialBalance += balances[i];
				if (list &&
						((listMaxSize <= 0) || (list->size() < listMaxSize))) {
					struct iotaAddrWithBalance listElem = {
							.addrIdx = addrIdx - addrs.size() + i,
							.balance = balances[i],
					};
					list->push_back(listElem);
				}
				if ((neededBalance != 0) &&
						(balance + partialBalance >= neededBalance)) {
					balance += partialBalance;
					addrIdx -= balances.size() - 1 - i;
					goto done;
				}
			}
		}
		if (partialBalance > 0) {
			balance += partialBalance;
			continue;
		}
		std::vector<bool> spent;
		if (!_iotaClient.wereAddressesSpentFrom(addrs, spent)) {
			DPRINTF("%s: couldn't get spent addresses\n", __FUNCTION__);
			return false;
		}
		bool spentAny = false;
		for (auto it = spent.cbegin(); it != spent.cend(); it++) {
			spentAny |= *it;
		}
		if (spentAny) {
			continue;
		}
		else {
			break;
		}
	}
done:
	if (totalBalance) {
		*totalBalance = balance;
	}
	if (nextAddrIdx) {
		*nextAddrIdx = addrIdx;
	}
	return true;
}

bool IotaWallet::findAddresses(std::vector<String> &addrs) {
	unsigned char addrBytes[NUM_HASH_BYTES];
	char addrChars[NUM_HASH_TRYTES + 1];
	unsigned int addrIndex;

	addrs.clear();
	addrChars[NUM_HASH_TRYTES] = '\0';
	for (addrIndex = 0; ; addrIndex++) {
		bool addrFound;

		get_public_addr(_seedBytes, addrIndex, _security, addrBytes);
		yield();
		bytes_to_chars(addrBytes, addrChars, NUM_HASH_BYTES);
		if (!findAddress(addrChars, &addrFound)) {
			return false;
		}
		if (!addrFound) {
			break;
		}
		addrs.push_back(addrChars);
	}
	DPRINTF("%s: found %d address(es)\n", __FUNCTION__, addrs.size());
	return true;
}

bool IotaWallet::findAddress(char *addr, bool *found) {
	std::vector<String> addresses;
	std::vector<String> bundles;
	std::vector<String> txs;

	addresses.push_back(addr);
	if (!_iotaClient.findTransactions(txs, bundles, addresses)) {
		DPRINTF("%s: couldn't find transactions\n", __FUNCTION__);
		return false;
	}
	*found = (txs.size() != 0);
	return true;
}

void *IotaWallet::allocBundle(int numInputs, bool withChange) {
	struct iotaWalletBundle *bundle =
			(struct iotaWalletBundle *) malloc(sizeof(*bundle));

	if (!bundle) {
		DPRINTF("%s: couldn't allocate memory for bundle\n", __FUNCTION__);
		return NULL;
	}
	memset(&bundle->descr, 0, sizeof(bundle->descr));
	memset(bundle->outTx.tag, '9', sizeof(bundle->outTx.tag));
	bundle->descr.output_txs = &bundle->outTx;
	bundle->descr.output_txs_length = 1;
	if (numInputs > 0) {
		bundle->descr.input_txs = (iota_wallet_tx_input_t *) malloc(
				numInputs * sizeof(iota_wallet_tx_input_t));
		if (!bundle->descr.input_txs) {
			DPRINTF("%s: couldn't allocate memory for input transactions\n",
					__FUNCTION__);
			free(bundle);
			return NULL;
		}
		bundle->descr.input_txs_length = numInputs;
	}
	if (withChange) {
		bundle->descr.change_tx = (iota_wallet_tx_output_t *) malloc(
				sizeof(iota_wallet_tx_output_t));
		if (!bundle->descr.change_tx) {
			DPRINTF("%s: couldn't allocate memory for change transaction\n",
					__FUNCTION__);
			free(bundle->descr.input_txs);
			free(bundle);
			return NULL;
		}
		memset(bundle->descr.change_tx->tag, '9',
				sizeof(bundle->descr.change_tx->tag));
	}
	bundle->descr.security = _security;
	bytes_to_chars(_seedBytes, bundle->descr.seed, sizeof(_seedBytes));
	return bundle;
}

void IotaWallet::freeBundle(void *bundle) {
	free(((struct iotaWalletBundle *)bundle)->descr.change_tx);
	free(((struct iotaWalletBundle *)bundle)->descr.input_txs);
	free(bundle);
}
