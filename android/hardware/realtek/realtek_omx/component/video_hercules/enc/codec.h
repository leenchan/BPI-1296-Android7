#ifndef RTK_ENCODER_H
#define RTK_ENCODER_H

#ifdef __cplusplus
extern "C"
{
#endif

#include <OMX_Types.h>
#include <OMX_Core.h>
#include <OMX_Component.h>
#include <OSAL_RTK.h>

#define ADD_SPS_PPS_WORKAROUND
#define VIDEO_COMPONENT_NAME "OMX.realtek.video.encoder"
#define IMAGE_COMPONENT_NAME "OMX.realtek.image.encoder"
#define USE_ION_ALLOC_FB
#define DEBUG_ANDROIDOPAQUE
#define VIDEO_HW_ION_FLAG (ION_FLAG_NONCACHED |ION_FLAG_SCPUACC | ION_FLAG_HWIPACC)
#define VIDEO_ION_FLAG (ION_FLAG_SCPUACC | ION_FLAG_HWIPACC)

#define SUPPORT_SE
#if ! defined(__GENERIC_LINUX__)
#define USE_SE_SERVICE
#endif

// RTK encoder interface

    /**
     * Structure for encoded stream
     */
    typedef struct STREAM_BUFFER
    {
        OMX_U8* bus_data;           // set by common
        OSAL_BUS_WIDTH bus_address; // set by common
        OMX_U32 buf_max_size;       // set by common
        OMX_U32 streamlen;          // set by codec
#ifdef ADD_SPS_PPS_WORKAROUND
        int picType;             // set by encoder , I, P, B
#endif /* end of ADD_SPS_PPS_WORKAROUND */
    } STREAM_BUFFER;

    typedef enum FRAME_TYPE
    {
        INTRA_FRAME,
        PREDICTED_FRAME
    } FRAME_TYPE;

    /**
     * Structure for raw input frame
     */
    typedef struct FRAME
    {
        OMX_U8* fb_bus_data;    // set by common
        OMX_U32 fb_bus_address; // set by common
        OMX_U32 fb_frameSize;       // set by common
        OMX_U32 fb_bufferSize;  //set by common
        FRAME_TYPE frame_type;  // set by common
        OMX_BOOL bInterleave;
    } FRAME;

    typedef enum CODEC_STATE
    {
        CODEC_OK,
        CODEC_CODED_INTRA,
        CODEC_CODED_PREDICTED,
        CODEC_CODED_SLICE,
        CODEC_ERROR_HW_TIMEOUT = -1,
        CODEC_ERROR_HW_BUS_ERROR = -2,
        CODEC_ERROR_HW_RESET = -3,
        CODEC_ERROR_SYSTEM = -4,
        CODEC_ERROR_UNSPECIFIED = -5,
        CODEC_ERROR_RESERVED = -6,
        CODEC_ERROR_INVALID_ARGUMENT = -7,
        CODEC_ERROR_BUFFER_OVERFLOW = -8,
        CODEC_ERROR_INVALID_STATE = -9
    } CODEC_STATE;

    typedef enum CODEC_CONFIG_TYPE
    {
        CODEC_SET_GOP_NUMBER = 0,
        CODEC_SET_INTRA_QP = 1,
        CODEC_SET_BITRATE = 2,
        CODEC_SET_FRAME_RATE = 3,
        CODEC_SET_INTRA_MB_REFRESH_NUMBER = 4,
        CODEC_SET_SLICE_INFO = 5,
        CODEC_FORCE_IDR = 6,
        CODEC_SET_NV21 = 7
    } CODEC_CONFIG_TYPE;

    typedef struct ENCODER_PROTOTYPE ENCODER_PROTOTYPE;

// internal ENCODER interface, which wraps up RTK API
    struct ENCODER_PROTOTYPE
    {
        void (*destroy)( ENCODER_PROTOTYPE*);
        CODEC_STATE (*stream_start)( ENCODER_PROTOTYPE*, STREAM_BUFFER*, OMX_U32, OMX_U32);
        CODEC_STATE (*stream_end)( ENCODER_PROTOTYPE*, STREAM_BUFFER*);
        CODEC_STATE (*encode)( ENCODER_PROTOTYPE*, FRAME*, STREAM_BUFFER*);
        void (*set_config)(ENCODER_PROTOTYPE*, CODEC_CONFIG_TYPE, OMX_S32);
    };

// common configuration structure for encoder pre-processor
    typedef struct PRE_PROCESSOR_CONFIG
    {
        OMX_U32 origWidth;
        OMX_U32 origHeight;

        // picture cropping
        OMX_U32 xOffset;
        OMX_U32 yOffset;

        // color format conversion
        OMX_COLOR_FORMATTYPE formatType;

        // picture rotation
        OMX_S32 angle;

        // H264 frame stabilization
        OMX_BOOL frameStabilization;

    } PRE_PROCESSOR_CONFIG;

    typedef struct RATE_CONTROL_CONFIG
    {
        // If different than zero it will enable the VOP/picture based rate control. The QP will be
        // changed between VOPs/pictures. Enabled by default.
        //OMX_U32 vopRcEnabled;
        // Value range is [0, 1]
        OMX_U32 nPictureRcEnabled;

        // If different than zero it will enable the macroblock based rate control. The QP will
        // be adjusting also inside a VOP. Enabled by default.
        //OMX_U32 mbRcEnabled;
        // Value range is [0, 1]
        OMX_U32 nMbRcEnabled;

        // If different than zero then frame skipping is enabled, i.e. the rate control is allowed
        // to skip frames if needed. When VBV is enabled, the rate control may have to skip
        // frames despite of this value. Disabled by default.
        //OMX_BOOL bVopSkipEnabled;
        // Value range is [0, 1]
        //OMX_U32 nSkippingEnabled;

        // Valid value range: [1, 31]
        // The default quantization parameter used by the encoder. If the rate control is
        // enabled then this value is used just at the beginning of the encoding process.
        // When the rate control is disabled then this QP value is used all the time. Default
        // value is 10.
        OMX_U32 nQpDefault;

        // The minimum QP that can be used by the RC in the stream. Default value is 1.
        // Valid value range: [1, 31]
        OMX_U32 nQpMin;

        // The maximum QP that can be used by the RC in the stream. Default value is 31.
        // Valid value range: [1, 31]
        OMX_U32 nQpMax;

        // The target bit rate as bits per second (bps) when the rate control is enabled. The
        // rate control is considered enabled when any of the VOP or MB based rate control is
        // enabled. Default value is the minimum between the maximum bitrate allowed for
        // the selected profile&level and 4mbps.
        OMX_U32 nTargetBitrate;

        //OMX_U32 eControlRate;

        // If different than zero it will enable the VBV model. Value 1 represents the video
        // buffer size defined by the standard. Otherwise this value represents the video
        // buffer size in units of 16384 bits. Enabled by default with size set by standard.
        // NOTE! This should always be ON since turning it OFF may not be 100% standard compatible.
        // (See RTK MPEG4 API User Manual)
        OMX_U32 nVbvEnabled;

        OMX_VIDEO_CONTROLRATETYPE eRateControl;

        // Hypothetical Reference Decoder model, [0,1]
        // restricts the instantaneous bitrate and
        // total bit amount of every coded picture.
        // Enabling HRD will cause tight constrains
        // on the operation of the rate control.
        // NOTE! If HRD is OFF output stream may not be
        // 100% standard compatible. (see RTK API user guide)
        // NOTE! If HDR is used, bitrate can't be changed after stream
        // has been created.
        // u32 hrd;
        OMX_U32 nHrdEnabled;

        // size in bits of Coded Picture Buffer used in HRD
        // NOTE: check if OMX supports this?
        // NOTE! Do not change unless you really know what you are doing!
        // u32 hrdCpbSize;
        //OMX_U32 hrdCobSize;
    } RATE_CONTROL_CONFIG;

    typedef struct ENCODER_COMMON_CONFIG
    {
        OMX_U32 nOutputWidth;
        OMX_U32 nOutputHeight;

        // frameRateNum
        // frameRateDenom
        OMX_U32 nInputFramerate;    // framerate in Q16 format

    } ENCODER_COMMON_CONFIG;

    typedef struct ERROR_CORRECT_CONFIG {
        OMX_BOOL bEnableHEC;
        OMX_BOOL bEnableDataPartitioning;
        OMX_BOOL bEnableRVLC;
    } ERROR_CORRECT_CONFIG;

#ifdef __cplusplus
}
#endif
#endif // RTK_ENCODER_H

