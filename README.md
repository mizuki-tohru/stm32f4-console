This project is the palm size console with LCD and a keyboard. 
This uses STM32F4 microcomputer. This supports a 400x240-dot mono LCD, keyboard, CMOS, RS422, and Bluetooth serial communication. Moreover, this supports analog inputs. 
Charge of the li-po battery using USB, and high-speed access to a microSD card are also possible. 
A console carries extended BASIC of a uBASIC base. BASIC supports access to the FAT filesystem to microSD, bitmap drawing, analog inputs, and serial port input and output. 

このプロジェクトは、LCDとキーボードを持つ、てのひらサイズのコンソールを自作する試みです。
STM32F4マイコンを用い、400x240ドットモノクロ液晶とキーボード、CMOS、RS422、そしてBluetoothシリアル通信をサポートします。また3チャンネルのアナログ入力をサポートします。
USBコネクタを用いたリチウムポリマバッテリの充電と、microSDカードへの高速なアクセスも可能です。
コンソールはuBasicベースの拡張BASICを搭載します。BASICからmicroSDへのFATバイルシステムへのアクセス、ビットマップ描画、アナログ入力及びシリアルポート入出力をサポートします。

フォント作成にはrubyと東雲フォントが必要です。単体テスト用にcunitを使用していますが、テストを行わないのであれば必要ありません。
