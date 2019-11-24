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
    printf("Couldn't get node info\n");
    return;
  }
  printf("App Name: %s\n", nodeInfo.appName.c_str());
  printf("App Version: %s\n", nodeInfo.appVersion.c_str());
  printf("JRE Version: %s\n", nodeInfo.jreVersion.c_str());
  printf("Available Processors: %d\n", nodeInfo.jreAvailableProcessors);
  printf("Free Memory: %llu\n", nodeInfo.jreFreeMemory);
  printf("Max Memory: %llu\n", nodeInfo.jreMaxMemory);
  printf("Total Memory: %llu\n", nodeInfo.jreTotalMemory);
  printf("Latest Milestone: %s\n", nodeInfo.latestMilestone.c_str());
  printf("Latest Milestone Index: %d\n", nodeInfo.latestMilestoneIndex);
  printf("Latest Solid Subtangle Milestone: %s\n",
    nodeInfo.latestSolidSubtangleMilestone.c_str());
  printf("Latest Solid Subtangle Milestone Index: %d\n",
    nodeInfo.latestSolidSubtangleMilestoneIndex);
  printf("Neighbors: %d\n", nodeInfo.neighbors);
  printf("Packet Queue Size: %d\n", nodeInfo.packetsQueueSize);
  printf("Tips: %d\n", nodeInfo.tips);
  printf("Transactions to Request: %d\n", nodeInfo.transactionsToRequest);
  printf("Features:");
  for (auto it = nodeInfo.features.cbegin(); it != nodeInfo.features.cend();
      it++) {
    printf(" %s", (*it).c_str());
  }
  printf("\nCoordinator Address: %s\n", nodeInfo.coordinatorAddress.c_str());
}

static void dumpTransaction(String txHash) {
  struct IotaTx tx;

  if (!iotaClient.getTransaction(txHash, &tx)) {
    printf("Couldn't get transaction\n");
    return;
  }
  printf("Signature or Message: %s\n", tx.signatureMessage.c_str());
  printf("Address: %s\n", tx.address.c_str());
  printf("Value: %lld\n", tx.value);
  printf("Obsolete Tag: %s\n", tx.obsoleteTag.c_str());
  printf("Tag: %s\n", tx.tag.c_str());
  printf("Timestamp: %lld\n", tx.timestamp);
  printf("Current Index: %lld\n", tx.currentIndex);
  printf("Last Index: %lld\n", tx.lastIndex);
  printf("Bundle: %s\n", tx.bundle.c_str());
  printf("Trunk: %s\n", tx.trunk.c_str());
  printf("Branch: %s\n", tx.branch.c_str());
  printf("Attachment Timestamp: %lld\n", tx.attachmentTimestamp);
  printf("Attachment Timestamp Lower Bound: %lld\n",
    tx.attachmentTimestampLowerBound);
  printf("Attachment Timestamp Upper Bound: %lld\n",
    tx.attachmentTimestampUpperBound);
  printf("Nonce: %s\n", tx.nonce.c_str());
}

void setup() {
  String seed =
    "DONOTUSETHISSEEDDONOTUSETHISSEEDDONOTUSETHISSEEDDONOTUSETHISSEEDDONOTUSETHISSEED9";

  Serial.begin(115200);

  WiFi.begin("WIFISSID", "WIFIPASSWORD");
  do {
    printf(".");
    delay(1000);
  } while (WiFi.status() != WL_CONNECTED);
  printf(" Wi-Fi connected\n");

  if (!iotaWallet.begin(seed)) {
    printf("Cannot initialize IOTA wallet\n");
    return;
  }

  printf("Node Info:\n");
  dumpNodeInfo();

  uint64_t balance;
  if (iotaWallet.getBalance(&balance)) {
    printf("\nIOTA balance: %llu\n", balance);
  }
  else {
    printf("\nCouldn't get IOTA balance\n");
  }

  printf("\nGenerating addresses from seed:\n");
  for (int index = 0; index < 10; index++) {
    printf("  index %d: %s\n", index,
      iotaWallet.getAddress(index).c_str());
  }

  std::vector<String> bundles;
  bundles.push_back(
    "KMMHSPIHTFLBLSHQEFENYIOSDZCLRWHU9IPUGEVJHHAXFGUANTFFOKGFRWBRDVAFBKRDGNC9ZTRAKBNIC");
  std::vector<String> txs;
  printf("\n");
  if (iotaClient.findTransactions(txs, bundles)) {
    printf("Found %d transaction(s) in bundle\n", txs.size());
    if (txs.size() > 0) {
      String &firstTx = txs[0];

      printf("First Transaction (%s):\n", firstTx.c_str());
      dumpTransaction(firstTx);
    }
  }
  else {
    printf("Couldn't find transactions in bundle\n");
  }
}

void loop() {
}
