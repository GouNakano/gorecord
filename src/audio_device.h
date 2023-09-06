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
// �^���f�[�^���j�b�g
//------------------------------
struct typDataUnit
{
private:
	//�i�[�f�[�^
	short *data;
	//�f�[�^�̐�
	int DataNum;
private:
	//������
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
	//�R�s�[
	void _copy(const typDataUnit& him)
	{
		//����������
		_init();
		//�f�[�^����ꍇ�̓R�s�[����
		if(him.data != nullptr)
		{
			//�R�s�[���s
			DataNum = him.DataNum;
			data    = new short[DataNum];

			memcpy(data,him.data,sizeof(short)*DataNum);
		}
	}

public:
	//�f�t�H���g�R���X�g���N�^
	typDataUnit()
	{
		data    = nullptr;
		DataNum = 0;
	}
	//�R�s�[�R���X�g���N�^
	typDataUnit(const typDataUnit& him)
	{
		data    = nullptr;
		DataNum = 0;
		//�R�s�[
		_copy(him);
	}
	//�f�X�g���N�^
	~typDataUnit()
	{
		_init();
	}
public:
	//=���Z�q
	typDataUnit& operator = (const typDataUnit& him)
	{
		_init();
		//�R�s�[
		_copy(him);

		return *this;
	}
public:
	//�f�[�^���𓾂�
	int getDataNum()
	{
		return DataNum;
	}
	//�f�[�^�̐擪�A�h���X�𓾂�
	short *getData()
	{
		return data;
	}

public:
	//�f�[�^�Z�b�g(���E�Z�b�g�łQ�Ɛ�����)
	bool setData(int sz,short *src)
	{
		//������
		_init();
		//�f�[�^�Z�b�g
		DataNum = sz;
		data    = new short[sz];
		memcpy(data,src,sizeof(short)*sz);

		return true;
	}
	//�f�[�^�Z�b�g(float�^����Z�b�g)
	bool setData(int sz,float *src)
	{
		static const double MAX_WAVE_HEIGHT = 65536.0;
		//������
		_init();
		//�f�[�^�Z�b�g
		DataNum = sz;
		data    = new short[sz];
		//float����short�ɕϊ����ăZ�b�g����
		for(int idx = 0;idx < sz;idx++)
		{
			float val = src[idx]*MAX_WAVE_HEIGHT;

			data[idx] = static_cast<short>(val);
		}

		return true;
	}
};
//------------------------------
// �I�[�f�B�I�f�o�C�X
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
		StopRequest, //��~�v��
		Stopped,     //��~���
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
	//wave�t�@�C���쐬
	HMMIO    hmmio;
	MMCKINFO mmckRiff;
	MMCKINFO mmckFmt;
	MMCKINFO mmckData;

	//�ŏI�G���[������
	std::string last_error;
	//�^���X���b�h
	std::thread *recorder;
	//AudioClient�ғ����
	bool isAudioClientActive;
	//�E�F�[�u�t�H�[�}�b�g
	WAVEFORMATEXTENSIBLE  wf;
	//wave�t�@�C���쐬�p�E�F�[�u�t�H�[�}�b�g
	WAVEFORMATEX wf2;
	//�^�����ʂ̐���wav�t�@�C����
	std::string wave_file;
	//�E�F�[�u���̃V�[�P���X�R���e�i
	std::deque<std::vector<short>> wave_deque;
	//���b�N�I�u�W�F�N�g
	std::mutex mtx_;

	UINT32 bufferFrameCount;

private:
	//�^���X���b�h
	void recordingTh();
public:
	//�R���X�g���N�^
	AudioDevice();
	//�f�X�g���N�^
	~AudioDevice();
public:
	//���\�[�X�m��
	bool ensure(int buffer_length_millisec);
	//���\�[�X���
	bool release();
	//�^���J�n
	bool start();
	//�^���I��
	bool stop();
	//���ݎ擾����Wave�f�[�^���擾����
	bool get_buffer(std::vector<short>& wave_inf);
	//�^�����ʂ̐���wav�t�@�C�������Z�b�g
	bool set_wav_file(const std::string& file_name);
public:
	int         get_sampling_rate();
	int         get_num_channels();
	IMMDevice  *get_default_device();
	void        reset_buffer();
	std::string get_last_error();
};
#endif
