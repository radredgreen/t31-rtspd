#include <string.h>    
#include <errno.h>      
#include <stdlib.h>

#include <imp/imp_isp.h>
#include <imp/imp_system.h>
#include <imp/imp_log.h>
#include <imp/imp_framesource.h>
#include <imp/imp_encoder.h>
#include <imp/imp_log.h>
#include <imp/imp_common.h>
#include <imp/imp_ivs.h>
#include <imp/imp_ivs_move.h>
#include "sample-common.h" 
#include "capture_and_encoding.h"

#define TAG "capture_and_encoding"

extern struct chn_conf chn[];
pthread_t ivs_tid;
IMPIVSInterface *inteface = NULL;
IMPCell ivs_cell = {DEV_ID_IVS, 0, 0};
IMPCell fs_for_ivs_cell = {DEV_ID_FS, 2, 2};

static int sample_ivs_move_init(int grp_num)
{
	int ret = 0;

	ret = IMP_IVS_CreateGroup(grp_num);
	if (ret < 0) {
		IMP_LOG_ERR(TAG, "IMP_IVS_CreateGroup(%d) failed\n", grp_num);
		return -1;
	}
	return 0;
}

static int sample_ivs_move_exit(int grp_num)
{
	int ret = 0;

	ret = IMP_IVS_DestroyGroup(grp_num);
	if (ret < 0) {
		IMP_LOG_ERR(TAG, "IMP_IVS_DestroyGroup(%d) failed\n", grp_num);
		return -1;
	}
	return 0;
}

static int sample_ivs_move_start(int grp_num, int chn_num, IMPIVSInterface **interface)
{
	int ret = 0;
	IMP_IVS_MoveParam param;
	int i = 0, j = 0;

	memset(&param, 0, sizeof(IMP_IVS_MoveParam));
	param.skipFrameCnt = 5;
	param.frameInfo.width = SENSOR_WIDTH_SECOND;
	param.frameInfo.height = SENSOR_HEIGHT_SECOND;
	param.roiRectCnt = 1;

	for(i=0; i<param.roiRectCnt; i++){
	  param.sense[i] = 4;
	}

	/* printf("param.sense=%d, param.skipFrameCnt=%d, param.frameInfo.width=%d, param.frameInfo.height=%d\n", param.sense, param.skipFrameCnt, param.frameInfo.width, param.frameInfo.height); */
	for (j = 0; j < 2; j++) {
		for (i = 0; i < 2; i++) {
		  if((i==0)&&(j==0)){
			param.roiRect[j * 2 + i].p0.x = i * param.frameInfo.width /* / 2 */;
			param.roiRect[j * 2 + i].p0.y = j * param.frameInfo.height /* / 2 */;
			param.roiRect[j * 2 + i].p1.x = (i + 1) * param.frameInfo.width /* / 2 */ - 1;
			param.roiRect[j * 2 + i].p1.y = (j + 1) * param.frameInfo.height /* / 2 */ - 1;
			printf("(%d,%d) = ((%d,%d)-(%d,%d))\n", i, j, param.roiRect[j * 2 + i].p0.x, param.roiRect[j * 2 + i].p0.y,param.roiRect[j * 2 + i].p1.x, param.roiRect[j * 2 + i].p1.y);
		  }
		  else
		    {
		      	param.roiRect[j * 2 + i].p0.x = param.roiRect[0].p0.x;
			param.roiRect[j * 2 + i].p0.y = param.roiRect[0].p0.y;
			param.roiRect[j * 2 + i].p1.x = param.roiRect[0].p1.x;;
			param.roiRect[j * 2 + i].p1.y = param.roiRect[0].p1.y;;
			printf("(%d,%d) = ((%d,%d)-(%d,%d))\n", i, j, param.roiRect[j * 2 + i].p0.x, param.roiRect[j * 2 + i].p0.y,param.roiRect[j * 2 + i].p1.x, param.roiRect[j * 2 + i].p1.y);
		    }
		}
	}
	*interface = IMP_IVS_CreateMoveInterface(&param);
	if (*interface == NULL) {
		IMP_LOG_ERR(TAG, "IMP_IVS_CreateGroup(%d) failed\n", grp_num);
		return -1;
	}

	ret = IMP_IVS_CreateChn(chn_num, *interface);
	if (ret < 0) {
		IMP_LOG_ERR(TAG, "IMP_IVS_CreateChn(%d) failed\n", chn_num);
		return -1;
	}

	ret = IMP_IVS_RegisterChn(grp_num, chn_num);
	if (ret < 0) {
		IMP_LOG_ERR(TAG, "IMP_IVS_RegisterChn(%d, %d) failed\n", grp_num, chn_num);
		return -1;
	}

	ret = IMP_IVS_StartRecvPic(chn_num);
	if (ret < 0) {
		IMP_LOG_ERR(TAG, "IMP_IVS_StartRecvPic(%d) failed\n", chn_num);
		return -1;
	}

	return 0;
}

static int sample_ivs_move_stop(int chn_num, IMPIVSInterface *interface)
{
	int ret = 0;

	ret = IMP_IVS_StopRecvPic(chn_num);
	if (ret < 0) {
		IMP_LOG_ERR(TAG, "IMP_IVS_StopRecvPic(%d) failed\n", chn_num);
		return -1;
	}
	sleep(1);

	ret = IMP_IVS_UnRegisterChn(chn_num);
	if (ret < 0) {
		IMP_LOG_ERR(TAG, "IMP_IVS_UnRegisterChn(%d) failed\n", chn_num);
		return -1;
	}

	ret = IMP_IVS_DestroyChn(chn_num);
	if (ret < 0) {
		IMP_LOG_ERR(TAG, "IMP_IVS_DestroyChn(%d) failed\n", chn_num);
		return -1;
	}

	IMP_IVS_DestroyMoveInterface(interface);

	return 0;
}

#if 0
static int sample_ivs_set_sense(int chn_num, int sensor)
{
	int ret = 0;
	IMP_IVS_MoveParam param;
	int i = 0;

	ret = IMP_IVS_GetParam(chn_num, &param);
	if (ret < 0) {
		IMP_LOG_ERR(TAG, "IMP_IVS_GetParam(%d) failed\n", chn_num);
		return -1;
	}

	for( i = 0 ; i < param.roiRectCnt ; i++){
	  param.sense[i] = sensor;
	}

	ret = IMP_IVS_SetParam(chn_num, &param);
	if (ret < 0) {
		IMP_LOG_ERR(TAG, "IMP_IVS_SetParam(%d) failed\n", chn_num);
		return -1;
	}

	return 0;
}
#endif

static void *sample_ivs_move_get_result_process(void *arg)
{
	int i = 0, ret = 0;
	int chn_num = (int)arg;
	IMP_IVS_MoveOutput *result = NULL;

	for (i = 0; true; i++) {
		ret = IMP_IVS_PollingResult(chn_num, IMP_IVS_DEFAULT_TIMEOUTMS);
		if (ret < 0) {
			IMP_LOG_ERR(TAG, "IMP_IVS_PollingResult(%d, %d) failed\n", chn_num, IMP_IVS_DEFAULT_TIMEOUTMS);
			return (void *)-1;
		}
		ret = IMP_IVS_GetResult(chn_num, (void **)&result);
		if (ret < 0) {
			IMP_LOG_ERR(TAG, "IMP_IVS_GetResult(%d) failed\n", chn_num);
			return (void *)-1;
		}
		printf("frame[%d], result->retRoi(%d,%d,%d,%d)\n", i, result->retRoi[0], result->retRoi[1], result->retRoi[2], result->retRoi[3]);

		ret = IMP_IVS_ReleaseResult(chn_num, (void *)result);
		if (ret < 0) {
			IMP_LOG_ERR(TAG, "IMP_IVS_ReleaseResult(%d) failed\n", chn_num);
			return (void *)-1;
		}
#if 0
		if (i % 20 == 0) {
			ret = sample_ivs_set_sense(chn_num, i % 5);
			if (ret < 0) {
				IMP_LOG_ERR(TAG, "sample_ivs_set_sense(%d, %d) failed\n", chn_num, i % 5);
				return (void *)-1;
			}
		}
#endif
	}

	return (void *)0;
}

static int sample_ivs_move_get_result_start(int chn_num, pthread_t *ptid)
{
	if (pthread_create(ptid, NULL, sample_ivs_move_get_result_process, (void *)chn_num) < 0) {
		IMP_LOG_ERR(TAG, "create sample_ivs_move_get_result_process failed\n");
		return -1;
	}

	return 0;
}

static int sample_ivs_move_get_result_stop(pthread_t tid)
{
	pthread_join(tid, NULL);
	return 0;
}


int destory()
{

	int ret, i;

	/* Exit sequence as follow */
	/* Exit sequence as follow */
	/* Step.10 stop to get ivs move result */
	ret = sample_ivs_move_get_result_stop(ivs_tid);
	if (ret < 0) {
		IMP_LOG_ERR(TAG, "sample_ivs_move_get_result_stop failed\n");
		return -1;
	}

	/* Step.11 ivs move stop */
	ret = sample_ivs_move_stop(2, inteface);
	if (ret < 0) {
		IMP_LOG_ERR(TAG, "sample_ivs_move_stop(0) failed\n");
		return -1;
	}
	/* Step.a Stream Off */
	ret = sample_framesource_streamoff();
	if (ret < 0) {
		IMP_LOG_ERR(TAG, "FrameSource StreamOff failed\n");
		return -1;
	}

	/* Step.13 UnBind */
	ret = IMP_System_UnBind(&chn[IVS_CHN_ID].framesource_chn, &ivs_cell);
	if (ret < 0) {
		IMP_LOG_ERR(TAG, "UnBind FrameSource channel%d and ivs0 failed\n", IVS_CHN_ID);
		return -1;
	}
	for (i = 0; i < FS_CHN_NUM; i++) {
		if (chn[i].enable) {
			if(IVS_CHN_ID == i) {
				ret = IMP_System_UnBind(&ivs_cell, &chn[i].imp_encoder);
				if (ret < 0) {
					IMP_LOG_ERR(TAG, "UnBind FrameSource channel%d and Encoder failed\n",i);
					return -1;
				}
			}else{
			ret = IMP_System_UnBind(&chn[i].framesource_chn, &chn[i].imp_encoder);
			if (ret < 0) {
				IMP_LOG_ERR(TAG, "UnBind FrameSource channel%d and Encoder failed\n",i);
				return -1;
			}
			}
		}
	}
	/* Step.14 ivs exit */
	ret = sample_ivs_move_exit(0);
	if (ret < 0) {
		IMP_LOG_ERR(TAG, "ivs mode exit failed\n");
		return -1;
	}


	/* Step.c Encoder exit */
	ret = sample_encoder_exit();
	if (ret < 0) {
		IMP_LOG_ERR(TAG, "Encoder exit failed\n");
		return -1;
	}

	/* Step.d FrameSource exit */
	ret = sample_framesource_exit();
	if (ret < 0) {
		IMP_LOG_ERR(TAG, "FrameSource exit failed\n");
		return -1;
	}

	/* Step.e System exit */
	ret = sample_system_exit();
	if (ret < 0) {
		IMP_LOG_ERR(TAG, "sample_system_exit() failed\n");
		return -1;
	}

	ret = IMP_Encoder_StopRecvPic(0);
	if (ret < 0) {
		IMP_LOG_ERR(TAG, "IMP_Encoder_StopRecvPic() failed\n");
		return -1;
	}

	return 0;
}

static int save_stream(int fd, IMPEncoderStream *stream)
{
        int ret, i, nr_pack = stream->packCount;

  //IMP_LOG_DBG(TAG, "----------packCount=%d, stream->seq=%u start----------\n", stream->packCount, stream->seq);
        for (i = 0; i < nr_pack; i++) {
    //IMP_LOG_DBG(TAG, "[%d]:%10u,%10lld,%10u,%10u,%10u\n", i, stream->pack[i].length, stream->pack[i].timestamp, stream->pack[i].frameEnd, *((uint32_t *)(&stream->pack[i].nalType)), stream->pack[i].sliceType);
                IMPEncoderPack *pack = &stream->pack[i];
                if(pack->length){
                        uint32_t remSize = stream->streamSize - pack->offset;
                        if(remSize < pack->length){
                                ret = write(fd, (void *)(stream->virAddr + pack->offset), remSize);
                                if (ret != remSize) {
                                        IMP_LOG_ERR(TAG, "stream write ret(%d) != pack[%d].remSize(%d) error:%s\n", ret, i, remSize, strerror(errno));
                                        return -1;
                                }
                                ret = write(fd, (void *)stream->virAddr, pack->length - remSize);
                                if (ret != (pack->length - remSize)) {
                                        IMP_LOG_ERR(TAG, "stream write ret(%d) != pack[%d].(length-remSize)(%d) error:%s\n", ret, i, (pack->length - remSize), strerror(errno));
                                        return -1;
                                }
                        }else {
                                ret = write(fd, (void *)(stream->virAddr + pack->offset), pack->length);
                                if (ret != pack->length) {
                                        IMP_LOG_ERR(TAG, "stream write ret(%d) != pack[%d].length(%d) error:%s\n", ret, i, pack->length, strerror(errno));
                                        return -1;
                                }
                        }
                }
        }
  //IMP_LOG_DBG(TAG, "----------packCount=%d, stream->seq=%u end----------\n", stream->packCount, stream->seq);
        return 0;
}

static int get_h264_stream(int fd, int chn)
{
	int ret;
	
	/* Polling H264 Stream, set timeout as 1000msec */
	ret = IMP_Encoder_PollingStream(chn, 100);
	if (ret < 0) {
		IMP_LOG_ERR(TAG, "Polling stream timeout\n");
	}

	IMPEncoderStream stream;
	/* Get H264 Stream */
	ret = IMP_Encoder_GetStream(chn, &stream, 1);
	if (ret < 0) {
		IMP_LOG_ERR(TAG, "IMP_Encoder_GetStream() failed\n");
		return -1;
	}
	
	ret = save_stream(fd, &stream);
	if (ret < 0) {
		close(fd);
		return ret;
	}
	
	IMP_Encoder_ReleaseStream(chn, &stream);

	return 0;
}

int get_stream(int fd, int chn)
{
	int  ret;
	
	ret = IMP_Encoder_StartRecvPic(chn);
	if (ret < 0){
		IMP_LOG_ERR(TAG, "IMP_Encoder_StartRecvPic(%d) failed\n", 1);
		return ret;
	}
	ret = get_h264_stream(fd, chn);
	if (ret < 0) {
		IMP_LOG_ERR(TAG, "Get H264 stream failed\n");
		return ret;
	}
/*	ret = IMP_Encoder_StopRecvPic(chn);
	if (ret < 0) {
		IMP_LOG_ERR(TAG, "IMP_Encoder_StopRecvPic() failed\n");
		return ret;
	}
*/
	return 0;
}

int capture_and_encoding()
{
	int ret = 0;
	int i = 0;

	
	printf(">>>>>caputre_and_encoding start\n");

	// undocumented functions to increase pool size
	//IMP_OSD_SetPoolSize(0x64000);
	//IMP_Encoder_SetPoolSize(0x100000);

	ret = sample_system_init();
	if (ret < 0) {
		IMP_LOG_ERR(TAG, "IMP_System_Init() failed\n");
		return -1;
	}

	/* Step.2 FrameSource init */
	IMP_LOG_INFO(TAG, "FrameSource init...\n");
	ret = sample_framesource_init();
	if (ret < 0) {
		IMP_LOG_ERR(TAG, "FrameSource init failed\n");
		return -1;
	}

	/* Step.3 Encoder init */
	for (i = 0; i < FS_CHN_NUM; i++) {
		if (chn[i].enable) {
			IMP_LOG_INFO(TAG, "Enc CreateGrp for ch %d...\n", i);
			ret = IMP_Encoder_CreateGroup(chn[i].index);
			if (ret < 0) {
				IMP_LOG_ERR(TAG, "IMP_Encoder_CreateGroup(%d) error !\n", i);
				return -1;
			}
		}
	}

	IMP_LOG_INFO(TAG, "EncoderInit...\n");
	ret = sample_encoder_init();
	if (ret < 0) {
		IMP_LOG_ERR(TAG, "Encoder init failed\n");
		return -1;
	}

	/* Step.4 ivs init */
	ret = sample_ivs_move_init(0);
	if (ret < 0) {
		IMP_LOG_ERR(TAG, "sample_ivs_move_init(0) failed\n");
		return -1;
	}



	/* Step.5 Bind */

	/* Bind framesource channel.1-output.1 to IVS group */
	/**
	 * FS.0 ----------------> Encoder.0(Main stream)
	 * FS.1 ----------------> IVS----> Encoder.1(Second stream)
	 */
    for (i = 0; i < FS_CHN_NUM; i++) {
        if (IVS_CHN_ID == i) {
            if (chn[i].enable) {

                ret = IMP_System_Bind(&chn[i].framesource_chn, &ivs_cell);
                if (ret < 0) {
                    IMP_LOG_ERR(TAG, "Bind FrameSource channel.1 output.1 and ivs0 failed\n");
                    return -1;
                }
                ret = IMP_System_Bind(&ivs_cell, &chn[i].imp_encoder);
                if (ret < 0) {
                    IMP_LOG_ERR(TAG, "Bind FrameSource channel%d and Encoder failed\n",i);
                    return -1;
                }
            }
        } else {
            if (chn[i].enable) {
                ret = IMP_System_Bind(&chn[i].framesource_chn, &chn[i].imp_encoder);
                if (ret < 0) {
                    IMP_LOG_ERR(TAG, "Bind FrameSource channel%d and Encoder failed\n",i);
                    return -1;
                }
            }
        }
    }

	/* Step.5 Stream On */
	printf("StreamOn...\n");
	ret = sample_framesource_streamon();
	if (ret < 0) {
		IMP_LOG_ERR(TAG, "ImpStreamOn failed\n");
		return -1;
	}
	/* Step.7 ivs move start */
	ret = sample_ivs_move_start(0, 2, &inteface);
	if (ret < 0) {
		IMP_LOG_ERR(TAG, "sample_ivs_move_start(0, 0) failed\n");
		return -1;
	}

	/* Step.8 start to get ivs move result */
	ret = sample_ivs_move_get_result_start(2, &ivs_tid);
	if (ret < 0) {
		IMP_LOG_ERR(TAG, "sample_ivs_move_get_result_start failed\n");
		return -1;
	}


	// start thread for activating night mode & IR cut filter
	pthread_t thread_info;
	pthread_create(&thread_info, NULL, sample_soft_photosensitive_ctrl, NULL);

	return 0;
}
