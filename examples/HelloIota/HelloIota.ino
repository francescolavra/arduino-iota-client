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

#include <IotaWallet.h>
#ifdef ESP8266
#include <ESP8266WiFi.h>
#else
#include <WiFi.h>
#endif

WiFiClient wifiClient;
IotaClient iotaClient(wifiClient, "node05.iotatoken.nl", 14265);
IotaWallet iotaWallet(iotaClient);

static void dumpNodeInfo() {
  struct iotaNodeInfo nodeInfo;

  if (!iotaClient.getNodeInfo(&nodeInfo)) {
    Serial.println("Couldn't get node info");
    return;
  }
  Serial.println("App Name: " + nodeInfo.appName);
  Serial.println("App Version: " + nodeInfo.appVersion);
  Serial.println("JRE Version: " + nodeInfo.jreVersion);
  Serial.println(
    "Available Processors: " + String(nodeInfo.jreAvailableProcessors));
  Serial.printf("Free Memory: %llu\n", nodeInfo.jreFreeMemory);
  Serial.printf("Max Memory: %llu\n", nodeInfo.jreMaxMemory);
  Serial.printf("Total Memory: %llu\n", nodeInfo.jreTotalMemory);
  Serial.println("Latest Milestone: " + nodeInfo.latestMilestone);
  Serial.println(
    "Latest Milestone Index: " + String(nodeInfo.latestMilestoneIndex));
  Serial.println("Latest Solid Subtangle Milestone: " +
    nodeInfo.latestSolidSubtangleMilestone);
  Serial.println("Latest Solid Subtangle Milestone Index: " +
    String(nodeInfo.latestSolidSubtangleMilestoneIndex));
  Serial.println("Neighbors: " + String(nodeInfo.neighbors));
  Serial.println("Packet Queue Size: " + String(nodeInfo.packetsQueueSize));
  Serial.println("Tips: " + String(nodeInfo.tips));
  Serial.println(
    "Transactions to Request: " + String(nodeInfo.transactionsToRequest));
  Serial.print("Features:");
  for (auto it = nodeInfo.features.cbegin(); it != nodeInfo.features.cend();
      it++) {
    Serial.print(" " + *it);
  }
  Serial.println();
  Serial.println("Coordinator Address: " + nodeInfo.coordinatorAddress);
}

static void dumpTransaction(String txHash) {
  struct IotaTx tx;

  if (!iotaClient.getTransaction(txHash, &tx)) {
    Serial.println("Couldn't get transaction");
    return;
  }
  Serial.println("Signature or Message: " + tx.signatureMessage);
  Serial.println("Address: " + tx.address);
  Serial.printf("Value: %lld\n", tx.value);
  Serial.println("Obsolete Tag: " + tx.obsoleteTag);
  Serial.println("Tag: " + tx.tag);
  Serial.printf("Timestamp: %lld\n", tx.timestamp);
  Serial.printf("Current Index: %lld\n", tx.currentIndex);
  Serial.printf("Last Index: %lld\n", tx.lastIndex);
  Serial.println("Bundle: " + tx.bundle);
  Serial.println("Trunk: " + tx.trunk);
  Serial.println("Branch: " + tx.branch);
  Serial.printf("Attachment Timestamp: %lld\n", tx.attachmentTimestamp);
  Serial.printf("Attachment Timestamp Lower Bound: %lld\n",
    tx.attachmentTimestampLowerBound);
  Serial.printf("Attachment Timestamp Upper Bound: %lld\n",
    tx.attachmentTimestampUpperBound);
  Serial.println("Nonce: " + tx.nonce);
}

void setup() {
  String seed =
    "DONOTUSETHISSEEDDONOTUSETHISSEEDDONOTUSETHISSEEDDONOTUSETHISSEEDDONOTUSETHISSEED9";

  Serial.begin(115200);
  WiFi.begin("WIFISSID", "WIFIPASSWORD");
  do {
    Serial.print('.');
    delay(1000);
  } while (WiFi.status() != WL_CONNECTED);
  Serial.println(" Wi-Fi connected");

  if (!iotaWallet.begin(seed)) {
    Serial.println("Cannot initialize IOTA wallet");
    return;
  }

  Serial.println("Node Info:");
  dumpNodeInfo();

  uint64_t balance;
  if (iotaWallet.getBalance(&balance)) {
    Serial.printf("\nIOTA balance: %llu\n", balance);
  }
  else {
    Serial.println("\nCouldn't get IOTA balance");
  }

  Serial.println("\nGenerating addresses from seed:");
  for (int index = 0; index < 10; index++) {
    Serial.printf("  index %d: %s\n", index,
      iotaWallet.getAddress(index).c_str());
  }

  std::vector<String> bundles;
  bundles.push_back(
    "KMMHSPIHTFLBLSHQEFENYIOSDZCLRWHU9IPUGEVJHHAXFGUANTFFOKGFRWBRDVAFBKRDGNC9ZTRAKBNIC");
  std::vector<String> txs;
  Serial.println();
  if (iotaClient.findTransactions(txs, bundles)) {
    Serial.printf("Found %d transaction(s) in bundle\n", txs.size());
    if (txs.size() > 0) {
      String &firstTx = txs[0];

      Serial.println("First Transaction (" + firstTx + "):");
      dumpTransaction(firstTx);
    }
  }
  else {
    Serial.println("Couldn't find transactions in bundle");
  }
}

void loop() {
}
