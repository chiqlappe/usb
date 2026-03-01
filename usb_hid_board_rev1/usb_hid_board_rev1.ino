
/*

USB HID BOARD rev1 用

*/


#ifndef USE_TINYUSB_HOST
  #error This example requires usb stack configured as host in "Tools -> USB Stack -> Adafruit TinyUSB Host"
#endif

// USBHost is defined in usbh_helper.h
#include "usbh_helper.h"


#define CS_PIN 8
#define CLK_PIN 9
#define LED_PIN 25


// 使用するピンのベース（開始）番号を定義
const uint BASE_PIN = 0;
// 8ビット分（GP0〜GP7）のマスクを作成
const uint32_t ALL_PINS_MASK = 0xFF << BASE_PIN;

volatile const uint8_t* hid_report;
volatile uint16_t hid_len;
volatile bool hid_new;

volatile uint8_t counter;
volatile uint8_t output_value;

void setup() {

  Serial1.setTX(12);
  Serial1.setRX(13);
  Serial1.begin(115200);
  Serial1.println("start");

  // 全ピンの初期化を一括で行う
  gpio_init_mask(ALL_PINS_MASK);           // 指定したピンをまとめて初期化
  gpio_set_dir_out_masked(ALL_PINS_MASK);  // 指定したピンをまとめて出力に設定

  pinMode(CLK_PIN, INPUT_PULLUP);
  pinMode(CS_PIN, INPUT_PULLUP);
  pinMode(LED_PIN, OUTPUT);

  hid_new = false;
  hid_len = 0;
  counter = 0;
  output_value = 0;

  gpio_put_masked(ALL_PINS_MASK, 0 << BASE_PIN);

  attachInterrupt(CLK_PIN, Interrupt, FALLING);

}


void Interrupt() {

  output_value = 0;

  if (gpio_get(CS_PIN)) {

    output_value = hid_len;
    counter = 0;

    hid_new = false;

  } else if (counter < hid_len) {

      output_value = hid_report[counter++];

      if (counter == hid_len && !hid_new) {
        hid_len = 0;
      }


  }

}


void loop() {

  // 8ビットデータを出力
  // gpio_put_masked(マスク, 値)
  // 第1引数で操作するピンを指定し、第2引数でそのピンの状態を指定します
  gpio_put_masked(ALL_PINS_MASK, (uint32_t)output_value << BASE_PIN);

  Serial1.flush();
}

//---------- Core1 ----------//
void setup1() {

  USBHost.begin(0);

}


void loop1() {

  USBHost.task();

}


extern "C" {

// HIDインターフェースを持つデバイスがマウントされたときに呼び出されます
// レポート記述子も使用できます。
// tuh_hid_parse_report_descriptor() は、一般的な/十分に単純な記述子を解析するために使用できます。
// 注: レポート記述子の長さがCFG_TUH_ENUMERATION_BUFSIZEより大きい場合、
// スキップされるため、report_desc = NULL、desc_len = 0となります。

void tuh_hid_mount_cb(uint8_t dev_addr, uint8_t instance, uint8_t const *desc_report, uint16_t desc_len) {
  (void) desc_report;
  (void) desc_len;
  uint16_t vid, pid;

  tuh_vid_pid_get(dev_addr, &vid, &pid);

/*
  tone(BUZZER,262,200);
  delay(100);
  tone(BUZZER,330,200);
  delay(100);
*/
  digitalWrite(LED_BUILTIN, HIGH);  // turn the LED on (HIGH is the voltage level)

  Serial1.printf("HID device address = %d, instance = %d is mounted\r\n", dev_addr, instance);
  Serial1.printf("VID = %04x, PID = %04x\r\n", vid, pid);

  if (!tuh_hid_receive_report(dev_addr, instance)) {
    Serial1.printf("Error: cannot request to receive report\r\n");
  }
}


// HIDインターフェースを持つデバイスがマウント解除されたときに呼び出されます
void tuh_hid_umount_cb(uint8_t dev_addr, uint8_t instance) {

/*
  tone(BUZZER,330,200);
  delay(100);
  tone(BUZZER,262,200);
  delay(100);
*/

  digitalWrite(LED_BUILTIN, LOW);  // turn the LED off

  Serial1.printf("HID device address = %d, instance = %d is unmounted\r\n", dev_addr, instance);
}


// 割り込みエンドポイント経由でデバイスからレポートを受信したときに呼び出されます
void tuh_hid_report_received_cb(uint8_t dev_addr, uint8_t instance, uint8_t const *report, uint16_t len) {

  hid_report = report;
  hid_len = len;
  hid_new = true;

/*
  for (uint16_t i = 0; i < hid_len; i++) {
    Serial1.printf("0x%02X ", hid_report[i]);
  }
  Serial1.println();
*/

  // レポートの受信を要求し続ける
  if (!tuh_hid_receive_report(dev_addr, instance)) {
    Serial1.printf("Error: cannot request to receive report\r\n");
  }
}

} // extern C
