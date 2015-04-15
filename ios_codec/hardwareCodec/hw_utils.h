//
//  hw_utils.h
//  hardwareCodec
//
//  Created by sudoku.huang on 2/4/15.
//  Copyright (c) 2015 YY. All rights reserved.
//

#ifndef _HARDWARE_UTILS_H__
#define _HARDWARE_UTILS_H__

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif
    
#define GET_WIDTH(m)     (((m) + 1) << 4)
#define GET_HEIGHT(m)    (((m) + 1) << 4)
    
    struct sps_parameters {
        uint8_t forbidden_zero_bit;
        uint8_t nal_ref_idc;
        uint8_t nal_unit_type;////, u(ptr, 5, offset));
        
        uint8_t profile_idc;////, profile_idc = u(ptr, 8, offset));
        
        uint8_t constraint_set0_flag;////, u(ptr, 1, offset));
        uint8_t constraint_set1_flag;////, u(ptr, 1, offset));
        uint8_t constraint_set2_flag;////, u(ptr, 1, offset));
        uint8_t constraint_set3_flag;////, u(ptr, 1, offset));
        uint8_t reserved_zero_4bits;////, u(ptr, 4, offset));
        
        uint8_t level_idc;////, u(ptr, 8, offset));
        
        uint32_t seq_parameter_set_id;////, ue(ptr, offset));
        
        uint32_t chroma_format_idc;////, ue(ptr, offset));
        uint32_t separate_colour_plane_flag;////, u(ptr, 1, offset));
        uint32_t bit_depth_luma_minus8;////, chroma_format_idc = ue(ptr, offset));
        uint32_t bit_depth_chroma_minus8;////, chroma_format_idc = ue(ptr, offset));
            
        uint8_t lossless_qpprime_flag;////, u(ptr, 1, offset));
        uint8_t seq_scaling_matrix_present_flag;////, u(ptr, 1, offset));
        uint8_t seq_scaling_list_present_flag[12]; ////, u(ptr, 1, offset));

        uint32_t log2_max_frame_num_minus4;////, ue(ptr, offset));
        uint32_t pic_order_cnt_type;////, pic_order_cnt_type = ue(ptr, offset));
        uint32_t log2_max_pic_order_cnt_lsb_minus4;////, ue(ptr, offset));
        uint8_t delta_pic_order_always_zero_flag;////, u(ptr, 1, offset));
        int32_t offset_for_non_ref_pic;////, se(ptr, offset));
        int32_t offset_for_top_to_bottom_field;////, se(ptr, offset));
        uint32_t num_ref_frames_in_pic_order_cnt_cycle;////, ue(ptr, offset));
        int32_t ref_frames_in_pic_order_cnt_cycle;//se(ptr, offset));
        uint32_t num_ref_frames;////, ue(ptr, offset));
        
        uint8_t gaps_in_frame_num_value_allowed_flag;////, u(ptr, 1, offset));
        
        uint32_t pic_width_in_mbs_minus_1;////, ue(ptr, offset));
        uint32_t pic_height_in_map_units_minus_1;////, ue(ptr, offset));
        
        uint8_t frame_mbs_only_flag;////, frame_mbs_only_flag = u(ptr, 1, offset));
        uint8_t mb_adaptive_frame_field_flag;////, frame_mbs_only_flag = u(ptr, 1, offset));
        uint8_t direct_8x8_inference_flag;////, u(ptr, 1, offset));
        uint8_t frame_cropping_flag;////, frame_cropping_flag = u(ptr, 1, offset));
        uint32_t frame_cropping_rect_left_offset;////, ue(ptr, offset));
        uint32_t frame_cropping_rect_right_offset;////, ue(ptr, offset));
        uint32_t frame_cropping_rect_top_offset;////, ue(ptr, offset));
        uint32_t frame_cropping_rect_bottom_offset;////, ue(ptr, offset));
        
        uint8_t vui_prameters_present_flag;////, vui_prameters_present_flag = u(ptr, 1, offset));
        
        uint8_t aspect_ratio_info_present_flag;////, u(ptr, 1, offset));
        uint8_t overscan_ifo_present_flag;////, u(ptr, 1, offset));
        uint8_t video_signal_type_present_flag;////, u(ptr, 1, offset));
        uint8_t chroma_loc_info_present_flag;////, u(ptr, 1, offset));
        uint8_t timing_info_present_flag;////, u(ptr, 1, offset));
        uint8_t nal_hrd_parameters_present_flag;////, u(ptr, 1, offset));
        uint8_t vd_hrd_parameters_present_flag;////, u(ptr, 1, offset));
        uint8_t pic_struct_present_flag;////, u(ptr, 1, offset));
        uint8_t bitstream_restriction_flag;////, bitstream_restriction_flag = u(ptr, 1, offset));

        uint8_t motion_vectors_over_pic_boundaries_flag;////, u(ptr, 1, offset));
        uint32_t max_bytes_per_pic_denom;////, ue(ptr, offset));
        uint32_t max_bits_per_pic_denom;////, ue(ptr, offset));
        uint32_t log2_max_mv_length_horizontal;////, ue(ptr, offset));
        uint32_t log2_max_mv_length_vertical;////, ue(ptr, offset));
        uint32_t num_reorder_frames;////, ue(ptr, offset));
        uint32_t max_dec_frame_buffering;////, ue(ptr, offset));
        
        uint8_t rbsp_stop_one_bit;////, u(ptr, 1, offset));        
    };
    
    int32_t analysis_sps(const uint8_t * sps, struct sps_parameters * params);
    void print_sps_parameters(struct sps_parameters * params);

#ifdef __cplusplus
}
#endif
        
#endif
