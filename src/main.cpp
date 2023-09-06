#include <vcl.h>
#include <windows.h>

#pragma hdrstop
#pragma argsused

#include <tchar.h>
#include <stdio.h>
#include "audio_device.h"


int _tmain(int argc, _TCHAR* argv[])
{
	//コマンドライン引数チェック
	if(argc < 2)
	{
		printf("使い方:\n");
		printf("gorecord 録音結果ファイル(拡張子は .wav)\n");
		printf("例: gorecord test.wav\n\n");
		system("PAUSE");
		return 0;
	}
	//開始メッセージ
	printf("録音開始しました。\n");
	printf("Zキー押下で録音処理を終了します。\n\n");

	//録音結果ファイル名
	std::string wav_file = argv[1];
	//オーディオデバイス初期化
	AudioDevice audio_device;
	if(audio_device.ensure(32) == false)
	{
		//エラーメッセージを受け取る
		std::string err_str = audio_device.get_last_error();
		//エラーメッセージ
		printf("オーディオデバイス初期化失敗[%s]\n",err_str.c_str());
		printf("録音処理を終了します。\n");
		system("PAUSE");
	}
	//録音結果の生成wavファイル名をセット
	audio_device.set_wav_file(wav_file);
	//録音開始
	if(audio_device.start() == false)
	{
		//エラーメッセージを受け取る
		std::string err_str = audio_device.get_last_error();
		//エラーメッセージ
		printf("録音開始失敗[%s]\n",err_str.c_str());
		printf("録音処理を終了します。\n");
		system("PAUSE");
	}
	//終了待ち
	while(true)
	{
		//スレッドチェック
		if(audio_device.get_recording_thread_error() == true)
		{
			//エラーメッセージ
			printf("録音スレッドにエラーが発生したため、録音処理を中止します。\n");
			system("PAUSE");
			//終了処理
			audio_device.stop();
			audio_device.release();
			break;
		}

		//Zキー押下待ち
		if(GetAsyncKeyState('Z') & 0x01)
		{
			//録音終了処理
			if(audio_device.stop() == false)
			{
				//エラーメッセージを受け取る
				std::string err_str = audio_device.get_last_error();
				//エラーメッセージ
				printf("録音終了失敗[%s]\n",err_str.c_str());
				printf("録音処理を終了します。\n");
				system("PAUSE");
				break;
			}
			//リソース解放
			audio_device.release();
			break;
		}
		Sleep(1);
	}
	return 0;
}
