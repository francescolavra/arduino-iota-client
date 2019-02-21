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

#ifndef _IOTA_CLIENT_H_
#define _IOTA_CLIENT_H_

#include <Arduino.h>
#include <ArduinoHttpClient.h>

#define ARDUINOJSON_USE_LONG_LONG	1
#include <ArduinoJson.h>

struct iotaNodeInfo {
	String appName;
	String appVersion;
	String jreVersion;
	int jreAvailableProcessors;
	unsigned long jreFreeMemory;
	unsigned long jreMaxMemory;
	unsigned long jreTotalMemory;
	String latestMilestone;
	int latestMilestoneIndex;
	String latestSolidSubtangleMilestone;
	int latestSolidSubtangleMilestoneIndex;
	int neighbors;
	int packetsQueueSize;
	int tips;
	int transactionsToRequest;
	std::vector<String> features;
	String coordinatorAddress;
};

struct IotaTx {
	String signatureMessage;
	String address;
	int64_t value;
	String obsoleteTag;
	String tag;
	int64_t timestamp;
	int64_t currentIndex;
	int64_t lastIndex;
	String bundle;
	String trunk;
	String branch;
	int64_t attachmentTimestamp;
	int64_t attachmentTimestampLowerBound;
	int64_t attachmentTimestampUpperBound;
	String nonce;
};

class IotaClient {
public:

	/** Create a IOTA client that communicates with a IOTA full node
      @param networkClient  Network client used to perform the underlying
             network communication
      @param host  IOTA node host, expressed as either host name or IP address
             with dotted notation
      @param port  IOTA node port
      @return none
	*/
	IotaClient(Client &networkClient, const char *host, int port);

	/** Retrieve node information from the remote IOTA node
      @param info  Pointer to node information structure that is filled with
             data received from the remote node
      @return true if node information request is successful, false otherwise
	*/
	bool getNodeInfo(struct iotaNodeInfo *info);

	/** Retrieve balance of a list of addresses
      @param addrs  List of addresses for which the balance must be retrieved
      @param balances  List that is filled with balance values (one value for
             each address)
      @return true if balance request is successful, false otherwise
	*/
	bool getBalances(std::vector<String> addrs,
			std::vector<uint64_t> &balances);

	/** Find transactions that match a set of criteria
      @param txs  List that is filled with hashes of retrieved transactions
      @param bundles  List of hashes of bundles to which transactions must
             belong; if empty, transactions can belong to any bundle
      @param addrs  List of addresses that must be contained in transactions; if
             empty, transactions can contain any address
      @param tags  List of tags that must be contained in transactions; if
             empty, transactions can contain any tag
      @param approvees  List of hashes of transactions that must be approved
             from the transactions to be retrieved; if empty, transactions can
             approve any transaction
      @return true if transaction request is successful, false otherwise
	*/
	bool findTransactions(std::vector<String> &txs,
			std::vector<String> bundles = std::vector<String>(),
			std::vector<String> addrs = std::vector<String>(),
			std::vector<String> tags = std::vector<String>(),
			std::vector<String> approvees = std::vector<String>());

	/** Retreive transaction data from a given transaction hash
      @param hash  Transaction hash
      @param tx  Pointer to structure that is filled with transaction data
             retrieved from the remote node
      @return true if transaction data request is successful, false otherwise
	*/
	bool getTransaction(String hash, struct IotaTx *tx);

	/** Retreive two transactions to be approved (tips) in the tangle
      @param depth  Random walk depth for the tip selection process
      @param trunk  Reference to string that will contain the hash of the first
             transaction to be approved
      @param branch  Reference to string that will contain the hash of the
             second transaction to be approved
      @return true if request is successful, false otherwise
	*/
	bool getTransactionsToApprove(int depth, String &trunk, String &branch);

	/** Attach bundle of transactions to the tangle, by doing Proof of Work
      @param trunk  Hash of trunk transaction to be approved when attaching
             transactions to the tangle; it can be retrieved via the
             getTransactionsToApprove() method
      @param branch  Hash of branch transaction to be approved when attaching
             transactions to the tangle; it can be retrieved via the
             getTransactionsToApprove() method
      @param txs  List of transactions to attach to the tangle as a bundle
      @param txsWithPoW  List that is filled with transaction data with Proof of
             Work
      @return true if request is successful, false otherwise
	*/
	bool attachToTangle(String trunk, String branch, int mwm,
			std::vector<String> txs, std::vector<String> &txsWithPoW);

	/** Store transactions in the tangle
      @param txs  List of transactions (with Proof of Work) to be stored in the
             tangle; it can be retrieved via the attachToTangle() method
      @return true if request is successful, false otherwise
	*/
	bool storeTransactions(std::vector<String> txs);

	/** Broadcast transactions to neighbor nodes
      @param txs  List of transactions (with Proof of Work) to be broadcast to
             neighbors; it can be retrieved via the attachToTangle() method
      @return true if request is successful, false otherwise
	*/
	bool broadcastTransactions(std::vector<String> txs);

	/** Check if IOTA addresses have been spent from
      @param addrs  List of addresses for which the check must be executed
      @param spent  List that will be filled with boolean values (one for each
             address supplied in the first argument) that indicate whether the
             addresses have been spent from
      @return true if request is successful, false otherwise
	*/
	bool wereAddressesSpentFrom(std::vector<String> addrs,
			std::vector<bool> &spent);

private:
	int sendRequest(JsonObject &jsonReq);
	HttpClient _client;
};

#endif
