/***************************************************************************
 *                                                                         *
 *   Copyright (C) 2017  Seamly, LLC                                       *
 *                                                                         *
 *   https://github.com/fashionfreedom/seamly2d                             *
 *                                                                         *
 ***************************************************************************
 **
 **  Seamly2D is free software: you can redistribute it and/or modify
 **  it under the terms of the GNU General Public License as published by
 **  the Free Software Foundation, either version 3 of the License, or
 **  (at your option) any later version.
 **
 **  Seamly2D is distributed in the hope that it will be useful,
 **  but WITHOUT ANY WARRANTY; without even the implied warranty of
 **  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 **  GNU General Public License for more details.
 **
 **  You should have received a copy of the GNU General Public License
 **  along with Seamly2D.  If not, see <http://www.gnu.org/licenses/>.
 **
 **************************************************************************

 ************************************************************************
 **
 **  @file   main.cpp
 **  @author Roman Telezhynskyi <dismine(at)gmail.com>
 **  @date   November 15, 2013
 **
 **  @brief
 **  @copyright
 **  This source code is part of the Valentine project, a pattern making
 **  program, whose allow create and modeling patterns of clothing.
 **  Copyright (C) 2013-2015 Seamly2D project
 **  <https://github.com/fashionfreedom/seamly2d> All Rights Reserved.
 **
 **  Seamly2D is free software: you can redistribute it and/or modify
 **  it under the terms of the GNU General Public License as published by
 **  the Free Software Foundation, either version 3 of the License, or
 **  (at your option) any later version.
 **
 **  Seamly2D is distributed in the hope that it will be useful,
 **  but WITHOUT ANY WARRANTY; without even the implied warranty of
 **  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 **  GNU General Public License for more details.
 **
 **  You should have received a copy of the GNU General Public License
 **  along with Seamly2D.  If not, see <http://www.gnu.org/licenses/>.
 **
 *************************************************************************/

/*
 * @brief entry point of Seamly2D application
 * @return non-zero value is code of the error
 */

#include "mainwindow.h"
#include "core/vapplication.h"
#include "../vpatterndb/vpiecenode.h"

#include <QApplication>
#include <QMessageBox> // For QT_REQUIRE_VERSION
#include <QTimer>

//---------------------------------------------------------------------------------------------------------------------
// Entry point of the program
int main(int argc, char *argv[])
{
    // Initialize Qt resources for various components
    Q_INIT_RESOURCE(cursor);
    Q_INIT_RESOURCE(icon);
    Q_INIT_RESOURCE(schema);
    Q_INIT_RESOURCE(theme);
    Q_INIT_RESOURCE(flags);
    Q_INIT_RESOURCE(icons);
    Q_INIT_RESOURCE(toolicon);
    Q_INIT_RESOURCE(sounds);

    // Ensure the required version of Qt is present
    QT_REQUIRE_VERSION(argc, argv, "5.15.2");

    // Register meta-type stream operators for VPieceNode
    // This allows serialization and deserialization of VPieceNode objects
    qRegisterMetaTypeStreamOperators<VPieceNode>("VPieceNode");

    //------------------------------------------------------------------------
    // On macOS, configure WebView / QtQuick compositing and stacking
    // Requires running Qt in layer-backed mode for correct rendering
    qWarning("Seamly2D: Setting QT_MAC_WANTS_LAYER=1 and QSG_RENDER_LOOP=basic");
    qputenv("QT_MAC_WANTS_LAYER", "1");
    //------------------------------------------------------------------------

    // Enable high DPI scaling, excluding macOS which supports it natively
#ifndef Q_OS_MAC
    initHighDpiScaling(argc, argv);
#endif //Q_OS_MAC

    // Create application instance
    VApplication app(argc, argv);

    // Initialize application options
    app.InitOptions();

    // Create main window
    MainWindow w;

    // Set window icon (excluding macOS)
#if !defined(Q_OS_MAC)
    app.setWindowIcon(QIcon(":/icon/64x64/icon64x64.png"));
#endif // !defined(Q_OS_MAC)

    // Set main window for the application
    app.setMainWindow(&w);

    // Initialize delay for correct fitbest zoom
    int msec = 0;

    // Before loading the pattern, show the main window if in GUI mode
    if (VApplication::IsGUIMode())
    {
        w.show();
        msec = 15; // set delay for correct the first fitbest zoom
    }

    // Use a single shot timer to process command after a delay
    QTimer::singleShot(msec, &w, SLOT(ProcessCMD()));

    // Start the application event loop
    return app.exec();
}
