#include "mbed.h"
#include "SdUsbConnect.h"
#include "EasyPlayback.h"
#include "EasyDec_WavCnv2ch.h"
#include "EasyDec_Mov.h"
#if MBED_CONF_APP_LCD
#include "EasyAttach_CameraAndLCD.h"
#include "JPEG_Converter.h"
#include "dcache-control.h"

#define FRAME_BUFFER_STRIDE    (((LCD_PIXEL_WIDTH * 2) + 31u) & ~31u)
#define FRAME_BUFFER_HEIGHT    (LCD_PIXEL_HEIGHT)

#if defined(__ICCARM__)
#pragma data_alignment=32
static uint8_t user_frame_buffer[2][FRAME_BUFFER_STRIDE * FRAME_BUFFER_HEIGHT];
#pragma data_alignment=32
static uint8_t JpegBuffer[1024 * 64]@ ".mirrorram";
#else
static uint8_t user_frame_buffer[2][FRAME_BUFFER_STRIDE * FRAME_BUFFER_HEIGHT]__attribute((aligned(32)));
static uint8_t JpegBuffer[1024 * 64]__attribute((section("NC_BSS"),aligned(32)));
#endif
static uint32_t frame_toggle = 0;
static DisplayBase Display;
static JPEG_Converter  Jcu;

static void JcuDecodeCallBackFunc(JPEG_Converter::jpeg_conv_error_t err_code) {
    Display.Graphics_Read_Change(DisplayBase::GRAPHICS_LAYER_0,
        (void *)user_frame_buffer[frame_toggle]);
    frame_toggle ^= 1;
}

static void MovCallBackFunc(void) {
    JPEG_Converter::bitmap_buff_info_t bitmap_buff_info;
    JPEG_Converter::decode_options_t   decode_options;

    bitmap_buff_info.width              = LCD_PIXEL_WIDTH;
    bitmap_buff_info.height             = LCD_PIXEL_HEIGHT;
    bitmap_buff_info.format             = JPEG_Converter::WR_RD_YCbCr422;
    bitmap_buff_info.buffer_address     = (void *)user_frame_buffer[frame_toggle];
    dcache_invalid(bitmap_buff_info.buffer_address, sizeof(user_frame_buffer[0]));

    decode_options.output_swapsetting   = JPEG_Converter::WR_RD_WRSWA_32_16_8BIT;
    decode_options.p_DecodeCallBackFunc = &JcuDecodeCallBackFunc;

    Jcu.decode((void *)JpegBuffer, &bitmap_buff_info, &decode_options);
}

static void clear_display(void) {
    for (uint32_t j = 0; j < sizeof(user_frame_buffer[0]); j += 2) {
        user_frame_buffer[frame_toggle][j + 0] = 0x00;
        user_frame_buffer[frame_toggle][j + 1] = 0x80;
    }
    dcache_clean(user_frame_buffer, sizeof(user_frame_buffer));
    Display.Graphics_Read_Change(DisplayBase::GRAPHICS_LAYER_0,
        (void *)user_frame_buffer[frame_toggle]);
    frame_toggle ^= 1;
}

static void Start_LCD_Display(void) {
    DisplayBase::rect_t rect;

    // Initialize the background to black
    for (uint32_t i = 0; i < 2; i++) {
        for (uint32_t j = 0; j < sizeof(user_frame_buffer[0]); j += 2) {
            user_frame_buffer[i][j + 0] = 0x00;
            user_frame_buffer[i][j + 1] = 0x80;
        }
    }
    dcache_clean(user_frame_buffer, sizeof(user_frame_buffer));

    rect.vs = 0;
    rect.vw = LCD_PIXEL_HEIGHT;
    rect.hs = 0;
    rect.hw = LCD_PIXEL_WIDTH;
    Display.Graphics_Read_Setting(
        DisplayBase::GRAPHICS_LAYER_0,
        (void *)user_frame_buffer[frame_toggle],
        FRAME_BUFFER_STRIDE,
        DisplayBase::GRAPHICS_FORMAT_YCBCR422,
        DisplayBase::WR_RD_WRSWA_32_16BIT,
        &rect
    );
    Display.Graphics_Start(DisplayBase::GRAPHICS_LAYER_0);
    frame_toggle ^= 1;

    ThisThread::sleep_for(50);
    EasyAttach_LcdBacklight(true);
}
#endif // MBED_CONF_APP_LCD

#define FILE_NAME_LEN          (64)
#define MOUNT_NAME             "storage"

static InterruptIn skip_btn(USER_BUTTON0);
static EasyPlayback AudioPlayer;

static void skip_btn_fall(void) {
    AudioPlayer.skip();
}

int main() {
    DIR  * d;
    struct dirent * p;
    char file_path[sizeof("/"MOUNT_NAME"/") + FILE_NAME_LEN];
    SdUsbConnect storage(MOUNT_NAME);

#if MBED_CONF_APP_LCD
    EasyAttach_Init(Display);
    Start_LCD_Display();
    EasyDec_Mov::attach(&MovCallBackFunc, JpegBuffer, sizeof(JpegBuffer));
#endif

    // decoder setting
    AudioPlayer.add_decoder<EasyDec_WavCnv2ch>(".wav");
    AudioPlayer.add_decoder<EasyDec_WavCnv2ch>(".WAV");
    AudioPlayer.add_decoder<EasyDec_Mov>(".mov");
    AudioPlayer.add_decoder<EasyDec_Mov>(".MOV");

    // volume control
    AudioPlayer.outputVolume(0.5);  // Volume control (min:0.0 max:1.0)

    // button setting
    skip_btn.fall(&skip_btn_fall);

    while (1) {
        storage.wait_connect();
        // file search
        d = opendir("/"MOUNT_NAME"/");
        while ((p = readdir(d)) != NULL) {
            size_t len = strlen(p->d_name);
            if (len < FILE_NAME_LEN) {
                // make file path
                sprintf(file_path, "/%s/%s", MOUNT_NAME, p->d_name);
                printf("%s\r\n", file_path);

                // playback
                AudioPlayer.play(file_path);
#if MBED_CONF_APP_LCD
                clear_display();
#endif
            }
        }
        closedir(d);
    }
}
