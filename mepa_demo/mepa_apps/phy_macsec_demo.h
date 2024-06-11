// Copyright (c) 2004-2020 Microchip Technology Inc. and its subsidiaries.
// SPDX-License-Identifier: MIT


/*============================================================
 * Keywords used in MACsec Commands                          |
 *============================================================
*/

/* MACsec Enable or Disable Command Keywords */
#define KEYWORD_BYPASS_NONE     "bypass_none"
#define KEYWORD_BYPASS_DISABLE  "bypass_disable"
#define KEYWORD_BYPASS_ENABLE   "bypass_enable"

/* MACsec Cipher Suit keywords */
#define KEYWORD_XPN             "xpn"
#define KEYWORD_NON_XPN         "non-xpn"

/* MACsec Frame Validation */
#define KEYWORD_STRICT          "strict"
#define KEYWORD_CHECK           "check"
#define KEYWORD_DISABLE         "disable"

/* MACsec SecY Configuration keywords */
#define KEYWORD_CREATE          "create"
#define KEYWORD_UPDATE          "update"
#define KEYWORD_TRUE            "true"
#define KEYWORD_FALSE           "false"
#define KEYWORD_EGRESS          "egress"
#define KEYWORD_INGRESS         "ingress"
#define KEYWORD_CONT_PORT       "cont_port"
#define KEYWORD_UNCONT_PORT     "uncont_port"
#define KEYWORD_DROP            "drop"

/* MACsec Pattern Matching Keywords */
#define KEYWORD_ETHTYPE         "ethtype"
#define KEYWORD_SRC_MAC         "src_mac"
#define KEYWORD_DST_MAC         "dst_mac"
#define KEYWORD_MATCH           "match"
#define KEYWORD_VLANID          "vlanid"
#define KEYWORD_VIDINNER        "vid-inner"

#define MAC_ADDRESS_LEN    17 /* Length of MAC address string in formate "00-00-00-00-00-00" */

typedef struct {
    mepa_bool_t     etype_parsed;         /* Ethertype Keyword Parsed */
    mepa_bool_t     src_mac_parsed;       /* Source MAC Address keyword Parsed */
    mepa_bool_t     dst_mac_parsed;       /* Destination MAC Address keyword Parsed */
    mepa_bool_t     match_parsed;         /* Match Keyword Parsed */
    mepa_bool_t     vlan_id_parsed;       /* Vlan id Keyword Parsed */
    mepa_bool_t     vlan_inner_id_parsed; /* Inner Vlan id keyword Parsed */
} keyword_parsed;

/* Stroring the required Parameters for the Application in structure */
typedef struct {
    mepa_etype_t     pattern_ethtype;
    mepa_mac_t       pattern_src_mac_addr;
    mepa_mac_t       pattern_dst_mac_addr;
    mepa_vid_t       vid;
    mepa_vid_t       vid_inner;
} macsec_store;

typedef struct {
    mepa_macsec_init_bypass_t   bypass_conf;          /* Bypass Configuration */
    mepa_bool_t                 protect_frames;       /* Frame Protection Enable or disable */
    mepa_validate_frames_t      frame_validate;       /* Frame Vlaidation Configuration */
    uint32_t                    port_id;              /* SecY Identifier */
    mepa_macsec_ciphersuite_t   cipher;               /* Cipher Suit Configuration */
    mepa_bool_t                 secy_create;          /* SecY Create or Update */
    mepa_bool_t                 egress_direction;     /* Egress or Ingress Direction */
    mepa_mac_t                  mac_addr;             /* SecY MAC Address */
    mepa_etype_t                pattern_ethtype;      /* Pattern match Ethertype */
    mepa_mac_t                  pattern_src_mac_addr; /* Pattern Match SRC MAC Address */
    mepa_mac_t                  pattern_dst_mac_addr; /* pattern Match DST MAC Address */
    mepa_macsec_match_action_t  match_action;         /* Action after PAttern Matched */
    uint16_t                    vid;                  /* Pattern Match VLAN ID */
    uint16_t                    vid_inner;            /* Pattern Match Inner VLAN ID */
    uint16_t                    pattern_match;        /* Parameter to be matched */
    mepa_bool_t                 conf;                 /* Confidentiality Enable */
} macsec_configuration;

