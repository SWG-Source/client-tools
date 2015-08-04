AUTOMAKE_OPTIONS=foreign
SUBDIRS=platform .

all-local:
	cd $(top_srcdir)/external/3rd/library/soePlatform/Base/$(platform) && $(MAKE) -j1 all
	cd $(top_srcdir)/external/3rd/library/soePlatform/ChatAPI2/ChatAPI/projects/Chat/ChatMono && $(MAKE) -j1 all
	cd $(top_srcdir)/external/3rd/library/soePlatform/ChatAPI2/ChatAPI/utils/Base/$(platform) && $(MAKE) -j1 all
	cd $(top_srcdir)/external/3rd/library/soePlatform/ChatAPI2/ChatAPI/utils/GenericAPI && $(MAKE) -j1 all
	cd $(top_srcdir)/external/3rd/library/soePlatform/ChatAPI2/ChatAPI/utils/UdpLibrary && $(MAKE) -j1 all
	cd $(top_srcdir)/external/3rd/library/soePlatform/ChatAPI2/ChatAPI/utils/Unicode && $(MAKE) -j1 all
	cd $(top_srcdir)/external/3rd/library/soePlatform/VChatAPI/projects/VChat/VChatAPI && $(MAKE) -j1 all
	cd $(top_srcdir)/external/3rd/library/soePlatform/VChatAPI/utils2.0/utils/Base && $(MAKE) -j1 all
	cd $(top_srcdir)/external/3rd/library/soePlatform/VChatAPI/utils2.0/utils/TcpLibrary && $(MAKE) -j1 all
	cd $(top_srcdir)/external/3rd/library/soePlatform/VChatAPI/utils2.0/utils/UdpLibrary && $(MAKE) -j1 all
	cd $(top_srcdir)/external/3rd/library/soePlatform/CSAssist/projects/CSAssist/CSAssistgameapi && $(MAKE) -j1 all
	cd $(top_srcdir)/external/3rd/library/soePlatform/CSAssist/utils/Base/$(platform) && $(MAKE) -j1 all
	cd $(top_srcdir)/external/3rd/library/soePlatform/CSAssist/utils/TcpLibrary && $(MAKE) -j1 all
	cd $(top_srcdir)/external/3rd/library/soePlatform/CTServiceGameAPI && $(MAKE) -j1 all
	cd $(top_srcdir)/external/3rd/library/platform/projects/Session/CommonAPI && $(MAKE) -j1 all
	cd $(top_srcdir)/external/3rd/library/platform/projects/Session/LoginAPI && $(MAKE) -j1 all

clean-local:
	cd $(top_srcdir)/external/3rd/library/soePlatform/Base/$(platform) && $(MAKE) -j1 clean

	cd $(top_srcdir)/external/3rd/library/soePlatform/ChatAPI2/ChatAPI/projects/Chat/ChatMono && $(MAKE) -j1 clean
	cd $(top_srcdir)/external/3rd/library/soePlatform/ChatAPI2/ChatAPI/utils/Base/$(platform) && $(MAKE) -j1 clean
	cd $(top_srcdir)/external/3rd/library/soePlatform/ChatAPI2/ChatAPI/utils/GenericAPI && $(MAKE) -j1 clean
	cd $(top_srcdir)/external/3rd/library/soePlatform/ChatAPI2/ChatAPI/utils/UdpLibrary && $(MAKE) -j1 clean
	cd $(top_srcdir)/external/3rd/library/soePlatform/ChatAPI2/ChatAPI/utils/Unicode && $(MAKE) -j1 clean
	cd $(top_srcdir)/external/3rd/library/soePlatform/VChatAPI/projects/VChat/VChatAPI && $(MAKE) -j1 clean
	cd $(top_srcdir)/external/3rd/library/soePlatform/VChatAPI/utils2.0/utils/Base && $(MAKE) -j1 clean
	cd $(top_srcdir)/external/3rd/library/soePlatform/VChatAPI/utils2.0/utils/TcpLibrary && $(MAKE) -j1 clean
	cd $(top_srcdir)/external/3rd/library/soePlatform/VChatAPI/utils2.0/utils/UdpLibrary && $(MAKE) -j1 clean
	cd $(top_srcdir)/external/3rd/library/soePlatform/CSAssist/projects/CSAssist/CSAssistgameapi && $(MAKE) -j1 clean
	cd $(top_srcdir)/external/3rd/library/soePlatform/CSAssist/utils/Base/$(platform) && $(MAKE) -j1 clean
	cd $(top_srcdir)/external/3rd/library/soePlatform/CSAssist/utils/TcpLibrary && $(MAKE) -j1 clean
	cd $(top_srcdir)/external/3rd/library/soePlatform/CTServiceGameAPI && $(MAKE) -j1 clean
	cd $(top_srcdir)/external/3rd/library/platform/projects/Session/CommonAPI && $(MAKE) -j1 clean
	cd $(top_srcdir)/external/3rd/library/platform/projects/Session/LoginAPI && $(MAKE) -j1 clean
