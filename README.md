# iws450ctl IWS450-US 制御コマンド


## コンパイルに必要なもの

libusbを使用します。libusbのライブラリ・ヘッダファイルが必要です。
本ソフトウェアはMakefileの中でlibusb-configコマンドを呼び出して
動的にコンパイルに必要なライブラリ引数を決定しています。
多くのディストリビューションでは、libusb-devパッケージをインストールすると、
libusb-configコマンドがインストールされます。
インストール方法はディストリビューションのマニュアルを参照ください。


## コマンドの実行に必要な作業

linuxの場合、usbデバイスはudevシステムによりアクセス権限が管理されています。
多くの場合、新規に検出されたUSBデバイスについては、
一般ユーザに対して書き込み権限は開放されません。
そのため、デバイスがUSBバスに現れた際にudevが適切なパーミッションを
設定するためのスクリプトを記述する必要があります。

詳しくは "udev usb パーミッション 設定" などでインターネットを検索してください。

デバイスを接続しても、必要なパーミッションがない場合には、コマンドは
"Operation is not permitted" 等の権限が無い旨のエラーを表示します。
udevの設定に必要なハードウェア情報は次の通りです。

     ベンダID:        16c0
     プロダクトID:    05df


### udev設定例(Debian Linux)

例として `/etc/udev/rules.d/99-usb-tokyodevices.rules` を作成し、以下の内容を記入します。

     SUBSYSTEM=="usb", ATTR{idVendor}=="16c0", ATTR{idProduct}=="05df", MODE="0666"


## コンパイル

makeを実行してください。実行ファイルiws450ctlが生成されます。

    % make 


## コマンドの使用方法

Windows版のコマンドと同じです。
東京デバイセズのWebサイトよりマニュアルをご覧ください。
https://tokyodevices.com/


## ライセンス

Apache License Version 2.0を適用します。License.txtをご覧ください。

