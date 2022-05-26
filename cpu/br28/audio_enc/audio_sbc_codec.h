#ifndef _AUDIO_SBC_CODEC_H_
#define _AUDIO_SBC_CODEC_H_

#include "generic/typedef.h"
#include "generic/list.h"
#include "media/audio_base.h"
#include "media/audio_stream.h"

struct sbc_emitter_hdl {
    struct audio_stream_entry entry;	// 音频流入口
};
extern struct sbc_emitter_hdl sbc_emitter;

extern void audio_sbc_enc_init(void);
extern int audio_sbc_enc_is_work(void);
extern int audio_sbc_enc_write(s16 *data, int len);
extern int audio_sbc_enc_get_rate(void);
extern int audio_sbc_enc_get_channel_num(void);
extern void bt_emitter_set_vol(u8 vol);
extern int audio_sbc_enc_reset_buf(u8 flag);
extern int audio_sbc_enc_write_alloc(s16 **buf, u32 *len);
extern int audio_sbc_enc_write_updata(u32 len);
extern void audio_sbc_enc_resume(void);

extern void audio_sbc_emitter_init(void);

extern void audio_sbc_enc_data_start_pre(void);
extern void audio_sbc_enc_data_stop_pre(void);

#endif

