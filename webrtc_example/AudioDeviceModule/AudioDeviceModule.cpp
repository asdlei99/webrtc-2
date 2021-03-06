// AudioDeviceModule.cpp: 定义控制台应用程序的入口点。
//

#include "stdafx.h"
#include "modules/audio_device/include/audio_device.h"
#include "modules/audio_device/include/audio_device_defines.h"
#include "modules/audio_mixer/audio_mixer_impl.h"
#include "modules/audio_processing/include/audio_processing.h"
#include "rtc_base/logging.h"


#include "AudioTransportCallback.h"

int main()
{
	// 音频设备操作
	rtc::scoped_refptr<webrtc::AudioDeviceModule> pADM = webrtc::AudioDeviceModule::Create(webrtc::AudioDeviceModule::kWindowsCoreAudio);
	if (pADM.get() == nullptr) {
		RTC_LOG(LS_ERROR) << "AudioDeviceModule::Create Failed : ";
		return -1;
	}

	pADM->Init();  // 注：需要调用否则都是报错
	// 获取录音设备数量
	int16_t num = pADM->RecordingDevices();
	RTC_LOG(INFO) << "RecordingDevices : " << num;
	if (num > 0) {
		char name[webrtc::kAdmMaxDeviceNameSize] = { '\0' };
		char guid[webrtc::kAdmMaxGuidSize] = { '\0' };
		int32_t ret = pADM->RecordingDeviceName(0, name, guid);
		RTC_LOG(INFO) << "RecordingDevices name : " << name << " guid : " << guid << " ret : " << ret;
	}

	num = pADM->PlayoutDevices();
	RTC_LOG(INFO) << "PlayoutDevices : " << num;
	for(int i=0; i < num; i++) {
		char name[webrtc::kAdmMaxDeviceNameSize] = { '\0' };
		char guid[webrtc::kAdmMaxGuidSize] = { '\0' };
		int32_t ret = pADM->PlayoutDeviceName(i, name, guid);
		RTC_LOG(INFO) << "["<< i <<"]:"<< "PlayoutDeviceName name : " << name << " guid : " << guid << " ret : " << ret;
	}

	// Playout device.
	{
		int32_t ret = pADM->SetPlayoutDevice(0);
		if (ret != 0) {
			RTC_LOG(LS_ERROR) << "SetPlayoutDevice ret = " << ret;
			return ret;
		}

		ret = pADM->InitSpeaker();
		if (ret != 0) {
			RTC_LOG(LS_ERROR) << "InitSpeaker ret = " << ret;
			return ret;
		}


		// Set number of channels
		bool available = false;
		if (pADM->StereoPlayoutIsAvailable(&available) != 0) {
			RTC_LOG(LS_ERROR) << "Failed to query stereo playout.";
		}
		if (pADM->SetStereoPlayout(available) != 0) {
			RTC_LOG(LS_ERROR) << "Failed to set stereo playout mode.";
		}
	}
	// Recording device.
	{
		int32_t ret = pADM->SetRecordingDevice(0);
		if (ret != 0) {
			RTC_LOG(LS_ERROR) << "SetRecordingDevice ret = " << ret;
			return ret;
		}

		ret = pADM->InitMicrophone();
		if (ret != 0) {
			RTC_LOG(LS_ERROR) << "InitMicrophone ret = " << ret;
			return ret;
		}
		// Set number of channels
		bool available = false;
		// Set number of channels
		if (pADM->StereoRecordingIsAvailable(&available) != 0) {
			RTC_LOG(LS_ERROR) << "Failed to query stereo recording.";
		}
		if (pADM->SetStereoRecording(available) != 0) {
			RTC_LOG(LS_ERROR) << "Failed to set stereo recording mode.";
		}
	}

	rtc::scoped_refptr<webrtc::AudioMixerImpl> pAMI = webrtc::AudioMixerImpl::Create();
	rtc::scoped_refptr<webrtc::AudioProcessing> pAPI = webrtc::AudioProcessingBuilder().Create();

	std::shared_ptr<AudioTransportCallback> cb(new AudioTransportCallback(pAMI.get(), pAPI.get()));

	pADM->RegisterAudioCallback(cb.get());

	uint32_t ret = 0; 
	// 需要初始化才能recording
	ret = pADM->InitRecording(); 
	if (ret != 0) {
		RTC_LOG(LS_ERROR) << "InitRecording ret = " << ret;
		return ret;
	}
	ret = pADM->InitPlayout();
	if (ret != 0) {
		RTC_LOG(LS_ERROR) << "InitPlayout ret = " << ret;
		return ret;
	}
	ret = pADM->StartRecording();
	if (ret != 0) {
		RTC_LOG(LS_ERROR) << "StartRecording ret = " << ret;
		return ret;
	}

	ret = pADM->StartPlayout();
	if (ret != 0) {
		RTC_LOG(LS_ERROR) << "StartPlayout ret = " << ret;
		return ret;
	}

	system("pause");
	pADM->StopRecording();
	pADM->StopPlayout();
	pADM->Terminate();
    return 0;
}

