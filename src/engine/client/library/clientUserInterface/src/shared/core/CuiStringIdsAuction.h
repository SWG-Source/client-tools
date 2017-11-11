//======================================================================
//
// CuiStringIdsAuction.h
// copyright (c) 2002 Sony Online Entertainment
//
//======================================================================

#ifndef INCLUDED_CuiStringIdsAuction_H
#define INCLUDED_CuiStringIdsAuction_H

#include "StringId.h"

//======================================================================

#define MAKE_STRING_ID(a, b) const StringId b(#a,#b);

//----------------------------------------------------------------------

namespace CuiStringIdsAuction
{
	MAKE_STRING_ID(ui_auc,      err_auction_terminal_error);
	MAKE_STRING_ID(ui_auc,      accept_bid_successful);
	MAKE_STRING_ID(ui_auc,      bid_successful);
	MAKE_STRING_ID(ui_auc,      buy);
	MAKE_STRING_ID(ui_auc,      buy_successful_local);
	MAKE_STRING_ID(ui_auc,      buy_successful_planet_prose);
	MAKE_STRING_ID(ui_auc,      buy_successful_galaxy_prose);
	MAKE_STRING_ID(ui_auc,      buy_vendor_buyer_successful);
	MAKE_STRING_ID(ui_auc,      buy_vendor_owner_successful_remote);
	MAKE_STRING_ID(ui_auc,      buy_vendor_owner_successful_local);
	MAKE_STRING_ID(ui_auc,      cancel_successful);
	MAKE_STRING_ID(ui_auc,      confirm_accept);
	MAKE_STRING_ID(ui_auc,      confirm_withdraw);
	MAKE_STRING_ID(ui_auc,      container_warning_prose);
	MAKE_STRING_ID(ui_auc,      cpt_auction_list);
	MAKE_STRING_ID(ui_auc,      cpt_vendor_list);
	MAKE_STRING_ID(ui_auc,      cpt_vendor_owner_list);
	MAKE_STRING_ID(ui_auc,      create_auction_successful);
	MAKE_STRING_ID(ui_auc,      create_sale_successful);
	MAKE_STRING_ID(ui_auc,      details_high_bid);
	MAKE_STRING_ID(ui_auc,      details_instant_sale);
	MAKE_STRING_ID(ui_auc,      details_vendor_price);
	MAKE_STRING_ID(ui_auc,      place_bid);
	MAKE_STRING_ID(ui_auc,      reject);
	MAKE_STRING_ID(ui_auc,      retrieve_successful);
	MAKE_STRING_ID(ui_auc,      withdraw);
	MAKE_STRING_ID(ui_auc,      waiting_auction_info);

	MAKE_STRING_ID(ui_auc,      waiting_reject);
	MAKE_STRING_ID(ui_auc,      confirm_reject);

	MAKE_STRING_ID(ui_auc,      list_tab_all);
	MAKE_STRING_ID(ui_auc,      list_tab_available);
	MAKE_STRING_ID(ui_auc,      list_tab_bids);
	MAKE_STRING_ID(ui_auc,      list_tab_sales);
	MAKE_STRING_ID(ui_auc,      list_tab_vendor_buyer_offers);
	MAKE_STRING_ID(ui_auc,      list_tab_vendor_buyer_selling);
	MAKE_STRING_ID(ui_auc,      list_tab_vendor_seller_offers);
	MAKE_STRING_ID(ui_auc,      list_tab_vendor_seller_selling);
	MAKE_STRING_ID(ui_auc,      list_tab_vendor_seller_stockroom);
	MAKE_STRING_ID(ui_auc,      list_tab_location);

	MAKE_STRING_ID(ui_auc,      err_accept_bid_invalid_auctioner);
	MAKE_STRING_ID(ui_auc,      err_accept_bid_invalid_item);
	MAKE_STRING_ID(ui_auc,      err_accept_bid_not_owner);
	MAKE_STRING_ID(ui_auc,      err_accept_bid_no_bids);
	MAKE_STRING_ID(ui_auc,      err_accept_bid_auction_already_complete);
	MAKE_STRING_ID(ui_auc,      err_bid_invalid);
	MAKE_STRING_ID(ui_auc,      err_bid_invalid_auctioner);
	MAKE_STRING_ID(ui_auc,      err_bid_invalid_container);
	MAKE_STRING_ID(ui_auc,      err_bid_invalid_item);
	MAKE_STRING_ID(ui_auc,      err_bid_not_enough_money);
	MAKE_STRING_ID(ui_auc,      err_bid_outbid);
	MAKE_STRING_ID(ui_auc,      err_bid_rejected);
	MAKE_STRING_ID(ui_auc,      err_bid_bid_too_high);
	MAKE_STRING_ID(ui_auc,      err_buy_invalid);
	MAKE_STRING_ID(ui_auc,      err_buy_invalid_auctioner);
	MAKE_STRING_ID(ui_auc,      err_buy_invalid_container);
	MAKE_STRING_ID(ui_auc,      err_buy_invalid_item);
	MAKE_STRING_ID(ui_auc,      err_buy_not_enough_money);
	MAKE_STRING_ID(ui_auc,      err_buy_rejected);
	MAKE_STRING_ID(ui_auc,      err_cancel_invalid_auctioner);
	MAKE_STRING_ID(ui_auc,      err_cancel_invalid_item);
	MAKE_STRING_ID(ui_auc,      err_cancel_not_owner);
	MAKE_STRING_ID(ui_auc,      err_cancel_auction_already_completed);
	MAKE_STRING_ID(ui_auc,      err_create_auction_already_auctioned);
	MAKE_STRING_ID(ui_auc,      err_create_auction_invalid_auctioner);
	MAKE_STRING_ID(ui_auc,      err_create_auction_invalid_container);
	MAKE_STRING_ID(ui_auc,      err_create_auction_invalid_item);
	MAKE_STRING_ID(ui_auc,      err_create_auction_invalid_length);
	MAKE_STRING_ID(ui_auc,      err_create_auction_invalid_price);
	MAKE_STRING_ID(ui_auc,      err_create_auction_not_owner);
	MAKE_STRING_ID(ui_auc,      err_create_auction_in_trade);
	MAKE_STRING_ID(ui_auc,      err_create_auction_in_crate);
	MAKE_STRING_ID(ui_auc,      err_create_auction_not_allowed);
	MAKE_STRING_ID(ui_auc,      err_create_auction_not_empty);
	MAKE_STRING_ID(ui_auc,      err_create_auction_unknown);
	MAKE_STRING_ID(ui_auc,      err_create_auction_not_enough_money_prose);
	MAKE_STRING_ID(ui_auc,      err_create_auction_is_biolinked);
	MAKE_STRING_ID(ui_auc,      err_create_auction_item_equipped);

	MAKE_STRING_ID(ui_auc,      err_create_sale_bid_too_high);
	MAKE_STRING_ID(ui_auc,      err_create_auction_bid_too_high);
	MAKE_STRING_ID(ui_auc,      err_create_sale_price_too_high);

	MAKE_STRING_ID(ui_auc,      err_create_sale_already_auctioned);
	MAKE_STRING_ID(ui_auc,      err_create_sale_invalid_auctioner);
	MAKE_STRING_ID(ui_auc,      err_create_sale_invalid_container);
	MAKE_STRING_ID(ui_auc,      err_create_sale_invalid_item);
	MAKE_STRING_ID(ui_auc,      err_create_sale_invalid_length);
	MAKE_STRING_ID(ui_auc,      err_create_sale_invalid_price);
	MAKE_STRING_ID(ui_auc,      err_create_sale_not_owner);
	MAKE_STRING_ID(ui_auc,      err_create_sale_in_trade);
	MAKE_STRING_ID(ui_auc,      err_create_sale_in_crate);
	MAKE_STRING_ID(ui_auc,      err_create_sale_not_allowed);
	MAKE_STRING_ID(ui_auc,      err_create_sale_not_empty);
	MAKE_STRING_ID(ui_auc,      err_create_sale_unknown);
	MAKE_STRING_ID(ui_auc,      err_create_sale_not_enough_money_prose);
	MAKE_STRING_ID(ui_auc,      err_create_sale_is_biolinked);
	MAKE_STRING_ID(ui_auc,      err_create_sale_item_equipped);
	MAKE_STRING_ID(ui_auc,      err_too_many_auctions);
	MAKE_STRING_ID(ui_auc,      err_create_sale_too_many_vendors);
	MAKE_STRING_ID(ui_auc,      err_create_sale_too_many_vendor_items);

	MAKE_STRING_ID(ui_auc,      err_sell_cannot_sell_intangible);

	MAKE_STRING_ID(ui_auc,      err_no_item_selected);
	MAKE_STRING_ID(ui_auc,      err_retrieve_invalid_auctioner);
	MAKE_STRING_ID(ui_auc,      err_retrieve_invalid_container);
	MAKE_STRING_ID(ui_auc,      err_retrieve_invalid_item);
	MAKE_STRING_ID(ui_auc,		err_retrieve_item_does_not_exist);
	MAKE_STRING_ID(ui_auc,		err_retrieve_reimbursed);
	MAKE_STRING_ID(ui_auc,      err_retrieve_not_owner);
	MAKE_STRING_ID(ui_auc,      err_selected_item_invalid);
	MAKE_STRING_ID(ui_auc,      err_sell_invalid_instant_price);
	MAKE_STRING_ID(ui_auc,      err_sell_invalid_min_bid);
	MAKE_STRING_ID(ui_auc,      err_sell_invalid_time);
	MAKE_STRING_ID(ui_auc,      err_sell_no_item_selected);
	MAKE_STRING_ID(ui_auc,      err_text_filter_token_too_small);
	MAKE_STRING_ID(ui_auc,      err_vendor_sell_invalid_price);
	MAKE_STRING_ID(ui_auc,      err_vendor_sell_no_item_selected);
	MAKE_STRING_ID(ui_auc,      err_vendor_terminal_error);
	MAKE_STRING_ID(ui_auc,      err_vendor_terminal_no_owner);
	MAKE_STRING_ID(ui_auc,      err_vendor_deactivated);
	MAKE_STRING_ID(ui_auc,      err_inventory_full);
	MAKE_STRING_ID(ui_auc,      err_retrieve_not_at_location_prose);
	MAKE_STRING_ID(ui_auc,      err_cannot_relist_offered_items);
	MAKE_STRING_ID(ui_auc,      err_not_bazaar);
	MAKE_STRING_ID(ui_auc,      err_not_vendor);

	MAKE_STRING_ID(ui_auc,      table_bid);
	MAKE_STRING_ID(ui_auc,      table_high_bidder);
	MAKE_STRING_ID(ui_auc,      table_sale_type);
	MAKE_STRING_ID(ui_auc,      table_location);
	MAKE_STRING_ID(ui_auc,      table_my_bid);
	MAKE_STRING_ID(ui_auc,      table_my_proxy);
	MAKE_STRING_ID(ui_auc,      table_name);
	MAKE_STRING_ID(ui_auc,      table_owner);
	MAKE_STRING_ID(ui_auc,      table_price);
	MAKE_STRING_ID(ui_auc,      table_time);
	MAKE_STRING_ID(ui_auc,      table_type);
	MAKE_STRING_ID(ui_auc,      table_premium);
	MAKE_STRING_ID(ui_auc,      table_distance_to);
	MAKE_STRING_ID(ui_auc,      table_entrance_fee);

	MAKE_STRING_ID(ui_auc,      waiting_accept_bid);
	MAKE_STRING_ID(ui_auc,      waiting_bid);
	MAKE_STRING_ID(ui_auc,      waiting_buy);
	MAKE_STRING_ID(ui_auc,      waiting_details);
	MAKE_STRING_ID(ui_auc,      waiting_list);
	MAKE_STRING_ID(ui_auc,      waiting_retrieve);
	MAKE_STRING_ID(ui_auc,      waiting_sell);
	MAKE_STRING_ID(ui_auc,      waiting_vendor_owner);
	MAKE_STRING_ID(ui_auc,      waiting_vendor_sell);
	MAKE_STRING_ID(ui_auc,      waiting_withdraw);

	MAKE_STRING_ID(ui_auc,      sale_type_auction);
	MAKE_STRING_ID(ui_auc,      sale_type_instant);

	MAKE_STRING_ID(ui_auc,      loc_planet);
	MAKE_STRING_ID(ui_auc,      loc_region);
	MAKE_STRING_ID(ui_auc,      loc_market_name);
	
	
	MAKE_STRING_ID(ui_auc,      details);
	MAKE_STRING_ID(ui_auc,      sell_new);
	MAKE_STRING_ID(ui_auc,      sell_from_stock);
	MAKE_STRING_ID(ui_auc,      relist_from_stock);
	MAKE_STRING_ID(ui_auc,      offer);
	MAKE_STRING_ID(ui_auc,      accept_bid);
	MAKE_STRING_ID(ui_auc,      retrieve);	

	MAKE_STRING_ID(ui_auc,      list_test_status_no_entries);
	MAKE_STRING_ID(ui_auc,      list_test_status_no_list);
	MAKE_STRING_ID(ui_auc,      list_test_status_no_list_filter);

	MAKE_STRING_ID(ui_auc,      page_text_prefix);

	MAKE_STRING_ID(ui_auc,      waypoint_to_vendor);
}

//======================================================================

#endif
