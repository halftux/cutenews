/*
 * Copyright (C) 2017 Stuart Howarth <showarth@marxoft.co.uk>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 3 as
 * published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

import QtQuick 1.1
import com.nokia.symbian 1.1
import cuteNews 1.0

AppWindow {
    id: appWindow

    showStatusBar: true
    showToolBar: true
    initialPage: MainPage {
        id: mainPage
    }

    MyInfoBanner {
        id: infoBanner
    }
    
    PopupLoader {
        id: popups
    }
    
    Connections {
        target: notifier
        onError: infoBanner.information(errorString)
        onReadArticlesDeleted: infoBanner.information(count + " " + qsTr("read articles deleted"));
    }
    
    Connections {
        target: plugins
        onError: infoBanner.information(errorString)
    }
    
    Connections {
        target: transfers
        onTransferAdded: infoBanner.information(qsTr("Enclosure added to downloads"))
    }

    Component.onCompleted: {
        plugins.load();
        transfers.load();
        
        if ((settings.updateSubscriptionsOnStartup) && (!settings.offlineModeEnabled)) {
            subscriptions.updateAll();
        }
    }
}