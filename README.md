# PCからの音を録音するgorecordコマンド

## 概要

動画や音楽などでPCから音声を出力する場合があると思います。
gorecordは、PCからの音声を音声ファイル(wav形式のみ)として出力します。
ループバック音源を入力データとしているため、
ステレオミキサーを有効にするなどのサウンド関連のPCの設定は不要です。

## 簡単な使用方法

gorecordコマンドは、binフォルダの中にあります。

gorecordコマンドのコマンドライン引数は以下の様になります。

gorecord 録音結果ファイル(拡張子は .wav)

例: gorecord test.wav

gorecordに録音した結果を格納するファイル名を渡すだけの仕様になります。

とりあえず簡単に録音が出来るように、binフォルダの中に、
バッチファイル　record_test.bat を用意しました。
record_test.batを用いた録音の方法は以下の様になります。

1. メディアプレイヤー等を用いてPCから音声が出ている状態にします。
2. record_test.batをダブルクリックして起動すると録音開始になります。
3. 録音を完了する時になったら、キーボードの　Zキー　を押下します。
4. PCからの音声出力を止めます。
5. 作成された、test.wav をダブルクリックして、再生して録音した音声が出力されているか確認します。

## 動作要件

OS : Windows10または11  
メモリ : 不問  
空きディスク容量 : 最低10MBで後は出力される録音ファイルの容量。(1分間録音で3MB程度)

## 開発環境

IED : C++Builder 10.4 Comminuty Edition  
言語 : C++(17)  
フレームワーク : .NET Framework(Windows10/11では標準でインストールされています)

## ソースコード及び使用技術

ソースコードは、srcフォルダの下に格納されています。  
クラスAudioDevice(audio_device.cpp,audio_device.h)では、  
ループバック音源の操作のために、.NET Frameworkの一部であるWASAPIを使用しています。  
従来のPCでは、サンプルレート 44100Hz、サンプル当たりの内容は16ビット整数が主流でしたが。  
近来では、サンプルレート 48000Hz(かそれ以上)、サンプル当たりの内容は32ビットfloatが登場しています。  
32ビットfloatの採用により、音質の向上が見込まれますが、  
今回は32ビットfloatのwavファイル作成が出来なかったため、  
16ビット整数に変換してwavファイルを作成しています。  
クラスAudioDeviceのソースコードはC++Builderで作成されたものですが、  
他のwindows開発環境(Visual C++など)で利用しやすい様に、  
C++Builder独自のコードなどは極力排除しています。
