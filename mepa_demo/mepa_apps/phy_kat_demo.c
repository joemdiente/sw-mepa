// Copyright (c) 2004-2020 Microchip Technology Inc. and its subsidiaries.
// SPDX-License-Identifier: MIT


/* Known Answer Test (KAT) MACsec Demo */
#include <stdio.h>
#include <unistd.h>
#include <ctype.h>
#include "microchip/ethernet/switch/api.h"
#include "microchip/ethernet/board/api.h"
#include <vtss_phy_api.h>
#include "main.h"
#include "trace.h"
#include "cli.h"
#include "port.h"

#define MAX_PORTS 20                     /* Number of Ports in EDSx */
#define IPV4_ETHERTYPE 0x800             /* Ethertype of IPv4 Packet */
#define PTP_ETHERTYPE 0x88f7             /* Ethertype of PTP packet */
#define MACSEC_ETHERTYPE 0x88E5          /* Ethertype of MACsec Packet */
#define PHY_IS_1G 1                      /* Connected PHY is 1G */
#define PHY_IS_10G 10                    /* Connected PHY is 10G */
#define PHY_IS_25G 25                    /* Connected PHY is 25G */
#define SAK_KEY_LEN 32                   /* SAK Key length */
#define MACSEC_ICV_16_BYTES 16           /* MACsec ICV byte length */
#define PHY10G_ENCR_SEQ_ID_BYTE_POS 61   /* Sequence id byte position in Encrypted packet */
#define PHY10G_PTP_ENCY_NXPN_EXPECT 168  /* Expected non-xpn encrypted data for sequence id field */
#define PHY10G_PTP_ENCY_XPN_EXPECT 88    /* Expected xpn encrypted data for sequence id field */
#define PTP_PKT_SEQ_ID_POSITION 45       /* Sequence id byte position in PTP raw packet */

uint8_t macsec_ports[MAX_PORTS] = {0};   /* MACsec Capable ports connected to EDSx */
uint8_t phy_connected[MAX_PORTS] = {0};  /* Type of PHY Connected to port */
uint8_t num_macsec_ports = 0;            /* Number of MACsec capable ports detected */
uint8_t dst_add[6] = {0xff, 0xff, 0xff, 0xff, 0xff, 0xf1};
uint8_t src_add[6] = {0xff, 0xff, 0xff, 0xff, 0xff, 0xf2};


meba_inst_t meba_phy_instance;
mepa_board_conf_t board_conf = {};

static mscc_appl_trace_module_t trace_module = {
    .name = "kat"
};
static mscc_appl_trace_group_t trace_groups[10] = {
    // TRACE_GROUP_DEFAULT
    {
        .name = "default",
        .level = MESA_TRACE_LEVEL_ERROR
    },
};

/* User input XPN or NON-XPN */
typedef struct {
    mepa_bool_t xpn;
    mepa_bool_t set;
} macsec_kat;


/*Expected PTP encrypted packet NON-XPN */
u8 exp_encr_ptp_nxpn[] = {0xFF,0xFF,0xFF,0xFF,0xFF,0xF1,0xFF,0xFF,0xFF,0xFF,0xFF,0xF2,0x88,0xE5,0x2C,0x00,
                          0x00,0x00,0x00,0x01,0xFF,0xFF,0xFF,0xFF,0xFF,0xF1,0x00,0x01,0xDE,0xAB,0xFB,0x77,
                          0x77,0xE7,0xDA,0x88,0xE3,0x82,0x81,0xDE,0x58,0xB3,0x94,0x9A,0x20,0x1B,0x4A,0x22,
                          0x84,0xAB,0x86,0x76,0x8B,0x5F,0x4C,0xCA,0xD1,0x61,0xF3,0xAC,0xEA,0xA8,0xB7,0x49,
                          0x7C,0x3A,0xB9,0x7D,0x11,0x95,0xA9,0x60,0xDB,0xBC,0x0F,0x4D,0x2A,0xB9,0x25,0x80,
                          0x91,0x81,0xB9,0x9E,0xBC,0x16,0x06,0x07,0x69,0xC8,0xFE,0x2B};

/* Expected PTP encrypted packet XPN */
u8 exp_encr_ptp_xpn[] = {0xFF,0xFF,0xFF,0xFF,0xFF,0xF1,0xFF,0xFF,0xFF,0xFF,0xFF,0xF2,0x88,0xE5,0x2C,0x00,
                         0x00,0x00,0x00,0x01,0xFF,0xFF,0xFF,0xFF,0xFF,0xF1,0x00,0x01,0xC7,0x05,0xAD,0x0F,
                         0x14,0xDF,0xA5,0xD8,0xD2,0xED,0xBB,0xD3,0x38,0x83,0xC7,0x72,0x2A,0xEE,0x5B,0x2C,
                         0x2F,0x6C,0x42,0x9B,0xFF,0xFE,0x81,0x9B,0x1B,0x59,0x3F,0xFC,0xF8,0x58,0xB2,0x59,
                         0x1E,0x2F,0x2B,0x51,0x94,0xBE,0xDC,0x12,0xCD,0x68,0x6F,0xB2,0x4C,0x6B,0x00,0x04,
                         0x25,0x96,0xF5,0x08,0xF2,0x5C,0x16,0x15,0x37,0xE1,0x89,0x9B};

/* Expected IPv4 Encrypted packet NON-XPN */
u8 exp_encr_frame_ipv4[] = {0xff,0xff,0xff,0xff,0xff,0xf1,0xff,0xff,0xff,0xff,0xff,0xf2,0x88,0xe5,0x2c,0x00,
                            0x00,0x00,0x00,0x01,0xff,0xff,0xff,0xff,0xff,0xf1,0x00,0x01,0x5e,0x5c,0xbe,0x75,
                            0x77,0xa0,0xda,0x88,0xe3,0x82,0x7e,0xcf,0x16,0x9c,0x54,0x32,0x56,0x1b,0x8a,0x8a,
                            0xf2,0xaa,0x86,0x76,0x8a,0x1f,0x4c,0x9d,0xd1,0x61,0xf3,0xac,0xea,0xa8,0xb7,0x36,
                            0x7c,0x3a,0xb9,0x7d,0x11,0x95,0xa9,0x60,0xdb,0xbc,0x0f,0x4d,0x7b,0x3d,0xca,0xca,
                            0x4e,0x42,0x0e,0xf0,0xf2,0x7a,0x7a,0x89,0x9c,0x30,0x42,0xea,0xed,0x00,0x2c,0xec,
                            0xa6,0x45,0xdc,0x4b,0x50,0xdc,0x1c,0x82,0xef,0xda,0xc0,0xf2,0x7c,0xa9,0x39,0xf8,
                            0x41,0x11,0xd0,0xd8,0x1f,0xd7,0x55,0x5e,0xdc,0x3e,0x0a,0xd6,0x35,0xf0,0xc5,0x72,
                            0x54,0x74,0xe2,0x94,0x63,0x07,0xa1,0xab,0x57,0x8e,0x35,0x3b,0x02,0x01,0xd0,0xb1,
                            0x39,0x81,0x8b,0xb7,0xf8,0x75,0x37,0x79,0x82}; 

/* Expected IPv4 Encrypted packet XPN */
u8 exp_encr_frame_xpn[] = {0xff,0xff,0xff,0xff,0xff,0xf1,0xff,0xff,0xff,0xff,0xff,0xf2,0x88,0xe5,0x2c,0x00,
                           0x00,0x00,0x00,0x01,0xff,0xff,0xff,0xff,0xff,0xf1,0x00,0x01,0x47,0xf2,0xe8,0x0d,
                           0x14,0x98,0xa5,0xd8,0xd2,0xed,0x44,0xc2,0x76,0xac,0x07,0xda,0x5c,0xee,0x9b,0x84,
                           0x59,0x6d,0x42,0x9b,0xfe,0xbe,0x81,0xcc,0x1b,0x59,0x3f,0xfc,0xf8,0x58,0xb2,0x26,
                           0x1e,0x2f,0x2b,0x51,0x94,0xbe,0xdc,0x12,0xcd,0x68,0x6f,0xb2,0x68,0xf0,0x93,0x33,
                           0xa6,0x54,0x3a,0xc4,0xbd,0x63,0xcb,0x97,0x4f,0x05,0x4e,0xcc,0x60,0x8e,0xa0,0x15,
                           0x2e,0xe7,0xa2,0x6e,0x19,0x65,0xc2,0x63,0xc2,0x38,0xc8,0x88,0xfb,0x82,0x85,0x3d,
                           0xf9,0x0f,0xea,0xc0,0x17,0x56,0x34,0xf1,0xba,0x94,0x79,0x72,0x04,0x86,0x16,0x5d,
                           0x79,0xac,0x42,0xfd,0x8a,0xa1,0xc0,0x0e,0x24,0xe0,0x18,0xc7,0x81,0x33,0x9a,0x18,
                           0x3c,0x12,0x59,0xcd,0xfd,0x06,0x99,0x88,0x21};

/* Expected IPv4 raw packet */
u8 exp_frame_ipv4[] = {0xff,0xff,0xff,0xff,0xff,0xf1,0xff,0xff,0xff,0xff,0xff,0xf2,0x08,0x00,0x45,0x00,
                       0x00,0x6b,0x00,0x00,0x00,0x00,0xff,0x11,0x4e,0x2f,0xc0,0xa8,0x76,0x00,0xc0,0xa8,
                       0x76,0x01,0x00,0x00,0x01,0x40,0x00,0x57,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
                       0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
                       0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
                       0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
                       0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
                       0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00};

/* Expected PTP raw packet */
u8 exp_frame_ptp[] = {0xff,0xff,0xff,0xff,0xff,0xf1,0xff,0xff,0xff,0xff,0xff,0xf2,0x88,0xf7,0x00,0x02,
                      0x00,0x2c,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
                      0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x01,0x00,0x7f,
                      0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00};

u8 exp_frame[153] = {0};

/* Static Key */
uint8_t aes_key_256[] = {10,13,7,10,2,11,13,0,3,14,10,12,8,3,5,10,6,15,6,2,0,15,13,12,11,5,0,6,11,3,4,5};
uint8_t hash_key_256[] = {115,162,61,128,18,29,226,213,168,80,37,63,207,67,18,14};
uint8_t salt_xpn[] = {0,1,2,3,4,5,6,7,8,9,0,1};
uint8_t short_sci[]= {0,0,0,1};

const mepa_mac_t port_macaddress = { .addr = {0xff, 0xff, 0xff, 0xff, 0xff, 0xf2}};
const mepa_mac_t peer_macaddress = { .addr = {0xff, 0xff, 0xff, 0xff, 0xff, 0xf1}};


static int loopback_conf (BOOL enable) {

    mepa_rc rc;
    mepa_loopback_t loopback;
    BOOL viper_connected = 0;

    /* M10G loopback */
    vtss_phy_10g_loopback_t loopback_10g;
    loopback_10g.lb_type = VTSS_LB_H3;
    loopback_10g.enable = enable ? TRUE : FALSE;

    /* 1G and 25G loopback */
    memset(&loopback, 0, sizeof(mepa_loopback_t));
    loopback.near_end_ena = enable ? TRUE : FALSE;
    
    for(int i = 0 ; i < num_macsec_ports; i++) {
        if(phy_connected[i] == PHY_IS_1G || phy_connected[i] == PHY_IS_25G) {
            if((rc = mepa_loopback_set(meba_phy_instance->phy_devices[macsec_ports[i]], &loopback)) != MEPA_RC_OK) {
                T_E("\n Error in loopback configuration on port %d\n", macsec_ports[i]);
                return MEPA_RC_ERROR;
            }
            if(phy_connected[i] == PHY_IS_1G) {
                viper_connected = TRUE;
            }
        } else if(phy_connected[i] == PHY_IS_10G) {
            if((rc = vtss_phy_10g_loopback_set(NULL, macsec_ports[i], &loopback_10g)) != MEPA_RC_OK) {
                T_E("\n Error in loopback configuration on port %d\n", macsec_ports[i]);
                return MEPA_RC_ERROR;
            }
        }
    }
    /* Wait for 2 sec */
    /* EPG Packet generator in Viper PHY is Expecting this delay after loopback to
     * generate the packet
     */
    if(viper_connected) {
        usleep(2000000);
    }
    T_D("\n Loopback %s on all MACsec capable ports\n", enable?"enabled" : "disabled");
    return MEPA_RC_OK;
}

static int  port_secy_del_macsec_dis() {

    mepa_macsec_port_t       macsec_port;
    macsec_port.port_id = 1;
    macsec_port.service_id = 0;
    mepa_macsec_secy_conf_t secy_conf;
    mepa_rc rc;
    for(int i = 0; i < num_macsec_ports; i++) {
        macsec_port.port_no = macsec_ports[i];
        /* Checking SecY is created or not if yes Delete that SecY */
        if((rc = mepa_macsec_secy_conf_get(meba_phy_instance->phy_devices[macsec_ports[i]], macsec_port, &secy_conf)) == MEPA_RC_OK) {
            if((rc = mepa_macsec_secy_conf_del(meba_phy_instance->phy_devices[macsec_ports[i]], macsec_port)) != MEPA_RC_OK) {
                T_E("\n Error in Delecting the SecY on port : %d\n", macsec_ports[i]);
                return MEPA_RC_ERROR;
            }
        }
        mepa_macsec_init_t init_data = { .enable = FALSE,
                                         .dis_ing_nm_macsec_en = TRUE,
                                         .mac_conf.lmac.dis_length_validate = FALSE,
                                         .mac_conf.hmac.dis_length_validate = FALSE,
                                         .bypass = MEPA_MACSEC_INIT_BYPASS_NONE };

        if ((rc = mepa_macsec_init_set(meba_phy_instance->phy_devices[macsec_ports[i]], &init_data)) != MEPA_RC_OK) {
            T_E("\n Error in Disabling MACsec on port : %d\n", macsec_ports[i]);
            return MEPA_RC_ERROR;
        }
    }
    T_D("\n MACsec Disabled on all ports");
    return MEPA_RC_OK;
}

static int port_macsec_encryption_test(cli_req_t *req) {

    mepa_rc rc;
    macsec_kat *mreq = req->module_req;
    uint16_t assoc_no = 0;
    BOOL confidentiality = 1;
    uint32_t next_pack_no = 0;
    uint32_t frame_length = 0;
    uint8_t  frame[MEPA_MACSEC_FRAME_CAPTURE_SIZE_MAX];
    uint32_t buf_length = MEPA_MACSEC_FRAME_CAPTURE_SIZE_MAX;
    BOOL egr_fail = FALSE;
    BOOL skip_seq_id = FALSE;

    mepa_macsec_port_t  macsec_port;
    macsec_port.port_id = 1;
    macsec_port.service_id = 0;

    T_D("\n Performing Encryption Test \n");
    for(int i = 0; i < num_macsec_ports; i++) {

        macsec_port.port_no = macsec_ports[i];
        /* MACsec Block Enable */
        mepa_macsec_init_t init_data = { .enable = TRUE,
                                         .dis_ing_nm_macsec_en = TRUE,
                                         .mac_conf.lmac.dis_length_validate = FALSE,
                                         .mac_conf.hmac.dis_length_validate = FALSE,
                                         .bypass = MEPA_MACSEC_INIT_BYPASS_NONE };

        if ((rc = mepa_macsec_init_set(meba_phy_instance->phy_devices[macsec_ports[i]], &init_data)) != MEPA_RC_OK) {
            T_E("\n Error in configuring MACsec init set on port : %d\n", macsec_ports[i]);
            return MEPA_RC_ERROR;
        }

        /* Default action configuration for non-matching frames */
        mepa_macsec_default_action_policy_t default_action_policy = {
            .ingress_non_control_and_non_macsec = MEPA_MACSEC_DEFAULT_ACTION_BYPASS,
            .ingress_control_and_non_macsec     = MEPA_MACSEC_DEFAULT_ACTION_BYPASS,
            .ingress_non_control_and_macsec     = MEPA_MACSEC_DEFAULT_ACTION_BYPASS,
            .ingress_control_and_macsec         = MEPA_MACSEC_DEFAULT_ACTION_BYPASS,
            .egress_control                     = MEPA_MACSEC_DEFAULT_ACTION_BYPASS,
            .egress_non_control                 = MEPA_MACSEC_DEFAULT_ACTION_BYPASS,
        };

        if ((rc = mepa_macsec_default_action_set(meba_phy_instance->phy_devices[macsec_ports[i]], macsec_ports[i], &default_action_policy)) != MEPA_RC_OK) {
            T_E("\n Error in configuration Default action set on port :%d \n", macsec_ports[i]);
            return MEPA_RC_ERROR;
        }

        /* Creating new SecY */
        mepa_macsec_secy_conf_t secy_conf;
        secy_conf.validate_frames        = MEPA_MACSEC_VALIDATE_FRAMES_STRICT;
        secy_conf.replay_protect         = FALSE;
        secy_conf.replay_window          = 0;
        secy_conf.protect_frames         = TRUE;
        secy_conf.always_include_sci     = TRUE;
        secy_conf.use_es                 = FALSE;
        secy_conf.use_scb                = FALSE;
        secy_conf.confidentiality_offset = 0;
        secy_conf.mac_addr               = peer_macaddress;
	
        if(mreq->xpn) {
            secy_conf.current_cipher_suite   = MEPA_MACSEC_CIPHER_SUITE_GCM_AES_XPN_256;
        } else {
            secy_conf.current_cipher_suite   = MEPA_MACSEC_CIPHER_SUITE_GCM_AES_256;
        }

        if ((rc = mepa_macsec_secy_conf_add(meba_phy_instance->phy_devices[macsec_ports[i]], macsec_port, &secy_conf)) != MEPA_RC_OK) {
            T_E("\n Error in creating the SecY on port : %d \n", macsec_ports[i]);
            return MEPA_RC_ERROR;
        }

        /* Pattern Matching rule */
        mepa_macsec_match_pattern_t pattern_match;
        pattern_match.priority            = MEPA_MACSEC_MATCH_PRIORITY_HIGH;
        pattern_match.match               = MEPA_MACSEC_MATCH_ETYPE;
        pattern_match.is_control          = TRUE;
        pattern_match.has_vlan_tag        = FALSE;
        pattern_match.has_vlan_inner_tag  = FALSE;
        if(phy_connected[i] == PHY_IS_1G) {
            pattern_match.etype           = IPV4_ETHERTYPE;
        } else {
            pattern_match.etype           = PTP_ETHERTYPE;
        }
        pattern_match.vid                 = 0;
        pattern_match.vid_inner           = 0;

        for (int j = 0; j < 8; j++) {
            pattern_match.hdr[j]          = 0;
            pattern_match.hdr_mask[j]     = 0;
        }
        pattern_match.src_mac             = port_macaddress;
        pattern_match.dest_mac            = peer_macaddress;

        if ((rc = mepa_macsec_pattern_set(meba_phy_instance->phy_devices[macsec_ports[i]], macsec_port, MEPA_MACSEC_DIRECTION_EGRESS,
                                              MEPA_MACSEC_MATCH_ACTION_CONTROLLED_PORT, &pattern_match)) != MEPA_RC_OK) {
            T_E("\n Error in Configuring Pattern matching rule on port : %d \n", macsec_ports[i]);
            return MEPA_RC_ERROR;
        }

        /* SecY Control port enable */
        if((rc = mepa_macsec_secy_controlled_set(meba_phy_instance->phy_devices[macsec_ports[i]], macsec_port, TRUE)) != MEPA_RC_OK) {
            T_E("\n Error in enabling the SecY controlled port on port : %d \n", macsec_ports[i]);
            return MEPA_RC_ERROR;
        }

        /* Tx Secure channel Creation */
        if((rc = mepa_macsec_tx_sc_set(meba_phy_instance->phy_devices[macsec_ports[i]], macsec_port)) != MEPA_RC_OK) {
            T_E("\n Error in creating Transmit Secure channel on port : %d\n", macsec_ports[i]);
            return MEPA_RC_ERROR;
        }

        mepa_macsec_sak_t sak;
        memcpy(sak.buf, aes_key_256, sizeof(aes_key_256));
        memcpy(sak.h_buf, hash_key_256, sizeof(hash_key_256));
        memcpy(sak.salt.buf, salt_xpn, sizeof(salt_xpn));
        sak.len = SAK_KEY_LEN;

        mepa_macsec_ssci_t ssci;
        memcpy(ssci.buf, short_sci, sizeof(short_sci));

        /* Expected Packet number in the 1st Encrypted PAcket */
        next_pack_no = 1;
        mepa_macsec_pkt_num_t next_pn;
        next_pn.pn = 1;
        next_pn.xpn = 1;

        if(mreq->xpn) {
            if((rc = mepa_macsec_tx_seca_set(meba_phy_instance->phy_devices[macsec_ports[i]], macsec_port, assoc_no, next_pn, confidentiality,
                                             &sak, &ssci)) != MEPA_RC_OK) {
                T_E("Error is creating Tx Secure asoosiation on port : %d\n", macsec_ports[i]);
                return MEPA_RC_ERROR;
            }
        } else {
            if((rc = mepa_macsec_tx_sa_set(meba_phy_instance->phy_devices[macsec_ports[i]], macsec_port,assoc_no,next_pack_no, confidentiality, &sak)) != MEPA_RC_OK) {
                T_E("Error is creating Tx Secure asoosiation on port %d\n", macsec_ports[i]);
                return MEPA_RC_ERROR;
            }
        }

        if((rc = mepa_macsec_tx_sa_activate(meba_phy_instance->phy_devices[macsec_ports[i]], macsec_port, assoc_no)) != MEPA_RC_OK) {
            T_E("\nError in Activating Tx Secure Assosiation on port : %d\n", macsec_ports[i]);
            return MEPA_RC_ERROR;
        }
    
        /* Egress Frame capture set */
        if((rc = mepa_macsec_frame_capture_set(meba_phy_instance->phy_devices[macsec_ports[i]], macsec_ports[i], MEPA_MACSEC_FRAME_CAPTURE_EGRESS)) != MEPA_RC_OK) {
            T_E("\nFrame capture set to Egress error on port %d\n", macsec_ports[i]);
            return MEPA_RC_ERROR;
        }

        /* Send Ethernet frame from the Packet generator */
        if(phy_connected[i] == PHY_IS_1G) {
            if((rc = vtss_phy_epg_gen_kat_frame(NULL, macsec_ports[i], FALSE)) != MEPA_RC_OK) {
                T_E("\n Error in generating IPv4 frame on port : %d\n", macsec_ports[i]);
                return MEPA_RC_ERROR;
            }
            /* Expected Encrypted frame */
            if(mreq->xpn) {
                memcpy(exp_frame, exp_encr_frame_xpn, sizeof(exp_encr_frame_xpn));
            } else {
               memcpy(exp_frame, exp_encr_frame_ipv4, sizeof(exp_encr_frame_ipv4));
            }
        } else if(phy_connected[i] == PHY_IS_10G) {
            vtss_phy_10g_pkt_gen_conf_t pkt_gen_conf = { 	
                     .enable       = TRUE,
                     .ptp          = TRUE,
                     .ingress      = FALSE,
                     .frames       = TRUE,
                     .frame_single = TRUE,
                     .etype        = PTP_ETHERTYPE,
                     .pkt_len      = 1,
                     .ipg_len      = 0,
                     .ptp_ts_sec   = 0,
                     .ptp_ts_ns    = 0,
                     .srate        = 0,
            };
	    memcpy(pkt_gen_conf.smac, src_add, sizeof(src_add));
	    memcpy(pkt_gen_conf.dmac, dst_add, sizeof(dst_add));

            if((rc = vtss_phy_10g_pkt_gen_conf(NULL, macsec_ports[i], &pkt_gen_conf)) != MEPA_RC_OK) {
                T_E("\n Error in generating PTP packet on port : %d\n", macsec_ports[i]);
                return MEPA_RC_ERROR;
            }
            printf("PTP sync message 60 Byte Packet is generated on port no : %d\n", macsec_ports[i]);
            if(mreq->xpn) {
                memcpy(exp_frame, exp_encr_ptp_xpn, sizeof(exp_encr_ptp_xpn));
            } else {
                memcpy(exp_frame, exp_encr_ptp_nxpn, sizeof(exp_encr_ptp_xpn));
            }
        }

        /* Frame Get */
        if((rc = mepa_macsec_frame_get(meba_phy_instance->phy_devices[macsec_ports[i]], macsec_ports[i], buf_length, &frame_length, &frame[0])) != MEPA_RC_OK) {
            T_E("\n Error in reading MACsec FIFO on Port : %d\n", macsec_ports[i]);
            return MEPA_RC_ERROR;
        }
        if(frame_length == 0) {
            egr_fail = TRUE;
            T_E("\nNo Frame is captured in MACsec FIFO on port : %d\n", macsec_ports[i]);
        }
        T_I("Captured encrypted frame length on port %d is:%d\n", macsec_ports[i],frame_length);

        /* Workarround for Malibu10G due to hardware limitations
         * The PKT_BIST in M10G generated PTP packet with seqence id field incrementing for each packet
         * For the first time when PKT_BIST generating packet with sequence id = 0 the KAT Demo will compare the 
         * entire encrypted packet after that while comparing it ommits 16 byte ICV and 1 byte Secuence id Field in Encrypte packet
         */
        if(phy_connected[i] == PHY_IS_10G && frame_length != 0) {
            if((mreq->xpn != 1 && frame[PHY10G_ENCR_SEQ_ID_BYTE_POS] != PHY10G_PTP_ENCY_NXPN_EXPECT) ||
               (mreq->xpn == 1 && frame[PHY10G_ENCR_SEQ_ID_BYTE_POS] != PHY10G_PTP_ENCY_XPN_EXPECT)) {

                frame_length = frame_length - MACSEC_ICV_16_BYTES;
                skip_seq_id = TRUE;
            }
        }
        T_I("Comparing Frame length on port %d is: %d\n\n", macsec_ports[i], skip_seq_id ? frame_length - 1 : frame_length);

        for(int j = 0; j < frame_length; j++) {
            if(skip_seq_id && j == PHY10G_ENCR_SEQ_ID_BYTE_POS) {
                skip_seq_id = FALSE;
                continue;
            }
            if(frame[j] != exp_frame[j]) {
                cli_printf("\nERROR : Encrypted Frame doesnot match with Expected Frame on %d port at %d byte\n, expected %d but value %d",
                           macsec_ports[i], j, exp_frame[j], frame[j]);
                egr_fail = TRUE;
            }
        }
    }

    if(egr_fail) {
        T_E("\n!!!!!!!!! Encryption Test Failed !!!!!!!!!!!\n\n");
        return MEPA_RC_ERROR;
    }

    if(mreq->xpn) {
        cli_printf("\n !!!!!!!!!!!!!!!!!!! XPN Encryption test passed on all MACsec Capable ports !!!!!!!!!!!!\n");
    } else {
        cli_printf("\n !!!!!!!!!!!!!!!!!!! NON - XPN Encryption test passed on all MACsec Capable ports !!!!!!!!!!!!\n");
    }
    return MEPA_RC_OK;    
}

static int port_macsec_decryption_test(cli_req_t *req) {

    mepa_rc rc;
    mepa_macsec_sci_t sci;
    BOOL ingr_fail = FALSE;
    uint16_t assoc_no = 0;
    uint32_t next_pack_no = 0;
    uint32_t frame_length = 0;
    uint8_t  frame[MEPA_MACSEC_FRAME_CAPTURE_SIZE_MAX];
    uint32_t buf_length = MEPA_MACSEC_FRAME_CAPTURE_SIZE_MAX;
    BOOL skip_seq_id = FALSE;

    macsec_kat *mreq = req->module_req;
    mepa_macsec_port_t  macsec_port;
    macsec_port.port_id = 1;
    macsec_port.service_id = 0;

    T_D("\n Performing Decryption Test \n");
    for(int i = 0; i < num_macsec_ports; i++) {

        macsec_port.port_no = macsec_ports[i];
        /* Pattern Matching rule */
        mepa_macsec_match_pattern_t pattern_match;
        pattern_match.priority            = MEPA_MACSEC_MATCH_PRIORITY_HIGH;
        pattern_match.match               = MEPA_MACSEC_MATCH_ETYPE;
        pattern_match.is_control          = FALSE;
        pattern_match.has_vlan_tag        = FALSE;
        pattern_match.has_vlan_inner_tag  = FALSE;
        pattern_match.etype               = MACSEC_ETHERTYPE;
        pattern_match.vid                 = 0;
        pattern_match.vid_inner           = 0;

        for (int j = 0; j < 8; j++) {
            pattern_match.hdr[j]          = 0;
            pattern_match.hdr_mask[j]     = 0;
        }
        pattern_match.src_mac             = port_macaddress;
        pattern_match.dest_mac            = peer_macaddress;

        if ((rc = mepa_macsec_pattern_set(meba_phy_instance->phy_devices[macsec_ports[i]], macsec_port, MEPA_MACSEC_DIRECTION_INGRESS,
                                          MEPA_MACSEC_MATCH_ACTION_CONTROLLED_PORT, &pattern_match)) != MEPA_RC_OK) {

            T_E("\n Error in Pattern matching rule configuration ingress on port : %d\n", macsec_ports[i]);
            return MEPA_RC_ERROR;
        }
        sci.mac_addr = peer_macaddress;
        sci.port_id = 1;

        /* Rx Secure channel Creation */
        if((rc = mepa_macsec_rx_sc_add(meba_phy_instance->phy_devices[macsec_ports[i]], macsec_port, &sci)) != MEPA_RC_OK) {
            T_E("\n Error in Rx Secure Channel Create on port : %d\n", macsec_ports[i]);
            return MEPA_RC_ERROR;
        }

        mepa_macsec_sak_t sak;
        memcpy(sak.buf, aes_key_256, sizeof(aes_key_256));
        memcpy(sak.h_buf, hash_key_256, sizeof(hash_key_256));
        memcpy(sak.salt.buf, salt_xpn, sizeof(salt_xpn));
        sak.len = SAK_KEY_LEN;

        mepa_macsec_ssci_t ssci;
        memcpy(ssci.buf, short_sci, sizeof(short_sci));

        /* Expected Packet Number for Decryption
         * Viper 1G will generate 300 IPv4 frames for Encryption Test
         * but M10G and M25G will generate only 1 PTP frame */
        mepa_macsec_pkt_num_t next_pn;
        if(phy_connected[i] == PHY_IS_1G) {
            next_pack_no = next_pn.pn = 301;
            next_pn.xpn = 301;
        } else {
            next_pack_no = next_pn.pn = 2;
            next_pn.xpn = 2;
        }

        if(mreq->xpn) {
            if((rc = mepa_macsec_rx_seca_set(meba_phy_instance->phy_devices[macsec_ports[i]], macsec_port, &sci, assoc_no, next_pn, &sak, &ssci)) != MEPA_RC_OK) {
                T_E("\n Error is creating Rx Secure Assosiation on port : %d\n", macsec_ports[i]);
                return MEPA_RC_ERROR;
            }
        } else {
            if((rc = mepa_macsec_rx_sa_set(meba_phy_instance->phy_devices[macsec_ports[i]], macsec_port, &sci, assoc_no, next_pack_no , &sak)) != MEPA_RC_OK) {
                T_E("\n Error is Creating Rx Secure Assosiation on port :%d\n", macsec_ports[i]);
                return MEPA_RC_ERROR;
            }
        } 

        if((rc = mepa_macsec_rx_sa_activate(meba_phy_instance->phy_devices[macsec_ports[i]], macsec_port, &sci, assoc_no)) != MEPA_RC_OK) {
            T_E("\n Error in activating Rx Secure Assosiation on port : %d\n", macsec_ports[i]);
            return MEPA_RC_ERROR;
        }

        /* Egress Frame capture set */
        if((rc = mepa_macsec_frame_capture_set(meba_phy_instance->phy_devices[macsec_ports[i]], macsec_ports[i], MEPA_MACSEC_FRAME_CAPTURE_INGRESS)) != MEPA_RC_OK) {
            T_E("\n Frame capture set for ingress on port %d\n", macsec_ports[i]);
            return MEPA_RC_ERROR;
        }

        /* Send Ethernet frame from the PAcket generator */
        if(phy_connected[i] == PHY_IS_1G) {
            if((rc = vtss_phy_epg_gen_kat_frame(NULL, macsec_ports[i], FALSE)) != MEPA_RC_OK) {
                T_E("\n Error in frame generation on port : %d", macsec_ports[i]);
                return MEPA_RC_ERROR;
            }
            memcpy(exp_frame, exp_frame_ipv4, sizeof(exp_frame_ipv4));
        } else if(phy_connected[i] == PHY_IS_10G) {
            vtss_phy_10g_pkt_gen_conf_t pkt_gen_conf = {
                     .enable       = TRUE,
                     .ptp          = TRUE,
                     .ingress      = FALSE,
                     .frames       = TRUE,
                     .frame_single = TRUE,
                     .etype        = PTP_ETHERTYPE,
                     .pkt_len      = 1,
                     .ipg_len      = 0,
                     .ptp_ts_sec   = 0,
                     .ptp_ts_ns    = 0,
                     .srate        = 0,
            };
            memcpy(pkt_gen_conf.smac, src_add, sizeof(src_add));
            memcpy(pkt_gen_conf.dmac, dst_add, sizeof(dst_add));

            if((rc = vtss_phy_10g_pkt_gen_conf(NULL, macsec_ports[i], &pkt_gen_conf)) != MEPA_RC_OK) {
                T_E("\n Error in generating PTP packet on port : %d\n", macsec_ports[i]);
                return MEPA_RC_ERROR;
            }
            printf("PTP sync message 60 Byte Packet is generated on port no : %d\n", macsec_ports[i]);
            memcpy(exp_frame, exp_frame_ptp, sizeof(exp_frame_ptp));
        }

        /* Frame Get */
        if((rc = mepa_macsec_frame_get(meba_phy_instance->phy_devices[macsec_ports[i]], macsec_ports[i], buf_length, &frame_length, &frame[0])) != MEPA_RC_OK) {
            T_E("\n Error in Frame get \n");
            return MEPA_RC_ERROR;
        }
        if(frame_length == 0) {
            ingr_fail = TRUE;
            T_E("\n No Frame is captured in MACsec FIFO on port : %d\n", macsec_ports[i]);
        }
        T_I("Captured decrypted frame length on port %d is:%d\n", macsec_ports[i],frame_length);

        /* Workarround to skip the Sequence id comparision in Decrypted PTP packet */
        if(phy_connected[i] == PHY_IS_10G && frame[PTP_PKT_SEQ_ID_POSITION] != 1) {
            skip_seq_id = TRUE;
        }

        T_I("Comparing frame length on port %d is:%d\n\n", macsec_ports[i], skip_seq_id ? frame_length - 1 : frame_length); 
        for(int j = 0; j < frame_length; j++) {
            if(skip_seq_id && j == PTP_PKT_SEQ_ID_POSITION) {
                continue;
            }

            if(frame[j] != exp_frame[j]) {
                cli_printf("\nERROR : Decrypted Frame doesnot match with Expected Frame on %d port at %d byte : expected %x but value %x",
                           macsec_ports[i], j, exp_frame[j], frame[j]);
                ingr_fail = TRUE;
            }
        }
    }

    if(ingr_fail) {
        T_E("\n!!!!! Decryption Test Failed !!!!!\n\n");
        return MEPA_RC_ERROR;
    }

    if(mreq->xpn) {
        cli_printf("\n!!!!!!!!!!!!!!!!!!! XPN Decryption test passed on all MACsec Capable ports !!!!!!!!!!!!");
    } else {
        cli_printf("\n!!!!!!!!!!!!!!!!!!! NON XPN Decryption test passed on all MACsec Capable ports !!!!!!!!!!!!");
    }
    return MEPA_RC_OK;
}

static int port_phy_scan(){
    mepa_rc rc;
    mepa_bool_t macsec_capable;
    mepa_phy_info_t phy_info;
    u8 phy_type = 0;

    for(int port_no = 0; port_no < MAX_PORTS; port_no++) {
	if(!meba_phy_instance->phy_devices[port_no]) {
            continue;
        }
        if((rc = mepa_macsec_is_capable(meba_phy_instance->phy_devices[port_no], port_no, &macsec_capable)) != MEPA_RC_OK) {
            continue;
        }
        if(macsec_capable == FALSE) {
            continue;
        }
        macsec_ports[num_macsec_ports] = port_no;
        mepa_phy_info_get(meba_phy_instance->phy_devices[port_no], &phy_info);
        if(phy_info.cap & MEPA_CAP_SPEED_MASK_1G) {
            phy_type = PHY_IS_1G;
        } else if(phy_info.cap & MEPA_CAP_SPEED_MASK_10G) {
            phy_type = PHY_IS_10G;
        } else if(phy_info.cap & 0x8) {
            phy_type = PHY_IS_25G;
        }
        phy_connected[num_macsec_ports] = phy_type;
        num_macsec_ports++;
    }
    printf("\n Number of MACsec capable ports detected : %d \n\n", num_macsec_ports);
    if(num_macsec_ports == 0) {
        cli_printf("\n No MACsec capable PHY's are connected\n");
        return MEPA_RC_ERROR;
    }
    return MEPA_RC_OK;
} 

static void cli_cmd_kat_demo(cli_req_t *req)
{
    macsec_kat *mreq = req->module_req;

    if(!mreq->set) {
         printf("\n Syntax : mepa-cmd kat demo <xpn|non-xpn> \n");
         T_E("\n Provide Paramter xpn or non-xpn \n");
         return;
    }

    /* PHY scan on all Ports to get MACsec capable PHYs */ 
    if(port_phy_scan() != MEPA_RC_OK) {
        T_E("\n No MACsec engine test performed");
        goto macsec_port_cnt;
    }

    /* Enable Loopback on the LINE side of PHY */
    if(loopback_conf(TRUE) != MEPA_RC_OK) {
        T_E("\nError in enabling loopback configuration \n");
        goto loopback_dis;
    }

    /* Encryption Test on MACsec capable ports */
    if(port_macsec_encryption_test(req) != MEPA_RC_OK) {
        T_E("\n Encryption Test Failed \n");
        goto dis_macsec;
    }

    /* Decryption Test on MACsec capable ports */
    if(port_macsec_decryption_test(req) != MEPA_RC_OK) {
        T_E("\n Decryption Test Failed \n");
    }

dis_macsec:
    /* Disabling MACsec */
    if(port_secy_del_macsec_dis() != MEPA_RC_OK) {
        T_E("\n Error in Disabling the MACsec \n");
        goto macsec_port_cnt;
    }

loopback_dis:
    /* Disabling the Loopback configured */
    if(loopback_conf(FALSE) != MEPA_RC_OK) {
        T_E("\nError in disabling loopback configuration \n");
    }

macsec_port_cnt:
    num_macsec_ports = 0;
    return;
}


static int cli_param_parse(cli_req_t *req)
{
    macsec_kat *mreq = req->module_req;
    const char     *found;
    if ((found = cli_parse_find(req->cmd, req->stx)) == NULL)
        return 1;

    if (!strncasecmp(found, "xpn", 3)) {
        mreq->xpn = 1;
        mreq->set = 1;
    } else if (!strncasecmp(found, "non-xpn", 7)) {
        mreq->xpn = 0;
        mreq->set = 1;
    } else {
        cli_printf("no match: %s\n", found);
    }
    return 0;
}

static cli_cmd_t cli_cmd_table[] = {
    {
        "kat demo [xpn|non-xpn]",
        "Known answer test for MACsec",
        cli_cmd_kat_demo,
    },
};

static cli_parm_t cli_parm_table[] = {
    {
        "xpn|non-xpn",
        "XPN Encrytion os Non-XPN",
        CLI_PARM_FLAG_SET,	
	cli_param_parse
    },
};

static void phy_cli_init(void)
{
    int i;

    /* Register CLI Commands */
    for (i = 0; i < sizeof(cli_cmd_table)/sizeof(cli_cmd_t); i++) {
        mscc_appl_cli_cmd_reg(&cli_cmd_table[i]);
    }

    /* Register CLI Params */
    for (i = 0; i < sizeof(cli_parm_table)/sizeof(cli_parm_t); i++) {
	mscc_appl_cli_parm_reg(&cli_parm_table[i]);
    }
}


void mscc_appl_kat_demo(mscc_appl_init_t *init)
{
    meba_phy_instance = init->board_inst;
    switch(init->cmd) {
    case MSCC_INIT_CMD_INIT:
        phy_cli_init();
        break;
    default:
        break;
    }
}

