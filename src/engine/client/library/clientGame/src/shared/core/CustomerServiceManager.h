// ============================================================================
// 
// CustomerServiceManager.h
// Copyright Sony Online Entertainment, Inc.
//
// ============================================================================

#ifndef INCLUDED_CustomerServiceManager_H
#define INCLUDED_CustomerServiceManager_H

class CustomerServiceCategory;
class CustomerServiceComment;
class CustomerServiceTicket;
class GetTicketsResponseMessage;

//-----------------------------------------------------------------------------
class CustomerServiceManager
{
public:

	typedef stdvector<CustomerServiceCategory>::fwd TicketCategoryList;
	typedef stdvector<CustomerServiceComment>::fwd  TicketCommentsList;
	typedef stdvector<CustomerServiceTicket>::fwd   TicketList;

	struct Messages
	{
		struct RequestTicketsResponse
		{
			typedef bool Response;
		};

		struct RequestTicketCommentsResponse
		{
			typedef bool Response;
		};

		struct RequestTicketCategoriesResponse
		{
			typedef bool Response;
		};
	};

public:

	static void install();
	static void gameStart();
	static void alter(float const deltaTime);

	static int getMaximumNumberOfCustomerServiceTicketsAllowed();
	
	static bool isConnected();
	static void requestConnection();
	static void requestTicketCreation(std::string const &playerName, unsigned int const category, unsigned int const subCategory, Unicode::String const &details, Unicode::String const &harassingPlayerName);
	static void requestTicketComments(unsigned int const ticketId);
	static void requestTicketCancellation(unsigned int const ticketId, bool const emitTransceiver = true);
	static void requestAppendTicketComment(unsigned int const ticketId, std::string const &playerName, Unicode::String const &comment);
	static void cancelTickets();

	static std::string getPlayerInformation();

	static TicketCategoryList const &getServiceCategoryList();
	static TicketCategoryList const &getBugCategoryList();
	static TicketList const &        getTicketList();
	static TicketCommentsList const &getTicketCommentsList();
	
	static bool getCategoryText(int const categoryId, Unicode::String &categoryString);
	static bool getSubCategoryText(int const categoryId, int const subCategoryId, Unicode::String &subCategoryString);

	// Do not call the following directly. They are called from CuiCSManagerListener.

	static void requestConnectionResponse(bool const connected);
	static void requestTicketCategoriesResponse(TicketCategoryList const &ticketCategoryList);
	static void requestTicketsResponse(bool const success, GetTicketsResponseMessage const &response);
	static void requestTicketCreationResponse(bool const success, unsigned int const tickedId);
	static void requestTicketCancellationResponse(bool const success, unsigned int const tickedId);
	static void requestTicketCommentsResponse(bool const success, TicketCommentsList const &comments);
	static void requestAppendTicketCommentResponse(bool const success, unsigned int const ticketId);
	static void ticketChangeResponseMessage(unsigned int const ticketId);
	static void requestNewTicketActivityResponseMessage(bool const newActivity, unsigned int const ticketCount);

private:

	// Disable

	CustomerServiceManager();
	~CustomerServiceManager();
	CustomerServiceManager(CustomerServiceManager const &);
	CustomerServiceManager &operator =(CustomerServiceManager const &);
};

// ============================================================================

#endif // INCLUDED_CustomerServiceManager_H
