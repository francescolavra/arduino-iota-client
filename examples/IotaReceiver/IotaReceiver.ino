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
IotaClient iotaClient(wifiClient, "node2.iota.town", 14265);
IotaWallet iotaWallet(iotaClient);
String receiveAddr;

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

  if (iotaWallet.getReceiveAddress(receiveAddr)) {
    Serial.printf("Please send IOTAs to this address: %s\n",
      receiveAddr.c_str());
    receiveAddr.remove(81);	/* remove address checksum */
  }
  else {
    Serial.println("Couldn't get receive address");
  }
}

void loop() {
  std::vector<String> addrs;
  std::vector<uint64_t> balances;

  addrs.push_back(receiveAddr);
  if (iotaClient.getBalances(addrs, balances)) {
    Serial.printf("Got %llu IOTAs so far\n", balances[0]);
  }
  else {
    Serial.println("Cannot get balances");
  }
  delay(10000);
}
