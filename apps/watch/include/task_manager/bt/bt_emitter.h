#ifndef _BT_EMITTER_H
#define _BT_EMITTER_H


extern void bt_search_device(void);
extern void emitter_search_noname(u8 status, u8 *addr, u8 *name);
extern u8 bt_emitter_stu_set(u8 *addr, u8 on);
extern u8 bt_emitter_stu_get(void);
extern int bt_emitter_mic_open(void);
extern void bt_emitter_mic_close(void);
extern void emitter_media_source(u8 *addr, u8 source, u8 en);
extern u8 emitter_search_result(char *name, u8 name_len, u8 *addr, u32 dev_class, char rssi);
extern void emitter_search_stop(u8 result);
extern void emitter_media_source(u8 *addr, u8 source, u8 en);
extern void bt_emitter_receiver_sw();
extern u8 bt_emitter_pp(u8 pp);
extern void emitter_or_receiver_switch(u8 flag);
extern u8 bt_emitter_role_get();
extern void bt_emitter_start_search_device();
extern void bt_emitter_stop_search_device();
extern void emitter_bt_connect(u8 *mac);
extern void emitter_open(u8 source);
extern void emitter_close(u8 source);
extern void bt_emitter_rx_bulk_change(u8 mode);
extern void bt_emitter_audio_set_mute(u8 mute);

extern void emitter_save_remote_name(u8 *addr, u8 *name);
extern u8 *emitter_get_remote_name(u8 *addr);
extern void emitter_search_remote(void *cb_priv, int (*cb)(void *priv, u8 *mac, u8 *name, u8 *out_del, u8 *out_exit));
extern void emitter_delete_remote_name(u8 *addr);
extern void emitter_delete_remote_all(void);

#endif
