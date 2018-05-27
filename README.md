# M5Stack_PulseSensor

ESP32を搭載したIoT端末M5Stackと心拍センサーを使ったサンプルプログラムです。

## M5Stack_PulseSensor

M5Stackと心拍センサーを使い、心拍波形と1分間の心拍数をM5StackのLCD画面に描きます。
心拍センサーはpulsesensor.comのものを使用。
このセンサーをArduinoでアクセスするライブラリー(ESP32/M5Stackで動作するようにしたもの)は以下にあります。

* [PulseSensorPlayground](https://github.com/AmbientDataInc/PulseSensorPlayground)

## M5Stack_PulseSensor_GPS

M5Stackと心拍センサーを使い、心拍波形と1分間の心拍数をM5StackのLCD画面に描きます。
30秒ごとにGPSで現在地を調べ、位置とその時の心拍数を[IoTデーター可視化サービスAmbient](https://ambidata.io)に送信して、
地図上に表示します。
