#include <iostream>
#include <base/media_util.h>
#include <fstream>

#include "mp4/mp4_stream_parser.h"
#include "mp4/es_descriptor.h"
#include "base/bind.h"
#include "base/text_track_config.h"
#include "base/media_tracks.h"

bool OnNewConfigs(std::unique_ptr<media::MediaTracks> tracks,
                  const media::StreamParser::TextTrackConfigMap& text_configs)
{
    std::cout << "OnNewConfigs: " << std::endl;
    std::cout << "\ttrack_size: " << tracks->tracks().size() <<std::endl;
    for(const auto& track : tracks->tracks())
    {
        std::cout << "\tkind: " << track->kind() << ", id: " << track->id() << ", label: " << track->label() << ", lang: " << track->language()<<std::endl;
    }

    return true;
}

void OnNewMediaSegment()
{
    //std::cout << "OnNewMediaSegment" << std::endl;
}

void OnEndOfMediaSegment()
{
    //std::cout << "OnEndOfMediaSegment" << std::endl;
}

int64_t sample_count = 0;

bool OnNewBuffers(const media::StreamParser::BufferQueueMap& buffer_queue_map)
{
    std::cout << "OnNewBuffers" << std::endl;
    for(const auto& elem : buffer_queue_map)
    {
        std::cout << "\tid: " << elem.first << ", size: " << elem.second.size() << std::endl;
        sample_count+=elem.second.size();
    }

    return true;
}

void OnEncryptedMediaInitData(media::EmeInitDataType type, const std::vector<uint8_t>& init_data)
{
    std::cout << "OnEncryptedMediaInitData: type: " << static_cast<int>(type) << ", initdata_size:" << init_data.size() << std::endl;
}

void OnSourceInitDone(const media::StreamParser::InitParameters& params)
{
    std::cout << "OnSourceInitDone: " << std::endl;
    std::cout << "\tduration : " << params.duration << std::endl;
    std::cout << "\ttimeline_offset : " << params.timeline_offset << std::endl;
    std::cout << "\tliveness : " << params.liveness << std::endl;
    std::cout << "\tdetected_audio_track_count : " << params.detected_audio_track_count << std::endl;
    std::cout << "\tdetected_video_track_count : " << params.detected_video_track_count << std::endl;
    std::cout << "\tdetected_text_track_count : " << params.detected_text_track_count << std::endl;
}

std::vector<uint8_t> read_file(const std::string& file_path)
{
    std::ifstream instream(file_path, std::ios::in | std::ios::binary);
    std::vector<uint8_t> data((std::istreambuf_iterator<char>(instream)), std::istreambuf_iterator<char>());
    return data;
}


int main()
{
    std::set<int> audio_object_types;
    audio_object_types.insert(media::mp4::kAC3);
    audio_object_types.insert(media::mp4::kEAC3);
    media::mp4::MP4StreamParser parser(audio_object_types, false, false);

    media::NullMediaLog media_log;
    parser.Init(
        base::BindOnce(&OnSourceInitDone),
        base::BindRepeating(&OnNewConfigs),
        base::BindRepeating(&OnNewBuffers),
        false, // text tracks
        base::BindRepeating(&OnEncryptedMediaInitData),
        base::BindRepeating(&OnNewMediaSegment),
        base::BindRepeating(&OnEndOfMediaSegment),
        &media_log);

    auto data = read_file("/Users/ademiroz/Downloads/batman_movie_video_1.mp4");

    parser.Parse(data.data(), data.size());
//    size_t index = 0;
//    while(index < data.size())
//    {
//        parser.Parse(data.data()+index, std::min<size_t>(1000, data.size()-index));
//        index+=1000;
//    }


    std::cout << "Sample Count: " << sample_count << std::endl;
    return 0;
}
