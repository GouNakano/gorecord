#ifndef AUDIO_DEVICEH
#define AUDIO_DEVICEH
#include <mmdeviceapi.h>
#include <Audioclient.h>
#include <mmiscapi.h>
#include <memory>
#include <vector>
#include <array>
#include <thread>
#include <deque>
#include <mutex>
#include <atomic>
//------------------------------
// 録音データユニット
//------------------------------
struct typDataUnit
{
private:
	//格納データ
	short *data;
	//データの数
	int DataNum;
private:
	//初期化
	void _init()
	{
		if(data != nullptr)
		{
			delete [] data;
			data    = nullptr;
			DataNum = 0;
		}
		else
		{
			DataNum = 0;
		}
	}
	//コピー
	void _copy(const typDataUnit& him)
	{
		//初期化する
		_init();
		//データある場合はコピーする
		if(him.data != nullptr)
		{
			//コピー実行
			DataNum = him.DataNum;
			data    = new short[DataNum];

			memcpy(data,him.data,sizeof(short)*DataNum);
		}
	}

public:
	//デフォルトコンストラクタ
	typDataUnit()
	{
		data    = nullptr;
		DataNum = 0;
	}
	//コピーコンストラクタ
	typDataUnit(const typDataUnit& him)
	{
		data    = nullptr;
		DataNum = 0;
		//コピー
		_copy(him);
	}
	//デストラクタ
	~typDataUnit()
	{
		_init();
	}
public:
	//=演算子
	typDataUnit& operator = (const typDataUnit& him)
	{
		_init();
		//コピー
		_copy(him);

		return *this;
	}
public:
	//データ数を得る
	int getDataNum()
	{
		return DataNum;
	}
	//データの先頭アドレスを得る
	short *getData()
	{
		return data;
	}

public:
	//データセット(左右セットで２と数える)
	bool setData(int sz,short *src)
	{
		//初期化
		_init();
		//データセット
		DataNum = sz;
		data    = new short[sz];
		memcpy(data,src,sizeof(short)*sz);

		return true;
	}
	//データセット(float型からセット)
	bool setData(int sz,float *src)
	{
		static const double MAX_WAVE_HEIGHT = 65536.0;
		//初期化
		_init();
		//データセット
		DataNum = sz;
		data    = new short[sz];
		//floatからshortに変換してセットする
		for(int idx = 0;idx < sz;idx++)
		{
			float val = src[idx]*MAX_WAVE_HEIGHT;

			data[idx] = static_cast<short>(val);
		}

		return true;
	}
};
//------------------------------
// オーディオデバイス
//------------------------------
class AudioDevice
{
private:
	static const int MAX_WAVE_QUEUE = 16;
private:
	enum class Status
	{
		Reinitializing,
		Constructed,
		Preparing,
		Playing,
		StopRequest, //停止要求
		Stopped,     //停止状態
	};

	IMMDeviceEnumerator *enumerator;
	IMMDevice           *device;
	IAudioClient        *audio_client;
	IAudioCaptureClient *capture_client;
	int                  sampling_rate;
	int                  num_channels;
	int                  bit_per_sample;
	UINT32               buffer_frame_count;
	std::atomic<Status>  status;
	//waveファイル作成
	HMMIO    hmmio;
	MMCKINFO mmckRiff;
	MMCKINFO mmckFmt;
	MMCKINFO mmckData;

	//最終エラー文字列
	std::string last_error;
	//録音スレッド
	std::thread *recorder;
	//AudioClient稼働状態
	bool isAudioClientActive;
	//ウェーブフォーマット
	WAVEFORMATEXTENSIBLE  wf;
	//waveファイル作成用ウェーブフォーマット
	WAVEFORMATEX wf2;
	//録音結果の生成wavファイル名
	std::string wave_file;
	//ウェーブ情報のシーケンスコンテナ
	std::deque<std::vector<short>> wave_deque;
	//ロックオブジェクト
	std::mutex mtx_;

	UINT32 bufferFrameCount;

private:
	//録音スレッド
	void recordingTh();
public:
	//コンストラクタ
	AudioDevice();
	//デストラクタ
	~AudioDevice();
public:
	//リソース確保
	bool ensure(int buffer_length_millisec);
	//リソース解放
	bool release();
	//録音開始
	bool start();
	//録音終了
	bool stop();
	//現在取得したWaveデータを取得する
	bool get_buffer(std::vector<short>& wave_inf);
	//録音結果の生成wavファイル名をセット
	bool set_wav_file(const std::string& file_name);
public:
	int         get_sampling_rate();
	int         get_num_channels();
	IMMDevice  *get_default_device();
	void        reset_buffer();
	std::string get_last_error();
};
#endif
