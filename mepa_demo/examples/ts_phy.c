// Copyright (c) 2004-2020 Microchip Technology Inc. and its subsidiaries.
// SPDX-License-Identifier: MIT


#include <unistd.h>
#include <stdio.h>
#include "cli.h"
#include "example.h"
#include <arpa/inet.h>
#include "microchip/ethernet/switch/api.h"
extern meba_inst_t meba_global_inst;

static uint8_t                src_mac[6] = {0,0,0,0,0,1};
static uint8_t                dst_mac[6] = {1,27,25,0,0,0};

static const char *phy_help_txt = "\
This is Timestamping PHY test.\n\
\n\
The SYNC frame is transmitted on a loop port and copied to CPU at reception.\n\
The log information is calculated based on received frame content.\n\
";

static struct {
    uint32_t                 family;
    mesa_port_no_t           tx_port;
    mesa_port_no_t           rx_port;
} state;

#define MACSEC_ENABLE (1)
#define MACSEC_DISABLE (0)
static int phy_macsec(const mepa_port_no_t port_no, const mepa_bool_t enable) {
    mepa_macsec_init_t macsec_init;
    macsec_init.enable = enable;
    macsec_init.dis_ing_nm_macsec_en = 1;
    macsec_init.mac_conf.lmac.dis_length_validate = 0;
    macsec_init.mac_conf.hmac.dis_length_validate = 0;
	macsec_init.bypass = MEPA_MACSEC_INIT_BYPASS_NONE;
    /* Init_set */
    if (meba_phy_macsec_init_set(meba_global_inst, port_no, &macsec_init) != MEPA_RC_OK) {
        cli_printf("macsec initialization port(%u) failed for enabling macsec\n", port_no);
        return MEPA_RC_ERROR;
    }
    macsec_init.bypass = MEPA_MACSEC_INIT_BYPASS_ENABLE;
	/* MACsec Bypass mode enabled */
    if (meba_phy_macsec_init_set(meba_global_inst, port_no, &macsec_init) != MEPA_RC_OK) {
        cli_printf("macsec initialization port(%u) failed for enabling macsec bypass\n", port_no);
        return MEPA_RC_ERROR;
    }
    return 0;
}


/* TS initialization ingress/egress configuration */
static int phy_ts_egress_init(const mepa_port_no_t port_no, 
		              const mepa_ts_pkt_encap_t encap,
			      const mepa_ts_ptp_clock_mode_t ck_mode) {
  
    mepa_ts_init_conf_t phy_conf;
    memset(&phy_conf, 0,sizeof(mepa_ts_init_conf_t));
    phy_conf.clk_freq = MEPA_TS_CLOCK_FREQ_125M;
    phy_conf.clk_src = MEPA_TS_CLOCK_SRC_EXTERNAL;
    phy_conf.rx_ts_pos = MEPA_TS_RX_TIMESTAMP_POS_IN_PTP;
    phy_conf.rx_ts_len = MEPA_TS_RX_TIMESTAMP_LEN_30BIT;
    phy_conf.tx_fifo_mode = MEPA_TS_FIFO_MODE_NORMAL;
    phy_conf.tx_ts_len = MEPA_TS_FIFO_TIMESTAMP_LEN_4BYTE;
    phy_conf.tc_op_mode = MEPA_TS_TC_OP_MODE_A;    
    if(meba_phy_ts_init_conf_set(meba_global_inst, port_no, &phy_conf) != MEPA_RC_OK){
        cli_printf("ts initialization port(%u) failed, n: %u\n", port_no);
	return MEPA_RC_ERROR;
    }
    mepa_ts_classifier_t cs_conf;
    memset(&cs_conf, 0,sizeof(mepa_ts_classifier_t));
    cs_conf.enable = 1,
    cs_conf.pkt_encap_type = encap;
    cs_conf.clock_id = 0;
    cs_conf.eth_class_conf.mac_match_mode = MEPA_TS_ETH_ADDR_MATCH_ANY;
    cs_conf.eth_class_conf.mac_match_select = MEPA_TS_ETH_MATCH_DEST_ADDR;
    memcpy(&cs_conf.eth_class_conf.mac_addr, dst_mac, sizeof(src_mac));
    cs_conf.eth_class_conf.vlan_check = 0;

    if (encap == MEPA_TS_ENCAP_ETH_IP_PTP) { 
        cs_conf.ip_class_conf.ip_ver = MEPA_TS_IP_VER_4;
        cs_conf.ip_class_conf.ip_match_mode = MEPA_TS_IP_MATCH_SRC;  /**<  match src, dest or either IP address */
        cs_conf.ip_class_conf.udp_sport_en = 0;   /**<  UDP Source port check enable */
        cs_conf.ip_class_conf.udp_dport_en = 0;   /**<  UDP Dest port check enable */
        cs_conf.ip_class_conf.udp_dport = 319;
    }
    cs_conf.eth_class_conf.vlan_conf.etype = (encap == MEPA_TS_ENCAP_ETH_IP_PTP) ? 0x0800: 0x88f7;
    if(meba_phy_ts_tx_classifier_conf_set(meba_global_inst, port_no, 0, &cs_conf) != MEPA_RC_OK){ 
        cli_printf("ts classifier port(%u) failed, n: %u\n", port_no);
        return MEPA_RC_ERROR;
    }
    mepa_ts_ptp_clock_conf_t ptp_conf;
    memset(&ptp_conf, 0, sizeof(ptp_conf));
    ptp_conf.enable = 1;
    ptp_conf.clk_mode = ck_mode;
    ptp_conf.delaym_type = MEPA_TS_PTP_DELAYM_E2E;                                                       
    if(meba_phy_ts_tx_clock_conf_set(meba_global_inst, port_no, 0, &ptp_conf) != MEPA_RC_OK){
        cli_printf("ts classifier port(%u) failed, n: %u\n", port_no);
        return MEPA_RC_ERROR;
    }
    return MEPA_RC_OK;
}

static int phy_ts_ingress_init(const mepa_port_no_t port_no, 
		               const mepa_ts_pkt_encap_t encap,
			       const mepa_ts_ptp_clock_mode_t ck_mode) {

    mepa_ts_init_conf_t phy_conf;
    memset(&phy_conf, 0,sizeof(mepa_ts_init_conf_t));
    phy_conf.clk_freq = MEPA_TS_CLOCK_FREQ_125M;
    phy_conf.clk_src = MEPA_TS_CLOCK_SRC_EXTERNAL;
    phy_conf.rx_ts_pos = MEPA_TS_RX_TIMESTAMP_POS_IN_PTP;
    phy_conf.rx_ts_len = MEPA_TS_RX_TIMESTAMP_LEN_30BIT;
    phy_conf.tx_fifo_mode = MEPA_TS_FIFO_MODE_NORMAL;
    phy_conf.tx_ts_len = MEPA_TS_FIFO_TIMESTAMP_LEN_4BYTE;
    phy_conf.tc_op_mode = MEPA_TS_TC_OP_MODE_A;
    if(meba_phy_ts_init_conf_set(meba_global_inst, port_no, &phy_conf) != MEPA_RC_OK){
        cli_printf("ts initialization port(%u) failed, n: %u\n", port_no);
        return MEPA_RC_ERROR;
    }
    mepa_ts_classifier_t cs_conf;
    memset(&cs_conf, 0,sizeof(mepa_ts_classifier_t));
    cs_conf.enable = 1,
    cs_conf.pkt_encap_type = encap;
    cs_conf.clock_id = 0;
    cs_conf.eth_class_conf.mac_match_mode = MEPA_TS_ETH_ADDR_MATCH_ANY;
    cs_conf.eth_class_conf.mac_match_select = MEPA_TS_ETH_MATCH_DEST_ADDR;
    memcpy(&cs_conf.eth_class_conf.mac_addr, dst_mac, sizeof(dst_mac));
    cs_conf.eth_class_conf.vlan_check = 0;

    if (encap == MEPA_TS_ENCAP_ETH_IP_PTP) {
        cs_conf.ip_class_conf.ip_ver = MEPA_TS_IP_VER_4;
        cs_conf.ip_class_conf.ip_match_mode = MEPA_TS_IP_MATCH_SRC;  /**<  match src, dest or either IP address */
        cs_conf.ip_class_conf.udp_sport_en = FALSE;   /**<  UDP Source port check enable */
        cs_conf.ip_class_conf.udp_dport_en = FALSE;   /**<  UDP Dest port check enable */
        cs_conf.ip_class_conf.udp_dport = 319;
    }
    cs_conf.eth_class_conf.vlan_conf.etype = (encap == MEPA_TS_ENCAP_ETH_IP_PTP) ? 0x0800: 0x88f7;
    if(meba_phy_ts_rx_classifier_conf_set(meba_global_inst, port_no, 0, &cs_conf) != MEPA_RC_OK){
        cli_printf("ts classifier port(%u) failed, n: %u\n", port_no);
        return MEPA_RC_ERROR;
    }
    mepa_ts_ptp_clock_conf_t ptp_conf;
    memset(&ptp_conf, 0, sizeof(ptp_conf));
    ptp_conf.enable = 1;
    ptp_conf.clk_mode = ck_mode;
    ptp_conf.delaym_type = MEPA_TS_PTP_DELAYM_E2E;
    if(meba_phy_ts_rx_clock_conf_set(meba_global_inst, port_no, 0, &ptp_conf) != MEPA_RC_OK){
        cli_printf("ts classifier port(%u) failed, n: %u\n", port_no);
        return MEPA_RC_ERROR;
    } 
    return MEPA_RC_OK;
}

static int phy_init(int argc, const char *argv[])
{
    uint32_t tx_port = ARGV_INT("tx-port", "The SYNC TX port");
    uint32_t rx_port = ARGV_INT("rx-port", "The SYNC RX port");

    EXAMPLE_BARRIER(argc);

    memset(&state, 0, sizeof(state));
    state.tx_port = tx_port - 1;
    state.rx_port = rx_port - 1;

    return MEPA_RC_OK;
}

static int phy_clean()
{
    mesa_vid_mac_t vid_mac;

    vid_mac.vid = 1;
    memcpy(vid_mac.mac.addr, dst_mac, sizeof(vid_mac.mac.addr));
    RC(mesa_mac_table_del(NULL, &vid_mac));

    return MEPA_RC_OK;
}

static const char* phy_help()
{
    return phy_help_txt;
}

static int phy_run(int argc, const char *argv[])
{
    uint32_t command = ARGV_RUN_INT("command", "The command to run.\n\
        value 0:  PHY as TC 1 Step \n\
        value 1:  Enable MACSec\n\
        value 2:  Disable MACSec\n");
    mepa_timestamp_t ts;
    EXAMPLE_BARRIER(argc);
    switch (command) {
    case 0:
        cli_printf("\n\nPTP(TC) PHY Standalone accuracy testing:\n");
        phy_ts_egress_init(state.rx_port, MEPA_TS_ENCAP_ETH_PTP, MEPA_TS_PTP_CLOCK_MODE_TC1STEP);
        phy_ts_egress_init(state.tx_port, MEPA_TS_ENCAP_ETH_PTP, MEPA_TS_PTP_CLOCK_MODE_TC1STEP);
        phy_ts_ingress_init(state.rx_port, MEPA_TS_ENCAP_ETH_PTP, MEPA_TS_PTP_CLOCK_MODE_TC1STEP);
        phy_ts_ingress_init(state.tx_port, MEPA_TS_ENCAP_ETH_PTP, MEPA_TS_PTP_CLOCK_MODE_TC1STEP);
	if((meba_phy_ts_mode_set(meba_global_inst, state.rx_port, 1) != MEPA_RC_OK) ||
	   (meba_phy_ts_mode_set(meba_global_inst, state.tx_port, 1) != MEPA_RC_OK)) {
            cli_printf("mode_set port(%u, %u) failed,\n", state.tx_port, state.rx_port);
            return MEPA_RC_ERROR;
        }
#define LTC_ADJ
#ifdef LTC_ADJ
	meba_phy_ts_ltc_ls_en(meba_global_inst, state.tx_port, MEPA_TS_CMD_SAVE);
	meba_phy_ts_ltc_set(meba_global_inst, state.tx_port, &ts);
	ts.nanoseconds = 32209456;
        meba_phy_ts_ltc_ls_en(meba_global_inst, state.tx_port, MEPA_TS_CMD_LOAD);
        meba_phy_ts_ltc_set(meba_global_inst, state.tx_port, &ts);    
        break;
#endif
    case 1:
        cli_printf("\n\nEnabling MACSEC without spike patch:\n");
        if(phy_macsec(state.tx_port, MACSEC_ENABLE) != MEPA_RC_OK) {
	   cli_printf("\n\nError enabling MACSEC without spike patch:\n");
	   return MEPA_RC_ERROR;
	}
        break;
    case 2:
        cli_printf("\n\nDisabling MACSEC:\n");
        if(phy_macsec(state.tx_port, MACSEC_DISABLE) != MEPA_RC_OK) {
           cli_printf("\n\nError disabling MACSec\n");
           return MEPA_RC_ERROR;
        }
        break;
    default:
        cli_printf("Unknown command\n");
        break;
    }

    return MEPA_RC_OK;
}

EXAMPLE(ts_phy, phy_init, phy_run, phy_clean, phy_help);
