#ifndef SDLHELPER_H
#define SDLHELPER_H

#include <SDL.h>

struct audio_chunk {
    struct audio_chunk *next;
    void *data;
    unsigned size;
    int loop;
};

struct audio_channel {
    unsigned                volume;
    unsigned                mute;
    struct audio_chunk     *chunk;           // played chunk
    struct audio_chunk    **chunk_tailp;     // tail of chunk list?
    unsigned                offset;          // data offset in chunk
};


int IsChannelMute(int channel);
void NUpdateVoice(void);
void av_step(void);
void av_silence(int channel);
void av_need_update(SDL_Rect *r);
void av_need_update_xy(int x1, int y1, int x2, int y2);
void MuteChannel(int channel, int mute);
char AnimSoundCheck(void);
void av_block(void);
void UpdateAudio(void);
void av_set_fading(int type, int from, int to, int steps, int preserve);
void av_sync(void);
void av_setup(void);
void play(struct audio_chunk *cp, int channel);

extern int screen_dirty;
extern int av_mouse_cur_x;
extern int av_mouse_cur_y;
extern int av_mouse_pressed_x;
extern int av_mouse_pressed_y;
extern int av_mouse_pressed_cur;
extern int av_mouse_pressed_latched;

#define AV_NUM_CHANNELS     2
#define AV_ALL_CHANNELS     -1
#define AV_SOUND_CHANNEL    0
#define AV_MUSIC_CHANNEL    1
#define AV_MAX_VOLUME       SDL_MIX_MAXVOLUME

#define AV_FADE_IN          0
#define AV_FADE_OUT         1

/*
 * 5 => 10x6 px rectangles
 * 4 => 20x12 ... etc
 * minimal = 1
 */
#define AV_DTREE_DEPTH       5
#define AV_DTREE_FILL_RATIO .8 // Whole rectangle will be marked as dirty if fill area is higher than this


#endif // SDLHELPER_H