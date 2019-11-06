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

#ifndef _IOTA_WALLET_H_
#define _IOTA_WALLET_H_

#include <Arduino.h>

#include "IotaClient.h"
#include "PoWClient.h"

#define IOTA_OK					0
#define IOTA_ERR_INV_ADDR		-1
#define IOTA_ERR_INV_TAG		-2
#define IOTA_ERR_NETWORK		-3
#define IOTA_ERR_FRAGM_BALANCE	-4
#define IOTA_ERR_INSUFF_BALANCE	-5
#define IOTA_ERR_POW			-6
#define IOTA_ERR_NO_MEM			-7

struct iotaAddrWithBalance {
	int addrIdx;
	uint64_t balance;
};

class IotaWallet {
public:

	/** Create an IOTA wallet that manages funds associated to a IOTA seed
      @param iotaClient  IOTA client used to communicate with full IOTA node
      @return none
	*/
	IotaWallet(IotaClient &iotaClient);

	/** Initialize IOTA wallet with seed
      @param seed  String containing 81-character IOTA seed
      @return true if supplied seed is valid, false otherwise
	*/
	bool begin(String seed);

	/** Retrieve current security level
      The security level is an integer number between 1 and 3 that is used to
      generate IOTA addresses and to sign transactions.
      @return current security level
	*/
	unsigned int getSecurityLevel();

	/** Configure security level
      The security level is an integer number between 1 and 3 that is used to
      generate IOTA addresses and to sign transactions.
      @param security  Security level
      @return true if supplied security level is valid, false otherwise
	*/
	bool setSecurityLevel(unsigned int security);

	/** Retrieve current minimum weight magnitude
      The minimum weight magnitude is an integer number used to perform Proof of
      Work when attaching transactions to the tangle.
      @return current minimum weight magnitude
	*/
	unsigned int getMinWeightMagnitude();

	/** Configure minimum weight magnitude
      The minimum weight magnitude is an integer number used to perform Proof of
      Work when attaching transactions to the tangle.
      @param mwm  Minimum weight magnitude
      @return none
	*/
	void setMinWeightMagnitude(unsigned int mwm);

	/** Configure Proof of Work client
      By default, Proof of Work is done by calling the attachToTangle API on the
      IOTA node to which the IOTA client is connected. With this method it is
      possible to perform Proof-of-Work by other means using a custom
      implementation (see the PoWClient abstract class).
      @param client PoWClient class implementation that will be used to perform
             Proof of Work instead of using the attachToTangle API
      @return none
	*/
	void setPoWClient(PoWClient &client);

	/** Retrieve IOTA balance in the wallet
      This method works by requesting from the connected IOTA full node the
      balances associated to a series of consecutive addresses derived from the
      seed, and summing those balances.
      @param balance  Pointer to variable that will hold the retrieved balance,
             expressed in IOTAs
      @param startAddrIdx  Starting index to be used to generate the first
             address for which the balance is requested from the full node; if
             -1 (default value), 0 is used as starting index
      @param nextAddrIdx  Pointer to variable where the index of the first
             address for which the balance has not been requested will be
             stored; if NULL (default value), this information is not returned;
             this parameter can be used when doing mutiple calls to this method
             where the next call uses a "startAddrIdx" argument set to the value
             stored in the "nextAddrIdx" argument in the previous call
      @return true if communication with the IOTA full node is successful, false
              otherwise
	*/
	bool getBalance(uint64_t *balance, unsigned int startAddrIdx = -1,
			unsigned int *nextAddrIdx = NULL);

	/** Retrieve an address that can be used to receive an IOTA transfer
      This method works by querying the connected IOTA full node whether
      addresses derived from the seed have been spent from, and returning the
      first address that has not been spent from.
      @param addr  Reference to string that will hold an address that can be
             used to receive an IOTA transfer
      @param withChecksum  boolean value indicating whether the returned address
             should have the 9-tryte checksum appended to it
      @param startIdx  Starting index to be used to generate the first address
             for which the spending status is requested from the full node; if
             -1 (default value), this method manages internally the address
             indexes, possibly returning results cached from previous method
             calls
      @param addrIdx  Pointer to variable where the index of the first
             unspent address will be stored; if NULL (default value), this
             information is not returned
      @return true if communication with the IOTA full node is successful, false
              otherwise
	*/
	bool getReceiveAddress(String &addr, bool withChecksum = true,
			unsigned int startIdx = -1, unsigned int *addrIdx = NULL);

	/** Attach an address to the tangle
      This method creates a zero-valued IOTA transaction with the specified
      address and attaches it to the tangle by doing Proof of Work.
      @param addr  Address to be attached to the tangle (the address checksum is
             not necessary)
      @return true if communication with the IOTA full node is successful, false
              otherwise
	*/
	bool attachAddress(String addr);

	/** Verifies address checksum for correctness
      @param addr  Address (with appended 9-tryte checksum) whose checksum has
             to be verified
      @return true if checksum is correct, false otherwise
	*/
	bool addrVerifyCksum(String addr);

	/** Send a IOTA amount to a recipient address
      This method works by requesting from the connected IOTA full node the
      balances associated to a series of consecutive addresses derived from the
      seed, until the transfer amount is covered. In addition, if the amount is
      less than the retrieved balance, the reaminder is sent to an unspent
      address (the "change" address), also derived from the seed.
      @param value  IOTA amount to be sent to recipient
      @param recipient  Address of recipient; it must have the 9-tryte checksum
             appended to it
      @param tag  Transaction tag (up to 27 trytes)
      @param inputStartIdx  Starting index to be used to generate the first
             address for which the balance is requested from the full node; if
             -1 (default value), 0 is used as starting index
      @param inputAddrIdx  Pointer to variable where the index of the first
             address for which the balance has not been used will be
             stored; if NULL (default value), this information is not returned;
             this parameter can be used when doing mutiple calls to this method
             where the next call uses a "inputStartIdx" argument set to the
             value stored in the "inputAddrIdx" argument in the previous call;
             in this way, unnecessary queries to the IOTA full node to retrieve
             the available balance can be avoided
      @param changeStartIdx  Starting index to be used to search for the address
             to which the remainder from the IOTA transfer is sent ("change"
             address); if -1 (default value), this method manages internally the
             change address, possibly using information cached from previous
             method calls
      @param changeAddrIdx  Pointer to variable where the index of the change
             address will be stored; if NULL (default value), this information
             is not returned
      @return result codes:
              IOTA_OK: transfer executed successfully
              IOTA_ERR_INV_ADDR: invalid recipient address
              IOTA_ERR_INV_TAG: invalid transaction tag
              IOTA_ERR_NETWORK: communication with IOTA full node failed
              IOTA_ERR_FRAGM_BALANCE: the IOTA amount needed for the transfer is
                                      split between too many addresses
              IOTA_ERR_INSUFF_BALANCE: the IOTA amount needed for the transfer
                                       is not available in addresses derived
                                       from the seed
              IOTA_ERR_POW: Proof of Work executed from user-supplied PowClient
                            failed
              IOTA_ERR_POW: memory allocation error
	*/
	int sendTransfer(uint64_t value, String recipient, String tag = "",
			unsigned int inputStartIdx = -1, unsigned int *inputAddrIdx = NULL,
			unsigned int changeStartIdx = -1,
			unsigned int *changeAddrIdx = NULL);

	/** Generate IOTA public address from private seed
      @param index  Index to be used to generate the address
      @param withChecksum  boolean value indicating whether the returned address
             should have the 9-tryte checksum appended to it
      @return Generated address, expressed as a 81-character string (with a
              9-character checksum appended if the "withChecksum" argument value
              is true)
	*/
	String getAddress(unsigned int index = 0, bool withChecksum = true);

	/** Retrieve address indexes with positive balance
      This method works by requesting from the connected IOTA full node the
      balances associated to a series of consecutive addresses derived from the
      seed, until the needed balance (if not zero) is covered.
      @param list  Pointer to list to be filled with iotaAddrWithBalance
             structures with information on address indexes and corresponding
             balance; if NULL, this information is not returned
      @param listMaxSize  Maximum number of elements to be inserted in the list;
             if zero, no limit is put on the umber of elements
      @param totalBalance  Pointer to variable that will hold the total
             retrieved balance, i.e. the sum of balances from addresses found
             with positive balance; if NULL, this information is not returned
      @param neededBalance  If not zero, the search for addresseswith positive
             balance is stopped as soon as the total retrieved balance is at
             least this amount
      @param startAddrIdx  Starting index to be used to generate the first
             address for which the balance is requested from the full node; if
             -1 (default value), 0 is used as starting index
      @param nextAddrIdx  Pointer to variable where the index of the first
             address for which the balance has not been retrieved will be
             stored; if NULL (default value), this information is not returned;
             this parameter can be used when doing mutiple calls to this method
             where the next call uses a "startAddrIdx" argument set to the
             value stored in the "nextAddrIdx" argument in the previous call;
             in this way, unnecessary queries to the IOTA full node to retrieve
             the available balance can be avoided
      @return true if communication with the IOTA full node is successful, false
              otherwise
	*/
	bool getAddrsWithBalance(std::vector<struct iotaAddrWithBalance> *list,
			int listMaxSize = 0, uint64_t *totalBalance = NULL,
			uint64_t neededBalance = 0, unsigned int startAddrIdx = -1,
			unsigned int *nextAddrIdx = NULL);

	/** Retrieve addresses with transactions in the tangle
      This method works by querying the connected IOTA full node to search for
      transactions containing addresses derived from the private seed, starting
      from address index 0. Note that if a given address is not returned in the
      list of addresses, that doens't mean that the address has never been used,
      because any transasctions using that address might have been purged from
      the IOTA node when making a snapshot.
      @param addrs  Reference to list to be filled with addresses for which at
             least a transaction has been found in the tangle
      @return true if communication with the IOTA full node is successful, false
              otherwise
	*/
	bool findAddresses(std::vector<String> &addrs);

private:
	bool findAddress(char *addr, bool *found);
	void *allocBundle(int numInputs, bool withChange);
	void freeBundle(void *bundle);
	unsigned char _seedBytes[48];
	unsigned int _security;
	unsigned int _mwm;
	IotaClient &_iotaClient;
	PoWClient *_PoWClient;
	int _firstUnspentAddr, _lastSpentAddr;
};

#endif
