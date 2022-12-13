// Copyright (c) 2004-2020 Microchip Technology Inc. and its subsidiaries.
// SPDX-License-Identifier: MIT

#ifndef _MEPA_MACSEC_DRV_API_H_
#define _MEPA_MACSEC_DRV_API_H_

#include <microchip/ethernet/phy/api.h>

typedef mepa_rc (*mepa_macsec_init_set_t)(struct mepa_device *dev, const mepa_macsec_init_t *const macsec_init);
typedef mepa_rc (*mepa_macsec_init_get_t)(struct mepa_device *dev, mepa_macsec_init_t *const macsec_init);
typedef mepa_rc (*mepa_macsec_secy_conf_add_t)(struct mepa_device *dev, const mepa_macsec_port_t port, const mepa_macsec_secy_conf_t *const conf);
typedef mepa_rc (*mepa_macsec_secy_conf_update_t)(struct mepa_device *dev, const mepa_macsec_port_t port, const mepa_macsec_secy_conf_t *const conf);
typedef mepa_rc (*mepa_macsec_secy_conf_get_t)(struct mepa_device *dev, const mepa_macsec_port_t port, mepa_macsec_secy_conf_t *const conf);
typedef mepa_rc (*mepa_macsec_secy_conf_del_t)(struct mepa_device *dev, const mepa_macsec_port_t port);
typedef mepa_rc (*mepa_macsec_secy_controlled_set_t)(struct mepa_device *dev, const mepa_macsec_port_t port, const mepa_bool_t enable);
typedef mepa_rc (*mepa_macsec_secy_controlled_get_t)(struct mepa_device *dev, const mepa_macsec_port_t port, mepa_bool_t *const enable);
typedef mepa_rc (*mepa_macsec_secy_port_status_get_t)(struct mepa_device *dev, const mepa_macsec_port_t port, mepa_macsec_secy_port_status_t *const status);
typedef mepa_rc (*mepa_macsec_port_get_next_t)(struct mepa_device *dev, const mepa_port_no_t port_no, const mepa_macsec_port_t *const search_macsec_port, mepa_macsec_port_t *const found_macsec_port);
typedef mepa_rc (*mepa_macsec_rx_sc_add_t)(struct mepa_device *dev, const mepa_macsec_port_t port, const mepa_macsec_sci_t *const sci);
typedef mepa_rc (*mepa_macsec_rx_sc_update_t)(struct mepa_device *dev, const mepa_macsec_port_t port, const mepa_macsec_sci_t *const sci, const mepa_macsec_rx_sc_conf_t *const conf);
typedef mepa_rc (*mepa_macsec_rx_sc_get_conf_t)(struct mepa_device *dev, const mepa_macsec_port_t port, const mepa_macsec_sci_t *const sci, mepa_macsec_rx_sc_conf_t *const conf);
typedef mepa_rc (*mepa_macsec_rx_sc_get_next_t)(struct mepa_device *dev, const mepa_macsec_port_t port, const mepa_macsec_sci_t *const search_sci, mepa_macsec_sci_t *const found_sci);
typedef mepa_rc (*mepa_macsec_rx_sc_del_t)(struct mepa_device *dev, const mepa_macsec_port_t port, const mepa_macsec_sci_t *const sci);
typedef mepa_rc (*mepa_macsec_rx_sc_status_get_t)(struct mepa_device *dev, const mepa_macsec_port_t port, const mepa_macsec_sci_t *const sci, mepa_macsec_rx_sc_status_t *const status);
typedef mepa_rc (*mepa_macsec_tx_sc_set_t)(struct mepa_device *dev, const mepa_macsec_port_t port);
typedef mepa_rc (*mepa_macsec_tx_sc_update_t)(struct mepa_device *dev, const mepa_macsec_port_t port, const mepa_macsec_tx_sc_conf_t *const conf);
typedef mepa_rc (*mepa_macsec_tx_sc_get_conf_t)(struct mepa_device *dev, const mepa_macsec_port_t port, mepa_macsec_tx_sc_conf_t *const conf);
typedef mepa_rc (*mepa_macsec_tx_sc_del_t)(struct mepa_device *dev, const mepa_macsec_port_t port);
typedef mepa_rc (*mepa_macsec_tx_sc_status_get_t)(struct mepa_device *dev, const mepa_macsec_port_t port, mepa_macsec_tx_sc_status_t *const status);
typedef mepa_rc (*mepa_macsec_rx_sa_set_t)(struct mepa_device *dev, const mepa_macsec_port_t port, const mepa_macsec_sci_t *const sci, const uint16_t an, const uint32_t lowest_pn, const mepa_macsec_sak_t *const sak);
typedef mepa_rc (*mepa_macsec_rx_sa_get_t)(struct mepa_device *dev, const mepa_macsec_port_t port, const mepa_macsec_sci_t *const sci, const uint16_t an, uint32_t *const lowest_pn, mepa_macsec_sak_t *const sak, mepa_bool_t *const active);
typedef mepa_rc (*mepa_macsec_rx_sa_activate_t)(struct mepa_device *dev, const mepa_macsec_port_t port, const mepa_macsec_sci_t *const sci, const uint16_t an);
typedef mepa_rc (*mepa_macsec_rx_sa_disable_t)(struct mepa_device  *dev, const mepa_macsec_port_t port, const mepa_macsec_sci_t *const sci, const uint16_t an);  
typedef mepa_rc (*mepa_macsec_rx_sa_del_t)(struct mepa_device *dev, const mepa_macsec_port_t port, const mepa_macsec_sci_t *const sci, const uint16_t an);
typedef mepa_rc (*mepa_macsec_rx_sa_lowest_pn_update_t)(struct mepa_device *dev, const mepa_macsec_port_t port, const mepa_macsec_sci_t *const sci, const uint16_t an, const uint32_t lowest_pn);
typedef mepa_rc (*mepa_macsec_rx_sa_status_get_t)(struct mepa_device *dev, const mepa_macsec_port_t port, const mepa_macsec_sci_t *const sci, const uint16_t an, mepa_macsec_rx_sa_status_t *const status);
typedef mepa_rc (*mepa_macsec_rx_seca_set_t)(struct mepa_device *dev, const mepa_macsec_port_t port, const mepa_macsec_sci_t *const sci, const uint16_t an, const mepa_macsec_pkt_num_t lowest_pn, const mepa_macsec_sak_t *const sak, const mepa_macsec_ssci_t *const ssci);
typedef mepa_rc (*mepa_macsec_rx_seca_get_t)(struct mepa_device *dev, const mepa_macsec_port_t port, const mepa_macsec_sci_t *const sci, const uint16_t an, mepa_macsec_pkt_num_t *const lowest_pn, mepa_macsec_sak_t *const sak, mepa_bool_t *const active, mepa_macsec_ssci_t *const ssci);
typedef mepa_rc (*mepa_macsec_rx_seca_lowest_pn_update_t)(struct mepa_device *dev, const mepa_macsec_port_t port, const mepa_macsec_sci_t *const sci, const uint16_t an, const mepa_macsec_pkt_num_t lowest_pn);
typedef mepa_rc (*mepa_macsec_tx_sa_set_t)(struct mepa_device *dev, const mepa_macsec_port_t port, const uint16_t an, const uint32_t next_pn, const mepa_bool_t confidentiality, const mepa_macsec_sak_t *const sak);
typedef mepa_rc (*mepa_macsec_tx_sa_get_t)(struct mepa_device *dev, const mepa_macsec_port_t port, const uint16_t an, uint32_t *const next_pn, mepa_bool_t *const confidentiality, mepa_macsec_sak_t *const sak, mepa_bool_t *const active);
typedef mepa_rc (*mepa_macsec_tx_sa_activate_t)(struct mepa_device *dev, const mepa_macsec_port_t port, const uint16_t an);
typedef mepa_rc (*mepa_macsec_tx_sa_disable_t)(struct mepa_device *dev, const mepa_macsec_port_t port, const uint16_t an);
typedef mepa_rc (*mepa_macsec_tx_sa_del_t)(struct mepa_device *dev, const mepa_macsec_port_t port, const uint16_t an);
typedef mepa_rc (*mepa_macsec_tx_sa_status_get_t)(struct mepa_device *dev, const mepa_macsec_port_t port, const uint16_t an, mepa_macsec_tx_sa_status_t *const status);
typedef mepa_rc (*mepa_macsec_tx_seca_set_t)(struct mepa_device *dev, const mepa_macsec_port_t port, const uint16_t an, const mepa_macsec_pkt_num_t next_pn, const mepa_bool_t confidentiality, const mepa_macsec_sak_t *const sak, const mepa_macsec_ssci_t *const ssci);
typedef mepa_rc (*mepa_macsec_tx_seca_get_t)(struct mepa_device *dev, const mepa_macsec_port_t port, const uint16_t an, mepa_macsec_pkt_num_t *const next_pn, mepa_bool_t *const confidentiality, mepa_macsec_sak_t *const sak, mepa_bool_t *const active, mepa_macsec_ssci_t *const ssci);
typedef mepa_rc (*mepa_macsec_controlled_counters_get_t)(struct mepa_device *dev, const mepa_macsec_port_t port, mepa_macsec_secy_port_counters_t *const counters);
typedef mepa_rc (*mepa_macsec_uncontrolled_counters_get_t)(struct mepa_device *dev, const mepa_port_no_t port_no, mepa_macsec_uncontrolled_counters_t *const counters);
typedef mepa_rc (*mepa_macsec_common_counters_get_t)(struct mepa_device *dev, const mepa_port_no_t port_no, mepa_macsec_common_counters_t *const counters);
typedef mepa_rc (*mepa_macsec_secy_cap_get_t)(struct mepa_device *dev, const mepa_port_no_t port_no, mepa_macsec_secy_cap_t *const cap);
typedef mepa_rc (*mepa_macsec_secy_counters_get_t)(struct mepa_device *dev, const mepa_macsec_port_t port, mepa_macsec_secy_counters_t *const counters);
typedef mepa_rc (*mepa_macsec_counters_update_t)(struct mepa_device *dev, const mepa_port_no_t port_no);
typedef mepa_rc (*mepa_macsec_counters_clear_t)(struct mepa_device *dev, const mepa_port_no_t port_no);
typedef mepa_rc (*mepa_macsec_rx_sc_counters_get_t)(struct mepa_device *dev, const mepa_macsec_port_t port, const mepa_macsec_sci_t *const sci, mepa_macsec_rx_sc_counters_t *const counters);
typedef mepa_rc (*mepa_macsec_tx_sc_counters_get_t)(struct mepa_device *dev, const mepa_macsec_port_t port, mepa_macsec_tx_sc_counters_t *const counters);
typedef mepa_rc (*mepa_macsec_tx_sa_counters_get_t)(struct mepa_device *dev, const mepa_macsec_port_t port, const uint16_t an, mepa_macsec_tx_sa_counters_t *const counters);
typedef mepa_rc (*mepa_macsec_rx_sa_counters_get_t)(struct mepa_device *dev, const mepa_macsec_port_t port, const mepa_macsec_sci_t *const sci, const uint16_t an, mepa_macsec_rx_sa_counters_t *const counters);
typedef mepa_rc (*mepa_macsec_control_frame_match_conf_set_t)(struct mepa_device *dev, const mepa_port_no_t port_no, const mepa_macsec_control_frame_match_conf_t *const conf, uint32_t *const rule_id);
typedef mepa_rc (*mepa_macsec_control_frame_match_conf_del_t)(struct mepa_device *dev, const mepa_port_no_t port_no, const uint32_t rule_id);
typedef mepa_rc (*mepa_macsec_control_frame_match_conf_get_t)(struct mepa_device *dev, const mepa_port_no_t port_no, mepa_macsec_control_frame_match_conf_t *const conf, uint32_t rule_id);
typedef mepa_rc (*mepa_macsec_pattern_set_t)(struct mepa_device *dev, const mepa_macsec_port_t port, const mepa_macsec_direction_t direction, const mepa_macsec_match_action_t action, const mepa_macsec_match_pattern_t *const pattern);
typedef mepa_rc (*mepa_macsec_pattern_del_t)(struct mepa_device *dev, const mepa_macsec_port_t port, const mepa_macsec_direction_t direction, const mepa_macsec_match_action_t action);
typedef mepa_rc (*mepa_macsec_pattern_get_t)(struct mepa_device *dev, const mepa_macsec_port_t port, const mepa_macsec_direction_t direction, const mepa_macsec_match_action_t action, mepa_macsec_match_pattern_t *const pattern);
typedef mepa_rc (*mepa_macsec_default_action_set_t)(struct mepa_device *dev, const mepa_port_no_t port_no, const mepa_macsec_default_action_policy_t *const policy);
typedef mepa_rc (*mepa_macsec_default_action_get_t)(struct mepa_device *dev, const mepa_port_no_t port_no, mepa_macsec_default_action_policy_t *const policy);
typedef mepa_rc (*mepa_macsec_bypass_mode_set_t)(struct mepa_device *dev, const mepa_port_no_t port_no, const mepa_macsec_bypass_mode_t *const bypass);
typedef mepa_rc (*mepa_macsec_bypass_mode_get_t)(struct mepa_device *dev, const mepa_port_no_t port_no, mepa_macsec_bypass_mode_t *const bypass);
typedef mepa_rc (*mepa_macsec_bypass_tag_set_t)(struct mepa_device *dev, const mepa_macsec_port_t port, const mepa_macsec_tag_bypass_t tag);
typedef mepa_rc (*mepa_macsec_bypass_tag_get_t)(struct mepa_device *dev, const mepa_macsec_port_t port, mepa_macsec_tag_bypass_t *const tag);
typedef mepa_rc (*mepa_macsec_mtu_set_t)(struct mepa_device *dev, const mepa_port_no_t port_no, const mepa_macsec_mtu_t *const mtu_conf);
typedef mepa_rc (*mepa_macsec_mtu_get_t)(struct mepa_device *dev, const mepa_port_no_t port_no, mepa_macsec_mtu_t *mtu_conf);
typedef mepa_rc (*mepa_macsec_frame_capture_set_t)(struct mepa_device *dev, const mepa_port_no_t port_no, const mepa_macsec_frame_capture_t capture);
typedef mepa_rc (*mepa_macsec_frame_get_t)(struct mepa_device *dev, const mepa_port_no_t port_no, const uint32_t buf_length, uint32_t *const return_length, uint8_t *const frame);
typedef mepa_rc (*mepa_macsec_event_enable_set_t)(struct mepa_device *dev, const mepa_port_no_t port_no, const mepa_macsec_event_t ev_mask, const mepa_bool_t enable);
typedef mepa_rc (*mepa_macsec_event_enable_get_t)(struct mepa_device *dev, const mepa_port_no_t port_no, mepa_macsec_event_t *const ev_mask);
typedef mepa_rc (*mepa_macsec_event_poll_t)(struct mepa_device *dev, const mepa_port_no_t port_no, mepa_macsec_event_t *const ev_mask);
typedef mepa_rc (*mepa_macsec_event_seq_threshold_set_t)(struct mepa_device *dev, const mepa_port_no_t port_no, const uint32_t threshold);
typedef mepa_rc (*mepa_macsec_event_seq_threshold_get_t)(struct mepa_device *dev, const mepa_port_no_t port_no, uint32_t *const threshold);
typedef mepa_rc (*mepa_macsec_egr_intr_sa_get_t)(struct mepa_device *dev, const mepa_port_no_t port_no, mepa_macsec_port_t *const port, uint16_t *const an);
typedef mepa_rc (*mepa_macsec_csr_read_t)(struct mepa_device *dev, const mepa_port_no_t port_no, const uint16_t mmd, const uint32_t addr, uint32_t *const value);
typedef mepa_rc (*mepa_macsec_csr_write_t)(struct mepa_device *dev, const mepa_port_no_t port_no, const uint32_t mmd, const uint32_t addr, const uint32_t value);

/** \brief PHY MACsec Driver */
typedef struct mepa_macsec_driver {
    mepa_macsec_init_set_t mepa_macsec_init_set;
    mepa_macsec_init_get_t mepa_macsec_init_get;
    mepa_macsec_secy_conf_add_t mepa_macsec_secy_conf_add;
    mepa_macsec_secy_conf_update_t mepa_macsec_secy_conf_update;
    mepa_macsec_secy_conf_get_t mepa_macsec_secy_conf_get;
    mepa_macsec_secy_conf_del_t mepa_macsec_secy_conf_del;
    mepa_macsec_secy_controlled_set_t mepa_macsec_secy_controlled_set;
    mepa_macsec_secy_controlled_get_t mepa_macsec_secy_controlled_get;
    mepa_macsec_secy_port_status_get_t mepa_macsec_secy_port_status_get;
    mepa_macsec_port_get_next_t mepa_macsec_port_get_next;
    mepa_macsec_rx_sc_add_t mepa_macsec_rx_sc_add;
    mepa_macsec_rx_sc_update_t mepa_macsec_rx_sc_update;
    mepa_macsec_rx_sc_get_conf_t mepa_macsec_rx_sc_get_conf;
    mepa_macsec_rx_sc_get_next_t mepa_macsec_rx_sc_get_next;
    mepa_macsec_rx_sc_del_t mepa_macsec_rx_sc_del;
    mepa_macsec_rx_sc_status_get_t mepa_macsec_rx_sc_status_get;
    mepa_macsec_tx_sc_set_t mepa_macsec_tx_sc_set;
    mepa_macsec_tx_sc_update_t mepa_macsec_tx_sc_update;
    mepa_macsec_tx_sc_get_conf_t mepa_macsec_tx_sc_get_conf;
    mepa_macsec_tx_sc_del_t mepa_macsec_tx_sc_del;
    mepa_macsec_tx_sc_status_get_t mepa_macsec_tx_sc_status_get;
    mepa_macsec_rx_sa_set_t mepa_macsec_rx_sa_set;
    mepa_macsec_rx_sa_get_t mepa_macsec_rx_sa_get;
    mepa_macsec_rx_sa_activate_t mepa_macsec_rx_sa_activate;
    mepa_macsec_rx_sa_disable_t mepa_macsec_rx_sa_disable;
    mepa_macsec_rx_sa_del_t mepa_macsec_rx_sa_del;
    mepa_macsec_rx_sa_lowest_pn_update_t mepa_macsec_rx_sa_lowest_pn_update;
    mepa_macsec_rx_sa_status_get_t mepa_macsec_rx_sa_status_get;
    mepa_macsec_rx_seca_set_t mepa_macsec_rx_seca_set;
    mepa_macsec_rx_seca_get_t mepa_macsec_rx_seca_get;
    mepa_macsec_rx_seca_lowest_pn_update_t mepa_macsec_rx_seca_lowest_pn_update;
    mepa_macsec_tx_sa_set_t mepa_macsec_tx_sa_set;
    mepa_macsec_tx_sa_get_t mepa_macsec_tx_sa_get;
    mepa_macsec_tx_sa_activate_t mepa_macsec_tx_sa_activate;
    mepa_macsec_tx_sa_disable_t mepa_macsec_tx_sa_disable;
    mepa_macsec_tx_sa_del_t mepa_macsec_tx_sa_del;
    mepa_macsec_tx_sa_status_get_t mepa_macsec_tx_sa_status_get;
    mepa_macsec_tx_seca_set_t mepa_macsec_tx_seca_set;
    mepa_macsec_tx_seca_get_t mepa_macsec_tx_seca_get;
    mepa_macsec_controlled_counters_get_t mepa_macsec_controlled_counters_get;
    mepa_macsec_uncontrolled_counters_get_t mepa_macsec_uncontrolled_counters_get;
    mepa_macsec_common_counters_get_t mepa_macsec_common_counters_get;
    mepa_macsec_secy_cap_get_t mepa_macsec_secy_cap_get;
    mepa_macsec_secy_counters_get_t mepa_macsec_secy_counters_get;
    mepa_macsec_counters_update_t mepa_macsec_counters_update;
    mepa_macsec_counters_clear_t mepa_macsec_counters_clear;
    mepa_macsec_rx_sc_counters_get_t mepa_macsec_rx_sc_counters_get;
    mepa_macsec_tx_sc_counters_get_t mepa_macsec_tx_sc_counters_get;
    mepa_macsec_tx_sa_counters_get_t mepa_macsec_tx_sa_counters_get;
    mepa_macsec_rx_sa_counters_get_t mepa_macsec_rx_sa_counters_get;
    mepa_macsec_control_frame_match_conf_set_t mepa_macsec_control_frame_match_conf_set;
    mepa_macsec_control_frame_match_conf_del_t mepa_macsec_control_frame_match_conf_del; 
    mepa_macsec_control_frame_match_conf_get_t mepa_macsec_control_frame_match_conf_get;
    mepa_macsec_pattern_set_t mepa_macsec_pattern_set;
    mepa_macsec_pattern_del_t mepa_macsec_pattern_del;
    mepa_macsec_pattern_get_t mepa_macsec_pattern_get;
    mepa_macsec_default_action_set_t mepa_macsec_default_action_set;
    mepa_macsec_default_action_get_t mepa_macsec_default_action_get;
    mepa_macsec_bypass_mode_set_t mepa_macsec_bypass_mode_set;
    mepa_macsec_bypass_mode_get_t mepa_macsec_bypass_mode_get;
    mepa_macsec_bypass_tag_set_t mepa_macsec_bypass_tag_set;
    mepa_macsec_bypass_tag_get_t mepa_macsec_bypass_tag_get;
    mepa_macsec_mtu_set_t mepa_macsec_mtu_set;
    mepa_macsec_mtu_get_t mepa_macsec_mtu_get;
    mepa_macsec_frame_capture_set_t mepa_macsec_frame_capture_set;
    mepa_macsec_frame_get_t mepa_macsec_frame_get;
    mepa_macsec_event_enable_set_t mepa_macsec_event_enable_set;
    mepa_macsec_event_enable_get_t mepa_macsec_event_enable_get;
    mepa_macsec_event_poll_t mepa_macsec_event_poll;
    mepa_macsec_event_seq_threshold_set_t mepa_macsec_event_seq_threshold_set;
    mepa_macsec_event_seq_threshold_get_t mepa_macsec_event_seq_threshold_get;
    mepa_macsec_egr_intr_sa_get_t mepa_macsec_egr_intr_sa_get;
    mepa_macsec_csr_read_t mepa_macsec_csr_read;
    mepa_macsec_csr_write_t mepa_macsec_csr_write;

} mepa_macsec_driver_t;

#endif


