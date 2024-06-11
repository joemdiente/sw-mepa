// Copyright (c) 2004-2020 Microchip Technology Inc. and its subsidiaries.
// SPDX-License-Identifier: MIT

#include <stdio.h>
#include <unistd.h>
#include <ctype.h>
#include "microchip/ethernet/switch/api.h"
#include "microchip/ethernet/board/api.h"
#include "main.h"
#include "trace.h"
#include "cli.h"
#include "port.h"
#include <mepa_apps/phy_macsec_demo.h>



/**
 * \brief   : Enables MACsec Block for given ports
 * \command : "macsec enable"
 */
static void cli_cmd_macsec_ena(cli_req_t *req);

/**
 * \brief   : Disables MACsec Block for given ports
 * \command : "macsec disable"
 */
static void cli_cmd_macsec_dis(cli_req_t *req);

/**
 * \brief   : Status of MACsec Block on all ports
 * \command : "macsec port_state"
 */
static void cli_cmd_macsec_port_state(cli_req_t *req);

/**
 * \brief   : Create Secure Entity or Update the Configuration of SecY
 * \command : "secy_create_upd"
 */
static void cli_cmd_secy_create(cli_req_t *req);

/**
 * \brief   : Delete Secure Entity(SecY)
 * \command : "secy_del"
 */
static void cli_cmd_macsec_secy_del(cli_req_t *req);

/**
 * \brief   : Pattern matching parameters
 * \command : "match_conf"
 */
static void cli_cmd_match_param(cli_req_t *req);

/**
 * \brief   : Configure Pattern Match for SecY
 * \command : "pattern_conf"
 */
static void cli_cmd_match_conf(cli_req_t *req);

/**
 * \brief   : Start MACsec Application
 * \command : "macsec"
 */
static void cli_cmd_macsec_demo(cli_req_t *req);




meba_inst_t meba_macsec_instance;

static mscc_appl_trace_module_t trace_module = {
    .name = "macsec-demo"
};
static mscc_appl_trace_group_t trace_groups[10] = {
    {
        .name = "default",
        .level = MESA_TRACE_LEVEL_ERROR
    },
};

#define MAX_PORTS    20                   /* Number of Ports in EDSx */
#define TXT_DISABLED 0
#define TXT_ENABLED  1
#define TXT_BYPASSED 2
#define TXT_NOT_APP  3
#define TXT_NOT_TRIG 4
#define TXT_TRIG     5

/* Structure objects to know the keyword parsed and to store the pattern values */
keyword_parsed keyword;
macsec_store   pattern_values;

static int mepa_dev_check(meba_inst_t meba_instance, mepa_port_no_t port_no) {
    if(!meba_instance->phy_devices[port_no]) {
        return MEPA_RC_ERROR;
    }
    return MEPA_RC_OK;
}


const char *cli_capable_txt(mesa_bool_t enabled)
{
    return (enabled ? "Capable" : "NotCapable");
}

const char *cli_enable_txt(int status)
{
    switch(status) {
    case 0:
         return "Disabled";
         break;
    case 1:
         return "Enabled";
         break;
    case 2:
         return "Bypassed";
         break;
    case 3:
        return "  --";
        break;
    case 4:
        return "Not Triggered";
        break;
    case 5:
        return "Triggered";
        break;
    }
    return 0;
}

static void cli_cmd_macsec_ena(cli_req_t *req)
{
    macsec_configuration *mreq = req->module_req;
    mepa_port_no_t  port_no;
    for(int iport = 0; iport < MAX_PORTS; iport++) {
        port_no = iport2uport(iport);
        if (req->port_list[port_no] == 0) {
            continue;
        }
        if(!meba_macsec_instance->phy_devices[iport]) {
            cli_printf(" Dev is Not Created for the port : %d\n", iport);
           return;
        }
        mepa_macsec_init_t init_get;
        mepa_rc rc;

        if ((rc = mepa_macsec_init_get(meba_macsec_instance->phy_devices[iport], &init_get)) != MEPA_RC_OK) {
            return;
        }
        if(mreq->bypass_conf == MEPA_MACSEC_INIT_BYPASS_DISABLE && init_get.enable != 1) {
            cli_printf("\n MACsec Block on port no %d is not in bypass to disable from bypass\n", iport);
           continue;
        }

        mepa_macsec_init_t init_data = { .enable = TRUE,
                                         .dis_ing_nm_macsec_en = TRUE,
                                         .mac_conf.lmac.dis_length_validate = FALSE,
                                         .mac_conf.hmac.dis_length_validate = FALSE,
                                         .bypass = mreq->bypass_conf};

        if ((rc = mepa_macsec_init_set(meba_macsec_instance->phy_devices[iport], &init_data)) != MEPA_RC_OK) {
            T_E("\n Error in Enabling MACsec on port : %d\n", iport);
            return;
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

        if ((rc = mepa_macsec_default_action_set(meba_macsec_instance->phy_devices[iport], iport, &default_action_policy)) != MEPA_RC_OK) {
            T_E("\n Error in configuration Default action set on port :%d \n", iport);
            return;
        }

        cli_printf("\n ...... MACsec Block Enabled on Port : %d......\n", iport);
    }
    return;
}

static void cli_cmd_macsec_dis(cli_req_t *req)
{
    macsec_configuration *mreq = req->module_req;
    mepa_rc rc;
    mepa_macsec_init_t init_data;
    mepa_port_no_t  port_no;
    mepa_macsec_init_t init_get;

    for(int iport = 0; iport < MAX_PORTS; iport++) {
        port_no = iport2uport(iport);
        if (req->port_list[port_no] == 0) {
            continue;
        }

        if ((rc = mepa_dev_check(meba_macsec_instance, iport)) != MEPA_RC_OK) {
            cli_printf(" Dev is Not Created for the port : %d\n", iport);
            return;
        }

        if ((rc = mepa_macsec_init_get(meba_macsec_instance->phy_devices[iport], &init_get)) != MEPA_RC_OK) {
            return;
        }

        if(init_get.enable != TRUE && mreq->bypass_conf == MEPA_MACSEC_INIT_BYPASS_ENABLE) {
            cli_printf("\n Enable MAC Block for this port %d....\n", iport);
            return;
        }

        init_data.enable = FALSE;
        init_data.dis_ing_nm_macsec_en = TRUE;
        init_data.mac_conf.lmac.dis_length_validate = FALSE;
        init_data.mac_conf.hmac.dis_length_validate = FALSE;
        init_data.bypass = mreq->bypass_conf;

        if ((rc = mepa_macsec_init_set(meba_macsec_instance->phy_devices[iport], &init_data)) != MEPA_RC_OK) {
            T_E("\n Error in Enabling MACsec on port : %d\n", iport);
            return;
        }
        cli_printf("\n ...... MACsec Block Disabled on Port : %d......\n", iport);
    }
    return;
}

static void cli_cmd_macsec_port_state(cli_req_t *req)
{
    mepa_rc rc;
    mepa_bool_t macsec_capable;
    mepa_macsec_init_t init_get;
    mepa_bool_t capable = 0;
    int status = 0;
    printf("\n");
    cli_printf("Port    MACsec Capable    Status");
    cli_printf("\n-----------------------------------\n");
    printf("\n");

    mepa_port_no_t  port_no;
    for(int iport = 0; iport < MAX_PORTS; iport++) {
        port_no = iport2uport(iport);
        if ((rc = mepa_dev_check(meba_macsec_instance, iport)) != MEPA_RC_OK) {
            capable = 0;
            status = TXT_NOT_APP;
            cli_printf("%-8u%-18s%s\n",
                   port_no,
                   cli_capable_txt(capable),
                   cli_enable_txt(status));
            continue;
        }
 
        if((rc = mepa_macsec_is_capable(meba_macsec_instance->phy_devices[iport], iport, &macsec_capable)) != MEPA_RC_NOT_IMPLEMENTED) {
            capable = macsec_capable;
            if ((rc = mepa_macsec_init_get(meba_macsec_instance->phy_devices[iport], &init_get)) == MEPA_RC_OK) {
                status = (init_get.enable == 1 && init_get.bypass != MEPA_MACSEC_INIT_BYPASS_ENABLE) ? TXT_ENABLED : TXT_DISABLED;
                if(init_get.bypass == MEPA_MACSEC_INIT_BYPASS_ENABLE)
                    status = TXT_BYPASSED;
            }
        } else {
            capable = 0;
            status = TXT_NOT_APP;
        }
        cli_printf("%-8u%-18s%s\n",
                   port_no,
                   cli_capable_txt(capable),
                   cli_enable_txt(status));
    }
    return;
}

static void cli_cmd_secy_create(cli_req_t *req)
{
    macsec_configuration *mreq = req->module_req;
    mepa_rc rc = MEPA_RC_ERROR;

    if ((rc = mepa_dev_check(meba_macsec_instance, req->port_no)) != MEPA_RC_OK) {
        printf(" Dev is Not Created for the port : %d\n", req->port_no);
        return;
    }

    mepa_macsec_port_t     macsec_port;
    macsec_port.port_no    = req->port_no;
    macsec_port.service_id = 0;
    macsec_port.port_id    = mreq->port_id;

    mepa_macsec_secy_conf_t secy_conf;
    secy_conf.validate_frames        = mreq->frame_validate;
    secy_conf.replay_protect         = FALSE;
    secy_conf.replay_window          = 0;
    secy_conf.protect_frames         = mreq->protect_frames;
    secy_conf.always_include_sci     = TRUE;     /* 16 Byte Sectag */
    secy_conf.use_es                 = FALSE;    /* when the System Identifier bytes of the SCI match the MAC source address ES = 1 */
    secy_conf.use_scb                = FALSE;
    secy_conf.confidentiality_offset = 0;
    memcpy(&secy_conf.mac_addr, &mreq->mac_addr, sizeof(mepa_mac_t));
    secy_conf.current_cipher_suite   = mreq->cipher;

    if(mreq->secy_create) {
        if ((rc = mepa_macsec_secy_conf_add(meba_macsec_instance->phy_devices[req->port_no], macsec_port, &secy_conf)) != MEPA_RC_OK) {
            T_E("\n Error in creating the SecY on port : %d \n", req->port_no);
            return;
        }

        /* SecY Control port enable */
        if((rc = mepa_macsec_secy_controlled_set(meba_macsec_instance->phy_devices[req->port_no], macsec_port, TRUE)) != MEPA_RC_OK) {
            T_E("\n Error in enabling the SecY controlled port on port : %d \n", req->port_no);
            return;
        }
    } else {
        if ((rc = mepa_macsec_secy_conf_update(meba_macsec_instance->phy_devices[req->port_no], macsec_port, &secy_conf)) != MEPA_RC_OK) {
            T_E("\n Error in creating the Updating SecY Parameters on port : %d \n", req->port_no);
            return;
        }
    }
    cli_printf("\n ...... SecY %s on Port : %d with port_id : %d......\n", mreq->secy_create ? "Created" : "Updated", req->port_no, mreq->port_id);


    return;
}

static void cli_cmd_macsec_secy_del(cli_req_t *req)
{
    macsec_configuration *mreq = req->module_req;
    mepa_rc rc = MEPA_RC_ERROR;

    if ((rc = mepa_dev_check(meba_macsec_instance, req->port_no)) != MEPA_RC_OK) {
        printf(" Dev is Not Created for the port : %d\n", req->port_no);
        return;
    }

    mepa_macsec_port_t  macsec_port;
    macsec_port.port_no    = req->port_no;
    macsec_port.service_id = 0;
    macsec_port.port_id    = mreq->port_id;

    if ((rc = mepa_macsec_secy_conf_del(meba_macsec_instance->phy_devices[req->port_no], macsec_port)) != MEPA_RC_OK) {
        T_E("\n Error in Deleting the SecY on port : %d \n", req->port_no);
        return;
    }
    cli_printf("\n ....... SecY Deleted on Port : %d with port_id : %d...... \n", req->port_no, mreq->port_id);
}

static void cli_cmd_match_param(cli_req_t *req)
{
    macsec_configuration *mreq = req->module_req;
    memcpy(&pattern_values.pattern_ethtype, &mreq->pattern_ethtype, sizeof(mepa_etype_t));
    memcpy(&pattern_values.pattern_src_mac_addr, &mreq->pattern_src_mac_addr, sizeof(mepa_mac_t));
    memcpy(&pattern_values.pattern_dst_mac_addr, &mreq->pattern_dst_mac_addr, sizeof(mepa_mac_t));
    memcpy(&pattern_values.vid, &mreq->vid, sizeof(mepa_vid_t));
    memcpy(&pattern_values.vid_inner, &mreq->vid_inner, sizeof(mepa_vid_t));
    cli_printf("\n ...... Match Parameters configured......\n");
    return;
}

static void cli_cmd_match_conf(cli_req_t *req)
{
    macsec_configuration *mreq = req->module_req;
    mepa_rc rc;

    if ((rc = mepa_dev_check(meba_macsec_instance, req->port_no)) != MEPA_RC_OK) {
        printf(" Dev is Not Created for the port : %d\n", req->port_no);
        return;
    }

    mepa_macsec_port_t     macsec_port;
    macsec_port.port_no    = req->port_no;
    macsec_port.service_id = 0;
    macsec_port.port_id    = mreq->port_id;

    mepa_macsec_direction_t direction;

    /* Pattern Matching rule */
    mepa_macsec_match_pattern_t pattern_match;
    memset(&pattern_match, 0 , sizeof(mepa_macsec_match_pattern_t));
    pattern_match.priority            = MEPA_MACSEC_MATCH_PRIORITY_HIGH;
    pattern_match.match               = mreq->pattern_match;
    pattern_match.is_control          = TRUE;
    pattern_match.etype               = pattern_values.pattern_ethtype;
    pattern_match.vid                 = pattern_values.vid;
    pattern_match.vid_inner           = pattern_values.vid_inner;

    if(pattern_match.match & MEPA_MACSEC_MATCH_HAS_VLAN) {
        pattern_match.has_vlan_tag       = TRUE;
    }
    if(pattern_match.match & MEPA_MACSEC_MATCH_HAS_VLAN_INNER) {
        pattern_match.has_vlan_inner_tag = TRUE;
    }
    
    memcpy(&pattern_match.src_mac, &pattern_values.pattern_src_mac_addr, sizeof(mepa_mac_t));
    memcpy(&pattern_match.dest_mac, &pattern_values.pattern_dst_mac_addr, sizeof(mepa_mac_t));

    if(mreq->egress_direction) {
        direction =  MEPA_MACSEC_DIRECTION_EGRESS;
    } else {
        direction = MEPA_MACSEC_DIRECTION_INGRESS;
    }

    if ((rc = mepa_macsec_pattern_set(meba_macsec_instance->phy_devices[req->port_no], macsec_port, direction, mreq->match_action, &pattern_match)) != MEPA_RC_OK) {
        T_E("\n Error in Configuring Pattern matching rule on port : %d \n", req->port_no);
        return;
    }
    cli_printf("\n ...... Pattern Matching rule is Configured ......\n");
    return;
}


static void cli_cmd_macsec_demo(cli_req_t *req)
{
    cli_printf("\n\t\t\t\t \033[1;31m ================================================================================================================\033[0m\n");
    cli_printf("\t\t\t\t\t  \t\t\t\t \033[1;31mMACSEC DEMO APPLICATION\033[0m\n");
    cli_printf("\n\t\t\t\t  \033[1;31m================================================================================================================\033[0m\n");
    return;
}


static int cli_param_parse_bypass_config(cli_req_t *req)
{
    const char     *found;
    macsec_configuration *mreq = req->module_req;
    int len = 0;
    if ((found = cli_parse_find(req->cmd, req->stx)) == NULL)
        return 1;
    len = strlen(found);
    if (!strncasecmp(found, KEYWORD_BYPASS_NONE, len)) {
        mreq->bypass_conf = MEPA_MACSEC_INIT_BYPASS_NONE;
    }
    else if (!strncasecmp(found, KEYWORD_BYPASS_DISABLE, len)) {
        mreq->bypass_conf = MEPA_MACSEC_INIT_BYPASS_DISABLE;
    }
    else if (!strncasecmp(found, KEYWORD_BYPASS_ENABLE, len)) {
        mreq->bypass_conf = MEPA_MACSEC_INIT_BYPASS_ENABLE;
    }
    return 0;
}

static int cli_cmd_parse_cipher_suit(cli_req_t *req)
{
    const char     *found;
    macsec_configuration *mreq = req->module_req;
    int len = 0;
    if ((found = cli_parse_find(req->cmd, req->stx)) == NULL)
        return 1;
    len = strlen(found);
    if (!strncasecmp(found, KEYWORD_XPN, len)) {
        mreq->cipher = MEPA_MACSEC_CIPHER_SUITE_GCM_AES_XPN_256;
    }
    else if (!strncasecmp(found, KEYWORD_NON_XPN, len)) {
        mreq->cipher = MEPA_MACSEC_CIPHER_SUITE_GCM_AES_256;
    }
    return 0;
}

static int cli_cmd_parse_frame_validate(cli_req_t *req)
{
    const char     *found;
    macsec_configuration *mreq = req->module_req;
    int len = 0;
    if ((found = cli_parse_find(req->cmd, req->stx)) == NULL)
        return 1;
    len = strlen(found);
    if (!strncasecmp(found, KEYWORD_STRICT, len)) {
        mreq->frame_validate = MEPA_MACSEC_VALIDATE_FRAMES_STRICT;
    }
    else if (!strncasecmp(found, KEYWORD_CHECK, len)) {
        mreq->frame_validate = MEPA_MACSEC_VALIDATE_FRAMES_CHECK;
    }
    else if (!strncasecmp(found, KEYWORD_DISABLE, len)) {
        mreq->frame_validate = MEPA_MACSEC_VALIDATE_FRAMES_DISABLED;
    }
    return 0;
}

static int cli_cmd_parse_u16_param(cli_req_t *req)
{
    uint16_t value;
    macsec_configuration *mreq = req->module_req;

    if (keyword.etype_parsed == 1) {
        cli_parm_u16(req, &value, 0, 0xFFFF);
        mreq->pattern_ethtype = value;
        keyword.etype_parsed = 0;
    }
    else if(keyword.match_parsed == 1) {
        cli_parm_u16(req, &value, 0, 0xFFFF);
        mreq->pattern_match = value;
        keyword.match_parsed = 0;
    }
    else if(keyword.vlan_id_parsed == 1) {
        cli_parm_u16(req, &value, 0, 0xFFFF);
        mreq->vid = value;
        keyword.vlan_id_parsed = 0;
    }
    else if(keyword.vlan_inner_id_parsed == 1) {
        cli_parm_u16(req, &value, 0, 0xFFFF);
        mreq->vid_inner = value;
        keyword.vlan_inner_id_parsed = 0;
    }
    else {
        cli_parm_u16(req, &value, 0, 0xFFFF);
	mreq->port_id = value;
    }
    return 0;
}


static int cli_cmd_parse_boolean(cli_req_t *req)
{
    const char     *found;
    macsec_configuration *mreq = req->module_req;
    int len = 0;
    if ((found = cli_parse_find(req->cmd, req->stx)) == NULL)
        return 1;
    len = strlen(found);
    if (!strncasecmp(found, KEYWORD_CREATE, len)) {
        mreq->secy_create = 1;
    }
    else if (!strncasecmp(found, KEYWORD_UPDATE, len)) {
        mreq->secy_create = 0;
    }
    else if (!strncasecmp(found, KEYWORD_TRUE, len)) {
        mreq->conf = 1;
        mreq->protect_frames = 1;
    }
    else if (!strncasecmp(found, KEYWORD_FALSE, len)) {
        mreq->conf = 0;
        mreq->protect_frames = 0;
    }
    else if (!strncasecmp(found, KEYWORD_EGRESS, len)) {
        mreq->egress_direction = 1;
    }
    else if (!strncasecmp(found, KEYWORD_INGRESS, len)) {
        mreq->egress_direction = 0;
    }
    return 0;
}

static int cli_cmd_parse_mac_addr(cli_req_t *req)
{
    macsec_configuration *mreq = req->module_req;
    uint32_t  i, mac[6];
    int error = 1;
    int len = 0;
    len = strlen(req->cmd);
    if(len > MAC_ADDRESS_LEN) {
        cli_printf("\n Invalid MAC Address argument\n");
    }
    if (sscanf(req->cmd, "%2x-%2x-%2x-%2x-%2x-%2x", &mac[0], &mac[1], &mac[2], &mac[3], &mac[4], &mac[5]) == 6) {
        if (keyword.dst_mac_parsed == 1) {
            for (i = 0; i < 6; i++) {
                mreq->pattern_dst_mac_addr.addr[i] = (mac[i] & 0xff);
            }
	    keyword.dst_mac_parsed = 0;
        }
	else if (keyword.src_mac_parsed == 1) {
            for (i = 0; i < 6; i++) {
                mreq->pattern_src_mac_addr.addr[i] = (mac[i] & 0xff);
             }
	     keyword.src_mac_parsed = 0;
        }
        else {
            for (i = 0; i < 6; i++) {
                mreq->mac_addr.addr[i] = (mac[i] & 0xff);
            }
        }
        error = 0;
    }
    else {
        cli_printf("\n Invalid MAC Address argument\n");
    }
    return error;
}

static int cli_cmd_parse_match_action(cli_req_t *req)
{
    const char     *found;
    macsec_configuration *mreq = req->module_req;
    int len = 0;
    if ((found = cli_parse_find(req->cmd, req->stx)) == NULL)
        return 1;
    len = strlen(found);
    if (!strncasecmp(found, KEYWORD_CONT_PORT, len)) {
        mreq->match_action = MEPA_MACSEC_MATCH_ACTION_CONTROLLED_PORT;
    }
    else if (!strncasecmp(found, KEYWORD_UNCONT_PORT, len)) {
        mreq->match_action = MEPA_MACSEC_MATCH_ACTION_UNCONTROLLED_PORT;
    }
    else if (!strncasecmp(found, KEYWORD_DROP, len)) {
        mreq->match_action = MEPA_MACSEC_MATCH_ACTION_DROP;
    }
    return 0;
}

static int cli_cmd_parse_keyword(cli_req_t *req)
{
    int len = 0;
    len = strlen(req->cmd);
    if (!strncasecmp(req->cmd, KEYWORD_ETHTYPE, len)) {
        keyword.etype_parsed = 1;
    }
    else if (!strncasecmp(req->cmd, KEYWORD_SRC_MAC, len)) {
        keyword.src_mac_parsed = 1;
    }
    else if (!strncasecmp(req->cmd, KEYWORD_DST_MAC, len)) {
        keyword.dst_mac_parsed = 1;
    }
    else if (!strncasecmp(req->cmd, KEYWORD_MATCH, len)) {
        keyword.match_parsed = 1;
    }
    else if (!strncasecmp(req->cmd, KEYWORD_VLANID, len)) {
        keyword.vlan_id_parsed = 1;
    }
    else if (!strncasecmp(req->cmd, KEYWORD_VIDINNER, len)) {
        keyword.vlan_inner_id_parsed = 1;
    }
    return 0;
}


static cli_cmd_t cli_cmd_macsec_table[] = {
    {
        "exit_macsec",
        "Exit MACsec Demo Application",
        cli_cmd_macsec_demo,
    },

    {
        "macsec enable <port_list> [bypass_none|bypass_disable]",
        "Enable the MACsec Block",
         cli_cmd_macsec_ena,
    },

    {
        "macsec disable <port_list> [bypass_none|bypass_enable]",
        "Disable the MACsec Block",
         cli_cmd_macsec_dis,
    },

    {
        "macsec port_state",
        "Status of MACsec Engine on all Ports",
         cli_cmd_macsec_port_state,
    },

    {
        "secy_create_upd [create|update] <port_no> port-id <port_id> mac-addr <mac_address> protect [true|false] validate [strict|check|disable] cipher [xpn|n-xpn]",
        "Create or Update the MACsec Secure Entity",
        cli_cmd_secy_create,
    },

    {
        "secy_del <port_no> port-id <port_id>",
        "Delete the Secure Entity",
        cli_cmd_macsec_secy_del,
    },    

    {
        "match_conf <port_no> ethtype <ether_type> src_mac <mac_address> dst_mac <mac_address> vlanid <vlan> vid-inner <vlan>",
        "Pattern Match Parameters",
        cli_cmd_match_param,
    },

    {
        "pattern_conf <port_no> port-id <port_id> [egress|ingress] [cont_port|uncont_port|drop] match <match_value>",
        "Pattern Match Configuration",
        cli_cmd_match_conf,
    },

};


static cli_cmd_t cli_cmd_table[] = {
    {
        "macsec",
        "MACsec Operational Demo",
        cli_cmd_macsec_demo,
    },
};

static cli_parm_t cli_parm_table[] = {

    {
        "bypass_none|bypass_disable",
        "\n\n\t bypass_none      : MAC block and MACsec Block Configuration \n"
        "\t bypass_disable   : Enable MACsec Block from Bypass \n",
        CLI_PARM_FLAG_SET,
        cli_param_parse_bypass_config,
    },

    {
        "bypass_none|bypass_enable",
        "\n\n\t bypass_none      : MAC block and MACsec Block Configuration \n"
        "\t bypass_enable    : Put MACsec Block in Bypass \n",
        CLI_PARM_FLAG_SET,
        cli_param_parse_bypass_config,
    },

    {
        "xpn|n-xpn",
        "\n\t xpn      : Extented Packet Number Cipher suit - 256 bit Key \n"
        "\t non-xpn  : Non Extented Packet Number Cipher suit - 256 bit Key \n",
        CLI_PARM_FLAG_SET,
        cli_cmd_parse_cipher_suit,
    },

    {
        "strict|check|disable",
        "\n\t strict   : Perform integrity check and drop failed frames \n"
        "\t check    : Perform integrity check do not drop failed frames \n"
        "\t disable  : Do not perform integrity check \n",
        CLI_PARM_FLAG_NONE,
        cli_cmd_parse_frame_validate,
    },

    {
        "<port_id>",
        "Secy identifier",
        CLI_PARM_FLAG_NONE,
        cli_cmd_parse_u16_param,
    },

    {
        "create|update",
        "\n\t create    : Create new MACsec Secure Entity \n"
        "\t update    : Update the Configuration of existing Secure Entity \n",
        CLI_PARM_FLAG_NONE,
        cli_cmd_parse_boolean,
    },

    {
        "<mac_address>",
        "MAC address (xx-xx-xx-xx-xx-xx)",
        CLI_PARM_FLAG_SET,
        cli_cmd_parse_mac_addr,
    },

    {
        "<ether_type>",
        "Ether-type of Packet",
        CLI_PARM_FLAG_NONE,
        cli_cmd_parse_u16_param,
    },

    {
        "egress|ingress",
        "Traffic Direction Egress or Ingress",
         CLI_PARM_FLAG_NONE,
         cli_cmd_parse_boolean,
    },

    {
        "cont_port|uncont_port|drop",
        "Forward Traffic to Controlled/Uncrontrolled Port or Drop Packet",
         CLI_PARM_FLAG_NONE,
         cli_cmd_parse_match_action,
    },

    {
        "<match_value>",
        "Parameter Needs to be Matched",
        CLI_PARM_FLAG_SET,
        cli_cmd_parse_u16_param,
    },

    {
        "true|false",
        "Enable or Disable",
         CLI_PARM_FLAG_NO_TXT,
         cli_cmd_parse_boolean,
    },

    {
        "<vlan>",
        "\n\t Vlan Id\n",
        CLI_PARM_FLAG_NONE,
        cli_cmd_parse_u16_param,
    },
	
    {
        "port-id",
         "",
         CLI_PARM_FLAG_NO_TXT,
         cli_cmd_parse_keyword,
    },

    {
        "mac-addr",
         "",
         CLI_PARM_FLAG_NO_TXT,
         cli_cmd_parse_keyword,
    },

    {
        "protect",
         "",
         CLI_PARM_FLAG_NO_TXT,
         cli_cmd_parse_keyword,
    },

    {
        "validate",
         "",
         CLI_PARM_FLAG_NO_TXT,
         cli_cmd_parse_keyword,
    },

    {
        "cipher",
         "",
         CLI_PARM_FLAG_NO_TXT,
         cli_cmd_parse_keyword,
    },

    {
        "src_mac",
         "",
         CLI_PARM_FLAG_NO_TXT,
         cli_cmd_parse_keyword,
    },

    {
        "dst_mac",
         "",
         CLI_PARM_FLAG_NO_TXT,
         cli_cmd_parse_keyword,
    },

    {
        "ethtype",
         "",
         CLI_PARM_FLAG_NO_TXT,
         cli_cmd_parse_keyword,
    },

    {
        "match",
        "",
        CLI_PARM_FLAG_NO_TXT,
        cli_cmd_parse_keyword,
    },

    {
        "vlanid",
        "",
        CLI_PARM_FLAG_NO_TXT,
        cli_cmd_parse_keyword,
    },

    {
        "vid-inner",
        "",
        CLI_PARM_FLAG_NO_TXT,
        cli_cmd_parse_keyword,
    }
};


static void phy_cli_init(void)
{
    int i;

    /* Register CLI Commands */
    for (i = 0; i < sizeof(cli_cmd_table)/sizeof(cli_cmd_t); i++) {
        mscc_appl_cli_cmd_reg(&cli_cmd_table[i]);
    }

    /* Register MACsec Commands */
    for (i = 0; i < sizeof(cli_cmd_macsec_table)/sizeof(cli_cmd_t); i++) {
        mscc_appl_macsec_cli_cmd_reg(&cli_cmd_macsec_table[i]);
    }

    /* Register CLI Params */
    for (i = 0; i < sizeof(cli_parm_table)/sizeof(cli_parm_t); i++) {
        mscc_appl_cli_parm_reg(&cli_parm_table[i]);
    }

}

void mepa_demo_appl_macsec_demo(mscc_appl_init_t *init)
{
    meba_macsec_instance = init->board_inst;
    switch(init->cmd) {
    case MSCC_INIT_CMD_INIT:
        phy_cli_init();
        break;

    default:
        break;
    }
}
