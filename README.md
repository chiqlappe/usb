# USB入力デバイスをPC-8001で使う

## 概要
- USBジョイスティックやトラックボールなどの一般的な入力デバイスを、PC-8001で使用可能にするプロジェクト
- USBホストインターフェースとしてラズパイpicoを採用
- PC-8001とのインターフェースとして8255(PPI)を採用（将来的にはpicoに統一する予定）

## インターフェース基板について
- テストが終了次第、一般公開する予定

## 仕様
### I/Oポート
|ポート番号|説明|データ方向|
|:--:|:--|:--:|
|$8C|データ受信用|入力|
|$8D|未使用||
|$8E|制御信号用|出力|
|$8F|PPI設定用|出力|

### 制御信号
|ビット番号|名称|説明|
|:--:|:--|:--|
|0|CS|モード切り替え|
|1|CLK|クロック|
|2|未使用||
|3|未使用||
|4|未使用||
|5|未使用||
|6|未使用||
|7|未使用||

### PPIのモード
|ポート名|モード|方向|
|:--:|:--|:--:|
|A|MODE0|入力|
|B|未使用||
|C-H|未使用||
|C-L||出力|

## HIDレポート（データ）について
- USB入力デバイスはそれぞれ固有のHIDレポート数を持っている
- レポートは8ビット単位(0~255)で取得できる
- レポートには入力デバイスが取得した移動量やボタンの押下状態などが記録されている
- データの位置や総数はデバイス毎に異なる

## HIDレポート（データ）取得方法

### 初期設定
- PPIにコントロールワード$90を出力する
```
OUT &H8F,&H90
```

### 取得手順(N-BASICの場合)
1. 制御信号を以下の通りに送信する
```basic
OUT &H8E,3 'CS=H,CLK=H
OUT &H8E,1 'CS=H,CLK=L
OUT &H8E,3 'CS=H,CLK=H
```
2. ポートAからHIDレポート数を受信できる
```basic
A=INP(&H8C)
```
3. 制御信号を以下の通りに送信する
```basic
OUT &H8E,2 'CS=L,CLK=H
OUT &H8E,0 'CS=L,CLK=L
OUT &H8E,2 'CS=L,CLK=H
```
4. ポートAからレポートを受信できる
```basic
A=INP(&H8C)
```
5. レポート数だけ3~4を繰り返す

### 動作確認用サンプルプログラム

```basic
10 'initialize PPI
20 OUT&H8F,&H90
30 'send CLOCK signal while CS is HI
40 OUT&H8E,3
50 OUT&H8E,1
60 OUT&H8E,3
70 'receive report number
80 N=INP(&H8C)
90 IF N=0 OR N=&HFF THEN 40
100 FOR I=1 TO N
110 'send CLOCK signal while CS is LOW
120 OUT&H8E,2
130 OUT&H8E,0
140 OUT&H8E,2
150 'receive report
160 D=INP(&H8C):PRINT RIGHT$("0"+HEX$(D),2)+" ";
170 NEXT I
180 PRINT
190 GOTO 40

```

## ラズパイpico用プログラムについて

1. ```usb_hid_board_rev1```プロジェクトをArduino IDEで開く
2. ツールメニュー内の ```USB Stack``` を ```Adafruit TinyUSB Host(native)``` にする
3. 書き込みボタンを押すとコンパイルが実行され、プログラムがpicoに書き込まれる

