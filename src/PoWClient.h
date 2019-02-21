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

#ifndef _POW_CLIENT_H_
#define _POW_CLIENT_H_

#include <Arduino.h>

class PoWClient {
public:

	/** Perform Proof of Work on a transaction bundle
      @param trunk  Hash of trunk transaction to be approved when attaching
             transactions to the tangle
      @param branch  Hash of branch transaction to be approved when attaching
             transactions to the tangle
      @param mwm  Minimum weight magnitude to be used when doing Proof of Work
      @param txs  List of transactions consituting the bundle on which Proof of
             Work should be performed
      @param txsWithPoW  List that is filled with transaction data with Proof of
             Work
      @return true if Proof of Work has been done successfully, false otherwise
	*/
	virtual bool pow(String trunk, String branch, int mwm,
			std::vector<String> txs, std::vector<String> &txsWithPoW) = 0;
};

#endif
