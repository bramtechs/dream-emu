#include "magma.h"
#include "external/pl_mpeg.h"

#define MAX_VIDEOS 12
static plm_t* Mpegs[MAX_VIDEOS];

static void ReceivedVideoFrame(plm_t* mpeg, plm_frame_t* frame, void* user) {
    uint8_t* pixels = (uint8_t*)user;
    assert(pixels);

    size_t widthPixels = frame->width * 3;
    plm_frame_to_rgb(frame, pixels, widthPixels);
}

static VideoID RetrieveVideoID() {
    for (int i = 0; i < MAX_VIDEOS; i++) {
        if (!Mpegs[i]) {
            return i;
        }
    }
    ERROR("Ran out of video slots!");
    panic();
}

static void InitVideo(Video& video) {
    plm_t* mpeg = Mpegs[video.id];
    assert(mpeg);

    // allocate memory
    video.frameData = new uint8_t[plm_get_width(mpeg) * plm_get_height(mpeg) * 3];

    // link methods
    plm_set_video_decode_callback(mpeg, ReceivedVideoFrame, (void*)video.frameData);

    // we don't use the audio from the video file itself because
    // I'm too dumb to figure out audio
    plm_set_audio_enabled(mpeg, false);
}

Video LoadVideo(const char* fileName) {
    Video video = {};

    video.id = RetrieveVideoID();
    Mpegs[video.id] = plm_create_with_filename(fileName);

    video.audio = LoadMusicStream(fileName);
    video.timeScale = 1.f;
    if (!video.audio.ctxData) {
        ERROR("Could not audio audio!");
    }
    InitVideo(video);
    return video;
}

Video LoadVideoWithAudio(const char* fileName, const char* audioFileName) {
    Video video = {};

    video.id = RetrieveVideoID();
    Mpegs[video.id] = plm_create_with_filename(fileName);

    video.audio = LoadMusicStream(fileName);
    video.timeScale = 1.f;
    if (!video.audio.ctxData) {
        ERROR("Could not audio audio!");
    }
    InitVideo(video);
    return video;
}

Video LoadVideoFromMemory(uint8_t* bytes, size_t length) {
    Video video = {};

    video.id = RetrieveVideoID();
    Mpegs[video.id] = plm_create_with_memory(bytes,length,true);

    video.timeScale = 1.f;
    InitVideo(video);
    return video;
}

Video LoadVideoWithAudioFromMemory(uint8_t* bytes, size_t length, Music audio) {
    Video video = LoadVideoFromMemory(bytes, length);
    video.audio = audio;
    return video;
}

void UnloadVideo(Video video, bool includeAudio) {
    plm_destroy(Mpegs[video.id]);
    Mpegs[video.id] = NULL;
    delete video.frameData;

    if (includeAudio) {
        UnloadMusicStream(video.audio);
    }
}

bool VideoHasAudio(Video video) {
    return video.audio.ctxData;
}

Image GetVideoFrame(Video video) {
    auto mpeg = Mpegs[video.id];
    assert(mpeg);
    assert(video.frameData);

    Image image;
    image.data = video.frameData;
    image.width = plm_get_width(mpeg);
    image.height = plm_get_height(mpeg);
    image.mipmaps = 1;
    image.format = PIXELFORMAT_UNCOMPRESSED_R8G8B8;

    return image;
}

Texture GetVideoFrameTexture(Video video) {
    // prevent spamming of logs
    SetTraceLogLevel(LOG_NONE);
    Image frame = GetVideoFrame(video);
    Texture frameTexture = LoadTextureFromImage(frame);
    // do not dispose frame here!
    SetTraceLogLevel(LOG_DEBUG); // TODO: Implement GetTraceLogLevel()
    return frameTexture;
}

void DrawVideoFrameRec(Video video, Rectangle dest, Color tint) {
    Texture texture = GetVideoFrameTexture(video);
    Rectangle source = {
        0.f, 0.f,
        (float)texture.width, (float)texture.height,
    };
    DrawTexturePro(texture, source, dest, { 0.f, 0.f }, 0.f, tint);
    //UnloadTexture(texture);
}

void DrawVideoFrameV(Video video, Vector2 pos, Color tint) {
    auto mpeg = Mpegs[video.id];
    assert(mpeg);

    Rectangle dest = {
        pos.x,
        pos.y,
        (float)plm_get_width(mpeg),
        (float)plm_get_height(mpeg)
    };
    DrawVideoFrameRec(video, dest, tint);
}

void DrawVideoFrame(Video video, int posX, int posY, Color tint) {
    DrawVideoFrameV(video, { (float) posX, (float) posY }, tint);
}

void PlayAndDrawVideoRec(Video video, Rectangle dest, Color tint) {
    AdvanceVideo(video, GetFrameTime());
    DrawVideoFrameRec(video, dest, tint);
}

void PlayAndDrawVideoV(Video video, Vector2 pos, Color tint) {
    AdvanceVideo(video, GetFrameTime());
    DrawVideoFrameV(video, pos, tint);
    PlayVideoAudio(video);
}

void PlayVideoAudio(Video video) {
    // play audio
    if (VideoHasAudio(video)) {
        if (GetMusicTimePlayed(video.audio) < GetMusicTimeLength(video.audio)) {
            if (!IsMusicStreamPlaying(video.audio)) {
                PlayMusicStream(video.audio);
            }
            UpdateMusicStream(video.audio);
            SetMusicPitch(video.audio, video.timeScale);
        }
        else {
            StopMusicStream(video.audio);
        }
    }
}

float GetVideoFrameRate(Video video) {
    plm_t* mpeg = Mpegs[video.id];
    assert(mpeg);
    return (float)plm_get_framerate(mpeg);
}

void AdvanceVideo(Video video, float delta) {
    plm_t* mpeg = Mpegs[video.id];
    assert(mpeg);
    plm_decode(mpeg, delta * video.timeScale);
}