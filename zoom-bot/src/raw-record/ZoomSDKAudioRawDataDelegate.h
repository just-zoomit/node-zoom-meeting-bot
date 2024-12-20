
#ifndef MEETING_SDK_LINUX_SAMPLE_ZOOMSDKAUDIORAWDATADELEGATE_H
#define MEETING_SDK_LINUX_SAMPLE_ZOOMSDKAUDIORAWDATADELEGATE_H

#include <iostream>
#include <fstream>
#include <sstream>
#include "zoom_sdk_raw_data_def.h"
#include "rawdata/rawdata_audio_helper_interface.h"
#include <functional>

#include "../util/Log.h"

using namespace std;
using namespace ZOOMSDK;

class ZoomSDKAudioRawDataDelegate : public IZoomSDKAudioRawDataDelegate {
    string m_dir = "out";
    string m_filename = "test.pcm";
    bool m_useMixedAudio;

    void writeToFile(const string& path, AudioRawData* data);
public:
    using RawAudioRecordingCallback = std::function<void(AudioRawData*, uint32_t)>;

    ZoomSDKAudioRawDataDelegate(bool useMixedAudio, RawAudioRecordingCallback m_audio_callback);

    void setDir(const string& dir);
    void setFilename(const string& filename);

    void onMixedAudioRawDataReceived(AudioRawData* data) override;
    void onOneWayAudioRawDataReceived(AudioRawData* data, uint32_t node_id) override;
    void onShareAudioRawDataReceived(AudioRawData* data) override;
private:
    RawAudioRecordingCallback m_audio_callback;
};


#endif //MEETING_SDK_LINUX_SAMPLE_ZOOMSDKAUDIORAWDATADELEGATE_H
