/*************************************************************************
    > File Name: writemp4.cpp
    > Author: sudoku.huang
    > Mail: sudoku.huang@gmail.com 
    > Created Time: Fri 13 Oct 2017 05:50:19 PM CST
 ************************************************************************/
//g++ writemp4.cpp -I/root/bin/include -L/root/bin/lib -lavformat -lavcodec -lswscale  -lpthread  -lavfilter -lx264 -lx265 -lavutil -ldl -lpng -lz  -lmp3lame -lswresample -D__STDC_CONSTANT_MACROS
//
#include <stdio.h>
#include <iostream>
extern "C"
{
#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>
#include <libavutil/avutil.h>
#include <libswscale/swscale.h>
}

using namespace std;

int flush_encoder(AVFormatContext *fmt_ctx,unsigned int stream_index);

int main(int argc, char *argv[])
{
    AVFormatContext *pFormatCtx=NULL;
    AVOutputFormat *fmt=NULL;
    AVStream *video_st=NULL;
    AVCodecContext *pCodecCtx=NULL;
    AVCodec *pCodec=NULL;

    uint8_t *picture_buf=NULL;
    AVFrame *picture=NULL;
    int size;
    int y_size, ret;

    //打开视频
    FILE *in_file = fopen(argv[1], "rb");
    if(!in_file)
    {
        cout<<"can not open file!"<<endl;
        return -1;
    }

    int in_w=atoi(argv[2]),in_h=atoi(argv[3]);
    int framenum=200;
    const char* out_file="src01.mp4";

    //[1] --注册所有ffmpeg组件
    avcodec_register_all();
    av_register_all();
    //[1]

    //[2] --初始化AVFormatContext结构体,根据文件名获取到合适的封装格式
    avformat_alloc_output_context2(&pFormatCtx,NULL,NULL,out_file);
    fmt = pFormatCtx->oformat;
    //[2]

    //[3] --打开文件
    if(avio_open(&pFormatCtx->pb,out_file,AVIO_FLAG_READ_WRITE))
    {
        cout<<"output file open fail!";
        goto end;
    }
    //[3]

    //[4] --初始化视频码流
    video_st = avformat_new_stream(pFormatCtx,0);
    if(video_st==NULL)
    { printf("failed allocating output stram\n");
        goto end;
    }
    video_st->time_base.num = 1;
    video_st->time_base.den =15;
    //[4]

    //[5] --编码器Context设置参数
    pCodecCtx = video_st->codec;
    pCodecCtx->codec_id = fmt->video_codec;
    pCodecCtx->codec_type = AVMEDIA_TYPE_VIDEO;
    pCodecCtx->pix_fmt = AV_PIX_FMT_YUV420P;
    pCodecCtx->width=in_w;
    pCodecCtx->height=in_h;
    pCodecCtx->time_base.num = 1;
    pCodecCtx->time_base.den = 15;
    pCodecCtx->bit_rate = 400000;
    pCodecCtx->gop_size = 12;

    if(pCodecCtx->codec_id == AV_CODEC_ID_H264)
    {
        pCodecCtx->qmin = 10;
        pCodecCtx->qmax = 51;
        pCodecCtx->qcompress = 0.6;
    }
    if (pCodecCtx->codec_id == AV_CODEC_ID_MPEG2VIDEO)
        pCodecCtx->max_b_frames = 2;
    if (pCodecCtx->codec_id == AV_CODEC_ID_MPEG1VIDEO)
        pCodecCtx->mb_decision = 2;
    //[5]

    //[6] --寻找编码器并打开编码器
    pCodec = avcodec_find_encoder(pCodecCtx->codec_id);
    if(!pCodec)
    {
        cout<<"no right encoder!"<<endl;
        goto end;
    }
    if(avcodec_open2(pCodecCtx,pCodec,NULL)<0)
    {
        cout<<"open encoder fail!"<<endl;
        goto end;
    }
    //[6]

    //输出格式信息
    av_dump_format(pFormatCtx,0,out_file,1);

    //初始化帧
    picture = av_frame_alloc();
    picture->width=pCodecCtx->width;
    picture->height=pCodecCtx->height;
    picture->format=pCodecCtx->pix_fmt;
    size = avpicture_get_size(pCodecCtx->pix_fmt,pCodecCtx->width,pCodecCtx->height);
    picture_buf = (uint8_t*)av_malloc(size);
    avpicture_fill((AVPicture*)picture,picture_buf,pCodecCtx->pix_fmt,pCodecCtx->width,pCodecCtx->height);

    //[7] --写头文件
    avformat_write_header(pFormatCtx,NULL);
    //[7]

    AVPacket pkt; //创建已编码帧
    y_size = pCodecCtx->width*pCodecCtx->height;
    av_new_packet(&pkt,size*3);

    //[8] --循环编码每一帧
    for(int i=0;i<framenum;i++)
    {
        //读入YUV
        if(fread(picture_buf,1,y_size*3/2,in_file)<0)
        {
            cout<<"read file fail!"<<endl;
            goto end;
        }
        else if(feof(in_file))
            break;

        picture->data[0] = picture_buf; //亮度Y
        picture->data[1] = picture_buf+y_size; //U
        picture->data[2] = picture_buf+y_size*5/4; //V
        //AVFrame PTS
        picture->pts=i;
        int got_picture = 0;

        //编码
        int ret = avcodec_encode_video2(pCodecCtx,&pkt,picture,&got_picture);
        if(ret<0)
        {
            cout<<"encoder fail!"<<endl;
            goto end;
        }

        if(got_picture == 1)
        {
            cout<<"encoder success!"<<endl;

            // parpare packet for muxing
            pkt.stream_index = video_st->index;
            av_packet_rescale_ts(&pkt, pCodecCtx->time_base, video_st->time_base);
            pkt.pos = -1;
            ret = av_interleaved_write_frame(pFormatCtx,&pkt);
            av_free_packet(&pkt);
        }
    }
    //[8]

    //[9] --Flush encoder
    ret = flush_encoder(pFormatCtx,0);
    if(ret < 0)
    {
        cout<<"flushing encoder failed!"<<endl;
        goto end;
    }
    //[9]

    //[10] --写文件尾
    av_write_trailer(pFormatCtx);
    //[10]

end:
    //释放内存
    if(video_st)
    {
        avcodec_close(video_st->codec);
        av_free(picture);
        av_free(picture_buf);
    }
    if(pFormatCtx)
    {
        avio_close(pFormatCtx->pb);
        avformat_free_context(pFormatCtx);
    }

    fclose(in_file);

    return 0;
}

int flush_encoder(AVFormatContext *fmt_ctx,unsigned int stream_index)
{
    int ret;
    int got_frame;
    AVPacket enc_pkt;
    if (!(fmt_ctx->streams[stream_index]->codec->codec->capabilities &
          CODEC_CAP_DELAY))
        return 0;
    while (1) {
        printf("Flushing stream #%u encoder\n", stream_index);
        enc_pkt.data = NULL;
        enc_pkt.size = 0;
        av_init_packet(&enc_pkt);
        ret = avcodec_encode_video2 (fmt_ctx->streams[stream_index]->codec, &enc_pkt,
                                     NULL, &got_frame);
        av_frame_free(NULL);
        if (ret < 0)
            break;
        if (!got_frame)
        {ret=0;break;}
        cout<<"success encoder 1 frame"<<endl;

        // parpare packet for muxing
        enc_pkt.stream_index = stream_index;
        av_packet_rescale_ts(&enc_pkt,
                             fmt_ctx->streams[stream_index]->codec->time_base,
                             fmt_ctx->streams[stream_index]->time_base);
        ret = av_interleaved_write_frame(fmt_ctx, &enc_pkt);
        if (ret < 0)
            break;
    }
    return ret;
}
