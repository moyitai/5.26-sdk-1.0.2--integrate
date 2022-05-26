#include "ui/ui.h"
#include "app_config.h"
#include "ui/ui_api.h"
#include "system/timer.h"
#include "key_event_deal.h"
#include "audio_config.h"
#include "jiffies.h"
#include "app_power_manage.h"
#include "asm/charge.h"
#include "audio_dec_file.h"
#include "music/music_player.h"
#include "music/music.h"
#include "app_task.h"
#ifndef CONFIG_MEDIA_NEW_ENABLE
#include "application/audio_eq_drc_apply.h"
#else
#include "audio_eq.h"
#endif

#define LOG_TAG_CONST       UI
#define LOG_TAG     		"[UI-ACTION]"
#define LOG_ERROR_ENABLE
#define LOG_DEBUG_ENABLE
#define LOG_INFO_ENABLE
/* #define LOG_DUMP_ENABLE */
#define LOG_CLI_ENABLE
#include "debug.h"



#if (!TCFG_LUA_ENABLE)

#if TCFG_UI_ENABLE_FILE

#if TCFG_SPI_LCD_ENABLE
#define STYLE_NAME  JL//必须要



#define BS_DIR_TYPE_FORLDER   	0
#define BS_DIR_TYPE_FILE   		1


#define abs(x)  ((x)>0?(x):-(x))


struct grid_set_info {
    int flist_index;  //文件列表首项所指的索引
    int cur_total;
    FILE *file;
    struct vfscan *fs;
    FS_DIR_INFO *dir_buf;
    int    show_temp;
#if (TCFG_LFN_EN)
    u8  lfn_buf[512];
#endif//TCFG_LFN_EN
};

static struct grid_set_info *ghandler = NULL;
#define __this 	(ghandler)
#define sizeof_this     (sizeof(struct grid_set_info))

static int close_file_handler();
static int brow_children_redraw(int id);
extern int ui_grid_dynamic_set_prepare(struct ui_grid *grid, int (*handler_prepare_cb)(void *, int, int, int));


static int handler_prepare_cb(void *ctrl, int count, int start, int end)
{

    FILE *f = NULL;
    struct vfs_attr attr;

    if (!__this || !__this->file) {
        return 0;
    }

    if (!start  && (1 == __this->flist_index)) {
        //针对start 是 0情况判断是否需要更新buf，减小重刷
        return 0;
    }

    start = !start + start;//针对0的情况

    if (start > __this->cur_total) {
        return 0;
    }

    fget_dir_info(__this->file, start, __this->show_temp, __this->dir_buf);
    __this->flist_index = start;//记录索引
#if 0
    int offset = start % __this->show_temp;
    for (int i = 0 ; i < __this->show_temp; i++) {
        __this->dir_buf[i].fn_type = 0;
        sprintf(__this->dir_buf[i].lfn_buf.lfn, "%d", start + 1 + i);
        __this->dir_buf[i].lfn_buf.lfn_cnt = strlen(__this->dir_buf[i].lfn_buf.lfn);
    }
#endif

    return 0;
}



static FS_DIR_INFO *file_list_read_by_index(u32 index)
{
    index = index - 1;//index 是 无符号 ,0 -1 会变成-1
    if (!__this || !__this->file) {
        return NULL;
    }
    if (index >= __this->cur_total) {
        return NULL;
    }
    index ++;
    index = index - __this->flist_index;
    int i = index % __this->show_temp;
    return &__this->dir_buf[i];
}

static int open_file_handler(int show_temp)
{

    close_file_handler();
    if (!dev_manager_get_total(1)) {
        return -1;
    }
    void *dev = dev_manager_find_active(1);
    if (!dev) {
        return -1;
    }
    __this = zalloc(sizeof_this);
    __this->show_temp = show_temp + !show_temp;
    __this->dir_buf = zalloc(sizeof(FS_DIR_INFO) *  __this->show_temp);

    fset_ext_type(dev_manager_get_root_path(dev), "MP1MP2MP3");
    fopen_dir_info(dev_manager_get_root_path(dev), &__this->file, 0);

    if (!__this->file) {
        return 0;
    }

    __this->cur_total = fenter_dir_info(__this->file, __this->dir_buf);

    fget_dir_info(__this->file, 1, __this->show_temp, __this->dir_buf);
    __this->flist_index = 1;//记录索引
    if (!__this->cur_total) {
        close_file_handler();
    }
    return 0;

}

static int close_file_handler()
{

    if (!__this) {
        return -1;
    }

    if (__this->fs) {
        fscan_release(__this->fs);
        __this->fs = NULL;
    }

    if (__this->file) {
        fclose(__this->file);
        __this->file = NULL;
    }

    if (__this->dir_buf) {
        free(__this->dir_buf);
        __this->dir_buf = NULL;
    }

    free(__this);
    __this = NULL;
    return 0;
}







static int file_select_enter(u32 index)
{
    FS_DIR_INFO *info = file_list_read_by_index(index);
    if (!info) {
        return -1;
    }

    if (!__this || !__this->file) {
        return -1;
    }

    if (info->dir_type == BS_DIR_TYPE_FORLDER) {
        __this->cur_total = fenter_dir_info(__this->file, info); //使用open获得的file，无需重新申请。
        fget_dir_info(__this->file, 1, __this->show_temp, __this->dir_buf);
        __this->flist_index = 1;//记录索引
    } else {
        if (app_get_curr_task() != APP_MUSIC_TASK) {
            music_task_set_parm(MUSIC_TASK_START_BY_SCLUST, info->sclust);
            /* dev_manager_set_active_by_logo(dev_logo); */
            music_set_start_auto_play(1);
            app_task_switch_to(APP_MUSIC_TASK);
        } else {
            app_task_put_key_msg(KEY_MUSIC_PLAYE_BY_DEV_SCLUST, info->sclust);
        }
        UI_HIDE_CURR_WINDOW();
        UI_SHOW_WINDOW(ID_WINDOW_MUSIC);
        return 0;
    }
    ui_grid_dynamic_set_item_by_id(VERTLIST_3, __this->cur_total + 1);
    brow_children_redraw(VERTLIST_3);//重刷本页面
    return 0;
}

static int grid_child_cb(void *_ctrl, int id, int type, int index)
{
    FS_DIR_INFO *info = file_list_read_by_index(index);
    switch (type) {
    case CTRL_TYPE_PROGRESS:
        break;
    case CTRL_TYPE_MULTIPROGRESS:
        break;
    case CTRL_TYPE_TEXT:
        struct ui_text *text = (struct ui_text *)_ctrl;
        if (!strcmp(text->source, "title")) {
            text->elm.css.invisible = !!index;
            break;
        }
        if (!index) {
            text->elm.css.invisible = !index;
            break;
        }
        if (!info) {
            return 0;
        }
        if (!strcmp(text->source, "name")) {
            text->attrs.offset = 0;
            text->attrs.format = "text";
            /* text->attrs.endian = FONT_ENDIAN_SMALL; */
            text->attrs.flags  = FONT_DEFAULT;

            /* log_info("#########cur index = %d,%s,%x\n",index,info->lfn_buf.lfn,info->lfn_buf.lfn); */
            if (info->fn_type) {
                text->attrs.endian = FONT_ENDIAN_SMALL;
                text->attrs.encode = FONT_ENCODE_UNICODE;
                text->attrs.str    = info->lfn_buf.lfn;
                text->attrs.strlen = info->lfn_buf.lfn_cnt;
            } else {
                /* put_buf(info->fname,info->len); */
                /* log_info(">>>>cur index = %d\n",index); */
                text->attrs.endian = 0;//FONT_ENDIAN_SMALL;
                text->attrs.encode = FONT_ENCODE_ANSI;
                text->attrs.str    = info->lfn_buf.lfn;
                text->attrs.strlen = strlen(text->attrs.str);
                if (text->attrs.strlen > 11) { //兼容文件系统对文件名支持支持问题
                    info->lfn_buf.lfn[12] = 0;
                    text->attrs.strlen = 11;
                }
            }
            text->elm.css.invisible = 0;
        }
        break;
    case CTRL_TYPE_NUMBER:
        struct ui_number *number = (struct ui_number *)_ctrl;
        break;
    case CTRL_TYPE_PIC:
        struct ui_pic *pic = (struct ui_pic *)_ctrl;

        if (!index) {
            pic->elm.css.invisible = !index;
            break;
        }

        if (!info) {
            return 0;
        }

        ui_pic_set_image_index(pic, !!(info->dir_type == BS_DIR_TYPE_FORLDER));
        pic->elm.css.invisible = 0;
        break;
    case CTRL_TYPE_TIME:
        break;
    }
    return 0;
}


static int file_switch_ontouch(void *ctr, struct element_touch_event *e)
{
    struct ui_grid *grid = (struct ui_grid *)ctr;
    int sel_item;
    static u8 move_flag = 0;
    switch (e->event) {
    case ELM_EVENT_TOUCH_R_MOVE:
    /* log_info("line:%d", __LINE__); */
    /* app_task_put_key_msg(KEY_CHANGE_PAGE, 1); */
    /* break; */
    case ELM_EVENT_TOUCH_L_MOVE:
        log_info("line:%d", __LINE__);
        /* app_task_put_key_msg(KEY_CHANGE_PAGE, 0); */
        int count = 0;
        if (__this && __this->file) {
            count = fexit_dir_info(__this->file);
            __this->cur_total = count;
        }
        if (!count) {
            UI_HIDE_CURR_WINDOW();
            UI_SHOW_WINDOW(ID_WINDOW_MUSIC);
            return TRUE;
        }
        fget_dir_info(__this->file, 1, __this->show_temp, __this->dir_buf);
        __this->flist_index = 1;//记录索引
        ui_grid_dynamic_set_item_by_id(VERTLIST_3, __this->cur_total + 1);
        brow_children_redraw(VERTLIST_3);//重刷本页面
        break;
    case ELM_EVENT_TOUCH_MOVE:
        int x = e->pos.x - grid->pos.x;
        int y = e->pos.y - grid->pos.y;
        /* log_info("%d %d x=%d,y=%d\n",e->pos.x,e->pos.y,grid->pos.x,grid->pos.y); */
#if 0
        static int last_y = 0;
        if (abs(last_y - y) > 100) {
            grid->pos.x = e->pos.x;
            grid->pos.y = e->pos.y;
            last_y = y;
            log_info("event_touch_up_lose....\n");
            break;
        }
#endif
        if (ui_grid_dynamic_slide(grid, 1, y)) {
            grid->pos.x = e->pos.x;
            grid->pos.y = e->pos.y;
        }
        move_flag = 1;
        break;
    case ELM_EVENT_TOUCH_DOWN:
        move_flag = 0;
        return false;//不接管消息
        break;
    case ELM_EVENT_TOUCH_UP:
        if (move_flag) {
            move_flag = 0;
            return false;//不接管消息
        }

        sel_item = ui_grid_dynamic_cur_item(grid);

        if (!sel_item) {
            return false;    //不接管消息
        }

        file_select_enter(sel_item);
        return false;//不接管消息

        break;
    default:
        return false;
        break;
    }
    return true;//接管消息
}



static int brows_children_init(struct ui_grid *grid)
{
    struct element *k;
    int count =  1;//
    if (!grid) {
        return 0;
    }

    if (__this) {
        count += __this->cur_total;
    }

    for (int i = 0; i < grid->avail_item_num; i++) {
        if (i < count) {
            list_for_each_child_element(k, &grid->item[i].elm) {
                grid_child_cb(k, k->id, ui_id2type(k->id), i);
            }
            grid->item[i].elm.css.invisible = 0;
        } else {
            grid->item[i].elm.css.invisible = 1;
        }
    }
    return 0;
}





static int brow_children_redraw(int id)
{
    struct ui_grid *grid = (struct ui_grid *)ui_core_get_element_by_id(id);
    if (!grid) {
        return 0;
    }
    ui_grid_dynamic_reset(grid, 0);
    brows_children_init(grid);
    ui_core_redraw(&grid->elm);
    return 0;
}


static int file_browse_enter_onchane(void *ctr, enum element_change_event e, void *arg)
{
    struct ui_grid *grid = (struct ui_grid *)ctr;
    int fnum = 0;

    switch (e) {
    case ON_CHANGE_INIT:
        open_file_handler(grid->avail_item_num);
        if (__this) {
            ui_grid_dynamic_create(grid, 1, __this->cur_total + 1, grid_child_cb);
            ui_grid_dynamic_set_prepare(grid, handler_prepare_cb);
        }
        break;
    case ON_CHANGE_RELEASE:
        close_file_handler();
        ui_grid_dynamic_release(grid);
        break;
    case ON_CHANGE_FIRST_SHOW:
        brows_children_init(grid);
        break;
    default:
        return false;
    }
    return false;
}


REGISTER_UI_EVENT_HANDLER(VERTLIST_3)
.onchange = file_browse_enter_onchane,
 .onkey = NULL,
  .ontouch = file_switch_ontouch,
};







#endif
#endif /* #if TCFG_APP_MUSIC_EN */
#endif /* #if (!TCFG_LUA_ENABLE) */

