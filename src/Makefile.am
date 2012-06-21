SUBDIRS=pixmaps

bin_PROGRAMS=aria
#aria_LDADD +=@GTK_LIBS@ @PTHREAD_LIBS@ @INTLLIBS@
CXXFLAGS=-O2 -Wall -DLOCALEDIR=\"$(localedir)\" -DDATADIR=\"$(datadir)\" -I$(top_builddir)/intl @GTK_CFLAGS@
#INCLUDES=@GTK_CFLAGS@
aria_SOURCES= main.cc initrc.cc download.cc gui_main.cc crc.cc\
            md5.c md5.h\
            md5check.cc md5check.h\
            gui_file.cc\
            gui_file_open_and_save.cc\
            gui_file_open_url.cc\
            gui_file_open_crc.cc\
            gui_file_open_md5.cc\
            gui_item.cc\
            gui_item_new_item.cc\
            gui_list.cc\
            gui_list_new_list.cc\
            gui_list_rename_list.cc\
            gui_file_find_hyperlink.cc\
            gui_edit.cc\
            gui_edit_search.cc\
            gui_download.cc\
            gui_download_check.cc\
            gui_option.cc\
            gui_popup.cc\
            gui_help.cc gui_toolbar.cc\
            gui_utils.cc gui_utils.h\
            ItemCache.cc ItemCache.h\
            SumInfo.cc SumInfo.h\
            ItemStatusSum.cc ItemStatusSum.h\
            LockList.cc LockList.h\
            Dialog.cc Dialog.h\
            FileBrowser.cc FileBrowser.h\
            PasteWindow.cc PasteWindow.h\
            HistoryWindow.cc HistoryWindow.h\
            Basket.cc Basket.h\
            HTMLparse.cc HTMLparse.h\
            TimerData.cc TimerData.h\
            ItemStatusReq.cc ItemStatusReq.h\
            ItemStatusRec.cc ItemStatusRec.h\
            CRCList.cc CRCList.h\
            CtrlSocket.cc CtrlSocket.h\
            ListEntry.cc ListEntry.h\
            ListManager.cc ListManager.h\
            ItemOption.cc ItemOption.h\
            Options.cc Options.h\
            ItemList.cc ItemList2.cc ItemList.h\
            ItemCell.cc ItemCell.h\
            ItemCellPartial.cc ItemCellPartial.h\
            Retrieve.cc Retrieve.h\
            RetrieveHTTP.cc RetrieveHTTP.h\
            RetrieveHTTP2.cc RetrieveHTTP2.h\
            RetrieveHTTPS.cc RetrieveHTTPS.h\
            RetrieveHTTPS2.cc RetrieveHTTPS2.h\
            RetrieveFTP.cc RetrieveFTP.h\
            RetrieveFTP2.cc RetrieveFTP2.h\
            URLcontainer.cc URLcontainer.h\
            HTTP_Header.cc HTTP_Header.h\
            HTTPcontainer.cc HTTPcontainer.h\
            FTPcontainer.cc FTPcontainer.h\
            ItemLogCell.cc ItemLogCell.h\
            ItemStatus.cc ItemStatus.h\
            ItemStatusDynamic.cc ItemStatusDynamic.h\
            ItemStatusLog.cc ItemStatusLog.h\
            ItemStatusPartial.cc ItemStatusPartial.h\
            ThreadSlot.cc ThreadSlot.h\
            ThreadManager.cc ThreadManager.h\
            ItemManager.cc ItemManager.h\
            AppOption.cc AppOption.h\
            CheckCList.cc\
            Proxyserver.cc Proxyserver.h\
            ProxyList.cc ProxyList.h\
            UseragentList.cc UseragentList.h\
            Socket.cc Socket.h\
            SocketPool.cc SocketPool.h\
            SocketPoolCell.cc SocketPoolCell.h\
            ServerTemplate.h ServerTemplate.cc\
            ServerTemplateList.h ServerTemplateList.cc\
            Command.h Command.cc\
            CommandList.h CommandList.cc\
            TagParse.h TagParse.cc\
            Session.h Session.cc\
            Cookie.h Cookie.cc\
            CookieList.h CookieList.cc\
            dirbrowser.cc\
            utils.cc utils.h\
            StringHash.cc StringHash.h\
            Base64.cc Base64.h\
            getopt.c getopt1.c getopt.h\
            aria_mime.h\
            aria.h

