/* ------------------------------------------------------------------------------------
** CAEN VX1742B constants and memory layout
** 
**
** <vx1742scope>.h
** 
** Date: March 2015
** Author: Christian Dorfer (dorfer@phys.ethz.ch)
** Info: No need for unions, endianess is handled in the TSI chip
** ------------------------------------------------------------------------------------*/

#ifndef _VX1742DEFS_H
#define _VX1742DEFS_H

// Constants
#define VX1742_CHANNELS 32
#define VX1742_GROUPS 4


// Constants for VME master map and BlockTransfer
const u_int vmebus_address = 0x32100000; 
const u_int window_size = 0x10000;
const u_int address_modifier = VME_A32;
const u_int options = 0;
const u_int buffer_size = 0x10000;


typedef struct{
  u_int ch_thres;                                       //0x1080
  u_int reserved1;                                      //0x1084 - 0x1088
  u_int status;                                         //0x1088
  u_int daugther_board_fw;                              //0x108C
  u_int reserved2;                                      //0x1090 - 0x1094
  u_int buffer_occupation;                              //0x1094
  u_int channel_dc_offset;                              //0x1098
  u_int reserved3;                                      //0x109C - 0x10A0
  u_int drs4_temperature;                               //0x10A0
  u_int dac_sel;                                        //0x10A4
  u_int ch_trg_enable_mask;                             //0x10A8
  u_int reserved4[8];                                   //0x10AC - 0x10CC
  u_int mem_calib_tables_enable;                        //0x10CC
  u_int mem_calib_tables_data;                          //0x10D0
  u_int trigger_threshold;                              //0x10D4
  u_int reserved5;                                      //0x10D8 - 0x10DC
  u_int trigger_dc_offset;                              //0x10DC
  u_int reserved6[40];                                  //0x10E0 - 0x1180
}st_group_n_conf; //used for group 0-3 (for <group n variable address> add n*0x100 to the addresses listed)


typedef struct{
  u_int monitor_signal      :4;
  u_int reserved_5         :15;
  u_int trigger_trn_enable  :1;
  u_int sig_trn_enable      :1;
  u_int reserved_4          :2; 
  u_int individual_trg      :1; 
  u_int reserved_3          :1;
  u_int trigger_polarity    :1; 
  u_int reserved_2          :1; 
  u_int reserved_1          :1;
  u_int test_mode           :1;
  u_int reserved_0          :3; 
}st_group_conf;


typedef struct{
  u_int reserved       :26;
  u_int buffer_mode     :1;
  u_int reserved_1      :1;
  u_int trigger_count   :1;
  u_int acq_control     :1;
  u_int reserved_0      :2;
}st_acq_control;


typedef struct{
  u_int reserved1      :23;
  u_int ready           :1; 
  u_int PLL_status      :1;
  u_int PLL_bypass      :1; 
  u_int clock_source    :1; 
  u_int event_full      :1;
  u_int event_ready     :1;
  u_int run             :1;
  u_int reserved2       :2;
}st_acq_status;


typedef struct{
  u_int sw_trigger      :1;
  u_int ext_trigger     :1;
  u_int reserved       :30;
}st_trigger_source;


typedef struct{
  u_int sw_trigger      :1;
  u_int ext_trigger     :1;
  u_int reserved       :26;
  u_int grp3_trg        :1;
  u_int grp2_trg        :1;
  u_int grp1_trg        :1;
  u_int grp0_trg        :1;
}st_trigger_out_mask;


typedef struct{
  u_int reserved1             :11;
  u_int busy_unlock            :1;
  u_int motherboad_probe       :2;
  u_int trg_out_mode_select    :2;
  u_int trg_out_mode           :1;
  u_int force_trg_out_mode     :1; 
  u_int reserved2              :6;
  u_int io_mode                :2;
  u_int lvds_direction_12_15   :1;
  u_int lvds_direction_8_11    :1;
  u_int lvds_direction_4_7     :1;
  u_int lvds_direction_0_3     :1;
  u_int panel_output_state     :1;
  u_int trg_clk_level          :1;
}st_front_panel_io_control;


typedef struct{
  u_int reserved  :24;
  u_int group3    :1;
  u_int group2    :1;
  u_int group1    :1;
  u_int group0    :1;
}st_group_en_mask;


typedef struct{
  u_int reserved    :16;
  u_int memory      :8;
  u_int board_type  :8;
}st_board_info;


typedef struct{
  u_int reserved       :24;
  u_int release         :1;
  u_int reloc           :1;
  u_int align64         :1;
  u_int berr            :1; 
  u_int optical_link    :1;
  u_int interrupt_level :3;
}st_vme_control;


typedef struct{
  u_int reserved1    :28;
  u_int fifo_empty    :1;
  u_int bus_error     :1;
  u_int reserved2     :1;
  u_int event_ready   :1;
}st_vme_status;


typedef struct{
  u_int checksum;                                            //0xF000                       
  u_int checksum_length2;                                    //0xF004
  u_int checksum_length1;                                    //0xF008
  u_int checksum_length0;                                    //0xF00C
  u_int constant2;                                           //0xF010
  u_int constant1;                                           //0xF014
  u_int constant0;                                           //0xF018
  u_int c_code;                                              //0xF01C
  u_int r_code;                                              //0xF020
  u_int oui2;                                                //0xF024
  u_int oui1;                                                //0xF028
  u_int oui0;                                                //0xF02C
  u_int vers;                                                //0xF030
  u_int board2;                                              //0xF034
  u_int board1;                                              //0xF038
  u_int board0;                                              //0xF03C
  u_int revis3;                                              //0xF040
  u_int revis2;                                              //0xF044
  u_int revis1;                                              //0xF048
  u_int revis0;                                              //0xF04C
  u_int reserved[12];                                        //0xF050 - 0xF080
  u_int sernum1;                                             //0xF080
  u_int sernum0;                                             //0xF084
}st_configuration_rom;



// VX1742 Buffer Layout
typedef struct{
  u_int output_buffer;                                      //0x0000
  u_int dummy1[1023];                                       //0x0004 - 0x0FFC
  u_int dummy2[32];                                         //0x0FFC - 0x1080
  st_group_n_conf group_n_conf[4];                          //0x1080 - 0x1480
  u_int dummy3[6880];                                       //0x1480 - 0x8000
  st_group_conf group_conf;                                 //0x8000 
  u_int group_config_bit_set;                               //0x8004
  u_int group_config_bit_clear;                             //0x8008
  u_int buffer_organisation;                                //0x800C
  u_int dummy4[4];                                          //0x8010 - 0x8020
  u_int custom_size;                                        //0x8020
  u_int dummy5[22];                                         //0x8024 - 0x807C
  u_int initial_testwave;                                   //0x807C
  u_int dummy6[22];                                         //0x8080 - 0x80D8
  u_int sampling_frequency;                                 //0x80D8
  u_int dummy7[9];                                          //0x80DC - 0x8100
  st_acq_control acq_control;                               //0x8100
  st_acq_status acq_status;                                 //0x8104
  u_int software_trigger;                                   //0x8108
  st_trigger_source trigger_src;                            //0x810C
  st_trigger_out_mask trigger_out_mask;                     //0x8110
  u_int post_trigger;                                       //0x8114
  u_int front_panel_io_data;                                //0x8118
  st_front_panel_io_control front_panel_io_control;         //0x811C
  st_group_en_mask group_en_mask;                           //0x8120
  u_int mother_roc_firmware;                                //0x8124
  u_int dummy8;                                             //0x8128 - 0812C
  u_int events_stored;                                      //0x812C
  u_int dummy9[2];                                          //0x8130 - 0x8138
  u_int monitor_dac;                                        //0x8138
  u_int dummy10;                                            //0x813C - 0x8140
  st_board_info board_info;                                 //0x8140
  u_int monitor_mode;                                       //0x8144
  u_int dummy11;                                            //0x8148 - 0x814C
  u_int event_size;                                         //0x814C
  u_int dummy12[7020];                                      //0x8150 - 0xEF00
  st_vme_control vme_control;                               //0xEF00
  st_vme_status vme_status;                                 //0xEF04
  u_int board_id;                                           //0xEF08
  u_int mc_base_control;                                    //0xEF0C
  u_int reloc_addr;                                         //0xEF10
  u_int intrr_stat_id;                                      //0xEF14
  u_int intrr_event_nr;                                     //0xEF18
  u_int blt_event_number;                                   //0xEF1C
  u_int scratch;                                            //0xEF20
  u_int software_reset;                                     //0xEF24
  u_int software_clear;                                     //0xEF28
  u_int flash_enable;                                       //0xEF2C
  u_int flash_data;                                         //0xEF30
  u_int configuration_reload;                               //0xEF34
  u_int dummy13[50];                                        //0xEF38 - 0xF000                
  st_configuration_rom configuration_rom;                   //0xF000 - 0xF088                      
  u_int dummy14[221];                                       //0xF088 - 0xF3FC                    
} volatile vx1742_regs_t;

#endif