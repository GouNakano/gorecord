#include <vcl.h>
#pragma hdrstop

#include <windows.h>
#include <stdexcept>
#include <chrono>
#include <cassert>
#include <iterator>
#include "audio_device.h"
//-----------------------------------
//コンストラクタ
//-----------------------------------
AudioDevice::AudioDevice()
{
	CoInitialize(nullptr);
	status                     = Status::Constructed;
	enumerator                 = nullptr;
	device                     = nullptr;
	audio_client               = nullptr;
	capture_client             = nullptr;
	recorder                   = nullptr;
	sampling_rate              = 0;
	num_channels               = 0;
	bit_per_sample             = 0;
	isAudioClientActive        = false; //AudioClient稼働状態
    wave_deque.clear();
}
//-----------------------------------
//デストラクタ
//-----------------------------------
AudioDevice::~AudioDevice()
{
	//ループバック取り込み終了
	stop();
	//リソース解放
	release();
}
//-----------------------------------
//初期化
//-----------------------------------
bool AudioDevice::ensure(int buffer_length_millisec)
{
	HRESULT hr;

	//enumeratorのCOMオブジェクトの作成
	hr = CoCreateInstance(__uuidof(MMDeviceEnumerator),nullptr,CLSCTX_ALL,IID_PPV_ARGS(&enumerator));
	if(FAILED(hr))
	{
		last_error = "IMMDeviceEnumeratorの生成に失敗しました。";
		return false;
	}
	//GetDefaultAudioEndpoint メソッドは、指定されたデータ フローの方向とロールの既定のオーディオ エンドポイントを取得します。
	hr = enumerator->GetDefaultAudioEndpoint(eRender,eConsole,&device);
	if(FAILED(hr))
	{
		last_error = "既定のオーディオエンドポイントの取得に失敗しました。";
		return false;
	}
	hr = device->Activate(__uuidof(IAudioClient),CLSCTX_ALL,nullptr,(void **)&audio_client);
	if (FAILED(hr))
	{
		last_error = "オーディオデバイスを作動させる事に失敗しました。";
		return false;
	}
	WAVEFORMATEX *mix_format = nullptr;
	hr = audio_client->GetMixFormat(&mix_format);
	if (FAILED(hr))
	{
		last_error = "ミックスフォーマットの取得に失敗しました。";
		return false;
	}
	//ウェーブフォーマットを確保
	memcpy(&wf,mix_format,sizeof(wf));

	sampling_rate  = mix_format->nSamplesPerSec;
	num_channels   = mix_format->nChannels;
	bit_per_sample = mix_format->wBitsPerSample;
	//Initialize メソッドは、オーディオ ストリームを初期化します。
	hr = audio_client->Initialize(
		AUDCLNT_SHAREMODE_SHARED,
		AUDCLNT_STREAMFLAGS_LOOPBACK,
		buffer_length_millisec * 10000,
		0,
		mix_format,
		nullptr
	);
	// Get the size of the allocated buffer.
	hr = audio_client->GetBufferSize(&bufferFrameCount);
	if(FAILED(hr))
	{
		last_error = "Failed to get buffer size.";
		return false;
	}
	//CoTaskMemAlloc 関数または CoTaskMemRealloc 関数の呼び出しによって以前に割り当てられたタスク メモリのブロックを解放します。
	CoTaskMemFree(mix_format);
	if(FAILED(hr))
	{
		last_error = "Failed to initialize audio client.";
		return false;
	}
	//GetBufferSize メソッドは、エンドポイント バッファーのサイズ (最大容量) を取得します。
	hr = audio_client->GetBufferSize(&buffer_frame_count);
	if (FAILED(hr))
	{
		last_error = "Failed to get buffer size.";
		return false;
	}
	//GetService メソッドは、オーディオ クライアント オブジェクトから追加のサービスにアクセスします。
	hr = audio_client->GetService(IID_PPV_ARGS(&capture_client));
	if(FAILED(hr))
	{
		last_error = "Failed get capture client.";
		return false;
	}
	return true;
}
//-----------------------------------
//リソース解放
//-----------------------------------
bool AudioDevice::release()
{
	if(enumerator != nullptr)
	{
		enumerator->Release();
		enumerator = nullptr;
	}
	if(device != nullptr)
	{
		device->Release();
		device = nullptr;
	}
	if(device != nullptr)
	{
		audio_client->Release();
		audio_client = nullptr;
	}
	if(capture_client != nullptr)
	{
		capture_client->Release();
		capture_client = nullptr;
	}
	return true;
}
//-----------------------------------
//録音開始
//-----------------------------------
bool AudioDevice::start()
{
	HRESULT hr;
	//稼働中チェック
	if(recorder != nullptr || isAudioClientActive == true)
	{
		//稼働中なので処理しない
		last_error = "Recording is running.";
		return false;
	}
	//Start メソッドは、オーディオ ストリームを開始します。
	hr = audio_client->Start();
	if(FAILED(hr))
	{
		last_error = "Failed to start recording.";
		return false;
	}
	//AudioClient稼働状態アクティブ
	isAudioClientActive = true;
	//状況は準備
	status = Status::Preparing;
	//waveファイル作成用ウェーブフォーマット設定
	wf2.wFormatTag      = WAVE_FORMAT_PCM;
	wf2.nChannels       = wf.Format.nChannels;
	wf2.nSamplesPerSec  = wf.Format.nSamplesPerSec;
	wf2.wBitsPerSample  = 16;
	wf2.nBlockAlign     = ((wf2.wBitsPerSample / 8) * wf2.nChannels);
	wf2.nAvgBytesPerSec = (wf2.nSamplesPerSec * wf2.nBlockAlign);
	wf2.cbSize          = sizeof(WAVEFORMATEX);

	//WAVEファイルオープン(waveファイル名が有効の場合)
	if(wave_file.empty() == false)
	{
		char *p_wfile = const_cast<char *>(wave_file.c_str());

		hmmio = mmioOpenA(p_wfile, nullptr, MMIO_CREATE | MMIO_WRITE);
		if (hmmio == nullptr)
		{
			last_error = "WAVEファイルオープンに失敗しました。";
			return false;

		}
	}
	else
	{
		hmmio = nullptr;
	}
	//WAVEファイルのヘッダー書き込み
	if(hmmio != nullptr)
	{
		mmckRiff.fccType = mmioStringToFOURCC(TEXT("WAVE"), 0);
		mmioCreateChunk(hmmio, &mmckRiff, MMIO_CREATERIFF);

		mmckFmt.ckid = mmioStringToFOURCC(TEXT("fmt "), 0);
		mmioCreateChunk(hmmio, &mmckFmt, 0);
		mmioWrite(hmmio, (char *)&wf2,sizeof(WAVEFORMATEX));
		mmioAscend(hmmio, &mmckFmt, 0);

		mmckData.ckid = mmioStringToFOURCC(TEXT("data"), 0);
		mmioCreateChunk(hmmio, &mmckData, 0);
	}

	//録音スレッド開始
	recorder = new std::thread(&AudioDevice::recordingTh,this);

	return true;
}
//-----------------------------------
//録音スレッドを停止
//-----------------------------------
bool AudioDevice::stop()
{
	HRESULT hr;
	//スレッドの存在チェック
	if(recorder == nullptr)
	{
		last_error = "Recording is not running.";
		return false;
	}
	// 録音スレッドを停止
	if(recorder->joinable())
	{
		//状況は停止要求
		status = Status::StopRequest;
		//停止まで待つ
		recorder->join();
		//スレッド削除
		delete recorder;
		recorder = nullptr;
		//状況は停止
		status = Status::Stopped;
	}
	//オーディオ ストリームを停止
	if(audio_client != nullptr)
	{
		//Stop メソッドは、オーディオ ストリームを停止します。
		hr = audio_client->Stop();
		//チェック
		if(FAILED(hr))
		{
			last_error = "Audio client is not able to stop.";
			return false;
		}
		//AudioClient稼働状態非アクティブ
		isAudioClientActive = true;
	}
	//WAVEファイルを閉じる
	if(hmmio != nullptr)
	{
		mmioAscend(hmmio, &mmckData, 0);
		mmioAscend(hmmio, &mmckRiff, 0);
		mmioClose(hmmio, 0);
	}
	return true;
}
//-----------------------------------
//サンプリングレートを得る
//-----------------------------------
int AudioDevice::get_sampling_rate()
{
	return sampling_rate;
}
//-----------------------------------
//チャンネル数を得る
//-----------------------------------
int AudioDevice::get_num_channels()
{
	return num_channels;
}
//-----------------------------------
//デフォルトデバイスを得る
//-----------------------------------
IMMDevice* AudioDevice::get_default_device()
{
	return device;
}
//-----------------------------------
//バッファを得る
//-----------------------------------
bool AudioDevice::get_buffer(std::vector<short>& wave_inf)
{
	//wave_deque競合回避のロック
	std::lock_guard<std::mutex> lock(mtx_);
	//要素数チェック
	if(wave_deque.size() < 1)
	{
		return false;
	}
	//先頭の要素を得る
	std::vector<short>& first = wave_deque.front();
	//コピー
	int sz = first.size();
	wave_inf.resize(first.size());
	std::copy(first.begin(),first.end(),wave_inf.begin());
	//先頭の要素削除
	wave_deque.pop_front();

	return true;;
}
//-----------------------------------
//バッファをリセット
//-----------------------------------
void AudioDevice::reset_buffer()
{
	//準備状態
	status = Status::Preparing;
}
//-----------------------------------
//最後のエラー文字列を得る
//-----------------------------------
std::string AudioDevice::get_last_error()
{
	return last_error;
}
//-----------------------------------
//録音スレッド
//-----------------------------------
void AudioDevice::recordingTh()
{
	HRESULT            hr;
	BYTE              *fragment;
	UINT32             num_frames_available;
	DWORD              flags;
	CRITICAL_SECTION   critical_section;
	std::vector<short> wave_data;
	std::vector<short> sum_wave_data;
	float              fval;
	double             dval;
	short              sval;
	float             *wave_float;
	short             *wave_short;
	char               buf[256];

	// クリティカルセクション初期化
	InitializeCriticalSection(&critical_section);

	//開始時刻
	DWORD st = GetTickCount();
	//スレッドループ
	while(true)
	{
		//停止要求チェック
		if(status == Status::StopRequest)
		{
			//停止状態フラグを立てて、スレッドを終了する
			status = Status::Stopped;
			break;
		}
		//AudioClient稼働状態チェック
		if(isAudioClientActive == false)
		{
			//AudioClient起動待ち
			Sleep(33);
			continue;
		}
		//データ取得体制が出来ているか
		if(status == Status::Constructed)
		{
			//出来ていないので処理を先送り
			Sleep(33);
			continue;
		}
		UINT32 packetLength = 0;
		//Sleep for half the buffer duration.
		Sleep(10);

		while(true)
		{
			//停止要求チェック
			if(status == Status::StopRequest)
			{
				break;
			}
			//合計したwave_data初期化
			sum_wave_data.clear();
			//パケットに残りがある分だけループ
			capture_client->GetNextPacketSize(&packetLength);

			while(packetLength > 0)
			{
				//停止要求チェック
				if(status == Status::StopRequest)
				{
					break;
				}
				//バッファを得る
				fragment             = nullptr;
				num_frames_available = 0;
				flags                = 0;
				hr = capture_client->GetBuffer(&fragment,&num_frames_available,&flags,nullptr,nullptr);
				if(FAILED(hr))
				{
					throw std::runtime_error("Failed to get buffer.");
				}
				//指定したクリティカル セクション オブジェクトの所有権を待機します。 この関数は、呼び出し元のスレッドに所有権が付与されたときに返されます。
				EnterCriticalSection(&critical_section);
				//パケットサイズを得る
				int packet_size = (sizeof(BYTE) * wf.Format.wBitsPerSample * num_frames_available * num_channels)/8;
				//パケットが無音かチェックする
				if((flags & AUDCLNT_BUFFERFLAGS_SILENT) != 0)
				{
					//無音なのでゼロで埋める
					memset(fragment,0,packet_size);
				}

				//パケット内のデータは、前のパケットのデバイス位置と相関してないかチェック
				if((flags & AUDCLNT_BUFFERFLAGS_DATA_DISCONTINUITY) != 0)
				{
					//指定したクリティカル セクション オブジェクトの所有権を解放します。
					LeaveCriticalSection(&critical_section);
					//バッファ解放
					hr = capture_client->ReleaseBuffer(num_frames_available);
					continue;
				}
				//waveデータの確保
				if(wave_data.size() != num_frames_available*num_channels)
				{
					wave_data.resize(num_frames_available*num_channels);
				}

				//ビット数による場合分け
				if(wf.Format.wBitsPerSample == 32)
				{
					//floatのwaveデータのアドレスを得る
					wave_float = reinterpret_cast<float *>(fragment);
					//floatのwaveデータをshortに変換する
					for(int cnt = 0;cnt < num_frames_available*num_channels;cnt++)
					{
						//サンプルビットによる切り替え
						if(wf.Format.wBitsPerSample == 32)
						{
							try
							{
								//32ビットfloatから変換して16ビットの範囲でセット
								dval = wave_float[cnt];
								dval *= 32768.0;
								sval  = static_cast<short>(dval);
								//リストの作成
								wave_data[cnt] = sval;
							}
							catch(Exception& e)
							{
								wave_data[cnt] = wave_data[cnt-1];
							}
						}
					}
				}
				else if(wf.Format.wBitsPerSample == 16)
				{
					//shortのwaveデータのアドレスを得る
					wave_short = reinterpret_cast<short *>(fragment);
					//waveデータセットする
					for(int cnt = 0;cnt < num_frames_available*2;cnt++)
					{
						//16ビットshort
						sval = wave_short[cnt];

						wave_data[cnt] = sval;
					}
				}
				else
				{
					throw std::runtime_error("Bits per sample is not supported.");
				}
				//ウェーブ情報のシーケンスコンテナ連結
				sum_wave_data.insert(sum_wave_data.end(),wave_data.begin(),wave_data.end());

//				wave_deque.push_back(wave_data);
//				//シーケンスコンテナの要素数チェック
//				while(wave_deque.size() > MAX_WAVE_QUEUE)
//				{
//					wave_deque.pop_front();
//				}
//				//waveファイル名が有効の場合は追記
//				if(hmmio != nullptr)
//				{
//					mmioWrite(hmmio,(char *)wave_data.data(),sizeof(short)*num_frames_available*2);
//				}
				//指定したクリティカル セクション オブジェクトの所有権を解放します。
				LeaveCriticalSection(&critical_section);
				//ReleaseBuffer メソッドはバッファーを解放します。
				hr = capture_client->ReleaseBuffer(num_frames_available);
				if(FAILED(hr))
				{
					throw std::runtime_error("Failed to release buffer.");
				}
				//パケットに残りがある分だけループ
				capture_client->GetNextPacketSize(&packetLength);
			}
			//wave_deque競合回避のロック
			std::lock_guard<std::mutex> lock(mtx_);
			//ウェーブ情報のシーケンスコンテナ追加
			wave_deque.push_back(sum_wave_data);
			//シーケンスコンテナの要素数チェック
			while(wave_deque.size() > MAX_WAVE_QUEUE)
			{
				wave_deque.pop_front();
			}
			//waveファイル名が有効の場合は追記
			if(hmmio != nullptr)
			{
				mmioWrite(hmmio,(char *)sum_wave_data.data(),sizeof(short)*sum_wave_data.size());
			}
			//停止する
			Sleep(1);
		}
	}
}
//-----------------------------------
//録音結果の生成wavファイル名をセット
//-----------------------------------
bool AudioDevice::set_wav_file(const std::string& file_name)
{
	//空文字列チェック
	if(file_name.empty() == true)
	{
		wave_file.clear();
	}
	else
	{
		wave_file = file_name;
	}

	return true;
}

