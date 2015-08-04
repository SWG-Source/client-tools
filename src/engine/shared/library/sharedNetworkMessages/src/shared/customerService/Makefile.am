AUTOMAKE_OPTIONS=foreign

lib_LTLIBRARIES=libsharedNetworkMessagessharedcustomerService.la
libsharedNetworkMessagessharedcustomerService_la_CXXFLAGS=-I$(srcdir)/../../../include/public \
	-I$(srcdir)/../../../include/private \
	-I$(engine_dir)/shared/library/sharedCompression/include/public \
	-I$(engine_dir)/shared/library/sharedDebug/include/public \
	-I$(engine_dir)/shared/library/sharedFoundation/include/public \
	-I$(engine_dir)/shared/library/sharedFoundationTypes/include/public \
	-I$(engine_dir)/shared/library/sharedLog/include/public \
	-I$(engine_dir)/shared/library/sharedMath/include/public \
	-I$(engine_dir)/shared/library/sharedMathArchive/include/public \
	-I$(engine_dir)/shared/library/sharedMessageDispatch/include/public \
	-I$(engine_dir)/shared/library/sharedMemoryManager/include/public \
	-I$(engine_dir)/shared/library/sharedSynchronization/include/public \
	-I$(engine_dir)/shared/library/sharedThread/include/public \
	 \
	-I$(udplibrary_dir) \
	-I$(archive_dir)/include \
	-I$(unicode_dir)/include \
	-I$(localization_dir)/include \
	-I$(unicodeArchive_dir)/include/public \
	-I$(localizationArchive_dir)/include/public \
	-I$(fileInterface_dir)/include/public \
	-Wno-ctor-dtor-privacy

libsharedNetworkMessagessharedcustomerService_la_SOURCES=AppendCommentMessage.cpp \
	AppendCommentMessage.h \
	AppendCommentResponseMessage.cpp \
	AppendCommentResponseMessage.h \
	CancelTicketMessage.cpp \
	CancelTicketMessage.h \
	CancelTicketResponseMessage.cpp \
	CancelTicketResponseMessage.h \
	ConnectPlayerMessage.cpp \
	ConnectPlayerMessage.h \
	ConnectPlayerResponseMessage.cpp \
	ConnectPlayerResponseMessage.h \
	CreateTicketMessage.cpp \
	CreateTicketMessage.h \
	CreateTicketResponseMessage.cpp \
	CreateTicketResponseMessage.h \
	CustomerServiceCategoryArchive.cpp \
	CustomerServiceCategoryArchive.h \
	CustomerServiceCategory.cpp \
	CustomerServiceCategory.h \
	CustomerServiceCommentArchive.cpp \
	CustomerServiceCommentArchive.h \
	CustomerServiceComment.cpp \
	CustomerServiceComment.h \
	CustomerServiceSearchResultArchive.cpp \
	CustomerServiceSearchResultArchive.h \
	CustomerServiceSearchResult.cpp \
	CustomerServiceSearchResult.h \
	CustomerServiceTicketArchive.cpp \
	CustomerServiceTicketArchive.h \
	CustomerServiceTicket.cpp \
	CustomerServiceTicket.h \
	DisconnectPlayerMessage.cpp \
	DisconnectPlayerMessage.h \
	DisconnectPlayerResponseMessage.cpp \
	DisconnectPlayerResponseMessage.h \
	GetArticleMessage.cpp \
	GetArticleMessage.h \
	GetArticleResponseMessage.cpp \
	GetArticleResponseMessage.h \
	GetCommentsMessage.cpp \
	GetCommentsMessage.h \
	GetCommentsResponseMessage.cpp \
	GetCommentsResponseMessage.h \
	GetTicketsMessage.cpp \
	GetTicketsMessage.h \
	GetTicketsResponseMessage.cpp \
	GetTicketsResponseMessage.h \
	NewTicketActivityMessage.cpp \
	NewTicketActivityMessage.h \
	NewTicketActivityResponseMessage.cpp \
	NewTicketActivityResponseMessage.h \
	RequestCategoriesMessage.cpp \
	RequestCategoriesMessage.h \
	RequestCategoriesResponseMessage.cpp \
	RequestCategoriesResponseMessage.h \
	SearchKnowledgeBaseMessage.cpp \
	SearchKnowledgeBaseMessage.h \
	SearchKnowledgeBaseResponseMessage.cpp \
	SearchKnowledgeBaseResponseMessage.h \
	VerifyPlayerNameResponseMessage.cpp \
	VerifyPlayerNameResponseMessage.h \
	VerifyPlayerNameMessage.cpp \
	VerifyPlayerNameMessage.h

libsharedNetworkMessagessharedcustomerService_la_LDFLAGS=-avoid-version

include $(engine_dir)/../make_include/engine.include
