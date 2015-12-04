VERSION_MAIN = 1
VERSION_SUB  = 5
VERSION_VAR  = 6N

# 1.5.6N SERIAL_DEBUGを外す。
# 1.5.6M 0x20にして、CONVTIMEを98にした。
# 1.5.6L 0x20を0x23へもどす。
# 1.5.6K BH1715_TRIGを0x20単発高解像度に変更。失敗。
# 1.5.6J i16BH1715readResult()のi32resultの換算を停止させる。
# 1.5.6H BH1715 対応
# 1.5.5 インタラクティブモードの出力がおかしい点を修正
# 1.5.4 BME280 対応、その他修正
# 1.5.3 ADXL345 シェイクモードの調整 FIFOの修正
# 1.5.2 ADXL345 シェイクモードの追加
# 1.5.1 省エネモードの追加
# 1.5.0 リモート設定機能を実装 TWE-Lite-2525Aのデフォルト送信先をApp_TweLiteに変更 ADXL345のモードを複数種選択可能
# 1.4.1 TWE-Lite 2525Aに対応 複数種のI2Cセンサに対応
# 1.4.0 TWX0003用に機能限定(SHT21のみ動作する 小サイズバイナリ)
# 1.3.8 書式モード出力で正しい値が出力されなかった
# 1.3.7 App_UART の RTS ピンの出力が動作していなかった
# 1.3.6 EndDevice_Remote(テスト実装)用に用意したParentからのMessagePoolの通信は、現時点では不要でありコメントアウト
# 1.3.5 ADT7410の温度計算の修正
# 1.3.4 LM61 の BIAS 設定の初期化が間違えている
# 1.3.3 マイナーバグフィックス
# 1.3.2 SDK 2014/06 の修正版での再ビルド
# 1.3.1 LIS3DH の動作不具合を修正
# 1.3.0 ソース公開版
# 1.2.7 初回リリース版
# 1.2.6 Parent: タイムスタンプ出力機能の追加, DOOR_TIMER: ドアを閉じたときにパケットが送られやすくした
# 1.2.5 Parent: UART 出力形式を追加
# 1.2.4 EndDevice Input: Standard で起床後にしばらく待ちを置く処理を追加 (DIO4 はLOに落ちてから、一定時間待つ)
# 1.2.3 EndDevice Input: DO3(DIO4) の出力が、起床時のみの問題を修正
# 1.2.2 ソースコードメンテ
# 1.2.1 UART 送信デモ対応, SimpleTagV3形式出力, B/O検出無効化
# 1.1.1 SHT21 センサー対応
# 1.0.1 ソースメンテナンス(EdInp)、暗号化の設定機能追加
# 0.9.3 App_UART の UART コマンド処理部を Interactive.c に移植（現時点ではコマンドは未定義）
# 0.9.2 ソースコードのメンテナンス, IO_TIMER を設定で有効に
# 0.9.1 IO_Timer 機能の追加(実験的)
# 0.9.0 Interactive モードの追加 (Parent/EndInp コンパイル確認まで)
# 0.8.2 Routerの動作確認、コメントなどのソース整備
# 0.8.1 消費電流の最適化、パケットのバイナリ化、ADC/PC/BATT測定 (Routerは未対応)
# 0.7.5 pcの測定機能 (enddevice_input)
# 0.7.4 ソースコードメンテナンス、メッセージプールのスロットテストコード追加
# 0.7.3 LCD表示の対応、Routerの送信遅延を設定した
# 0.7.2 暗号化対応確認 (ToCoNet 0.9.9 にて)
# 0.7.1 暗号化対応確認
# 0.6.1 ソースコードメンテナンス
# 0.6.0 子機の発報を親機に伝達後、メッセージプールに記録する
# 0.5.2 親機出力に送信元ルータアドレスを追加, 子機のローカルID(0-9)を設定可能にした
