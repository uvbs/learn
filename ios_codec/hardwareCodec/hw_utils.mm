//
//  hw_utils.mm
//  hardwareCodec
//
//  Created by sudoku.huang on 2/4/15.
//  Copyright (c) 2015 YY. All rights reserved.
//

#include "hw_utils.h"

#include <stdio.h>
#include <stdlib.h>

#ifdef __cplusplus
extern "C" {
#endif

#define BYTE_MASK          (7)
#define BYTE_ALIGN         (~BYTE_MASK)
#define IS_BIT_SET(m, n)   (0x01 & (m >> (7 - (n))))

#define BASELINE           66
#define MAIN               77
#define EXTENDED           88
#define FREXT_HP           100
#define FREXT_Hi10P        110
#define FREXT_Hi422        122
#define FREXT_Hi444        244
#define FREXT_CAVLC444     44

#define YUV420             (1)
#define YUV422             (2)
#define YUV444             (3)

    static uint32_t u(const uint8_t * data, int size, int & off_bit)
    {
        int off_byte = off_bit >> 3;
        int off_in_byte = off_bit & BYTE_MASK;
        
        const uint8_t * ptr = data + off_byte;
        uint32_t value = 0;
        off_bit += size;
        
        while ((size --) > 0) {
            value = (value << 1) | IS_BIT_SET(* ptr, off_in_byte);
            //printf("value:%u, offset_in_byte:%d\n", value, off_in_byte);
            
            off_in_byte ++;
            if (off_in_byte & 0x08) {
                ptr ++;
                off_in_byte = 0;
            }
        }
        
        return value;
    }

    static uint32_t ue(const uint8_t * data, int & off_bit)
    {
        int off_byte = off_bit >> 3;
        int off_in_byte = off_bit & BYTE_MASK;
        
        const uint8_t * ptr = data + off_byte;
        int zero_cnt = 0;
        
        while (true) {
            //printf("zero_cnt:%u, byte:0x%02x, offset_in_byte:%d\n", zero_cnt, * ptr, off_in_byte);
            
            if (IS_BIT_SET(* ptr, off_in_byte)) {
                break;
            }
            
            zero_cnt ++;
            off_in_byte ++;
            
            if (off_in_byte & 0x08) {
                ptr ++;
                off_in_byte = 0;
            }
        }
        
        off_bit += zero_cnt;
        
        return u(data, zero_cnt + 1, off_bit) - 1;
    }

    int se(const uint8_t * data, int & off_bit)
    {
        int off_byte = off_bit >> 3;
        int off_in_byte = off_bit & BYTE_MASK;
        
        const uint8_t * ptr = data + off_byte;
        int zero_cnt = 0;
        
        while (true) {
            //printf("zero_cnt\n, byte:0x%02x, offset_in_byte:%d\n", zero_cnt, * ptr, off_in_byte);
            
            if (IS_BIT_SET(* ptr, off_in_byte)) {
                break;
            }
            
            zero_cnt ++;
            off_in_byte ++;
            
            if (off_in_byte & 0x08) {
                ptr ++;
                off_in_byte = 0;
            }
        }
        
        off_bit += zero_cnt;
        
        uint32_t tvalue = u(data, zero_cnt + 1, off_bit);
        int value = 0;
        if (tvalue & 1) {
            value = -1 * (tvalue >> 1);
        } else {
            value = tvalue >> 1;
        }
        
        return value;
    }
    
    void scaling_list(const uint8_t * data, int & off_bit, int size_scalinglist)
    {
        int last_scale = 8;
        int next_scale = 8;
        int delta_scale;
        
        for (int i = 0; i < size_scalinglist; i++) {
            if (next_scale) {
                delta_scale = se(data, off_bit);
                next_scale  = (last_scale + delta_scale + 256) & 255;
            }
            last_scale = (!next_scale) ? last_scale : next_scale;
        }
    }
    
    int32_t analysis_sps(const uint8_t * sps, struct sps_parameters * params)
    {
        /* //sps samples to test.
         const uint8_t sps_128x96[] = {0x00, 0x00, 0x00, 0x01,
         0x67, 0x42, 0x00, 0x0a, 0xf8, 0x41, 0xa2};
         const uint8_t sps_640x480[] = {0x00, 0x00, 0x00, 0x01,
         0x27, 0x42, 0x00, 0x1e, 0xab, 0x40, 0x50, 0x1e, 0xc8};
         const uint8_t sps_1280x720[] = {0x00, 0x00, 0x00, 0x01,
         0x67, 0x42, 0x80, 0x1F, 0xDA, 0x01, 0x40, 0x16, 0xE8, 0x06, 0xD0, 0xA1, 0x35};
         */
        
        if (sps == NULL || params == NULL) {
            return -1;
        }
        
        int offset = 0;
        const uint8_t * ptr = sps;
        
        params->forbidden_zero_bit                          = u(ptr, 1, offset);
        params->nal_ref_idc                                 = u(ptr, 2, offset);
        params->nal_unit_type                               = u(ptr, 5, offset);
        if (params->nal_unit_type != 0x07) { // not invalid sps.
            return -1;
        }
        
        params->profile_idc                                 = u(ptr, 8, offset);
        
        params->constraint_set0_flag                        = u(ptr, 1, offset);
        params->constraint_set1_flag                        = u(ptr, 1, offset);
        params->constraint_set2_flag                        = u(ptr, 1, offset);
        params->constraint_set3_flag                        = u(ptr, 1, offset);
        params->reserved_zero_4bits                         = u(ptr, 4, offset);
        
        params->level_idc                                   = u(ptr, 8, offset);
        
        params->seq_parameter_set_id                        = ue(ptr, offset);
        
        if(params->profile_idc == 100 || params->profile_idc == 110 ||
           params->profile_idc == 122 || params->profile_idc == 244 || params->profile_idc ==  44 ||
           params->profile_idc ==  83 || params->profile_idc ==  86 || params->profile_idc == 118 ||
           params->profile_idc == 128 || params->profile_idc == 144) {
            
            params->chroma_format_idc                       = ue(ptr, offset);
            if (params->chroma_format_idc == YUV444) {
                params->separate_colour_plane_flag          = u(ptr, 1, offset);
            }
            
            params->bit_depth_luma_minus8                   = ue(ptr, offset);
            params->bit_depth_chroma_minus8                 = ue(ptr, offset);
            
            params->lossless_qpprime_flag                   = u(ptr, 1, offset);
            params->seq_scaling_matrix_present_flag         = u(ptr, 1, offset);
            
            if (params->seq_scaling_matrix_present_flag) {
                int nscalinglist = 8;//(params->chroma_format_idc != YUV444) ? 8 : 12;
                for (int i = 0; i < nscalinglist; i++) {
                    params->seq_scaling_list_present_flag[i] = u(ptr, 1, offset);
                    if (params->seq_scaling_list_present_flag[i]) {
                        if (i < 6) {
                            scaling_list(ptr, offset, 16);
                        } else {
                            scaling_list(ptr, offset, 64);
                        }
                    }
                }
            }
        }
        
        params->log2_max_frame_num_minus4                   = ue(ptr, offset);
        params->pic_order_cnt_type                          = ue(ptr, offset);
        if (!params->pic_order_cnt_type) {
            params->log2_max_pic_order_cnt_lsb_minus4       = ue(ptr, offset);
        } else if (params->pic_order_cnt_type == 1) {
            params->delta_pic_order_always_zero_flag        = u(ptr, 1, offset);
            params->offset_for_non_ref_pic                  = se(ptr, offset);
            params->offset_for_top_to_bottom_field          = se(ptr, offset);
            params->num_ref_frames_in_pic_order_cnt_cycle   = ue(ptr, offset);
            
            for (int i = 0; i < params->num_ref_frames_in_pic_order_cnt_cycle; i ++) {
                se(ptr, offset); //just skip, not get.
            }
        }
        
        params->num_ref_frames                              = ue(ptr, offset);
        
        params->gaps_in_frame_num_value_allowed_flag        = u(ptr, 1, offset);
        
        params->pic_width_in_mbs_minus_1                    = ue(ptr, offset);
        params->pic_height_in_map_units_minus_1             = ue(ptr, offset);
        
        params->frame_mbs_only_flag                         = u(ptr, 1, offset);
        if (!params->frame_mbs_only_flag ) {
            params->mb_adaptive_frame_field_flag            = u(ptr, 1, offset);
        }
        
        params->direct_8x8_inference_flag                   = u(ptr, 1, offset);
        params->frame_cropping_flag                         = u(ptr, 1, offset);
        if (params->frame_cropping_flag) {
            params->frame_cropping_rect_left_offset         = ue(ptr, offset);
            params->frame_cropping_rect_right_offset        = ue(ptr, offset);
            params->frame_cropping_rect_top_offset          = ue(ptr, offset);
            params->frame_cropping_rect_bottom_offset       = ue(ptr, offset);
        }
        
        params->vui_prameters_present_flag                  = u(ptr, 1, offset);
        if (params->vui_prameters_present_flag) {
            params->aspect_ratio_info_present_flag          = u(ptr, 1, offset);
            if (params->aspect_ratio_info_present_flag == 0xFF) {
                u(ptr, 16, offset); //sar_width
                u(ptr, 16, offset); //sar_height
            }
            params->overscan_ifo_present_flag               = u(ptr, 1, offset);
            if (params->overscan_ifo_present_flag) {
                u(ptr, 1, offset); //overscan_appropriate_flag
            }
            params->video_signal_type_present_flag          = u(ptr, 1, offset);
            if (params->video_signal_type_present_flag) {
                u(ptr, 3, offset); //video_format
                u(ptr, 1, offset); //video_full_range_flag
                if (u(ptr, 1, offset)) { //color_description_present_flag
                    u(ptr, 8, offset); //color_primaries
                    u(ptr, 8, offset); //transfer_characteristics
                    u(ptr, 8, offset); //matrix)ciefficients
                }
            }
            params->chroma_loc_info_present_flag            = u(ptr, 1, offset);
            params->timing_info_present_flag                = u(ptr, 1, offset);
            params->nal_hrd_parameters_present_flag         = u(ptr, 1, offset);
            params->vd_hrd_parameters_present_flag          = u(ptr, 1, offset);
            params->pic_struct_present_flag                 = u(ptr, 1, offset);
            params->bitstream_restriction_flag              = u(ptr, 1, offset);
            if (params->bitstream_restriction_flag) {
                params->motion_vectors_over_pic_boundaries_flag = u(ptr, 1, offset);
                params->max_bytes_per_pic_denom                 = ue(ptr, offset);
                params->max_bits_per_pic_denom                  = ue(ptr, offset);
                params->log2_max_mv_length_horizontal           = ue(ptr, offset);
                params->log2_max_mv_length_vertical             = ue(ptr, offset);
                params->num_reorder_frames                      = ue(ptr, offset);
                params->max_dec_frame_buffering                 = ue(ptr, offset);
            }
        }
        params->rbsp_stop_one_bit                           = u(ptr, 1, offset);
        
        return 0;
    }
    
    void print_sps_parameters(struct sps_parameters * params)
    {
        printf("%u <- forbidden_zero_bit\n", params->forbidden_zero_bit);
        printf("%u <- nal_ref_idc\n", params->nal_ref_idc);
        printf("%u <- nal_unit_type\n", params->nal_unit_type);
        
        printf("%u <- profile_idc\n", params->profile_idc);
        
        printf("%u <- constraint_set0_flag\n", params->constraint_set0_flag);
        printf("%u <- constraint_set1_flag\n", params->constraint_set1_flag);
        printf("%u <- constraint_set2_flag\n", params->constraint_set2_flag);
        printf("%u <- constraint_set3_flag\n", params->constraint_set3_flag);
        printf("%u <- reserved_zero_4bits\n", params->reserved_zero_4bits);
        
        printf("%u <- level_idc\n", params->level_idc);
        printf("%u <- seq_parameter_set_id\n", params->seq_parameter_set_id);
        
        if((params->profile_idc == FREXT_HP) ||
           (params->profile_idc == FREXT_Hi10P) ||
           (params->profile_idc == FREXT_Hi422) ||
           (params->profile_idc == FREXT_Hi444) ||
           (params->profile_idc == FREXT_CAVLC444)) {
            printf("\t%u <- chroma_format_idc\n", params->chroma_format_idc);
            if (params->chroma_format_idc == YUV444) {
                printf("\t\t%u <- separate_colour_plane_flag\n", params->separate_colour_plane_flag);
            }
            
            printf("\t%u <- bit_depth_luma_minus8\n", params->bit_depth_luma_minus8);
            printf("\t%u <- bit_depth_chroma_minus8\n", params->bit_depth_chroma_minus8);
            printf("\t%u <- lossless_qpprime_flag\n", params->lossless_qpprime_flag);
            printf("\t%u <- seq_scaling_matrix_present_flag\n", params->seq_scaling_matrix_present_flag);
            
            int nscalinglist = (params->chroma_format_idc != YUV444) ? 8 : 12;
            for (int i = 0; i < nscalinglist; i++) {
                printf("\t\t%u <- seq_scaling_list_present_flag[%d]\n", i, params->seq_scaling_list_present_flag[i]);
            }
        }
        
        printf("%u <- log2_max_frame_num_minus4\n", params->log2_max_frame_num_minus4);
        printf("%u <- pic_order_cnt_type\n", params->pic_order_cnt_type);
        if (!params->pic_order_cnt_type) {
            printf("\t%u <- log2_max_pic_order_cnt_lsb_minus4\n", params->log2_max_pic_order_cnt_lsb_minus4);
        } else if (params->pic_order_cnt_type == 1) {
            printf("\t%u <- delta_pic_order_always_zero_flag\n", params->delta_pic_order_always_zero_flag);
            printf("\%u <- toffset_for_non_ref_pic\n", params->offset_for_non_ref_pic);
            printf("\t%u <- offset_for_top_to_bottom_field\n", params->offset_for_top_to_bottom_field);
            printf("\t%u <- num_ref_frames_in_pic_order_cnt_cycle\n", params->num_ref_frames_in_pic_order_cnt_cycle);
        }
        
        printf("%u <- num_ref_frames\n", params->num_ref_frames);
        printf("%u <- gaps_in_frame_num_value_allowed_flag\n", params->gaps_in_frame_num_value_allowed_flag);
        
        printf("%u <- pic_width_in_mbs_minus_1\n", params->pic_width_in_mbs_minus_1);
        printf("%u <- pic_height_in_map_units_minus_1\n", params->pic_height_in_map_units_minus_1);
        
        printf("%u <- frame_mbs_only_flag\n", params->frame_mbs_only_flag);
        if (!params->frame_mbs_only_flag) {
            printf("\t%u <- mb_adaptive_frame_field_flag\n", params->mb_adaptive_frame_field_flag);
        }
        printf("%u <- direct_8x8_inference_flag\n", params->direct_8x8_inference_flag);
        printf("%u <- frame_cropping_flag\n", params->frame_cropping_flag);
        if (params->frame_cropping_flag) {
            printf("\t%u <- frame_cropping_rect_left_offset\n", params->frame_cropping_rect_left_offset);
            printf("\t%u <- frame_cropping_rect_right_offset\n", params->frame_cropping_rect_right_offset);
            printf("\t%u <- frame_cropping_rect_top_offset\n", params->frame_cropping_rect_top_offset);
            printf("\t%u <- frame_cropping_rect_bottom_offset\n", params->frame_cropping_rect_bottom_offset);
        }
        
        printf("%u <- vui_prameters_present_flag\n", params->vui_prameters_present_flag);
        if (params->vui_prameters_present_flag) {
            printf("\t%u <- aspect_ratio_info_present_flag\n", params->aspect_ratio_info_present_flag);
            printf("\t%u <- overscan_ifo_present_flag\n", params->overscan_ifo_present_flag);
            printf("\t%u <- video_signal_type_present_flag\n", params->video_signal_type_present_flag);
            printf("\t%u <- chroma_loc_info_present_flag\n", params->chroma_loc_info_present_flag);
            printf("\t%u <- timing_info_present_flag\n", params->timing_info_present_flag);
            printf("\t%u <- nal_hrd_parameters_present_flag\n", params->nal_hrd_parameters_present_flag);
            printf("\t%u <- vd_hrd_parameters_present_flag\n", params->vd_hrd_parameters_present_flag);
            printf("\t%u <- pic_struct_present_flag\n", params->pic_struct_present_flag);
            printf("\t%u <- bitstream_restriction_flag\n", params->bitstream_restriction_flag);
            if (params->bitstream_restriction_flag) {
                printf("\t\t%u <- motion_vectors_over_pic_boundaries_flag\n", params->motion_vectors_over_pic_boundaries_flag);
                printf("\t\t%u <- max_bytes_per_pic_denom\n", params->max_bytes_per_pic_denom);
                printf("\t\t%u <- max_bits_per_pic_denom\n", params->max_bits_per_pic_denom);
                printf("\t\t%u <- log2_max_mv_length_horizontal\n", params->log2_max_mv_length_horizontal);
                printf("\t\t%u <- log2_max_mv_length_vertical\n", params->log2_max_mv_length_vertical);
                printf("\t\t%u <- num_reorder_frames\n", params->num_reorder_frames);
                printf("\t\t%u <- max_dec_frame_buffering\n", params->max_dec_frame_buffering);
            }
        }
        printf("%u <- rbsp_stop_one_bit\n", params->rbsp_stop_one_bit);
    }

#ifdef __cplusplus
}
#endif
