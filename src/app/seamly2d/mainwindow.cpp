/******************************************************************************
 *   @file   mainwindow.cpp
 **  @author Douglas S Caskey
 **  @date   17 Sep, 2023
 **
 **  @brief
 **  @copyright
 **  This source code is part of the Seamly2D project, a pattern making
 **  program to create and model patterns of clothing.
 **  Copyright (C) 2017-2023 Seamly2D project
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

/************************************************************************
 **
 **  @file   mainwindow.cpp
 **  @author Roman Telezhynskyi <dismine(at)gmail.com>
 **  @date   November 15, 2013
 **
 **  @brief
 **  @copyright
 **  This source code is part of the Valentina project, a pattern making
 **  program, whose allow create and modeling patterns of clothing.
 **  Copyright (C) 2013 Valentina project
 **  <https://bitbucket.org/dismine/valentina> All Rights Reserved.
 **
 **  Valentina is free software: you can redistribute it and/or modify
 **  it under the terms of the GNU General Public License as published by
 **  the Free Software Foundation, either version 3 of the License, or
 **  (at your option) any later version.
 **
 **  Valentina is distributed in the hope that it will be useful,
 **  but WITHOUT ANY WARRANTY; without even the implied warranty of
 **  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 **  GNU General Public License for more details.
 **
 **  You should have received a copy of the GNU General Public License
 **  along with Valentina.  If not, see <http://www.gnu.org/licenses/>.
 **
 *************************************************************************/

#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "../vgeometry/vspline.h"
#include "../ifc/exception/vexceptionobjecterror.h"
#include "../ifc/exception/vexceptionconversionerror.h"
#include "../ifc/exception/vexceptionemptyparameter.h"
#include "../ifc/exception/vexceptionwrongid.h"
#include "../ifc/exception/vexceptionundo.h"
#include "version.h"
#include "core/vapplication.h"
#include "../vmisc/customevents.h"
#include "../vmisc/vsettings.h"
#include "../vmisc/def.h"
#include "../vmisc/qxtcsvmodel.h"
#include "../vmisc/dialogs/dialogexporttocsv.h"
#include "undocommands/rename_draftblock.h"
#include "core/vtooloptionspropertybrowser.h"
#include "options.h"
#include "../ifc/xml/vpatternconverter.h"
#include "../vmisc/logging.h"
#include "../vformat/measurements.h"
#include "../ifc/xml/multi_size_converter.h"
#include "../ifc/xml/individual_size_converter.h"
#include "../vwidgets/vwidgetpopup.h"
#include "../vwidgets/vmaingraphicsscene.h"
#include "../vwidgets/mouse_coordinates.h"
#include "../vtools/tools/drawTools/drawtools.h"
#include "../vtools/dialogs/tooldialogs.h"
#include "tools/pattern_piece_tool.h"
#include "tools/nodeDetails/vtoolinternalpath.h"
#include "tools/nodeDetails/anchorpoint_tool.h"
#include "tools/union_tool.h"
#include "dialogs/dialogs.h"

#include "../vtools/undocommands/addgroup.h"
#include "../vtools/undocommands/label/showpointname.h"
#include "../vpatterndb/vpiecepath.h"
#include "../qmuparser/qmuparsererror.h"
#include "../vtools/dialogs/support/editlabeltemplate_dialog.h"

#include <QInputDialog>
#include <QtDebug>
#include <QMessageBox>
#include <QShowEvent>
#include <QScrollBar>
#include <QFileDialog>
#include <QSourceLocation>
#include <QUndoStack>
#include <QAction>
#include <QProcess>
#include <QSettings>
#include <QTimer>
#include <QtGlobal>
#include <QDesktopWidget>
#include <QDesktopServices>
#include <chrono>
#include <thread>
#include <QFileSystemWatcher>
#include <QComboBox>
#include <QFontComboBox>
#include <QTextCodec>
#include <QDoubleSpinBox>
#include <QSharedPointer>

#if defined(Q_OS_MAC)
#include <QMimeData>
#include <QDrag>
#endif //defined(Q_OS_MAC)

QT_WARNING_PUSH
QT_WARNING_DISABLE_CLANG("-Wmissing-prototypes")
QT_WARNING_DISABLE_INTEL(1418)

Q_LOGGING_CATEGORY(vMainWindow, "v.mainwindow")

QT_WARNING_POP

const QString autosavePrefix = QStringLiteral(".autosave");

// String below need for getting translation for key Ctrl
const QString strQShortcut   = QStringLiteral("QShortcut"); // Context
const QString strCtrl        = QStringLiteral("Ctrl"); // String

//---------------------------------------------------------------------------------------------------------------------
/**
 * @brief MainWindow constructor.
 * @param parent parent widget.
 */
// Constructor for MainWindow class, derived from MainWindowsNoGUI
MainWindow::MainWindow(QWidget *parent)
    : MainWindowsNoGUI(parent)
    , ui(new Ui::MainWindow)
    , watcher(new QFileSystemWatcher(this))
    , currentTool(Tool::Arrow)
    , lastUsedTool(Tool::Arrow)
    , draftScene(nullptr)
    , pieceScene(nullptr)
    , mouseCoordinates(nullptr)
    , infoToolButton(nullptr)
    , helpLabel(nullptr)
    , isInitialized(false)
    , mChanges(false)
    , mChangesAsked(true)
    , patternReadOnly(false)
    , dialogTable(nullptr)
    , dialogTool()
    , historyDialog(nullptr)
    , fontComboBox(nullptr)
    , fontSizeComboBox(nullptr)
    , draftBlockComboBox(nullptr)
    , draftBlockLabel(nullptr)
    , mode(Draw::Calculation)
    , currentBlockIndex(0)
    , currentToolBoxIndex(0)
    , isToolOptionsDockVisible(true)
    , isGroupsDockVisible(true)
    , isLayoutsDockVisible(false)
    , isToolboxDockVisible(true)
    , drawMode(true)
    , recentFileActs()
    , separatorAct(nullptr)
    , leftGoToStage(nullptr)
    , rightGoToStage(nullptr)
    , autoSaveTimer(nullptr)
    , guiEnabled(true)
    , gradationHeights(nullptr)
    , gradationSizes(nullptr)
    , gradationHeightsLabel(nullptr)
    , gradationSizesLabel(nullptr)
    , toolProperties(nullptr)
    , groupsWidget(nullptr)
    , patternPiecesWidget(nullptr)
    , lock(nullptr)
    , zoomScaleSpinBox(nullptr)
    , m_penToolBar(nullptr)
    , m_penReset(nullptr)
    , m_zoomToPointComboBox(nullptr)
{
    // Initialize recentFileActs array elements to nullptr
    for (int i = 0; i < MaxRecentFiles; ++i)
    {
        recentFileActs[i] = nullptr;
    }

    // Create necessary connections and set up initial configurations
    CreateActions();
    InitScenes();
    doc = new VPattern(pattern, &mode, draftScene, pieceScene);
    connect(doc, &VPattern::ClearMainWindow, this, &MainWindow::Clear);
    connect(doc, &VPattern::patternChanged, this, &MainWindow::patternChangesWereSaved);
    connect(doc, &VPattern::UndoCommand, this, &MainWindow::fullParseFile);
    connect(doc, &VPattern::setGuiEnabled, this, &MainWindow::setGuiEnabled);
    // ...

    // Show the main window maximized
    showMaximized();

    // Initialize docks, menus, and toolbars
    InitDocksContain();
    CreateMenus();
    initDraftToolBar();
    initPointNameToolBar();
    initModesToolBar();
    InitToolButtons();
    initPenToolBar();

    // Set initial status message in the status bar
    helpLabel = new QLabel(QObject::tr("Create a new pattern piece to start working."));
    ui->statusBar->addWidget(helpLabel);

    // Initialize other toolbars and connect signals
    initToolsToolBar();
    connect(qApp->getUndoStack(), &QUndoStack::cleanChanged, this, &MainWindow::patternChangesWereSaved);

    // Initialize auto-save functionality
    InitAutoSave();

    // Set default index for the draft toolbox
    ui->draft_ToolBox->setCurrentIndex(0);

    // Read settings and initialize toolbar visibility
    ReadSettings();
    initToolBarVisibility();

    // Set current file to an empty string
    setCurrentFile("");

    // Configure locale settings for Windows
    WindowsLocale();

    // Connect signal for listWidget
    connect(ui->listWidget, &QListWidget::currentRowChanged, this, &MainWindow::showLayoutPages);

    // Connect signal for listWidget    
    connect(watcher, &QFileSystemWatcher::fileChanged, this, &MainWindow::MeasurementsChanged);

    // Connect focus change signal for handling SeamlyMe measurements changes
    connect(qApp, &QApplication::focusChanged, this, [this](QWidget *old, QWidget *now)
    {
        // Handle focus in
        if (old == nullptr && isAncestorOf(now) == true)
        {
            static bool asking = false;
            if (!asking && mChanges && not mChangesAsked)
            {
                asking = true;
                mChangesAsked = true;
                // Prompt user to sync measurements if changes detected
                const auto answer = QMessageBox::question(this, tr("Measurements"),
                                                    tr("Measurements were changed. Do you want to sync measurements now?"),
                                                        QMessageBox::Yes|QMessageBox::No, QMessageBox::Yes);
                if (answer == QMessageBox::Yes)
                {
                    SyncMeasurements();
                }
                asking = false;
            }
        }
        // ...
    });

#if defined(Q_OS_MAC)
    // Configure icon size and appearance for Mac OS
    ui->draft_ToolBar->setIconSize(QSize(24, 24));
    ui->mode_ToolBar->setIconSize(QSize(24, 24));
    ui->edit_Toolbar->setIconSize(QSize(24, 24));
    ui->zoom_ToolBar->setIconSize(QSize(24, 24));

    setUnifiedTitleAndToolBarOnMac(true);

    // Create Mac OS Dock Menu
    QMenu *menu = new QMenu(this);
    // ...
#endif //defined(Q_OS_MAC)
}
// end MainWindow::MainWindow()

//---------------------------------------------------------------------------------------------------------------------
// Constructor for MainWindow class, derived from MainWindowsNoGUI
// Function to add a new draft block to the document
// Function to add a new draft block to the document
void MainWindow::addDraftBlock(const QString &blockName)
{
    // Attempt to append a new draft block to the document
    if (doc->appendDraftBlock(blockName) == false)
    {
        // Log a warning if there is an error creating the draft block
        qCWarning(vMainWindow, "Error creating draft block with the name %s.", qUtf8Printable(blockName));
        return; // Exit the function if the draft block creation fails
    }

    // If it's the first draft block, initialize origins and enable pieces mode
    if (draftBlockComboBox->count() == 0)
    {
        draftScene->InitOrigins();
        draftScene->enablePiecesMode(qApp->Seamly2DSettings()->getShowControlPoints());
        pieceScene->InitOrigins();
    }

    // Block signals temporarily to avoid unwanted signals during ComboBox manipulation
    draftBlockComboBox->blockSignals(true);
    // Add the new block to the ComboBox
    draftBlockComboBox->addItem(blockName);

    // Clear graphical objects in the pattern
    pattern->ClearGObjects();

    // Emit a signal to hide options for the previous tool
    emit ui->view->itemClicked(nullptr);

    // Generate a label for the new pattern piece
    const QString label = doc->GenerateLabel(LabelType::NewPatternPiece);
    // Get the starting position for the new point in the draft block
    const QPointF startPosition = draftBlockStartPosition();
    // Create a new point at the starting position
    VPointF *point = new VPointF(startPosition.x(), startPosition.y(), label, 5, 10);
    // Create a tool base point for the new point
    auto spoint = VToolBasePoint::Create(0, blockName, point, draftScene, doc, pattern, Document::FullParse,
                                            Source::FromGui);
    // Emit a signal indicating that the point was clicked in the view
    ui->view->itemClicked(spoint);

    // Enable tools and widgets
    setToolsEnabled(true);
    setWidgetsEnabled(true);

    // Set the current index in the ComboBox to the newly added block
    const qint32 index = draftBlockComboBox->findText(blockName);
    if (index != -1)
    { // -1 for not found
        draftBlockComboBox->setCurrentIndex(index);
    }
    else
    {
        draftBlockComboBox->setCurrentIndex(0);
    }
    // Unblock signals after manipulating ComboBox
    draftBlockComboBox->blockSignals(false);

    // Show the best fit for the new pattern piece in the view
    VMainGraphicsView::NewSceneRect(ui->view->scene(), ui->view, spoint);
    ui->view->zoom100Percent();

    // Enable the "New Draft" action in the UI
    ui->newDraft_Action->setEnabled(true);
    // Set help label text to an empty string
    helpLabel->setText("");
    // Update groups widget
    groupsWidget->updateGroups();
}
// End MainWindow::addDraftBlock()


//---------------------------------------------------------------------------------------------------------------------
// Function to determine the starting position for a new point in a draft block
QPointF MainWindow::draftBlockStartPosition() const
{
    // Define constants for initial origin and margin values
    const qreal originX = 30.0;
    const qreal originY = 40.0;
    const qreal margin = 40.0;

    // Check if there is more than one draft block
    if (draftBlockComboBox->count() > 1)
    {
        // Get the bounding rectangle of visible items in the draft scene
        const QRectF rect = draftScene->visibleItemsBoundingRect();

        // Determine the starting position based on the aspect ratio of the bounding rectangle
        if (rect.width() <= rect.height())
        {
            // If the width is less than or equal to the height, position to the right of the bounding rectangle
            return QPointF(rect.width() + margin, originY);
        }
        else
        {
            // If the height is less than the width, position below the bounding rectangle
            return QPointF(originX, rect.height() + margin);
        }
    }
    else
    {
        // If there is only one draft block, use the default origin position
        return QPointF(originX, originY);
    }
}

// End MainWindow::draftBlockStartPosition()

//---------------------------------------------------------------------------------------------------------------------
// Function to initialize graphics scenes and connections
void MainWindow::InitScenes()
{
    // Create a new Draft scene and set it as the current scene
    draftScene = new VMainGraphicsScene(this);
    currentScene = draftScene;
    qApp->setCurrentScene(&currentScene);

    // Connect signals for enabling item move and selection in the draft scene
    connect(this, &MainWindow::EnableItemMove, draftScene, &VMainGraphicsScene::EnableItemMove);
    connect(this, &MainWindow::ItemsSelection, draftScene, &VMainGraphicsScene::ItemsSelection);

    // Connect signals for enabling specific item types' selection in the draft scene
    connect(this, &MainWindow::EnableLabelSelection, draftScene, &VMainGraphicsScene::ToggleLabelSelection);
    connect(this, &MainWindow::EnablePointSelection, draftScene, &VMainGraphicsScene::TogglePointSelection);
    connect(this, &MainWindow::EnableLineSelection, draftScene, &VMainGraphicsScene::ToggleLineSelection);
    // ... (similar connections for other item types)

    // Connect signals for enabling hover effect on specific item types in the draft scene
    connect(this, &MainWindow::EnableLabelHover, draftScene, &VMainGraphicsScene::ToggleLabelHover);
    connect(this, &MainWindow::EnablePointHover, draftScene, &VMainGraphicsScene::TogglePointHover);
    connect(this, &MainWindow::EnableLineHover, draftScene, &VMainGraphicsScene::ToggleLineHover);
    // ... (similar connections for other item types)

    // Connect mouseMove signal from draft scene to MainWindow's MouseMove function
    connect(draftScene, &VMainGraphicsScene::mouseMove, this, &MainWindow::MouseMove);

    // Create a new piece scene
    pieceScene = new VMainGraphicsScene(this);

    // Connect signals for enabling item move and selection in the piece scene
    connect(this, &MainWindow::EnableItemMove, pieceScene, &VMainGraphicsScene::EnableItemMove);
    connect(this, &MainWindow::EnableNodeLabelSelection, pieceScene, &VMainGraphicsScene::ToggleNodeLabelSelection);
    connect(this, &MainWindow::EnableNodePointSelection, pieceScene, &VMainGraphicsScene::ToggleNodePointSelection);
    connect(this, &MainWindow::enablePieceSelection, pieceScene, &VMainGraphicsScene::togglePieceSelection);

    // Connect signals for enabling hover effect on specific item types in the piece scene
    connect(this, &MainWindow::EnableNodeLabelHover, pieceScene, &VMainGraphicsScene::ToggleNodeLabelHover);
    connect(this, &MainWindow::EnableNodePointHover, pieceScene, &VMainGraphicsScene::ToggleNodePointHover);
    connect(this, &MainWindow::enablePieceHover, pieceScene, &VMainGraphicsScene::togglePieceHover);

    // Connect mouseMove signal from piece scene to MainWindow's MouseMove function
    connect(pieceScene, &VMainGraphicsScene::mouseMove, this, &MainWindow::MouseMove);

    // Set the current scene in the graphics view
    ui->view->setScene(currentScene);

    // Set the current transform for both draft and piece scenes
    draftScene->setCurrentTransform(ui->view->transform());
    pieceScene->setCurrentTransform(ui->view->transform());

    // Connect signals for mouse release and zoom scale change in the graphics view
    connect(ui->view, &VMainGraphicsView::mouseRelease, this, [this](){EndVisualization(true);});
    connect(ui->view, &VMainGraphicsView::signalZoomScaleChanged, this, &MainWindow::zoomScaleChanged);

    // Set the size policy for the graphics view
    QSizePolicy policy(QSizePolicy::Expanding, Q
}
// End MainWindow::InitScenes()

//---------------------------------------------------------------------------------------------------------------------
// Function to open a measurement file and return a QSharedPointer to the MeasurementDoc
QSharedPointer<MeasurementDoc> MainWindow::openMeasurementFile(const QString &fileName)
{
    // Create a QSharedPointer to MeasurementDoc to manage the measurement document's lifecycle
    QSharedPointer<MeasurementDoc> measurements;

    // Check if the provided fileName is empty
    if (fileName.isEmpty())
    {
        return measurements; // Return an empty QSharedPointer if the fileName is empty
    }

    try
    {
        // Create a new MeasurementDoc with the given pattern
        measurements = QSharedPointer<MeasurementDoc>(new MeasurementDoc(pattern));

        // Set the size and height attributes based on the pattern's size and height
        measurements->setSize(VContainer::rsize());
        measurements->setHeight(VContainer::rheight());

        // Set the XML content of the measurement document using the provided fileName
        measurements->setXMLContent(fileName);

        // Check if the measurement file has an unknown format
        if (measurements->Type() == MeasurementsType::Unknown)
        {
            VException exception(tr("Measurement file has unknown format."));
            throw exception;
        }

        // Convert the measurement file content if it is of Multisize type
        if (measurements->Type() == MeasurementsType::Multisize)
        {
            MultiSizeConverter converter(fileName);
            measurements->setXMLContent(converter.Convert()); // Read again after conversion
        }
        else
        {
            IndividualSizeConverter converter(fileName);
            measurements->setXMLContent(converter.Convert()); // Read again after conversion
        }

        // Check if each known name in the measurement file is valid
        if (!measurements->eachKnownNameIsValid())
        {
            VException exception(tr("Measurement file contains invalid known measurement(s)."));
            throw exception;
        }

        // Check and validate required measurements in the measurement document
        checkRequiredMeasurements(measurements.data());

        // Check if the Multisize measurement file uses inches (unsupported by the application)
        if (measurements->Type() == MeasurementsType::Multisize)
        {
            if (measurements->MUnit() == Unit::Inch)
            {
                qCCritical(vMainWindow, "%s\n\n%s", qUtf8Printable(tr("Wrong units.")),
                            qUtf8Printable(tr("Application doesn't support multisize table with inches.")));
                measurements->clear();
                
                // Exit the application with an error code if not in GUI mode
                if (!VApplication::IsGUIMode())
                {
                    qApp->exit(V_EX_DATAERR);
                }
                return measurements;
            }
        }
    }

    // Catch any VException that may occur during the file processing
    catch (VException &exception)
    {
        qCCritical(vMainWindow, "%s\n\n%s\n\n%s", qUtf8Printable(tr("File exception.")),
                    qUtf8Printable(exception.ErrorMessage()), qUtf8Printable(exception.DetailedInformation()));

        // Clear the measurement document and exit the application with an error code if not in GUI mode
        measurements->clear();
        if (!VApplication::IsGUIMode())
        {
            qApp->exit(V_EX_NOINPUT);
        }
        return measurements;
    }

    // Return the QSharedPointer to the MeasurementDoc after successful processing
    return measurements;
}

// End MainWindow::OPenMeasurementFile()

//---------------------------------------------------------------------------------------------------------------------
// Function to load measurements from a file into the application
bool MainWindow::loadMeasurements(const QString &fileName)
{
    // Open the measurement file and obtain a QSharedPointer to MeasurementDoc
    QSharedPointer<MeasurementDoc> measurements = openMeasurementFile(fileName);

    // Check if the QSharedPointer is null (indicating a failure during file opening)
    if (measurements->isNull())
    {
        return false; // Return false if the opening of the measurement file fails
    }

    // Check if the pattern unit is inches and the measurement file type is Multisize
    if (qApp->patternUnit() == Unit::Inch && measurements->Type() == MeasurementsType::Multisize)
    {
        qWarning() << tr("Gradation doesn't support inches");
        return false; // Return false if inches are not supported for Multisize measurement files
    }

    try
    {
        // Set the pattern type in the application and initialize the status bar
        qApp->setPatternType(measurements->Type());
        initStatusBar();

        // Clear variables of type Measurement in the pattern
        pattern->ClearVariables(VarType::Measurement);

        // Read measurements from the MeasurementDoc
        measurements->readMeasurements();
    }

    // Catch an exception related to empty parameters in the MeasurementDoc
    catch (VExceptionEmptyParameter &exception)
    {
        qCCritical(vMainWindow, "%s\n\n%s\n\n%s", qUtf8Printable(tr("File exception.")),
                    qUtf8Printable(exception.ErrorMessage()), qUtf8Printable(exception.DetailedInformation()));

        // Exit the application with an error code if not in GUI mode
        if (!VApplication::IsGUIMode())
        {
            qApp->exit(V_EX_NOINPUT);
        }
        return false; // Return false if an exception is caught
    }

    // Process based on the type of measurement file (Multisize or Individual)
    if (measurements->Type() == MeasurementsType::Multisize)
    {
        // Set the size in the VContainer based on the converted BaseSize from the measurement file
        VContainer::setSize(UnitConvertor(measurements->BaseSize(), measurements->MUnit(),
                                          *measurements->GetData()->GetPatternUnit()));

        // Log information about the loaded Multisize measurement file
        qCInfo(vMainWindow, "Multisize file %s was loaded.", qUtf8Printable(fileName));

        // Set the height in the VContainer based on the converted BaseHeight from the measurement file
        VContainer::setHeight(UnitConvertor(measurements->BaseHeight(), measurements->MUnit(),
                                            *measurements->GetData()->GetPatternUnit()));

        // Mark the pattern as changed and emit an update signal for the pattern label
        doc->SetPatternWasChanged(true);
        emit doc->UpdatePatternLabel();
    }
    else if (measurements->Type() == MeasurementsType::Individual)
    {
        // Set size and height for Individual measurement files
        setSizeHeightForIndividualM();

        // Log information about the loaded Individual measurement file
        qCInfo(vMainWindow, "Individual file %s was loaded.", qUtf8Printable(fileName));
    }

    return true; // Return true to indicate successful loading of measurements
}
// End MainWindow::loadMeasurements()

//---------------------------------------------------------------------------------------------------------------------
// Function to update measurements based on a file, size, and height
bool MainWindow::updateMeasurements(const QString &fileName, int size, int height)
{
    // Open the measurement file and obtain a QSharedPointer to MeasurementDoc
    QSharedPointer<MeasurementDoc> measurements = openMeasurementFile(fileName);

    // Check if the QSharedPointer is null (indicating a failure during file opening)
    if (measurements->isNull())
    {
        return false; // Return false if the opening of the measurement file fails
    }

    // Check if the pattern type in the application matches the type of the opened measurement file
    if (qApp->patternType() != measurements->Type())
    {
        qCCritical(vMainWindow, "%s", qUtf8Printable(tr("Measurement files types have not match.")));

        // Exit the application with an error code if not in GUI mode
        if (!VApplication::IsGUIMode())
        {
            qApp->exit(V_EX_DATAERR);
        }
        return false; // Return false if the types do not match
    }

    try
    {
        // Clear variables of type Measurement in the pattern
        pattern->ClearVariables(VarType::Measurement);

        // Read measurements from the MeasurementDoc
        measurements->readMeasurements();
    }

    // Catch an exception related to empty parameters in the MeasurementDoc
    catch (VExceptionEmptyParameter &exception)
    {
        qCCritical(vMainWindow, "%s\n\n%s\n\n%s", qUtf8Printable(tr("File exception.")),
                    qUtf8Printable(exception.ErrorMessage()), qUtf8Printable(exception.DetailedInformation()));

        // Exit the application with an error code if not in GUI mode
        if (!VApplication::IsGUIMode())
        {
            qApp->exit(V_EX_NOINPUT);
        }
        return false; // Return false if an exception is caught
    }

    // Process based on the type of measurement file (Multisize or Individual)
    if (measurements->Type() == MeasurementsType::Multisize)
    {
        // Set the size and height in the VContainer based on the provided values
        VContainer::setSize(size);
        VContainer::setHeight(height);

        // Mark the pattern as changed and emit an update signal for the pattern label
        doc->SetPatternWasChanged(true);
        emit doc->UpdatePatternLabel();
    }
    else if (measurements->Type() == MeasurementsType::Individual)
    {
        // Set size and height for Individual measurement files
        setSizeHeightForIndividualM();
    }

    return true; // Return true to indicate successful update of measurements
}
// End MainWindow::updateMeasurements()

//---------------------------------------------------------------------------------------------------------------------
// Function to check if all required measurements are present in the provided MeasurementDoc
void MainWindow::checkRequiredMeasurements(const MeasurementDoc *measurements)
{
    // Get the list of all measurements from the provided MeasurementDoc
    auto tempMeasurements = measurements->ListAll();

    // Get the list of measurements from the current document (pattern)
    auto docMeasurements = doc->ListMeasurements();

    // Create a set of measurement names that are present in the pattern but not in the provided MeasurementDoc
    const QSet<QString> match = QSet<QString>(docMeasurements.begin(), docMeasurements.end()).
                                    subtract(QSet<QString>(tempMeasurements.begin(), tempMeasurements.end()));

    // Check if there are missing measurements
    if (!match.isEmpty())
    {
        // Convert the set of measurement names to a list for further processing
        QList<QString> list = match.values();

        // Translate measurement names to user-friendly names using TrVars
        for (int i = 0; i < list.size(); ++i)
        {
            list[i] = qApp->TrVars()->MToUser(list.at(i));
        }

        // Throw a VException indicating missing required measurements
        VException exception(tr("Measurement file doesn't include all the required measurements."));
        exception.AddMoreInformation(tr("Please provide additional measurements: %1").arg(QStringList(list).join(", ")));
        throw exception;
    }
}
// End MainWindow::checkRequiredMeasurements() 

//---------------------------------------------------------------------------------------------------------------------
/**
 * @brief Template function to set a tool button in the UI based on the provided parameters and show dialog.
 * @param checked true if tool button checked.
 * @param t tool type.
 * @param cursor path tool cursor icon.
 * @param toolTip first tooltipe.
 * @param closeDialogSlot function what handle after close dialog.
 */
template <typename Dialog, typename Func>
void MainWindow::SetToolButton(bool checked, Tool t, const QString &cursor, const QString &toolTip,
                                Func closeDialogSlot)
{
    if (checked)
    {
        // If the tool is checked, cancel any existing tool, disable item movement, and set the current tool
        CancelTool();
        emit EnableItemMove(false);
        currentTool = lastUsedTool = t;

        // Load the cursor resource for the tool, considering HiDPI versions
        auto cursorResource = cursor;
        if (qApp->devicePixelRatio() >= 2)
        {
            auto cursorHidpiResource = QString(cursor).replace(".png", "@2x.png");
            if (QFileInfo(cursorResource).exists())
            {
                cursorResource = cursorHidpiResource;
            }
        }

        // Set the cursor for the graphics view
        QPixmap pixmap(cursorResource);
        QCursor cur(pixmap, 2, 2);
        ui->view->viewport()->setCursor(cur);

        // Set the help label text and hide tool options in the view
        helpLabel->setText(toolTip);
        ui->view->setShowToolOptions(false);

        // Create a shared pointer to the tool's dialog based on the provided template parameter
        dialogTool = QSharedPointer<Dialog>(new Dialog(pattern, 0, this));

        // Perform additional setup based on the selected tool
        switch(t)
        {
            case Tool::ArcIntersectAxis:
                dialogTool->setWindowTitle("Point - Intersect Arc and Axis");
                break;
            case Tool::Midpoint:
                dialogTool->Build(t);
                break;
            case Tool::InternalPath:
            case Tool::AnchorPoint:
            case Tool::InsertNodes:
                dialogTool->SetPiecesList(doc->getActivePatternPieces());
                break;
            default:
                break;
        }

        // Get the current scene as a VMainGraphicsScene
        VMainGraphicsScene *scene = qobject_cast<VMainGraphicsScene *>(currentScene);
        SCASSERT(scene != nullptr)

        // Connect signals and slots between the scene and the tool's dialog
        connect(scene, &VMainGraphicsScene::ChosenObject, dialogTool.data(), &DialogTool::ChosenObject);
        connect(scene, &VMainGraphicsScene::SelectedObject, dialogTool.data(), &DialogTool::SelectedObject);
        connect(dialogTool.data(), &DialogTool::DialogClosed, this, closeDialogSlot);
        connect(dialogTool.data(), &DialogTool::ToolTip, this, &MainWindow::ShowToolTip);
        emit ui->view->itemClicked(nullptr);
    }
    else
    {
        // If the tool is not checked, ensure the corresponding tool button is checked
        if (QToolButton *tButton = qobject_cast<QToolButton *>(this->sender()))
        {
            tButton->setChecked(true);
        }
    }
}
// End MainWindow::SetToolButton()

//---------------------------------------------------------------------------------------------------------------------
/**
 * @brief Template function to set a tool button with apply functionality in the UI based on the provided parameters.
 * 
 * @param checked 
 * @param t 
 * @param cursor 
 * @param toolTip 
 * @param closeDialogSlot 
 * @param applyDialogSlot 
 * @return template <typename Dialog, typename Func, typename Func2> 
 */
template <typename Dialog, typename Func, typename Func2>
void MainWindow::SetToolButtonWithApply(bool checked, Tool t, const QString &cursor, const QString &toolTip,
                                        Func closeDialogSlot, Func2 applyDialogSlot)
{
    // Sets a tool button in the UI and shows a dialog with additional functionality for applying changes

    if (checked)
    {
        // Call the SetToolButton template function to set the tool button and show the dialog
        SetToolButton<Dialog>(checked, t, cursor, toolTip, closeDialogSlot);

        // Connect the DialogApplied signal from the dialog to the applyDialogSlot function
        connect(dialogTool.data(), &DialogTool::DialogApplied, this, applyDialogSlot);
    }
    else
    {
        // If the tool is not checked, ensure the corresponding tool button is checked
        if (QToolButton *tButton = qobject_cast<QToolButton *>(this->sender()))
        {
            tButton->setChecked(true);
        }
    }
}
// EndMainWindow::SetToolButtonWithApply()

//---------------------------------------------------------------------------------------------------------------------
/**
 * @brief Template function to handle the closure of a dialog associated with a drawing tool
 * @param result result working dialog.
 */
template <typename DrawTool>
void MainWindow::ClosedDialog(int result)
{
    /* Handle the closure of a dialog associated with a drawing tool, 
    creating a new QGraphicsItem based on the specified drawing tool 
    and updating the view accordingly.
    */

    // Ensure that the dialogTool is not null
    SCASSERT(!dialogTool.isNull())

    // Check if the dialog result is Accepted
    if (result == QDialog::Accepted)
    {
        // Get the current scene as a VMainGraphicsScene
        VMainGraphicsScene *scene = qobject_cast<VMainGraphicsScene *>(currentScene);
        SCASSERT(scene != nullptr)

        // Create a new QGraphicsItem using the DrawTool template parameter
        QGraphicsItem *tool = dynamic_cast<QGraphicsItem *>(DrawTool::Create(dialogTool, scene, doc, pattern));

        // Do not check for nullptr! See issue #719. (Note: Comment explaining an exception)
        ui->view->itemClicked(tool);
    }

    // Handle the arrow tool (resetting to the default arrow tool)
    handleArrowTool(true);
}
// End MainWindow::ClosedDialog()

//---------------------------------------------------------------------------------------------------------------------
/**
 * @brief Template function to handle the closure of a dialog associated with a drawing tool, with an apply functionality.
 * @param result result working dialog.
 */
template <typename DrawTool>
void MainWindow::ClosedDialogWithApply(int result, VMainGraphicsScene *scene)
{

    /* Handle the closure of a dialog associated with a drawing tool, 
        applying changes if accepted and performing necessary cleanup.
    */

    // Ensure that the dialogTool is not null
    SCASSERT(!dialogTool.isNull())

    // Check if the dialog result is Accepted
    if (result == QDialog::Accepted)
    {
        // Apply changes using the ApplyDialog template function with the specified DrawTool
        ApplyDialog<DrawTool>(scene);
    }

    // Retrieve the associated tool from the dialog
    DrawTool *vtool = qobject_cast<DrawTool *>(dialogTool->GetAssociatedTool()); // Don't check for nullptr here

    // If there was an associated tool, perform necessary cleanup and connect signals
    if (dialogTool->GetAssociatedTool() != nullptr)
    {
        // Assert that vtool is not nullptr
        SCASSERT(vtool != nullptr)

        // Destroy the link to the dialog and connect the tool's ToolTip signal to the MainWindow's ShowToolTip slot
        vtool->DialogLinkDestroy();
        connect(vtool, &DrawTool::ToolTip, this, &MainWindow::ShowToolTip);
    }

    // Handle the arrow tool (resetting to the default arrow tool)
    handleArrowTool(true);

    // Click on the tool in the view (Note: Don't check for nullptr here)
    ui->view->itemClicked(vtool);

    // If the insert is not at the end of the file, perform lite parse and update history
    if (doc->getCursor() > 0)
    {
        doc->LiteParseTree(Document::LiteParse);
        if (historyDialog)
        {
            historyDialog->updateHistory();
        }
    }
}
// End MainWindow::ClosedDialogWithApply()

//---------------------------------------------------------------------------------------------------------------------
/**
 * @brief Template function to apply changes from a dialog associated with a drawing tool.
 * 
 * @param scene 
 * @return template <typename DrawTool> 
 */
template <typename DrawTool>
void MainWindow::ApplyDialog(VMainGraphicsScene *scene)
{
    /* Apply changes from a dialog associated with a drawing tool, 
        either by creating a new associated tool or updating an existing one.
    */

    // Ensure that the dialogTool is not null
    SCASSERT(!dialogTool.isNull())

    // Check if the associated tool is not already created with apply
    if (dialogTool->GetAssociatedTool() == nullptr)
    {
        // Assert that the scene is not nullptr
        SCASSERT(scene != nullptr)

        // Create a new associated tool using the specified DrawTool template parameter
        dialogTool->SetAssociatedTool(DrawTool::Create(dialogTool, scene, doc, pattern));
    }
    else
    {
        // Update the associated tool with data from the dialog
        DrawTool *vtool = qobject_cast<DrawTool *>(dialogTool->GetAssociatedTool());
        SCASSERT(vtool != nullptr)
        vtool->FullUpdateFromGuiApply();
    }
}
// End MainWindow::ApplyDialog()

//---------------------------------------------------------------------------------------------------------------------
/**
 * @brief Template function to handle the closure of a drawing dialog with apply functionality.
 * 
 * @param result 
 * @return template <typename DrawTool> 
 */
template <typename DrawTool>
void MainWindow::ClosedDrawDialogWithApply(int result)
{
    /* Simple wrapper that calls another template function (ClosedDialogWithApply) with 
        the specified DrawTool template parameter and the result of the dialog closure. 
        The draftScene is passed as the graphics scene to the underlying function.
    */

    // Call the ClosedDialogWithApply template function with the specified DrawTool and result,
    // passing the draftScene as the graphics scene
    ClosedDialogWithApply<DrawTool>(result, draftScene);
}
// End MainWindow::ClosedDrawDialogWithApply()

//---------------------------------------------------------------------------------------------------------------------
/**
 * @brief Template function to apply changes from a drawing dialog associated with a specified DrawTool.
 * 
 * @return template <typename DrawTool> 
 */
template <typename DrawTool>
void MainWindow::ApplyDrawDialog()
{
    /* Simple wrapper that calls another template function (ApplyDialog) with 
        the specified DrawTool template parameter, applying changes from a drawing dialog 
        associated with the provided DrawTool. The draftScene is passed as the graphics 
        scene to the underlying function.
    */

    // Call the ApplyDialog template function with the specified DrawTool,
    // passing the draftScene as the graphics scene
    ApplyDialog<DrawTool>(draftScene);
}
// End MainWindow::ApplyDrawDialog()

//---------------------------------------------------------------------------------------------------------------------
/**
 * @brief Template function to handle the closure of a dialog associated with drawing pieces, with apply functionality.
 * 
 * @param result 
 * @return template <typename DrawTool> 
 */
template <typename DrawTool>
void MainWindow::ClosedPiecesDialogWithApply(int result)
{
    /* Wrapper that calls another template function (ClosedDialogWithApply) with 
        the specified DrawTool template parameter and the result of the dialog closure. 
        The pieceScene is passed as the graphics scene to the underlying function. 
        Additionally, the function enables relevant UI elements 
        if there are pieces in the pattern data.
    */

    // Call the ClosedDialogWithApply template function with the specified DrawTool and result,
    // passing the pieceScene as the graphics scene
    ClosedDialogWithApply<DrawTool>(result, pieceScene);

    // Enable relevant UI elements if there are pieces in the pattern data
    if (pattern->DataPieces()->size() > 0)
    {
        ui->anchorPoint_ToolButton->setEnabled(true);
        ui->internalPath_ToolButton->setEnabled(true);
        ui->insertNodes_ToolButton->setEnabled(true);
        ui->anchorPoint_Action->setEnabled(true);
        ui->internalPath_Action->setEnabled(true);
        ui->insertNodes_Action->setEnabled(true);
    }
}
// End MainWindow::ClosedPiecesDialogWithApply()

//---------------------------------------------------------------------------------------------------------------------
/**
 * @brief Template function to apply changes from a dialog associated with drawing pieces using a specified DrawTool.
 * 
 * @return template <typename DrawTool> 
 */
template <typename DrawTool>
void MainWindow::applyPiecesDialog()
{
    /* Simple wrapper that calls another template function (ApplyDialog) 
        with the specified DrawTool template parameter, 
        applying changes from a dialog associated with drawing pieces. 
        The pieceScene is passed as the graphics scene to the underlying function.
    */

    // Call the ApplyDialog template function with the specified DrawTool,
    // passing the pieceScene as the graphics scene
    ApplyDialog<DrawTool>(pieceScene);
}
// End MainWindow::applyPiecesDialog()

//---------------------------------------------------------------------------------------------------------------------
//Points
//---------------------------------------------------------------------------------------------------------------------
/**
 * @brief Function to handle the activation of the Midpoint Tool in the MainWindow
 * 
 */
void MainWindow::handleMidpointTool(bool checked)
{
    /* Sets the tool to select a point upon release (ToolSelectPointByRelease). 
        It then uses a template function (SetToolButtonWithApply) to set the Midpoint Tool, 
        providing specific cursor, tooltips, and connecting appropriate slots for 
        dialog closure and application of changes. 
        The tool is associated with DialogAlongLine and VToolAlongLine.
    */

    // Call ToolSelectPointByRelease to set the tool to select a point upon release
    ToolSelectPointByRelease();

    // Use the SetToolButtonWithApply template function with DialogAlongLine and VToolAlongLine
    // to set the Midpoint Tool, with specific cursor and tooltips, and connect appropriate slots
    SetToolButtonWithApply<DialogAlongLine>
    (
        checked,
        Tool::Midpoint,
        ":/cursor/midpoint_cursor.png",
        tr("<b>Tool::Point - Midpoint on Line</b>: Select first point"),
        &MainWindow::ClosedDrawDialogWithApply<VToolAlongLine>,
        &MainWindow::ApplyDrawDialog<VToolAlongLine>
    );
}
// End MainWindow::handleMidpointTool()

//---------------------------------------------------------------------------------------------------------------------
/**
 * @brief Function to handle the activation of the Point at Distance and Angle Tool in the MainWindow
 * 
 * @param checked Boolean flag indicating whether the tool button is checked
 */
void MainWindow::handlePointAtDistanceAngleTool(bool checked)
{
    /* Sets the tool to select a point upon release (ToolSelectPointByRelease). 
        It then uses a template function (SetToolButtonWithApply) to set the Point at Distance and Angle Tool, 
        providing specific cursor, tooltips, and connecting appropriate slots for dialog closure and application of changes. 
        The tool is associated with DialogEndLine and VToolEndLine.
    */

    // Call ToolSelectPointByRelease to set the tool to select a point upon release
    ToolSelectPointByRelease();

    // Use the SetToolButtonWithApply template function with DialogEndLine and VToolEndLine
    // to set the Point at Distance and Angle Tool, with specific cursor and tooltips, and connect appropriate slots
    SetToolButtonWithApply<DialogEndLine>
    (
        checked,
        Tool::EndLine,
        ":/cursor/endline_cursor.png",
        tr("<b>Tool::Point - Length and Angle</b>: Select point"),
        &MainWindow::ClosedDrawDialogWithApply<VToolEndLine>,
        &MainWindow::ApplyDrawDialog<VToolEndLine>
    );
}
// End MainWindow::handlePointAtDistanceAngleTool()

//---------------------------------------------------------------------------------------------------------------------
/**
 * @brief Function to handle the activation of the Along Line Tool in the MainWindow
 * 
 * @param checked Boolean flag indicating whether the tool button is checked
 */
void MainWindow::handleAlongLineTool(bool checked)
{
    /* Sets the tool to select a point upon release (ToolSelectPointByRelease). 
    It then uses a template function (SetToolButtonWithApply) to set the Along Line Tool, 
    providing specific cursor, tooltips, and connecting appropriate slots for 
    dialog closure and application of changes. 
    The tool is associated with DialogAlongLine and VToolAlongLine.
    */

    // Call ToolSelectPointByRelease to set the tool to select a point upon release
    ToolSelectPointByRelease();

    // Use the SetToolButtonWithApply template function with DialogAlongLine and VToolAlongLine
    // to set the Along Line Tool, with specific cursor and tooltips, and connect appropriate slots
    SetToolButtonWithApply<DialogAlongLine>
    (
        checked,
        Tool::AlongLine,
        ":/cursor/alongline_cursor.png",
        tr("<b>Tool::Point - On Line:</b> Select first point"),
        &MainWindow::ClosedDrawDialogWithApply<VToolAlongLine>,
        &MainWindow::ApplyDrawDialog<VToolAlongLine>
    );
}
// End MainWindow::handleAlongLineTool()

//---------------------------------------------------------------------------------------------------------------------
/**
 * @brief Function to handle the activation of the Normal Tool in the MainWindow
 * 
 * @param checked Boolean flag indicating whether the tool button is checked
 */
void MainWindow::handleNormalTool(bool checked)
{
    /* Sets the tool to select a point upon release (ToolSelectPointByRelease). 
        It then uses a template function (SetToolButtonWithApply) to set the Normal Tool, 
        providing specific cursor, tooltips, and connecting appropriate slots for 
        dialog closure and application of changes. 
        The tool is associated with DialogNormal and VToolNormal.
    */

    // Call ToolSelectPointByRelease to set the tool to select a point upon release
    ToolSelectPointByRelease();

    // Use the SetToolButtonWithApply template function with DialogNormal and VToolNormal
    // to set the Normal Tool, with specific cursor and tooltips, and connect appropriate slots
    SetToolButtonWithApply<DialogNormal>
    (
        checked,
        Tool::Normal,
        ":/cursor/normal_cursor.png",
        tr("<b>Tool::Point - On Perpendicular:</b> Select first point of line"),
        &MainWindow::ClosedDrawDialogWithApply<VToolNormal>,
        &MainWindow::ApplyDrawDialog<VToolNormal>
    );
}
// End MainWindow::handleNormalTool()

//---------------------------------------------------------------------------------------------------------------------
/**
 * @brief Handles the activation/deactivation of the Bisector tool.
 * 
 * @param checked True if the tool button is checked.
 */
void MainWindow::handleBisectorTool(bool checked)
{
    // Release any previous tool selections and set up for a new point selection.
    ToolSelectPointByRelease();

    // Set up the Bisector tool button with its associated dialog and actions.
    SetToolButtonWithApply<DialogBisector>
    (
        checked,
        Tool::Bisector,
        ":/cursor/bisector_cursor.png",
        tr("<b>Tool::Point - On Bisector:</b> Select first point of angle"),
        &MainWindow::ClosedDrawDialogWithApply<VToolBisector>,
        &MainWindow::ApplyDrawDialog<VToolBisector>
    );
}
// End MainWindow::handleBisectorTool()

//---------------------------------------------------------------------------------------------------------------------
/**
 * @brief Handles the activation/deactivation of the Shoulder Point tool.
 * 
 * @param checked True if the tool button is checked.
 */
void MainWindow::handleShoulderPointTool(bool checked)
{
    // Release any previous tool selections and set up for a new point selection.
    ToolSelectPointByRelease();

    // Set up the Shoulder Point tool button with its associated dialog and actions.
    SetToolButtonWithApply<DialogShoulderPoint>
    (
        checked,
        Tool::ShoulderPoint,
        ":/cursor/shoulder_cursor.png",
        tr("<b>Tool::Point - Length to Line:</b> Select point"),
        &MainWindow::ClosedDrawDialogWithApply<VToolShoulderPoint>,
        &MainWindow::ApplyDrawDialog<VToolShoulderPoint>
    );
}
// End MainWindow::handleShoulderPointTool()

//---------------------------------------------------------------------------------------------------------------------
/**
 * @brief Handles the activation/deactivation of the Point of Contact tool.
 * 
 * @param checked True if the tool button is checked.
 */
void MainWindow::handlePointOfContactTool(bool checked)
{
    // Release any previous tool selections and set up for a new point selection.
    ToolSelectPointByRelease();

    // Set up the Point of Contact tool button with its associated dialog and actions.
    SetToolButtonWithApply<DialogPointOfContact>
    (
        checked,
        Tool::PointOfContact,
        ":/cursor/pointcontact_cursor.png",
        tr("<b>Tool::Point - Intersect Arc and Line:</b> Select first point of line"),
        &MainWindow::ClosedDrawDialogWithApply<VToolPointOfContact>,
        &MainWindow::ApplyDrawDialog<VToolPointOfContact>
    );
}
// End MainWindow::handlePointOfContactTool()

//---------------------------------------------------------------------------------------------------------------------
/**
 * @brief Handles the activation/deactivation of the Triangle tool.
 * 
 * @param checked True if the tool button is checked.
 */
void MainWindow::handleTriangleTool(bool checked)
{
    // Release any previous tool selections and set up for a new point selection.
    ToolSelectPointByRelease();

    // Set up the Triangle tool button with its associated dialog and actions.
    SetToolButtonWithApply<DialogTriangle>
    (
        checked,
        Tool::Triangle,
        ":/cursor/triangle_cursor.png",
        tr("<b>Tool::Point - Intersect Axis and Triangle:</b> Select first point of axis"),
        &MainWindow::ClosedDrawDialogWithApply<VToolTriangle>,
        &MainWindow::ApplyDrawDialog<VToolTriangle>
    );
}
// End MainWindow::handleTriangleTool()

//---------------------------------------------------------------------------------------------------------------------
/**
 * @brief Handles the activation/deactivation of the Point Intersect XY tool.
 * 
 * @param checked True if the tool button is checked.
 */
void MainWindow::handlePointIntersectXYTool(bool checked)
{
    // Release any previous tool selections and set up for a new point selection.
    ToolSelectPointByRelease();

    // Set up the Point Intersect XY tool button with its associated dialog and actions.
    SetToolButtonWithApply<PointIntersectXYDialog>
    (
        checked,
        Tool::PointOfIntersection,
        ":/cursor/pointofintersect_cursor.png",
        tr("<b>Tool::Point - Intersect XY</b> Select point for X value (vertical)"),
        &MainWindow::ClosedDrawDialogWithApply<PointIntersectXYTool>,
        &MainWindow::ApplyDrawDialog<PointIntersectXYTool>
    );
}
// End MainWindow::handlePointIntersectXYTool()

//---------------------------------------------------------------------------------------------------------------------
/**
 * @brief Handles the activation/deactivation of the Height tool.
 * 
 * @param checked True if the tool button is checked.
 */
void MainWindow::handleHeightTool(bool checked)
{
    // Release any previous tool selections and set up for a new point selection.
    ToolSelectPointByRelease();

    // Set up the Height tool button with its associated dialog and actions.
    SetToolButtonWithApply<DialogHeight>
    (
        checked,
        Tool::Height,
        ":/cursor/height_cursor.png",
        tr("<b>Tool::Point - Intersect Line and Perpendicular:</b> Select base point"),
        &MainWindow::ClosedDrawDialogWithApply<VToolHeight>,
        &MainWindow::ApplyDrawDialog<VToolHeight>
    );
}
// End MainWindow::handleHeightTool()

//---------------------------------------------------------------------------------------------------------------------
/**
 * @brief Handles the activation/deactivation of the LineIntersectAxis tool.
 * 
 * @param checked True if the tool button is checked.
 */
void MainWindow::handleLineIntersectAxisTool(bool checked)
{
    // Release any previous tool selections and set up for a new point selection.
    ToolSelectPointByRelease();

    // Set up the LineIntersectAxis tool button with its associated dialog and actions.
    SetToolButtonWithApply<DialogLineIntersectAxis>
    (
        checked,
        Tool::LineIntersectAxis,
        ":/cursor/line_intersect_axis_cursor.png",
        tr("<b>Tool::Point - Intersect Line and Axis:</b> Select first point of line"),
        &MainWindow::ClosedDrawDialogWithApply<VToolLineIntersectAxis>,
        &MainWindow::ApplyDrawDialog<VToolLineIntersectAxis>
    );
}
// End MainWindow::handleLineIntersectAxisTool()

//---------------------------------------------------------------------------------------------------------------------
//Lines
//---------------------------------------------------------------------------------------------------------------------
/**
 * @brief Handles the activation/deactivation of the Line tool.
 * 
 * @param checked True if the tool button is checked.
 */
void MainWindow::handleLineTool(bool checked)
{
    // Release any previous tool selections and set up for a new point selection.
    ToolSelectPointByRelease();

    // Set up the Line tool button with its associated dialog and actions.
    SetToolButtonWithApply<DialogLine>
    (
        checked,
        Tool::Line,
        ":/cursor/line_cursor.png",
        tr("<b>Tool::Line:</b>: Select first point"),
        &MainWindow::ClosedDrawDialogWithApply<VToolLine>,
        &MainWindow::ApplyDrawDialog<VToolLine>
    );
}
// End MainWindow::handleLineTool()

//---------------------------------------------------------------------------------------------------------------------
/**
 * @brief Handles the activation/deactivation of the Line Intersect tool.
 * 
 * @param checked True if the tool button is checked.
 */
void MainWindow::handleLineIntersectTool(bool checked)
{
    // Release any previous tool selections and set up for a new point selection.
    ToolSelectPointByRelease();

    // Set up the Line Intersect tool button with its associated dialog and actions.
    SetToolButtonWithApply<DialogLineIntersect>
    (
        checked,
        Tool::LineIntersect,
        ":/cursor/intersect_cursor.png",
        tr("<b>Tool::Point - Intersect Lines:</b> Select first point of first line"),
        &MainWindow::ClosedDrawDialogWithApply<VToolLineIntersect>,
        &MainWindow::ApplyDrawDialog<VToolLineIntersect>
    );
}
// End MainWindow::handleLineIntersectTool()

//---------------------------------------------------------------------------------------------------------------------
// Curves
//---------------------------------------------------------------------------------------------------------------------
/**
 * @brief Handles the activation/deactivation of the Curve (Spline) tool.
 * 
 * @param checked True if the tool button is checked.
 */
void MainWindow::handleCurveTool(bool checked)
{
    // Set up for point selection upon mouse press.
    ToolSelectPointByPress();

    // Set up the Curve (Spline) tool button with its associated dialog and actions.
    SetToolButtonWithApply<DialogSpline>
    (
        checked,
        Tool::Spline,
        ":/cursor/spline_cursor.png",
        tr("<b>Tool::Curve - Interactive:</b> Select start point of curve"),
        &MainWindow::ClosedDrawDialogWithApply<VToolSpline>,
        &MainWindow::ApplyDrawDialog<VToolSpline>
    );
}
// End MainWindow::handleCurveTool()

//---------------------------------------------------------------------------------------------------------------------
/**
 * @brief Handles the activation/deactivation of the Spline tool.
 * 
 * @param checked True if the tool button is checked.
 */
void MainWindow::handleSplineTool(bool checked)
{
    // Set up for point selection upon mouse press.
    ToolSelectPointByPress();

    // Set up the Spline tool button with its associated dialog and actions.
    SetToolButtonWithApply<DialogSplinePath>
    (
        checked,
        Tool::SplinePath,
        ":/cursor/splinepath_cursor.png",
        tr("<b>Tool::Spline - Interactive:</b> Select start point of spline"),
        &MainWindow::ClosedDrawDialogWithApply<VToolSplinePath>,
        &MainWindow::ApplyDrawDialog<VToolSplinePath>
    );
}
// End MainWindow::handleSplineTool()

//---------------------------------------------------------------------------------------------------------------------
/**
 * @brief Handles the activation/deactivation of the Cubic Bezier Curve tool with control points.
 * 
 * @param checked True if the tool button is checked.
 */
void MainWindow::handleCurveWithControlPointsTool(bool checked)
{
    // Set up for point selection upon mouse release.
    ToolSelectPointByRelease();

    // Set up the Cubic Bezier Curve tool button with its associated dialog and actions.
    SetToolButtonWithApply<DialogCubicBezier>
    (
        checked,
        Tool::CubicBezier,
        ":/cursor/cubic_bezier_cursor.png",
        tr("<b>Tool::Curve - Fixed:</b> Select first point of curve"),
        &MainWindow::ClosedDrawDialogWithApply<VToolCubicBezier>,
        &MainWindow::ApplyDrawDialog<VToolCubicBezier>
    );
}
// End MainWindow::handleCurveWithControlPointsTool()

//---------------------------------------------------------------------------------------------------------------------
/**
 * @brief Handles the activation/deactivation of the Spline tool with fixed control points.
 * 
 * @param checked True if the tool button is checked.
 */
void MainWindow::handleSplineWithControlPointsTool(bool checked)
{
    // Set up for point selection upon mouse release.
    ToolSelectPointByRelease();

    // Set up the Spline tool button with fixed control points and its associated dialog and actions.
    SetToolButtonWithApply<DialogCubicBezierPath>
    (
        checked,
        Tool::CubicBezierPath,
        ":/cursor/cubic_bezier_path_cursor.png",
        tr("<b>Tool::Spline - Fixed:</b> Select first point of spline"),
        &MainWindow::ClosedDrawDialogWithApply<VToolCubicBezierPath>,
        &MainWindow::ApplyDrawDialog<VToolCubicBezierPath>
    );
}
// End MainWindow::handleSplineWithControlPointsTool()

//---------------------------------------------------------------------------------------------------------------------
/**
 * @brief Handles the activation/deactivation of the tool for placing a point along a curve.
 * 
 * @param checked True if the tool button is checked.
 */
void MainWindow::handlePointAlongCurveTool(bool checked)
{
    // Set up for spline (curve) selection.
    ToolSelectSpline();

    // Set up the tool button for placing a point along a curve with its associated dialog and actions.
    SetToolButtonWithApply<DialogCutSpline>
    (
        checked,
        Tool::CutSpline,
        ":/cursor/spline_cut_point_cursor.png",
        tr("<b>Tool::Point - On Curve:</b> Select first point of curve"),
        &MainWindow::ClosedDrawDialogWithApply<VToolCutSpline>,
        &MainWindow::ApplyDrawDialog<VToolCutSpline>
    );
}
// End MainWindow::handlePointAlongCurveTool()

//---------------------------------------------------------------------------------------------------------------------
/**
 * @brief Handles the activation/deactivation of the tool for placing a point along a spline path.
 * 
 * @param checked True if the tool button is checked.
 */
void MainWindow::handlePointAlongSplineTool(bool checked)
{
    // Set up for spline path selection.
    ToolSelectSplinePath();

    // Set up the tool button for placing a point along a spline path with its associated dialog and actions.
    SetToolButtonWithApply<DialogCutSplinePath>
    (
        checked,
        Tool::CutSplinePath,
        ":/cursor/splinepath_cut_point_cursor.png",
        tr("<b>Tool::Point - On Spline:</b> Select spline"),
        &MainWindow::ClosedDrawDialogWithApply<VToolCutSplinePath>,
        &MainWindow::ApplyDrawDialog<VToolCutSplinePath>
    );
}
// End MainWindow::handlePointAlongSplineTool()

//---------------------------------------------------------------------------------------------------------------------
/**
 * @brief Handles the activation/deactivation of the tool for placing a point along a spline path.
 * 
 * @param checked True if the tool button is checked.
 */
void MainWindow::handlePointAlongSplineTool(bool checked)
{
    // Set up for spline path selection.
    ToolSelectSplinePath();

    // Set up the tool button for placing a point along a spline path with its associated dialog and actions.
    SetToolButtonWithApply<DialogCutSplinePath>
    (
        checked,
        Tool::CutSplinePath,
        ":/cursor/splinepath_cut_point_cursor.png",
        tr("<b>Tool::Point - On Spline:</b> Select spline"),
        &MainWindow::ClosedDrawDialogWithApply<VToolCutSplinePath>,
        &MainWindow::ApplyDrawDialog<VToolCutSplinePath>
    );
}
// End MainWindow::handleCurveIntersectCurveTool()

//---------------------------------------------------------------------------------------------------------------------
/**
 * @brief Handles the activation/deactivation of the tool for finding the intersection point between a curve and an axis.
 * 
 * @param checked True if the tool button is checked.
 */
void MainWindow::handleCurveIntersectAxisTool(bool checked)
{
    // Selects all draft objects to prepare for the operation.
    selectAllDraftObjectsTool();

    // Sets up the tool button for finding the intersection point between a curve and an axis
    // along with its associated dialog and actions.
    SetToolButtonWithApply<DialogCurveIntersectAxis>
    (
        checked,
        Tool::CurveIntersectAxis,
        ":/cursor/curve_intersect_axis_cursor.png",
        tr("<b>Tool::Point - Intersect Curve and Axis:</b> Select curve"),
        &MainWindow::ClosedDrawDialogWithApply<VToolCurveIntersectAxis>,
        &MainWindow::ApplyDrawDialog<VToolCurveIntersectAxis>
    );
}
// End MainWindow::handleCurveIntersectAxisTool()

//---------------------------------------------------------------------------------------------------------------------
//Arcs
//---------------------------------------------------------------------------------------------------------------------
/**
 * @brief Handles the activation/deactivation of the tool for drawing arcs with specified radius and angles.
 * 
 * @param checked True if the tool button is checked.
 */
void MainWindow::handleArcTool(bool checked)
{
    // Selects the points by release, enabling interaction with the tool.
    ToolSelectPointByRelease();

    // Sets up the tool button for drawing arcs with specified radius and angles
    // along with its associated dialog and actions.
    SetToolButtonWithApply<DialogArc>
    (
        checked,
        Tool::Arc,
        ":/cursor/arc_cursor.png",
        tr("<b>Tool::Arc - Radius and Angles:</b> Select point of center of arc"),
        &MainWindow::ClosedDrawDialogWithApply<VToolArc>,
        &MainWindow::ApplyDrawDialog<VToolArc>
    );
}

// End MainWindow::handleArcTool()

//---------------------------------------------------------------------------------------------------------------------
/**
 * @brief Handles the activation/deactivation of the tool for placing a point along an existing arc.
 * 
 * @param checked True if the tool button is checked.
 */
void MainWindow::handlePointAlongArcTool(bool checked)
{
    // Selects the arc to interact with the tool.
    ToolSelectArc();

    // Sets up the tool button for placing a point along an existing arc
    // along with its associated dialog and actions.
    SetToolButtonWithApply<DialogCutArc>
    (
        checked,
        Tool::CutArc,
        ":/cursor/arc_cut_cursor.png",
        tr("<b>Tool::Point - On Arc:</b> Select arc"),
        &MainWindow::ClosedDrawDialogWithApply<VToolCutArc>,
        &MainWindow::ApplyDrawDialog<VToolCutArc>
    );
}

// End MainWindow::handlePointAlongArcTool()

//---------------------------------------------------------------------------------------------------------------------
/**
 * @brief Handles the activation/deactivation of the tool for finding the intersection of an arc and an axis.
 * 
 * @param checked True if the tool button is checked.
 */
void MainWindow::handleArcIntersectAxisTool(bool checked)
{
    // Selects all draft objects to ensure the tool operates on the entire scene.
    selectAllDraftObjectsTool();

    // Sets up the tool button for finding the intersection of an arc and an axis
    // along with its associated dialog and actions. It reuses the handleCurveIntersectAxisTool
    // function with a different cursor and tool tip.
    SetToolButtonWithApply<DialogCurveIntersectAxis>
    (
        checked,
        Tool::ArcIntersectAxis,
        ":/cursor/arc_intersect_axis_cursor.png",
        tr("<b>Tool::Point - Intersect Arc and Axis:</b> Select arc"),
        &MainWindow::ClosedDrawDialogWithApply<VToolCurveIntersectAxis>,
        &MainWindow::ApplyDrawDialog<VToolCurveIntersectAxis>
    );
}
// End MainWindow::handleArcIntersectAxisTool()

//---------------------------------------------------------------------------------------------------------------------
/**
 * @brief Handles the activation/deactivation of the tool for finding the point of intersection between two arcs.
 * 
 * @param checked True if the tool button is checked.
 */
void MainWindow::handlePointOfIntersectionArcsTool(bool checked)
{
    // Selects an arc as the target for the point of intersection tool.
    ToolSelectArc();

    // Sets up the tool button for finding the point of intersection between two arcs
    // along with its associated dialog and actions.
    SetToolButtonWithApply<DialogPointOfIntersectionArcs>
    (
        checked,
        Tool::PointOfIntersectionArcs,
        "://cursor/point_of_intersection_arcs.png",
        tr("<b>Tool::Point - Intersect Arcs:</b> Select first an arc"),
        &MainWindow::ClosedDrawDialogWithApply<VToolPointOfIntersectionArcs>,
        &MainWindow::ApplyDrawDialog<VToolPointOfIntersectionArcs>
    );
}

// End MainWindow::handlePointOfIntersectionArcsTool()

//---------------------------------------------------------------------------------------------------------------------
/**
 * @brief Handles the activation/deactivation of the tool for finding the point of intersection between two circles.
 * 
 * @param checked True if the tool button is checked.
 */
void MainWindow::handlePointOfIntersectionCirclesTool(bool checked)
{
    // Selects a point as the reference for the point of intersection tool.
    ToolSelectPointByRelease();

    // Sets up the tool button for finding the point of intersection between two circles
    // along with its associated dialog and actions.
    SetToolButtonWithApply<IntersectCirclesDialog>
    (
        checked,
        Tool::PointOfIntersectionCircles,
        "://cursor/point_of_intersection_circles.png",
        tr("<b>Tool::Point - Intersect Circles:</b> Select first circle center"),
        &MainWindow::ClosedDrawDialogWithApply<IntersectCirclesTool>,
        &MainWindow::ApplyDrawDialog<IntersectCirclesTool>
    );
}
// End MainWindow::handlePointOfIntersectionCirclesTool()

//---------------------------------------------------------------------------------------------------------------------
/**
 * @brief Handles the activation/deactivation of the tool for finding a point of intersection between a circle and tangent line.
 * 
 * @param checked True if the tool button is checked.
 */
void MainWindow::handlePointFromCircleAndTangentTool(bool checked)
{
    // Selects a point as the reference for the point of intersection tool.
    ToolSelectPointByRelease();

    // Sets up the tool button for finding a point of intersection between a circle and tangent line
    // along with its associated dialog and actions.
    SetToolButtonWithApply<IntersectCircleTangentDialog>
    (
        checked,
        Tool::PointFromCircleAndTangent,
        "://cursor/point_from_circle_and_tangent_cursor.png",
        tr("<b>Tool::Point - Intersect Circle and Tangent:</b> Select point on tangent"),
        &MainWindow::ClosedDrawDialogWithApply<IntersectCircleTangentTool>,
        &MainWindow::ApplyDrawDialog<IntersectCircleTangentTool>
    );
}
// End MainWindow::handlePointFromCircleAndTangentTool()

//---------------------------------------------------------------------------------------------------------------------
/**
 * @brief Handles the activation/deactivation of the tool for finding a point of intersection between an arc and a tangent line.
 * 
 * @param checked True if the tool button is checked.
 */
void MainWindow::handlePointFromArcAndTangentTool(bool checked)
{
    // Selects a point on the arc as the reference for the point of intersection tool.
    ToolSelectPointArc();

    // Sets up the tool button for finding a point of intersection between an arc and tangent line
    // along with its associated dialog and actions.
    SetToolButtonWithApply<DialogPointFromArcAndTangent>
    (
        checked,
        Tool::PointFromArcAndTangent,
        "://cursor/point_from_arc_and_tangent_cursor.png",
        tr("<b>Tool::Point - Intersect Arc and Tangent:</b> Select point on tangent"),
        &MainWindow::ClosedDrawDialogWithApply<VToolPointFromArcAndTangent>,
        &MainWindow::ApplyDrawDialog<VToolPointFromArcAndTangent>
    );
}

// End MainWindow::handlePointFromArcAndTangentTool()

//---------------------------------------------------------------------------------------------------------------------
/**
 * @brief Handles the activation/deactivation of the tool for creating an arc with a specified radius and length.
 * 
 * @param checked True if the tool button is checked.
 */
void MainWindow::handleArcWithLengthTool(bool checked)
{
    // Selects a point on release as a reference for creating an arc with a specified radius and length.
    ToolSelectPointByRelease();

    // Sets up the tool button for creating an arc with a specified radius and length
    // along with its associated dialog and actions.
    SetToolButtonWithApply<DialogArcWithLength>
    (
        checked,
        Tool::ArcWithLength,
        "://cursor/arc_with_length_cursor.png",
        tr("<b>Tool::Arc - Radius and Length:</b> Select point of the center of the arc"),
        &MainWindow::ClosedDrawDialogWithApply<VToolArcWithLength>,
        &MainWindow::ApplyDrawDialog<VToolArcWithLength>
    );
}

// End MainWindow::handleArcWithLengthTool()

//Elliptical Arcs
//---------------------------------------------------------------------------------------------------------------------
/**
 * @brief Handles the activation/deactivation of the tool for creating an elliptical arc.
 * 
 * @param checked True if the tool button is checked.
 */
void MainWindow::handleEllipticalArcTool(bool checked)
{
    // Selects a point on release as a reference for creating an elliptical arc.
    ToolSelectPointByRelease();

    // Sets up the tool button for creating an elliptical arc along with its associated dialog and actions.
    SetToolButtonWithApply<DialogEllipticalArc>
    (
        checked,
        Tool::EllipticalArc,
        ":/cursor/el_arc_cursor.png",
        tr("<b>Tool::Arc - Elliptical:</b> Select point of center of elliptical arc"),
        &MainWindow::ClosedDrawDialogWithApply<VToolEllipticalArc>,
        &MainWindow::ApplyDrawDialog<VToolEllipticalArc>
    );
}

// End MainWindow::handleEllipticalArcTool()

//---------------------------------------------------------------------------------------------------------------------
//Operations
//---------------------------------------------------------------------------------------------------------------------
/**
 * @brief Handles the activation/deactivation of the tool for creating a group of objects.
 * 
 * @param checked True if the tool button is checked.
 */
void MainWindow::handleGroupTool(bool checked)
{
    // Selects objects to include in the group.
    ToolSelectGroupObjects();

    // Constructs the tooltip with instructions for creating a group, including keyboard shortcuts.
    const QString tooltip = tr("<b>Tool::Operations - Create Group:</b> Select one or more objects -"
                                " Hold <b>%1</b> for multiple selection, "
                                "Press <b>ENTER</b> to finish group creation ")
                                .arg(QCoreApplication::translate(strQShortcut.toUtf8().constData(),
                                                                    strCtrl.toUtf8().constData()));

    // Sets up the tool button for creating a group along with its associated dialog and actions.
    SetToolButton<AddToGroupDialog>
    (
        checked,
        Tool::Group,
        ":/cursor/group_cursor.png",
        tooltip,
        &MainWindow::ClosedEditGroupDialog
    );
}
// End MainWindow::handleGroupTool()

//---------------------------------------------------------------------------------------------------------------------
/**
 * @brief Handles the closure of the dialog for editing a group, including processing the result.
 * 
 * @param result The result code indicating whether the dialog was accepted or rejected.
 */
void MainWindow::ClosedEditGroupDialog(int result)
{
    // Ensures that the dialogTool is not null.
    SCASSERT(dialogTool != nullptr)

    // Processes the result when the dialog is accepted.
    if (result == QDialog::Accepted)
    {
        // Attempts to cast the dialogTool to AddToGroupDialog.
        QSharedPointer<AddToGroupDialog> dialog = dialogTool.objectCast<AddToGroupDialog>();
        // Ensures that the cast is successful.
        SCASSERT(dialog != nullptr)

        // Retrieves the name and data of the group from the dialog.
        QString gName = dialog->getName();
        QMap<quint32, quint32>  gData = dialog->getGroupData();

        // Adds the group items to the document and obtains the corresponding QDomElement.
        QDomElement group = doc->addGroupItems(gName, gData);

        // Checks if the group is locked and displays a message if it is.
        if (group.isNull())
        {
            QMessageBox::information(this, tr("Add Group Objects"), tr("Group is Locked. Unlock to add objects"),
                                        QMessageBox::Ok, QMessageBox::Ok);
        }
    }

    // Restores the arrow tool after handling the group dialog.
    handleArrowTool(true);
}
// End MainWindow::ClosedEditGroupDialog()

//---------------------------------------------------------------------------------------------------------------------
/**
 * @brief Handles the activation of the rotation tool.
 * 
 * @param checked Indicates whether the tool button is checked.
 */
void MainWindow::handleRotationTool(bool checked)
{
    // Selects operation objects required for rotation.
    ToolSelectOperationObjects();

    // Constructs the tooltip for the rotation tool.
    const QString tooltip = tr("<b>Tool::Operations - Rotation:</b> Select one or more objects -"
                                " Hold <b>%1</b> for multiple selection, "
                                "Press <b>ENTER</b> to confirm selection")
                                .arg(QCoreApplication::translate(strQShortcut.toUtf8().constData(),
                                                                    strCtrl.toUtf8().constData()));

    // Sets up the rotation tool button with associated dialogs and actions.
    SetToolButtonWithApply<DialogRotation>
    (
        checked,
        Tool::Rotation,
        ":/cursor/rotation_cursor.png",
        tooltip,
        &MainWindow::ClosedDrawDialogWithApply<VToolRotation>,
        &MainWindow::ApplyDrawDialog<VToolRotation>
    );
}
// End MainWindow::handleRotationTool()

//---------------------------------------------------------------------------------------------------------------------
/**
 * @brief Handles the activation of the mirror by line tool.
 * 
 * @param checked Indicates whether the tool button is checked.
 */
void MainWindow::handleMirrorByLineTool(bool checked)
{
    // Selects operation objects required for mirror by line.
    ToolSelectOperationObjects();

    // Constructs the tooltip for the mirror by line tool.
    const QString tooltip = tr("<b>Tool::Operations - Mirror by Line:</b> Select one or more objects -"
                                " Hold <b>%1</b> for multiple selection, "
                                "Press <b>ENTER</b> to confirm selection")
                                .arg(QCoreApplication::translate(strQShortcut.toUtf8().constData(),
                                                                    strCtrl.toUtf8().constData()));

    // Sets up the mirror by line tool button with associated dialogs and actions.
    SetToolButtonWithApply<DialogMirrorByLine>
    (
        checked,
        Tool::MirrorByLine,
        ":/cursor/mirror_by_line_cursor.png",
        tooltip,
        &MainWindow::ClosedDrawDialogWithApply<VToolMirrorByLine>,
        &MainWindow::ApplyDrawDialog<VToolMirrorByLine>
    );
}
// End MainWindow::handleMirrorByLineTool()

//---------------------------------------------------------------------------------------------------------------------
/**
 * @brief Handles the activation of the mirror by axis tool.
 * 
 * @param checked Indicates whether the tool button is checked.
 */
void MainWindow::handleMirrorByAxisTool(bool checked)
{
    // Selects operation objects required for mirror by axis.
    ToolSelectOperationObjects();

    // Constructs the tooltip for the mirror by axis tool.
    const QString tooltip = tr("<b>Tool::Operations - Mirror by Axis:</b> Select one or more objects -"
                                " Hold <b>%1</b> for multiple selection, "
                                "Press <b>ENTER</b> to confirm selection")
                                .arg(QCoreApplication::translate(strQShortcut.toUtf8().constData(),
                                                                    strCtrl.toUtf8().constData()));

    // Sets up the mirror by axis tool button with associated dialogs and actions.
    SetToolButtonWithApply<DialogMirrorByAxis>
    (
        checked,
        Tool::MirrorByAxis,
        ":/cursor/mirror_by_axis_cursor.png",
        tooltip,
        &MainWindow::ClosedDrawDialogWithApply<VToolMirrorByAxis>,
        &MainWindow::ApplyDrawDialog<VToolMirrorByAxis>
    );
}
// End MainWindow::handleMirrorByAxisTool()

//---------------------------------------------------------------------------------------------------------------------
/**
 * @brief Handles the activation of the move tool for object operations.
 * 
 * @param checked Indicates whether the tool button is checked.
 */
void MainWindow::handleMoveTool(bool checked)
{
    // Selects operation objects required for the move operation.
    ToolSelectOperationObjects();

    // Constructs the tooltip for the move tool.
    const QString tooltip = tr("<b>Tool::Operations - Move:</b> Select one or more objects -"
                                " Hold <b>%1</b> for multiple selection, "
                                "Press <b>ENTER</b> to confirm selection")
                                .arg(QCoreApplication::translate(strQShortcut.toUtf8().constData(),
                                                                    strCtrl.toUtf8().constData()));

    // Sets up the move tool button with associated dialogs and actions.
    SetToolButtonWithApply<DialogMove>
    (
        checked,
        Tool::Move,
        ":/cursor/move_cursor.png",
        tooltip,
        &MainWindow::ClosedDrawDialogWithApply<VToolMove>,
        &MainWindow::ApplyDrawDialog<VToolMove>
    );
}

// End MainWindow::handleMoveTool()

//---------------------------------------------------------------------------------------------------------------------
void MainWindow::handleTrueDartTool(bool checked)
{
    ToolSelectPointByRelease();
    SetToolButtonWithApply<DialogTrueDarts>
    (
        checked,
        Tool::TrueDarts,
        "://cursor/true_darts_cursor.png",
        tr("<b>Tool::Operations - TrueDarts:</b> Select the first base line point"),
        &MainWindow::ClosedDrawDialogWithApply<VToolTrueDarts>,
        &MainWindow::ApplyDrawDialog<VToolTrueDarts>
    );
}

//---------------------------------------------------------------------------------------------------------------------
/**
 * @brief handlePatternPieceTool handler for pattern piece tool.
 * @param checked true - button checked.
 */
void MainWindow::handlePatternPieceTool(bool checked)
{
    selectAllDraftObjectsTool();
    SetToolButtonWithApply<PatternPieceDialog>
    (
        checked,
        Tool::Piece,
        ":/cursor/new_piece_cursor.png",
        tr("<b>Tool::Piece - Add New Pattern Piece:</b> Select main path of objects clockwise."),
        &MainWindow::ClosedPiecesDialogWithApply<PatternPieceTool>,
        &MainWindow::applyPiecesDialog<PatternPieceTool>
    );
}

//---------------------------------------------------------------------------------------------------------------------
void MainWindow::handleAnchorPointTool(bool checked)
{
    ToolSelectPointByRelease();
    SetToolButton<AnchorPointDialog>
    (
        checked,
        Tool::AnchorPoint,
        ":/cursor/anchor_point_cursor.png",
        tr("<b>Tool::Piece - Add Anchor Point:</b> Select anchor point"),
        &MainWindow::ClosedDialogAnchorPoint);
}

//---------------------------------------------------------------------------------------------------------------------
void MainWindow::ClosedDialogAnchorPoint(int result)
{
    SCASSERT(dialogTool != nullptr);
    if (result == QDialog::Accepted)
    {
        AnchorPointTool::Create(dialogTool, doc, pattern);
    }
    handleArrowTool(true);
    doc->LiteParseTree(Document::LiteParse);
}

//---------------------------------------------------------------------------------------------------------------------
void MainWindow::handleInternalPathTool(bool checked)
{
    selectAllDraftObjectsTool();
    SetToolButton<DialogInternalPath>
    (
        checked,
        Tool::InternalPath,
        ":/cursor/path_cursor.png",
        tr("<b>Tool::Piece - Internal Path:</b> Select path objects, use <b>SHIFT</b> to reverse curve direction"),
        &MainWindow::ClosedDialogInternalPath
    );
}

//---------------------------------------------------------------------------------------------------------------------
void MainWindow::ClosedDialogInternalPath(int result)
{
    SCASSERT(dialogTool != nullptr);
    if (result == QDialog::Accepted)
    {
        VToolInternalPath::Create(dialogTool, pieceScene, doc, pattern);
    }
    handleArrowTool(true);
    doc->LiteParseTree(Document::LiteParse);
}

//---------------------------------------------------------------------------------------------------------------------
void MainWindow::handleInsertNodesTool(bool checked)
{
    ToolSelectOperationObjects();
    const QString tooltip = tr("<b>Tool::Piece - Insert Nodes:</b> Select one or more objects -"
                               " Hold <b>%1</b> for multiple selection, "
                               "Press <b>ENTER</b> to confirm selection")
                               .arg(QCoreApplication::translate(strQShortcut.toUtf8().constData(),
                                                                strCtrl.toUtf8().constData()));
    SetToolButton<InsertNodesDialog>
    (
        checked,
        Tool::InsertNodes,
        "://cursor/insert_nodes_cursor.png",
        tooltip,
        &MainWindow::ClosedInsertNodesDialog
    );
}

//---------------------------------------------------------------------------------------------------------------------
void MainWindow::ClosedInsertNodesDialog(int result)
{
    SCASSERT(dialogTool != nullptr);
    if (result == QDialog::Accepted)
    {
        QSharedPointer<InsertNodesDialog> tool = dialogTool.objectCast<InsertNodesDialog>();
        SCASSERT(tool != nullptr)
        PatternPieceTool::insertNodes(tool->getNodes(), tool->getPieceId(), pieceScene, pattern, doc);
    }
    handleArrowTool(true);
    doc->LiteParseTree(Document::LiteParse);
}

//---------------------------------------------------------------------------------------------------------------------
/**
 * @brief handleUnionTool handler for Union tool.
 * @param checked true - button checked.
 */
void MainWindow::handleUnionTool(bool checked)
{
    selectPieceTool();
    SetToolButton<UnionDialog>
    (
        checked,
        Tool::Union,
        ":/cursor/union_cursor.png",
        tr("<b>Tool::Details - Union:</b> Select pattern piece"),
        &MainWindow::closeUnionDialog
    );
}

//---------------------------------------------------------------------------------------------------------------------
/**
 * @brief closeUnionDialog actions after closing Union tool dialog.
 * @param result result of dialog working.
 */
void MainWindow::closeUnionDialog(int result)
{
    ClosedDialog<UnionTool>(result);
    doc->LiteParseTree(Document::LiteParse);
}

//---------------------------------------------------------------------------------------------------------------------
/**
 * @brief handleNewLayout handler for New Layout tool.
 * @param tButton - toolbutton.
 * @param checked true - button checked.
 */
void MainWindow::handleNewLayout(bool checked)
{
      toolLayoutSettings(ui->layoutSettings_ToolButton, checked);
}

//---------------------------------------------------------------------------------------------------------------------
/**
 * @brief ShowTool  highlight tool.Tip show tools tooltip.
 * @param toolTip tooltip text.
 */
void MainWindow::ShowToolTip(const QString &toolTip)
{
    helpLabel->setText(toolTip);
}

//---------------------------------------------------------------------------------------------------------------------
/**
 * @brief triggers the update of the groups
 */
void MainWindow::updateGroups()
{
    groupsWidget->updateGroups();
}

void MainWindow::showAllGroups()
{
    groupsWidget->showAllGroups();
}

void MainWindow::hideAllGroups()
{
    groupsWidget->hideAllGroups();
}

void MainWindow::lockAllGroups()
{
    groupsWidget->lockAllGroups();
}

void MainWindow::unlockAllGroups()
{
    groupsWidget->unlockAllGroups();
}

void MainWindow::addGroupToList()
{
    groupsWidget->addGroupToList();
}
void MainWindow::deleteGroupFromList()
{
    groupsWidget->deleteGroupFromList();
}

void MainWindow::editGroup()
{
    groupsWidget->editGroup();
}

void MainWindow::addSelectedItemsToGroup()
{
    qCDebug(vMainWindow, "Add Selected items to Group.");
}

//---------------------------------------------------------------------------------------------------------------------
/**
 * @brief showEvent handle after show window.
 * @param event show event.
 */
void MainWindow::showEvent(QShowEvent *event)
{
    QMainWindow::showEvent(event);
    if (event->spontaneous())
    {
        return;
    }

    if (isInitialized)
    {
        return;
    }
    // do your init stuff here

    MinimumScrollBar();

    isInitialized = true;//first show windows are held
}

//---------------------------------------------------------------------------------------------------------------------
void MainWindow::changeEvent(QEvent *event)
{
    if (event->type() == QEvent::LanguageChange)
    {
        // retranslate designer form (single inheritance approach)
        ui->retranslateUi(this);
        undoAction->setText(tr("&Undo"));
        redoAction->setText(tr("&Redo"));
        helpLabel->setText(QObject::tr("Changes applied."));
        draftBlockLabel->setText(tr("Draft Block:"));

        if (mode == Draw::Calculation)
        {
            ui->groups_DockWidget->setWindowTitle(tr("Group Manager"));
        }
        else
        {
            ui->groups_DockWidget->setWindowTitle(tr("Pattern Pieces"));
        }

        UpdateWindowTitle();
        initPenToolBar();
        emit pieceScene->LanguageChanged();
    }
    // remember to call base class implementation
    QMainWindow::changeEvent(event);
}

//---------------------------------------------------------------------------------------------------------------------
/**
 * @brief closeEvent handle after close window.
 * @param event close event.
 */
void MainWindow::closeEvent(QCloseEvent *event)
{
    qCDebug(vMainWindow, "Closing main window");
    if (MaybeSave())
    {
        FileClosedCorrect();

        event->accept();
        QApplication::closeAllWindows();
    }
    else
    {
        qCDebug(vMainWindow, "Closing canceled.");
        event->ignore();
    }
}

//---------------------------------------------------------------------------------------------------------------------
void MainWindow::customEvent(QEvent *event)
{
    if (event->type() == UNDO_EVENT)
    {
        qApp->getUndoStack()->undo();
    }
}

//---------------------------------------------------------------------------------------------------------------------
void MainWindow::CleanLayout()
{
    qDeleteAll (scenes);
    scenes.clear();
    shadows.clear();
    papers.clear();
    ui->listWidget->clear();
    groupsWidget->clear();
    SetLayoutModeActions();
}

//---------------------------------------------------------------------------------------------------------------------
void MainWindow::PrepareSceneList()
{
    for (int i=1; i<=scenes.size(); ++i)
    {
        QListWidgetItem *item = new QListWidgetItem(ScenePreview(i-1), QString::number(i));
        ui->listWidget->addItem(item);
    }

    if (!scenes.isEmpty())
    {
        ui->listWidget->setCurrentRow(0);
        SetLayoutModeActions();
    }
}

//---------------------------------------------------------------------------------------------------------------------
void MainWindow::exportToCSVData(const QString &fileName, const DialogExportToCSV &dialog)
{
    QxtCsvModel csv;

    csv.insertColumn(0);
    csv.insertColumn(1);
    csv.insertColumn(2);

    if (dialog.WithHeader())
    {
        csv.setHeaderText(0, tr("Name"));
        csv.setHeaderText(1, tr("The calculated value"));
        csv.setHeaderText(2, tr("Formula"));
    }

    const QMap<QString, QSharedPointer<VIncrement> > increments = pattern->variablesData();
    QMap<QString, QSharedPointer<VIncrement> >::const_iterator i;
    QMap<quint32, QString> map;
    //Sorting QHash by id
    for (i = increments.constBegin(); i != increments.constEnd(); ++i)
    {
        QSharedPointer<VIncrement> incr = i.value();
        map.insert(incr->getIndex(), i.key());
    }

    qint32 currentRow = -1;
    QMapIterator<quint32, QString> iMap(map);
    while (iMap.hasNext())
    {
        iMap.next();
        QSharedPointer<VIncrement> incr = increments.value(iMap.value());
        currentRow++;

        csv.insertRow(currentRow);
        csv.setText(currentRow, 0, incr->GetName()); // name
        csv.setText(currentRow, 1, qApp->LocaleToString(*incr->GetValue())); // calculated value

        QString formula;
        try
        {
            formula = qApp->TrVars()->FormulaToUser(incr->GetFormula(), qApp->Settings()->GetOsSeparator());
        }
        catch (qmu::QmuParserError &error)
        {
            Q_UNUSED(error)
            formula = incr->GetFormula();
        }

        csv.setText(currentRow, 2, formula); // formula
    }

    csv.toCSV(fileName, dialog.WithHeader(), dialog.Separator(), QTextCodec::codecForMib(dialog.SelectedMib()));
}

void MainWindow::handleExportToCSV()
{
    QString file = tr("untitled");
    if(!qApp->getFilePath().isEmpty())
    {
        QString filePath = qApp->getFilePath();
        file = QFileInfo(filePath).baseName();
    }
    exportToCSV(file);
}

//---------------------------------------------------------------------------------------------------------------------
void MainWindow::LoadIndividual()
{
    const QString filter = tr("Individual measurements") + QLatin1String(" (*.") + smisExt +
                                                           QLatin1String(" *.") + vitExt  + QLatin1String(")");

    //Use standard path to individual measurements
    const QString dir = qApp->Seamly2DSettings()->getIndividualSizePath();

    bool usedNotExistedDir = false;

    QDir directory(dir);

    if (!directory.exists())
    {
        usedNotExistedDir = directory.mkpath(".");
    }

    const QString filename = fileDialog(this, tr("Open file"), dir, filter, nullptr, QFileDialog::DontUseNativeDialog,
                                        QFileDialog::ExistingFile, QFileDialog::AcceptOpen);


    if (!filename.isEmpty())
    {
        if (loadMeasurements(filename))
        {
            if (!doc->MPath().isEmpty())
            {
                watcher->removePath(AbsoluteMPath(qApp->getFilePath(), doc->MPath()));
            }

            qCInfo(vMainWindow, "Individual file %s was loaded.", qUtf8Printable(filename));

            ui->unloadMeasurements_Action->setEnabled(true);

            doc->SetMPath(RelativeMPath(qApp->getFilePath(), filename));
            watcher->addPath(filename);
            patternChangesWereSaved(false);

            ui->editCurrent_Action->setEnabled(true);
            helpLabel->setText(tr("Measurements loaded"));
            doc->LiteParseTree(Document::LiteParse);

            UpdateWindowTitle();
        }
    }

    if (usedNotExistedDir)
    {
        QDir directory(dir);
        directory.rmpath(".");
    }
}

//---------------------------------------------------------------------------------------------------------------------
void MainWindow::LoadMultisize()
{
    const QString filter = tr("Multisize measurements")  + QLatin1String(" (*.") + smmsExt +
                                                           QLatin1String(" *.") + vstExt  + QLatin1String(")");

    //Use standard path to multisize measurements
    QString dir = qApp->Seamly2DSettings()->getMultisizePath();
    dir = VCommonSettings::prepareMultisizeTables(dir);

    const QString filename = fileDialog(this, tr("Open file"), dir, filter, nullptr, QFileDialog::DontUseNativeDialog,
                                        QFileDialog::ExistingFile, QFileDialog::AcceptOpen);

    if (!filename.isEmpty())
    {
        QString hText;
        if (!gradationHeights.isNull())
        {
            hText = gradationHeights->currentText();
        }
        QString sText;
        if (!gradationSizes.isNull())
        {
            sText = gradationSizes->currentText();
        }

        if(loadMeasurements(filename))
        {
            if (!doc->MPath().isEmpty())
            {
                watcher->removePath(AbsoluteMPath(qApp->getFilePath(), doc->MPath()));
            }

            qCInfo(vMainWindow, "Multisize file %s was loaded.", qUtf8Printable(filename));

            ui->unloadMeasurements_Action->setEnabled(true);

            doc->SetMPath(RelativeMPath(qApp->getFilePath(), filename));
            watcher->addPath(filename);
            patternChangesWereSaved(false);

            ui->editCurrent_Action->setEnabled(true);
            helpLabel->setText(tr("Measurements loaded"));
            doc->LiteParseTree(Document::LiteParse);

            UpdateWindowTitle();

            if (qApp->patternType() == MeasurementsType::Multisize)
            {
                if (!hText.isEmpty() && not gradationHeights.isNull())
                {
                    gradationHeights->setCurrentText(hText);
                }

                if (!sText.isEmpty() && not gradationSizes.isNull())
                {
                    gradationSizes->setCurrentText(sText);
                }
            }
        }
    }
}

//---------------------------------------------------------------------------------------------------------------------
void MainWindow::UnloadMeasurements()
{
    if (doc->MPath().isEmpty())
    {
        ui->unloadMeasurements_Action->setDisabled(true);
        return;
    }

    if (doc->ListMeasurements().isEmpty())
    {
        watcher->removePath(AbsoluteMPath(qApp->getFilePath(), doc->MPath()));
        if (qApp->patternType() == MeasurementsType::Multisize)
        {
            initStatusBar();
        }
        qApp->setPatternType(MeasurementsType::Unknown);
        doc->SetMPath(QString());
        emit doc->UpdatePatternLabel();
        patternChangesWereSaved(false);
        ui->editCurrent_Action->setEnabled(false);
        ui->unloadMeasurements_Action->setDisabled(true);
        helpLabel->setText(tr("Measurements unloaded"));

        UpdateWindowTitle();
    }
    else
    {
        qCWarning(vMainWindow, "%s",
                  qUtf8Printable(tr("Couldn't unload measurements. Some of them are used in the pattern.")));
    }
}

//---------------------------------------------------------------------------------------------------------------------
void MainWindow::ShowMeasurements()
{
    if (!doc->MPath().isEmpty())
    {
        const QString absoluteMPath = AbsoluteMPath(qApp->getFilePath(), doc->MPath());

        QStringList arguments;
        if (qApp->patternType() == MeasurementsType::Multisize)
        {
            arguments = QStringList()
                    << absoluteMPath
                    << "-u"
                    << UnitsToStr(qApp->patternUnit())
                    << "-e"
                    << QString().setNum(static_cast<int>(UnitConvertor(VContainer::height(), doc->MUnit(), Unit::Cm)))
                    << "-s"
                    << QString().setNum(static_cast<int>(UnitConvertor(VContainer::size(), doc->MUnit(), Unit::Cm)));
        }
        else
        {
            arguments = QStringList() << absoluteMPath
                                      << "-u"
                                      << UnitsToStr(qApp->patternUnit());
        }

        if (isNoScaling)
        {
            arguments.append(QLatin1String("--") + LONG_OPTION_NO_HDPI_SCALING);
        }

        const QString seamlyme = qApp->SeamlyMeFilePath();
        const QString workingDirectory = QFileInfo(seamlyme).absoluteDir().absolutePath();
        QProcess::startDetached(seamlyme, arguments, workingDirectory);
    }
    else
    {
        ui->editCurrent_Action->setEnabled(false);
    }
}

//---------------------------------------------------------------------------------------------------------------------
void MainWindow::MeasurementsChanged(const QString &path)
{
    mChanges = false;
    QFileInfo checkFile(path);
    if (checkFile.exists())
    {
        mChanges = true;
        mChangesAsked = false;
    }
    else
    {
        for(int i=0; i<=1000; i=i+10)
        {
            if (checkFile.exists())
            {
                mChanges = true;
                mChangesAsked = false;
                break;
            }
            else
            {
                std::this_thread::sleep_for(std::chrono::milliseconds(10));
            }
        }
    }

    UpdateWindowTitle();
    ui->syncMeasurements_Action->setEnabled(mChanges);
}

//---------------------------------------------------------------------------------------------------------------------
void MainWindow::SyncMeasurements()
{
    if (mChanges)
    {
        const QString path = AbsoluteMPath(qApp->getFilePath(), doc->MPath());
        if(updateMeasurements(path, static_cast<int>(VContainer::size()), static_cast<int>(VContainer::height())))
        {
            if (!watcher->files().contains(path))
            {
                watcher->addPath(path);
            }
            const QString msg = tr("Measurements have been synced");
            qCDebug(vMainWindow, "%s", qUtf8Printable(msg));
            helpLabel->setText(msg);
            VWidgetPopup::PopupMessage(this, msg);
            doc->LiteParseTree(Document::LiteParse);
            mChanges = false;
            mChangesAsked = true;
            UpdateWindowTitle();
            ui->syncMeasurements_Action->setEnabled(mChanges);
        }
        else
        {
            qCWarning(vMainWindow, "%s", qUtf8Printable(tr("Couldn't sync measurements.")));
        }
    }
}

//---------------------------------------------------------------------------------------------------------------------
#if defined(Q_OS_MAC)
void MainWindow::OpenAt(QAction *where)
{
    const QString path = qApp->getFilePath().left(qApp->getFilePath().indexOf(where->text())) + where->text();
    if (path == qApp->getFilePath())
    {
        return;
    }
    QProcess process;
    process.start(QStringLiteral("/usr/bin/open"), QStringList() << path, QIODevice::ReadOnly);
    process.waitForFinished();
}
#endif //defined(Q_OS_MAC)

//---------------------------------------------------------------------------------------------------------------------
/**
 * @brief initStatusBar initialize horizontal bar for presenting status information
 */
void MainWindow::initStatusBar()
{
    if (!mouseCoordinates.isNull())
    {
        delete mouseCoordinates;
    }
    if (!infoToolButton.isNull())
    {
        delete infoToolButton;
    }
    if (!gradationHeights.isNull())
    {
        delete gradationHeights;
    }
    if (!gradationSizes.isNull())
    {
        delete gradationSizes;
    }
    if (!gradationHeightsLabel.isNull())
    {
        delete gradationHeightsLabel;
    }
    if (!gradationSizesLabel.isNull())
    {
        delete gradationSizesLabel;
    }

    if (qApp->patternType() == MeasurementsType::Multisize)
    {
        const QStringList listHeights = MeasurementVariable::ListHeights(doc->GetGradationHeights(), qApp->patternUnit());
        const QStringList listSizes = MeasurementVariable::ListSizes(doc->GetGradationSizes(), qApp->patternUnit());

        gradationHeightsLabel = new QLabel(tr("Height:"), this);
        gradationHeights = SetGradationList(gradationHeightsLabel, listHeights);

        // set default height
        SetDefaultHeight();

        connect(gradationHeights.data(), static_cast<void (QComboBox::*)(int)>(&QComboBox::currentIndexChanged),
                this, &MainWindow::ChangedHeight);

        gradationSizesLabel = new QLabel(tr("Size:"), this);
        gradationSizes = SetGradationList(gradationSizesLabel, listSizes);

        // set default size
        SetDefaultSize();

        connect(gradationSizes.data(), static_cast<void (QComboBox::*)(int)>(&QComboBox::currentIndexChanged),
                this, &MainWindow::ChangedSize);

    }

    mouseCoordinates = new MouseCoordinates(qApp->patternUnit());
    ui->statusBar->addPermanentWidget((mouseCoordinates));

    infoToolButton = new QToolButton();
    infoToolButton->setDefaultAction(ui->documentInfo_Action);
    ui->statusBar->addPermanentWidget((infoToolButton));
}

//---------------------------------------------------------------------------------------------------------------------
QComboBox *MainWindow::SetGradationList(QLabel *label, const QStringList &list)
{
    QComboBox *comboBox = new QComboBox(this);
    comboBox->addItems(list);
    ui->statusBar->addPermanentWidget(label);
    ui->statusBar->addPermanentWidget(comboBox);

    return comboBox;
}

//---------------------------------------------------------------------------------------------------------------------
void MainWindow::initModesToolBar()
{
    leftGoToStage = new QLabel(this);
    leftGoToStage->setPixmap(QPixmap("://icon/24x24/fast_forward_left_to_right_arrow.png"));
    ui->mode_ToolBar->insertWidget(ui->pieceMode_Action, leftGoToStage);

    rightGoToStage = new QLabel(this);
    rightGoToStage->setPixmap(QPixmap("://icon/24x24/left_to_right_arrow.png"));
    ui->mode_ToolBar->insertWidget(ui->layoutMode_Action, rightGoToStage);
}

//---------------------------------------------------------------------------------------------------------------------
/**
 * @brief initPointNameToolBar enable Point Name toolbar.
 */
void MainWindow::initPointNameToolBar()
{
    fontComboBox = new QFontComboBox ;
    fontComboBox->setCurrentFont(qApp->Seamly2DSettings()->getPointNameFont());
    ui->pointName_ToolBar->insertWidget(ui->showPointNames_Action,fontComboBox);
    fontComboBox->setSizeAdjustPolicy(QComboBox::AdjustToContents);
    fontComboBox->setEnabled(true);

    connect(fontComboBox, static_cast<void (QFontComboBox::*)(const QFont &)>(&QFontComboBox::currentFontChanged),
            this, [this](QFont font)
            {
                qApp->Seamly2DSettings()->setPointNameFont(font);
                upDateScenes();
            });

    fontSizeComboBox = new QComboBox ;
    ui->pointName_ToolBar->insertWidget(ui->showPointNames_Action,fontSizeComboBox);
    fontSizeComboBox->setSizeAdjustPolicy(QComboBox::AdjustToContents);
    fontSizeComboBox->addItem("6", QVariant(static_cast<int>(6)));
    fontSizeComboBox->addItem("7", QVariant(static_cast<int>(7)));
    fontSizeComboBox->addItem("8", QVariant(static_cast<int>(8)));
    fontSizeComboBox->addItem("9", QVariant(static_cast<int>(9)));
    fontSizeComboBox->addItem("10", QVariant(static_cast<int>(10)));
    fontSizeComboBox->addItem("11", QVariant(static_cast<int>(11)));
    fontSizeComboBox->addItem("12", QVariant(static_cast<int>(12)));
    fontSizeComboBox->addItem("13", QVariant(static_cast<int>(13)));
    fontSizeComboBox->addItem("14", QVariant(static_cast<int>(14)));
    fontSizeComboBox->addItem("15", QVariant(static_cast<int>(15)));
    fontSizeComboBox->addItem("16", QVariant(static_cast<int>(16)));
    fontSizeComboBox->addItem("18", QVariant(static_cast<int>(18)));
    fontSizeComboBox->addItem("20", QVariant(static_cast<int>(20)));
    fontSizeComboBox->addItem("22", QVariant(static_cast<int>(22)));
    fontSizeComboBox->addItem("24", QVariant(static_cast<int>(24)));
    fontSizeComboBox->addItem("26", QVariant(static_cast<int>(26)));
    fontSizeComboBox->addItem("28", QVariant(static_cast<int>(28)));
    fontSizeComboBox->addItem("32", QVariant(static_cast<int>(32)));
    fontSizeComboBox->addItem("36", QVariant(static_cast<int>(36)));
    fontSizeComboBox->addItem("40", QVariant(static_cast<int>(40)));
    fontSizeComboBox->addItem("44", QVariant(static_cast<int>(44)));
    fontSizeComboBox->addItem("48", QVariant(static_cast<int>(48)));
    fontSizeComboBox->addItem("54", QVariant(static_cast<int>(54)));
    fontSizeComboBox->addItem("60", QVariant(static_cast<int>(60)));
    fontSizeComboBox->addItem("66", QVariant(static_cast<int>(66)));
    fontSizeComboBox->addItem("72", QVariant(static_cast<int>(72)));
    fontSizeComboBox->addItem("80", QVariant(static_cast<int>(80)));
    fontSizeComboBox->addItem("96", QVariant(static_cast<int>(96)));

    int index = fontSizeComboBox->findData(qApp->Seamly2DSettings()->getPointNameSize());
    if (index < 0 || index > 28)
    {
        index = 18;
    }
    fontSizeComboBox->setCurrentIndex(index);

    connect(fontSizeComboBox, &QComboBox::currentTextChanged, this, [this](QString text)
            {
                qApp->Seamly2DSettings()->setPointNameSize(text.toInt());
                upDateScenes();
            });
    fontSizeComboBox->setEnabled(true);
}

//---------------------------------------------------------------------------------------------------------------------
/**
 * @brief initDraftToolBar enable draw toolbar.
 */
void MainWindow::initDraftToolBar()
{
    draftBlockLabel = new QLabel(tr("Draft Block:"));
    ui->draft_ToolBar->addWidget(draftBlockLabel);

    // By using Qt UI Designer we can't add QComboBox to toolbar
    draftBlockComboBox = new QComboBox;
    ui->draft_ToolBar->addWidget(draftBlockComboBox);
    draftBlockComboBox->setSizeAdjustPolicy(QComboBox::AdjustToContents);
    draftBlockComboBox->setEnabled(false);

    connect(draftBlockComboBox,  static_cast<void (QComboBox::*)(int)>(&QComboBox::currentIndexChanged),
            this, [this](int index){changeDraftBlock(index);});

    connect(ui->renameDraft_Action, &QAction::triggered, this, [this]()
    {
        const QString activeDraftBlock = doc->getActiveDraftBlockName();
        const QString draftBlockName = createDraftBlockName(activeDraftBlock);
        if (draftBlockName.isEmpty())
        {
            return;
        }
        RenameDraftBlock *draftBlock = new RenameDraftBlock(doc, draftBlockName, draftBlockComboBox);
        qApp->getUndoStack()->push(draftBlock);
        fullParseFile();
    });
}

//---------------------------------------------------------------------------------------------------------------------
void MainWindow::initToolsToolBar()
{
    /*First we will try use Standard Shortcuts from Qt, but because keypad "-" and "+" not the same keys like in main
    keypad, shortcut Ctrl+"-" or "+" from keypad will not working with standard shortcut (QKeySequence::ZoomIn or
    QKeySequence::ZoomOut). For example "+" is Qt::Key_Plus + Qt::KeypadModifier for keypad.
    Also for me don't work Qt:CTRL and work Qt::ControlModifier.*/

    QList<QKeySequence> zoomInShortcuts;
    zoomInShortcuts.append(QKeySequence(QKeySequence::ZoomIn));
    zoomInShortcuts.append(QKeySequence(Qt::ControlModifier + Qt::Key_Plus + Qt::KeypadModifier));
    ui->zoomIn_Action->setShortcuts(zoomInShortcuts);
    connect(ui->zoomIn_Action, &QAction::triggered, ui->view, &VMainGraphicsView::zoomIn);

    QList<QKeySequence> zoomOutShortcuts;
    zoomOutShortcuts.append(QKeySequence(QKeySequence::ZoomOut));
    zoomOutShortcuts.append(QKeySequence(Qt::ControlModifier + Qt::Key_Minus + Qt::KeypadModifier));
    ui->zoomOut_Action->setShortcuts(zoomOutShortcuts);
    connect(ui->zoomOut_Action, &QAction::triggered, ui->view, &VMainGraphicsView::zoomOut);

    QList<QKeySequence> zoom100PercentShortcuts;
    zoom100PercentShortcuts.append(QKeySequence(Qt::ControlModifier + Qt::Key_0));
    zoom100PercentShortcuts.append(QKeySequence(Qt::ControlModifier + Qt::Key_0 + Qt::KeypadModifier));
    ui->zoom100Percent_Action->setShortcuts(zoom100PercentShortcuts);
    connect(ui->zoom100Percent_Action, &QAction::triggered, ui->view, &VMainGraphicsView::zoom100Percent);

    QList<QKeySequence> zoomToFitShortcuts;
    zoomToFitShortcuts.append(QKeySequence(Qt::ControlModifier + Qt::Key_Equal));
    ui->zoomToFit_Action->setShortcuts(zoomToFitShortcuts);
    connect(ui->zoomToFit_Action, &QAction::triggered, ui->view, &VMainGraphicsView::zoomToFit);

    QList<QKeySequence> zoomToSelectedShortcuts;
    zoomToSelectedShortcuts.append(QKeySequence(Qt::ControlModifier + Qt::Key_Right));
    ui->zoomToSelected_Action->setShortcuts(zoomToSelectedShortcuts);
    connect(ui->zoomToSelected_Action, &QAction::triggered, this, &MainWindow::zoomToSelected);

    QList<QKeySequence> zoomToPreviousShortcuts;
    zoomToPreviousShortcuts.append(QKeySequence(Qt::ControlModifier + Qt::Key_Left));
    ui->zoomToPrevious_Action->setShortcuts(zoomToPreviousShortcuts);
    connect(ui->zoomToPrevious_Action, &QAction::triggered,  this, &MainWindow::zoomToPrevious);

    QList<QKeySequence> zoomToAreaShortcuts;
    zoomToAreaShortcuts.append(QKeySequence(Qt::ControlModifier + Qt::Key_A));
    ui->zoomToArea_Action->setShortcuts(zoomToAreaShortcuts);
    connect(ui->zoomToArea_Action, &QAction::toggled, this, &MainWindow::zoomToArea);

    resetPanShortcuts();
    connect(ui->zoomPan_Action, &QAction::toggled, this, &MainWindow::zoomPan);

    QList<QKeySequence> zoomToPointShortcuts;
    zoomToPointShortcuts.append(QKeySequence(Qt::ControlModifier + Qt::AltModifier + Qt::Key_P));
    ui->zoomToPoint_Action->setShortcuts(zoomToPointShortcuts);
    connect(ui->zoomToPoint_Action, &QAction::triggered, this, &MainWindow::showZoomToPointDialog);

    m_zoomToPointComboBox = new QComboBox(ui->zoom_ToolBar);
    m_zoomToPointComboBox->setEnabled(false);
    m_zoomToPointComboBox->setToolTip(ui->zoomToPoint_Action->toolTip());
    ui->zoom_ToolBar->addWidget(m_zoomToPointComboBox);
    connect(m_zoomToPointComboBox, &QComboBox::currentTextChanged, this, &MainWindow::zoomToPoint);

    if (zoomScaleSpinBox != nullptr)
    {
        delete zoomScaleSpinBox;
    }
    zoomScaleSpinBox = new QDoubleSpinBox();
    zoomScaleSpinBox->setDecimals(1);
    zoomScaleSpinBox->setAlignment(Qt::AlignRight);
    zoomScaleSpinBox->setSingleStep(0.1);
    zoomScaleSpinBox->setMinimum(qFloor(VMainGraphicsView::MinScale()*1000)/10.0);
    zoomScaleSpinBox->setMaximum(qFloor(VMainGraphicsView::MaxScale()*1000)/10.0);
    zoomScaleSpinBox->setSuffix("%");
    zoomScaleSpinBox->setMaximumWidth(80);
    zoomScaleSpinBox->setKeyboardTracking(false);
    ui->zoom_ToolBar->insertWidget(ui->zoomIn_Action,zoomScaleSpinBox);

    zoomScaleChanged(ui->view->transform().m11());
    connect(zoomScaleSpinBox, QOverload<double>::of(&QDoubleSpinBox::valueChanged),
            this,[this](double d){ui->view->zoomByScale(d/100.0);});

}

void MainWindow::initToolBarVisibility()
{
    updateToolBarVisibility();
    connect(ui->tools_ToolBox_ToolBar, &QToolBar::visibilityChanged, this, [this](bool visible)
    {
        ui->tools_ToolBox_ToolBar->setVisible(visible);
        qApp->Settings()->setShowToolsToolBar(visible);
    });
    connect(ui->points_ToolBar, &QToolBar::visibilityChanged, this, [this](bool visible)
    {
        ui->points_ToolBar->setVisible(visible);
        qApp->Settings()->setShowPointToolBar(visible);
    });
    connect(ui->lines_ToolBar, &QToolBar::visibilityChanged, this, [this](bool visible)
    {
        ui->lines_ToolBar->setVisible(visible);
        qApp->Settings()->setShowLineToolBar(visible);
    });
    connect(ui->curves_ToolBar, &QToolBar::visibilityChanged, this, [this](bool visible)
    {
        ui->curves_ToolBar->setVisible(visible);
        qApp->Settings()->setShowCurveToolBar(visible);
    });
    connect(ui->arcs_ToolBar, &QToolBar::visibilityChanged, this, [this](bool visible)
    {
        ui->arcs_ToolBar->setVisible(visible);
        qApp->Settings()->setShowArcToolBar(visible);
    });
    connect(ui->operations_ToolBar, &QToolBar::visibilityChanged, this, [this](bool visible)
    {
        ui->operations_ToolBar->setVisible(visible);
        qApp->Settings()->setShowOpsToolBar(visible);
    });
    connect(ui->pieces_ToolBar, &QToolBar::visibilityChanged, this, [this](bool visible)
    {
        ui->pieces_ToolBar->setVisible(visible);
        qApp->Settings()->setShowPieceToolBar(visible);
    });
    connect(ui->details_ToolBar, &QToolBar::visibilityChanged, this, [this](bool visible)
    {
        ui->details_ToolBar->setVisible(visible);
        qApp->Settings()->setShowDetailsToolBar(visible);
    });
    connect(ui->layout_ToolBar, &QToolBar::visibilityChanged, this, [this](bool visible)
    {
        ui->layout_ToolBar->setVisible(visible);
        qApp->Settings()->setShowLayoutToolBar(visible);
    });
}

//---------------------------------------------------------------------------------------------------------------------
/**
 * @brief initPenToolBar enable default color, line wight & type toolbar.
 */
void MainWindow::initPenToolBar()
{
    if (m_penToolBar != nullptr)
    {
        disconnect(m_penToolBar, nullptr, this, nullptr);
        delete m_penToolBar;
    }
    m_penToolBar = new PenToolBar(tr("Pen Toolbar"), this);
    m_penToolBar->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    m_penToolBar->setObjectName("penToolBar");
    this->addToolBar(Qt::TopToolBarArea, m_penToolBar);

    connect(m_penToolBar, &PenToolBar::penChanged, this, &MainWindow::penChanged);
}

//---------------------------------------------------------------------------------------------------------------------
/**
 * @brief initPropertyEditor initialize the Properties Editor.
 */
void MainWindow::initPropertyEditor()
{
    qCDebug(vMainWindow, "Initialize the Tool Property Editor.");
    if (toolProperties != nullptr)
    {
        disconnect(toolProperties, nullptr, this, nullptr);
        delete toolProperties;
    }
    toolProperties = new VToolOptionsPropertyBrowser(pattern, ui->toolProperties_DockWidget);

    connect(ui->view, &VMainGraphicsView::itemClicked, toolProperties, &VToolOptionsPropertyBrowser::itemClicked);
    connect(doc, &VPattern::FullUpdateFromFile, toolProperties, &VToolOptionsPropertyBrowser::updateOptions);
}

/**
 * Called when something changed in the pen tool bar
 * (e.g. color, weight, or type).
 */
void MainWindow::penChanged(Pen pen)
{
    doc->setDefaultPen(pen);
}

void MainWindow::updateToolBarVisibility()
{
    setToolBarVisibility(ui->tools_ToolBox_ToolBar, qApp->Settings()->getShowToolsToolBar());
    setToolBarVisibility(ui->points_ToolBar, qApp->Settings()->getShowPointToolBar());
    setToolBarVisibility(ui->lines_ToolBar, qApp->Settings()->getShowLineToolBar());
    setToolBarVisibility(ui->curves_ToolBar, qApp->Settings()->getShowCurveToolBar());
    setToolBarVisibility(ui->arcs_ToolBar, qApp->Settings()->getShowArcToolBar());
    setToolBarVisibility(ui->operations_ToolBar, qApp->Settings()->getShowOpsToolBar());
    setToolBarVisibility(ui->pieces_ToolBar, qApp->Settings()->getShowPieceToolBar());
    setToolBarVisibility(ui->details_ToolBar, qApp->Settings()->getShowDetailsToolBar());
    setToolBarVisibility(ui->layout_ToolBar, qApp->Settings()->getShowLayoutToolBar());
}

void MainWindow::setToolBarVisibility(QToolBar *toolbar, bool visible)
{
    toolbar->blockSignals(true);
    toolbar->setVisible(visible);
    toolbar->blockSignals(false);
}

void MainWindow::zoomScaleChanged(qreal scale)
{
    zoomScaleSpinBox->blockSignals(true);
    zoomScaleSpinBox->setValue(qFloor(scale*1000)/10.0);
    zoomScaleSpinBox->blockSignals(false);
    qCDebug(vMainWindow, "Value %f\n", (qreal(qFloor(scale*1000)/10.0)));
}

//---------------------------------------------------------------------------------------------------------------------
void MainWindow::zoomToSelected()
{
    if (qApp->getCurrentScene() == draftScene)
    {
        ui->view->zoomToRect(doc->ActiveDrawBoundingRect());
    }
    else if (qApp->getCurrentScene() == pieceScene)
    {
        QGraphicsItem *item = qApp->getCurrentScene()->focusItem();
        {
            if ((item != nullptr) && (item->type() == QGraphicsItem::UserType + static_cast<int>(Tool::Piece)))
            {
                QRectF rect;
                rect = item->boundingRect();
                rect.translate(item->scenePos());
                ui->view->zoomToRect(rect);
            }
        }
    }
}

void MainWindow::zoomToPrevious()
{
    VMainGraphicsScene *scene = qobject_cast<VMainGraphicsScene *>(currentScene);
    SCASSERT(scene != nullptr)

    /*Set transform for current scene*/
    scene->swapTransforms();
    ui->view->setTransform(scene->transform());
    zoomScaleChanged(ui->view->transform().m11());
}

//---------------------------------------------------------------------------------------------------------------------
void MainWindow::zoomToArea(bool checked)
{
      ui->view->zoomToAreaEnabled(checked);

      if (ui->zoomToArea_Action->isChecked())
      {
          ui->zoomPan_Action->setChecked(false);
      }
}

//---------------------------------------------------------------------------------------------------------------------
void MainWindow::zoomPan(bool checked)
{
    ui->view->zoomPanEnabled(checked);
    if (checked)
    {
        ui->zoomToArea_Action->setChecked(false);
    }
}

//---------------------------------------------------------------------------------------------------------------------
/**
 * @brief zoomToPoint show dialog for choosing a point and update the graphics view to focus on it.
 */
void MainWindow::showZoomToPointDialog()
{
    QStringList pointNames = draftPointNamesList();

    bool ok;
    QString pointName = QInputDialog::getItem(this, tr("Zoom to Point"), tr("Point:"), pointNames, 0, true, &ok,
                                              Qt::WindowSystemMenuHint | Qt::WindowTitleHint);
    if (!ok || pointName.isEmpty()) return;

    zoomToPoint(pointName);
}

//---------------------------------------------------------------------------------------------------------------------
/**
 * @brief zoomToPoint show dialog for choosing a point and update the graphics view to focus on it.
 * @param pointName name of to zoom into.
 */
void MainWindow::zoomToPoint(const QString &pointName)
{
    const QHash<quint32, QSharedPointer<VGObject> > *objects = pattern->DataGObjects();
    QHash<quint32, QSharedPointer<VGObject> >::const_iterator i;

    for (i = objects->constBegin(); i != objects->constEnd(); ++i)
    {
        QSharedPointer<VGObject> object = i.value();
        const quint32 objectId = object->getIdObject();
        const QString objectName = object->name();

        if (objectName == pointName)
        {
            VPointF *point = (VPointF*)object.data();
            ui->view->zoom100Percent();
            ui->view->centerOn(point->toQPointF());

            // show point name if it's hidden
            // TODO: Need to make this work with operation's and dart tools
            quint32 toolId = point->getIdTool();
            const quint32 objId = point->getIdObject();
            if (objId != NULL_ID)
            {
                toolId = objId;
            }
            if (VAbstractTool *tool = qobject_cast<VAbstractTool *>(VAbstractPattern::getTool(toolId)))
            {
                tool->setPointNameVisiblity(toolId, true);
            }

            // show any hiden groups containing object
            QMap<quint32,QString> groups = doc->getGroupsContainingItem(toolId, objectId, true);
            groupsWidget->showGroups(groups);

            // Reset combobox so same point can be selected again
            m_zoomToPointComboBox->blockSignals(true);
            m_zoomToPointComboBox->setCurrentIndex(-1);
            m_zoomToPointComboBox->blockSignals(false);

            return;
        }
    }
}

//---------------------------------------------------------------------------------------------------------------------
void MainWindow::InitToolButtons()
{
    connect(ui->arrowPointer_ToolButton, &QToolButton::clicked, this, &MainWindow::handleArrowTool);

    // This check helps to find missed tools
    Q_STATIC_ASSERT_X(static_cast<int>(Tool::LAST_ONE_DO_NOT_USE) == 53, "Check if all tools were connected.");

    connect(ui->pointAtDistanceAngle_ToolButton, &QToolButton::clicked,
            this, &MainWindow::handlePointAtDistanceAngleTool);
    connect(ui->line_ToolButton,           &QToolButton::clicked, this, &MainWindow::handleLineTool);
    connect(ui->alongLine_ToolButton,      &QToolButton::clicked, this, &MainWindow::handleAlongLineTool);
    connect(ui->shoulderPoint_ToolButton,  &QToolButton::clicked, this, &MainWindow::handleShoulderPointTool);
    connect(ui->normal_ToolButton,         &QToolButton::clicked, this, &MainWindow::handleNormalTool);
    connect(ui->bisector_ToolButton,       &QToolButton::clicked, this, &MainWindow::handleBisectorTool);
    connect(ui->lineIntersect_ToolButton,  &QToolButton::clicked, this, &MainWindow::handleLineIntersectTool);
    connect(ui->curve_ToolButton,          &QToolButton::clicked, this, &MainWindow::handleCurveTool);
    connect(ui->curveWithCPs_ToolButton,   &QToolButton::clicked, this, &MainWindow::handleCurveWithControlPointsTool);
    connect(ui->arc_ToolButton,            &QToolButton::clicked, this, &MainWindow::handleArcTool);
    connect(ui->spline_ToolButton,         &QToolButton::clicked, this, &MainWindow::handleSplineTool);
    connect(ui->splineWithCPs_ToolButton,  &QToolButton::clicked, this, &MainWindow::handleSplineWithControlPointsTool);
    connect(ui->pointOfContact_ToolButton, &QToolButton::clicked, this, &MainWindow::handlePointOfContactTool);
    connect(ui->addPatternPiece_ToolButton,&QToolButton::clicked, this, &MainWindow::handlePatternPieceTool);
    connect(ui->internalPath_ToolButton,   &QToolButton::clicked, this, &MainWindow::handleInternalPathTool);
    connect(ui->height_ToolButton,         &QToolButton::clicked, this, &MainWindow::handleHeightTool);
    connect(ui->triangle_ToolButton,       &QToolButton::clicked, this, &MainWindow::handleTriangleTool);
    connect(ui->pointIntersectXY_ToolButton,    &QToolButton::clicked, this, &MainWindow::handlePointIntersectXYTool);
    connect(ui->pointAlongCurve_ToolButton,     &QToolButton::clicked, this, &MainWindow::handlePointAlongCurveTool);
    connect(ui->pointAlongSpline_ToolButton,    &QToolButton::clicked, this, &MainWindow::handlePointAlongSplineTool);
    connect(ui->unitePieces_ToolButton,         &QToolButton::clicked, this, &MainWindow::handleUnionTool);
    connect(ui->pointAlongArc_ToolButton,       &QToolButton::clicked, this, &MainWindow::handlePointAlongArcTool);
    connect(ui->lineIntersectAxis_ToolButton,   &QToolButton::clicked, this, &MainWindow::handleLineIntersectAxisTool);
    connect(ui->curveIntersectAxis_ToolButton,  &QToolButton::clicked, this, &MainWindow::handleCurveIntersectAxisTool);
    connect(ui->arcIntersectAxis_ToolButton,    &QToolButton::clicked, this, &MainWindow::handleArcIntersectAxisTool);
    connect(ui->layoutSettings_ToolButton,      &QToolButton::clicked, this, &MainWindow::handleNewLayout);
    connect(ui->pointOfIntersectionArcs_ToolButton,    &QToolButton::clicked,
            this, &MainWindow::handlePointOfIntersectionArcsTool);
    connect(ui->pointOfIntersectionCircles_ToolButton, &QToolButton::clicked,
            this, &MainWindow::handlePointOfIntersectionCirclesTool);
    connect(ui->pointOfIntersectionCurves_ToolButton,         &QToolButton::clicked,
            this, &MainWindow::handleCurveIntersectCurveTool);
    connect(ui->pointFromCircleAndTangent_ToolButton,  &QToolButton::clicked,
            this, &MainWindow::handlePointFromCircleAndTangentTool);
    connect(ui->pointFromArcAndTangent_ToolButton,     &QToolButton::clicked,
            this, &MainWindow::handlePointFromArcAndTangentTool);
    connect(ui->arcWithLength_ToolButton,  &QToolButton::clicked, this, &MainWindow::handleArcWithLengthTool);
    connect(ui->trueDarts_ToolButton,      &QToolButton::clicked, this, &MainWindow::handleTrueDartTool);
    connect(ui->exportDraftBlocks_ToolButton, &QToolButton::clicked, this, &MainWindow::exportDraftBlocksAs);
    connect(ui->group_ToolButton,          &QToolButton::clicked, this, &MainWindow::handleGroupTool);
    connect(ui->rotation_ToolButton,       &QToolButton::clicked, this, &MainWindow::handleRotationTool);
    connect(ui->mirrorByLine_ToolButton,   &QToolButton::clicked, this, &MainWindow::handleMirrorByLineTool);
    connect(ui->mirrorByAxis_ToolButton,   &QToolButton::clicked, this, &MainWindow::handleMirrorByAxisTool);
    connect(ui->move_ToolButton,           &QToolButton::clicked, this, &MainWindow::handleMoveTool);
    connect(ui->midpoint_ToolButton,       &QToolButton::clicked, this, &MainWindow::handleMidpointTool);
    connect(ui->exportLayout_ToolButton,   &QToolButton::clicked, this, &MainWindow::exportLayoutAs);
    connect(ui->exportPiecesAs_ToolButton, &QToolButton::clicked, this, &MainWindow::exportPiecesAs);
    connect(ui->ellipticalArc_ToolButton,  &QToolButton::clicked, this, &MainWindow::handleEllipticalArcTool);
    connect(ui->anchorPoint_ToolButton,    &QToolButton::clicked, this, &MainWindow::handleAnchorPointTool);
    connect(ui->insertNodes_ToolButton,     &QToolButton::clicked, this, &MainWindow::handleInsertNodesTool);
}

void MainWindow::handlePointsMenu()
{
    qCDebug(vMainWindow, "Points Menu selected. \n");

    QMenu menu;

    QAction *action_PointAtDA           = menu.addAction(QIcon(":/toolicon/32x32/segment.png"),                tr("Length and Angle") + "\tL, A");
    QAction *action_PointAlongLine      = menu.addAction(QIcon(":/toolicon/32x32/along_line.png"),             tr("On Line") + "\tO, L");
    QAction *action_Midpoint            = menu.addAction(QIcon(":/toolicon/32x32/midpoint.png"),               tr("Midpoint") + "\t" + tr("Shift+O, Shift+L"));
    QAction *action_AlongPerpendicular  = menu.addAction(QIcon(":/toolicon/32x32/normal.png"),                 tr("On Perpendicular") + "\tO, P");
    QAction *action_Bisector            = menu.addAction(QIcon(":/toolicon/32x32/bisector.png"),               tr("On Bisector") + "\tO, B");
    QAction *action_Shoulder            = menu.addAction(QIcon(":/toolicon/32x32/shoulder.png"),               tr("Length to Line") + "\tP, S");
    QAction *action_PointOfContact      = menu.addAction(QIcon(":/toolicon/32x32/point_of_contact.png"),       tr("Intersect Arc and Line") + "\tA, L");
    QAction *action_Triangle            = menu.addAction(QIcon(":/toolicon/32x32/triangle.png"),               tr("Intersect Axis and Triangle") + "\tX, T");
    QAction *action_PointIntersectXY    = menu.addAction(QIcon(":/toolicon/32x32/point_intersectxy_icon.png"), tr("Intersect XY") + "\tX, Y");
    QAction *action_PerpendicularPoint  = menu.addAction(QIcon(":/toolicon/32x32/height.png"),                 tr("Intersect Line and Perpendicular") + "\tL, P");
    QAction *action_PointIntersectAxis  = menu.addAction(QIcon(":/toolicon/32x32/line_intersect_axis.png"),    tr("Intersect Line and Axis") + "\tL, X");

    QAction *selectedAction = menu.exec(QCursor::pos());

    if(selectedAction == nullptr)
    {
        return;
    }
    else if (selectedAction == action_Midpoint)
    {
        ui->draft_ToolBox->setCurrentWidget(ui->points_Page);
        ui->midpoint_ToolButton->setChecked(true);
        handleMidpointTool(true);
    }
    else if (selectedAction == action_PointAtDA)
    {
        ui->draft_ToolBox->setCurrentWidget(ui->points_Page);
        ui->pointAtDistanceAngle_ToolButton->setChecked(true);
        handlePointAtDistanceAngleTool(true);
    }
    else if (selectedAction == action_PointAlongLine)
    {
        ui->draft_ToolBox->setCurrentWidget(ui->points_Page);
        ui->alongLine_ToolButton->setChecked(true);
        handleAlongLineTool(true);
    }
    else if (selectedAction == action_AlongPerpendicular)
    {
        ui->draft_ToolBox->setCurrentWidget(ui->points_Page);
        ui->normal_ToolButton->setChecked(true);
        handleNormalTool(true);
    }
    else if (selectedAction == action_Bisector)
    {
        ui->draft_ToolBox->setCurrentWidget(ui->points_Page);
        ui->bisector_ToolButton->setChecked(true);
        handleBisectorTool(true);
    }
    else if (selectedAction == action_Shoulder)
    {
        ui->draft_ToolBox->setCurrentWidget(ui->points_Page);
        ui->shoulderPoint_ToolButton->setChecked(true);
        handleShoulderPointTool(true);
    }
    else if (selectedAction == action_PointOfContact)
    {
        ui->draft_ToolBox->setCurrentWidget(ui->points_Page);
        ui->pointOfContact_ToolButton->setChecked(true);
        handlePointOfContactTool(true);
    }
    else if (selectedAction == action_Triangle)
    {
        ui->draft_ToolBox->setCurrentWidget(ui->points_Page);
        ui->triangle_ToolButton->setChecked(true);
        handleTriangleTool(true);
    }
    else if (selectedAction == action_PointIntersectXY)
    {
        ui->draft_ToolBox->setCurrentWidget(ui->points_Page);
        ui->pointIntersectXY_ToolButton->setChecked(true);
        handlePointIntersectXYTool(true);
    }
    else if (selectedAction == action_PerpendicularPoint)
    {
        ui->draft_ToolBox->setCurrentWidget(ui->points_Page);
        ui->height_ToolButton->setChecked(true);
        handleHeightTool(true);
    }
    else if (selectedAction == action_PointIntersectAxis)
    {
        ui->draft_ToolBox->setCurrentWidget(ui->points_Page);
        ui->lineIntersectAxis_ToolButton->setChecked(true);
        handleLineIntersectAxisTool(true);
    }
}

void MainWindow::handleLinesMenu()
{
    qCDebug(vMainWindow, "Lines Menu selected. \n");
    QMenu menu;

    QAction *action_Line          = menu.addAction(QIcon(":/toolicon/32x32/line.png"),      tr("Line") + "\t" + tr("Alt+L"));
    QAction *action_LineIntersect = menu.addAction(QIcon(":/toolicon/32x32/intersect.png"), tr("Intersect Lines") + "\tI, L");

    QAction *selectedAction = menu.exec(QCursor::pos());

    if(selectedAction == nullptr)
    {
        return;
    }
    else if (selectedAction == action_Line)
    {
        ui->draft_ToolBox->setCurrentWidget(ui->lines_Page);
        ui->line_ToolButton->setChecked(true);
        handleLineTool(true);
    }
    else if (selectedAction == action_LineIntersect)
    {
        ui->draft_ToolBox->setCurrentWidget(ui->lines_Page);
        ui->lineIntersect_ToolButton->setChecked(true);
        handleLineIntersectTool(true);
    }
}

void MainWindow::handleArcsMenu()
{
    qCDebug(vMainWindow, "Arcs Menu selected. \n");
    QMenu menu;

    QAction *action_Arc              = menu.addAction(QIcon(":/toolicon/32x32/arc.png"),                           tr("Radius and Angles") + "\t" + tr("Alt+A"));
    QAction *action_PointAlongArc    = menu.addAction(QIcon(":/toolicon/32x32/arc_cut.png"),                       tr("Point on Arc") + "\tO, A");
    QAction *action_ArcIntersectAxis = menu.addAction(QIcon(":/toolicon/32x32/arc_intersect_axis.png"),            tr("Intersect Arc and Axis") + "\tA, X");
    QAction *action_ArcIntersectArc  = menu.addAction(QIcon(":/toolicon/32x32/point_of_intersection_arcs.png"),    tr("Intersect Arcs") + "\tI, A");
    QAction *action_CircleIntersect  = menu.addAction(QIcon(":/toolicon/32x32/point_of_intersection_circles.png"), tr("Intersect Circles") + "\t" + tr("Shift+I, Shift+C"));
    QAction *action_CircleTangent    = menu.addAction(QIcon(":/toolicon/32x32/point_from_circle_and_tangent.png"), tr("Intersect Circle and Tangent") + "\tC, T");
    QAction *action_ArcTangent       = menu.addAction(QIcon(":/toolicon/32x32/point_from_arc_and_tangent.png"),    tr("Intersect Arc and Tangent") + "\tA, T");
    QAction *action_ArcWithLength    = menu.addAction(QIcon(":/toolicon/32x32/arc_with_length.png"),               tr("Radius and Length") + "\t" + tr("Alt+Shift+A"));
    QAction *action_EllipticalArc    = menu.addAction(QIcon(":/toolicon/32x32/el_arc.png"),                        tr("Elliptical") + "\t" + tr("Alt+E"));

    QAction *selectedAction = menu.exec(QCursor::pos());

    if(selectedAction == nullptr)
    {
        return;
    }
    else if (selectedAction == action_Arc)
    {
        ui->draft_ToolBox->setCurrentWidget(ui->arcs_Page);
        ui->arc_ToolButton->setChecked(true);
        handleArcTool(true);
    }
    else if (selectedAction == action_PointAlongArc)
    {
        ui->draft_ToolBox->setCurrentWidget(ui->arcs_Page);
        ui->pointAlongArc_ToolButton->setChecked(true);
        handlePointAlongArcTool(true);
    }
    else if (selectedAction == action_ArcIntersectAxis)
    {
        ui->draft_ToolBox->setCurrentWidget(ui->arcs_Page);
        ui->arcIntersectAxis_ToolButton->setChecked(true);
        handleArcIntersectAxisTool(true);
    }
    else if (selectedAction == action_ArcIntersectArc)
    {
        ui->draft_ToolBox->setCurrentWidget(ui->arcs_Page);
        ui->pointOfIntersectionArcs_ToolButton->setChecked(true);
        handlePointOfIntersectionArcsTool(true);
    }
    else if (selectedAction == action_CircleIntersect)
    {
        ui->draft_ToolBox->setCurrentWidget(ui->arcs_Page);
        ui->pointOfIntersectionCircles_ToolButton->setChecked(true);
        handlePointOfIntersectionCirclesTool(true);
    }
    else if (selectedAction == action_CircleTangent)
    {
        ui->draft_ToolBox->setCurrentWidget(ui->arcs_Page);
        ui->pointFromCircleAndTangent_ToolButton->setChecked(true);
        handlePointFromCircleAndTangentTool(true);
    }
    else if (selectedAction == action_ArcTangent)
    {
        ui->draft_ToolBox->setCurrentWidget(ui->arcs_Page);
        ui->pointFromArcAndTangent_ToolButton->setChecked(true);
        handlePointFromArcAndTangentTool(true);
    }
    else if (selectedAction == action_ArcWithLength)
    {
        ui->draft_ToolBox->setCurrentWidget(ui->arcs_Page);
        ui->arcWithLength_ToolButton->setChecked(true);
        handleArcWithLengthTool(true);
    }
    else if (selectedAction == action_EllipticalArc)
    {
        ui->draft_ToolBox->setCurrentWidget(ui->arcs_Page);
        ui->ellipticalArc_ToolButton->setChecked(true);
        handleEllipticalArcTool(true);
    }
}

void MainWindow::handleCurvesMenu()
{
    qCDebug(vMainWindow, "Curves Menu selected. \n");
    QMenu menu;

    QAction *action_Curve                = menu.addAction(QIcon(":/toolicon/32x32/spline.png"),               tr("Curve - Interactive") + "\t" + tr("Alt+C"));
    QAction *action_Spline               = menu.addAction(QIcon(":/toolicon/32x32/splinePath.png"),           tr("Spline - Interactive") + "\t" + tr("Alt+S"));
    QAction *action_CurveWithCPs         = menu.addAction(QIcon(":/toolicon/32x32/cubic_bezier.png"),         tr("Curve - Fixed") + "\t" + tr("Alt+Shift+C"));
    QAction *action_SplineWithCPs        = menu.addAction(QIcon(":/toolicon/32x32/cubic_bezier_path.png"),    tr("Spline - Fixed") + "\t" + tr("Alt+Shift+S"));
    QAction *action_PointAlongCurve      = menu.addAction(QIcon(":/toolicon/32x32/spline_cut_point.png"),     tr("Point on Curve") + "\tO, C");
    QAction *action_PointAlongSpline     = menu.addAction(QIcon(":/toolicon/32x32/splinePath_cut_point.png"), tr("Point on Spline") + "\tO, S");
    QAction *action_CurveIntersectCurve  = menu.addAction(QIcon(":/toolicon/32x32/intersection_curves.png"),  tr("Intersect Curves") + "\tI, C");
    QAction *action_CurveIntersectAxis   = menu.addAction(QIcon(":/toolicon/32x32/curve_intersect_axis.png"), tr("Intersect Curve & Axis") + "\tC, X");

    QAction *selectedAction = menu.exec(QCursor::pos());

    if(selectedAction == nullptr)
    {
        return;
    }
    else if (selectedAction == action_Curve)
    {
        ui->draft_ToolBox->setCurrentWidget(ui->curves_Page);
        ui->curve_ToolButton->setChecked(true);
        handleCurveTool(true);
    }
    else if (selectedAction == action_Spline)
    {
        ui->draft_ToolBox->setCurrentWidget(ui->curves_Page);
        ui->spline_ToolButton->setChecked(true);
        handleSplineTool(true);
    }
    else if (selectedAction == action_PointAlongCurve)
    {
        ui->draft_ToolBox->setCurrentWidget(ui->curves_Page);
        ui->pointAlongCurve_ToolButton->setChecked(true);
        handlePointAlongCurveTool(true);
    }
    else if (selectedAction == action_PointAlongSpline)
    {
        ui->draft_ToolBox->setCurrentWidget(ui->curves_Page);
        ui->pointAlongSpline_ToolButton->setChecked(true);
        handlePointAlongSplineTool(true);
    }
    else if (selectedAction == action_CurveWithCPs)
    {
        ui->draft_ToolBox->setCurrentWidget(ui->curves_Page);
        ui->curveWithCPs_ToolButton->setChecked(true);
        handleCurveWithControlPointsTool(true);
    }
    else if (selectedAction == action_SplineWithCPs)
    {
        ui->draft_ToolBox->setCurrentWidget(ui->curves_Page);
        ui->splineWithCPs_ToolButton->setChecked(true);
        handleSplineWithControlPointsTool(true);
    }
    else if (selectedAction == action_CurveIntersectCurve)
    {
        ui->draft_ToolBox->setCurrentWidget(ui->curves_Page);
        ui->pointOfIntersectionCurves_ToolButton->setChecked(true);
        handleCurveIntersectCurveTool(true);
    }
    else if (selectedAction == action_CurveIntersectAxis)
    {
        ui->draft_ToolBox->setCurrentWidget(ui->curves_Page);
        ui->curveIntersectAxis_ToolButton->setChecked(true);
        handleCurveIntersectAxisTool(true);
    }
}

void MainWindow::handleCirclesMenu()
{
    qCDebug(vMainWindow, "Circles Menu selected. \n");

}

void MainWindow::handleOperationsMenu()
{
    qCDebug(vMainWindow, "Operations Menu selected. \n");
    QMenu menu;

    QAction *action_Group        = menu.addAction(QIcon(":/toolicon/32x32/group.png"),          tr("Add Objects to Group") + "\tG");
    QAction *action_Rotate       = menu.addAction(QIcon(":/toolicon/32x32/rotation.png"),       tr("Rotate") + "\tR");
    QAction *action_MirrorByLine = menu.addAction(QIcon(":/toolicon/32x32/mirror_by_line.png"), tr("Mirror by Line") + "\tM, L");
    QAction *action_MirrorByAxis = menu.addAction(QIcon(":/toolicon/32x32/mirror_by_axis.png"), tr("Mirror by Axis") + "\tM, A");
    QAction *action_Move         = menu.addAction(QIcon(":/toolicon/32x32/move.png"),           tr("Move") + "\t" + tr("Alt+M"));
    QAction *action_TrueDarts    = menu.addAction(QIcon(":/toolicon/32x32/true_darts.png"),     tr("True Darts") + "\tT, D");
    QAction *action_ExportDraftBlocks = menu.addAction(QIcon(":/toolicon/32x32/export.png"),    tr("Export Draft Blocks") + "\tE, D");

    QAction *selectedAction = menu.exec(QCursor::pos());

    if(selectedAction == nullptr)
    {
        return;
    }
    else if (selectedAction == action_Group)
    {
        ui->draft_ToolBox->setCurrentWidget(ui->operations_Page);
        ui->group_ToolButton->setChecked(true);
        handleGroupTool(true);
    }
    else if (selectedAction == action_Rotate)
    {
        ui->draft_ToolBox->setCurrentWidget(ui->operations_Page);
        ui->rotation_ToolButton->setChecked(true);
        handleRotationTool(true);
    }
    else if (selectedAction == action_MirrorByLine)
    {
        ui->draft_ToolBox->setCurrentWidget(ui->operations_Page);
        ui->mirrorByLine_ToolButton->setChecked(true);
        handleMirrorByLineTool(true);
    }
    else if (selectedAction == action_MirrorByAxis)
    {
        ui->draft_ToolBox->setCurrentWidget(ui->operations_Page);
        ui->mirrorByAxis_ToolButton->setChecked(true);
        handleMirrorByAxisTool(true);
    }
    else if (selectedAction == action_Move)
    {
        ui->draft_ToolBox->setCurrentWidget(ui->operations_Page);
        ui->move_ToolButton->setChecked(true);
        handleMoveTool(true);
    }
    else if (selectedAction == action_TrueDarts)
    {
        ui->draft_ToolBox->setCurrentWidget(ui->operations_Page);
        ui->trueDarts_ToolButton->setChecked(true);
        handleTrueDartTool(true);
    }
    else if (selectedAction == action_ExportDraftBlocks)
    {
        ui->draft_ToolBox->setCurrentWidget(ui->operations_Page);
        exportDraftBlocksAs();
    }
}

void MainWindow::handlePieceMenu()
{
    QMenu menu;

    QAction *action_Piece        = menu.addAction(QIcon(":/toolicon/32x32/new_detail.png"),   tr("New Pattern Piece") + "\tN, P");
    QAction *action_AnchorPoint  = menu.addAction(QIcon(":/toolicon/32x32/anchor_point.png"), tr("Add AnchorPoint") + "\tA, P");
    QAction *action_InternalPath = menu.addAction(QIcon(":/toolicon/32x32/path.png"),         tr("Create Internal Path") + "\tI, N");
    QAction *action_InsertNodes  = menu.addAction(QIcon(":/toolicon/32x32/insert_nodes_icon.png"), tr("Insert Nodes in Path") + "\tI, P");

    action_AnchorPoint->setEnabled(pattern->DataPieces()->size() > 0);
    action_InternalPath->setEnabled(pattern->DataPieces()->size() > 0);
    action_InsertNodes->setEnabled(pattern->DataPieces()->size() > 0);

    QAction *selectedAction = menu.exec(QCursor::pos());

    if(selectedAction == nullptr)
    {
        return;
    }
    else if (selectedAction == action_Piece)
    {
        ui->draft_ToolBox->setCurrentWidget(ui->piece_Page);
        ui->addPatternPiece_ToolButton->setChecked(true);
        handlePatternPieceTool(true);
    }
    else if (selectedAction == action_AnchorPoint)
    {
        ui->draft_ToolBox->setCurrentWidget(ui->piece_Page);
        ui->anchorPoint_ToolButton->setChecked(true);
        handleAnchorPointTool(true);
    }
    else if (selectedAction == action_InternalPath)
    {
        ui->draft_ToolBox->setCurrentWidget(ui->piece_Page);
        ui->internalPath_ToolButton->setChecked(true);
        handleInternalPathTool(true);
    }
    else if (selectedAction == action_InsertNodes)
    {
        ui->draft_ToolBox->setCurrentWidget(ui->piece_Page);
        ui->insertNodes_ToolButton->setChecked(true);
        handleInsertNodesTool(true);
    }
}

void MainWindow::handlePatternPiecesMenu()
{
    qCDebug(vMainWindow, "PatternPieces Menu selected. \n");
    QMenu menu;

    QAction *action_Union        = menu.addAction(QIcon(":/toolicon/32x32/union.png"),  tr("Union Tool") + "\tU");
    QAction *action_ExportPieces = menu.addAction(QIcon(":/toolicon/32x32/export.png"), tr("Export Pattern Pieces") + "\tE, P");
    QAction *selectedAction      = menu.exec(QCursor::pos());

    if(selectedAction == nullptr)
    {
        return;
    }
    else if (selectedAction == action_Union)
    {
        ui->piece_ToolBox->setCurrentWidget(ui->details_Page);
        ui->unitePieces_ToolButton->setChecked(true);
        handleUnionTool(true);
    }
    else if (selectedAction == action_ExportPieces)
    {
        ui->piece_ToolBox->setCurrentWidget(ui->details_Page);
        exportPiecesAs();
    }
}

void MainWindow::handleLayoutMenu()
{
    qCDebug(vMainWindow, "Layout Menu selected. \n");


    QMenu menu;

    QAction *action_NewLayout    = menu.addAction(QIcon(":/toolicon/32x32/layout_settings.png"), tr("New Print Layout") + "\tN, L");
    QAction *action_ExportLayout = menu.addAction(QIcon(":/toolicon/32x32/export.png"), tr("Export Layout") + "\tE, L");

    QAction *selectedAction = menu.exec(QCursor::pos());

    if(selectedAction == nullptr)
    {
        return;
    }
    else if (selectedAction == action_NewLayout)
    {
        ui->layout_ToolBox->setCurrentWidget(ui->layout_Page);
        ui->layoutSettings_ToolButton->setChecked(true);
        handleNewLayout(true);
    }
    else if (selectedAction == action_ExportLayout)
    {
        ui->layout_ToolBox->setCurrentWidget(ui->layout_Page);
        exportLayoutAs();
    }
}

//---------------------------------------------------------------------------------------------------------------------
/**
 * @brief mouseMove save mouse position and show user.
 * @param scenePos position mouse.
 */
void MainWindow::MouseMove(const QPointF &scenePos)
{
    if (mouseCoordinates)
    {
        mouseCoordinates->updateCoordinates(scenePos);
    }
}

//---------------------------------------------------------------------------------------------------------------------
QT_WARNING_PUSH
QT_WARNING_DISABLE_GCC("-Wswitch-default")
/**
 * @brief CancelTool cancel tool.
 */
void MainWindow::CancelTool()
{
    // This check helps to find missed tools in the switch
    Q_STATIC_ASSERT_X(static_cast<int>(Tool::LAST_ONE_DO_NOT_USE) == 53, "Not all tools were handled.");

    qCDebug(vMainWindow, "Canceling tool.");
    dialogTool.clear();
    qCDebug(vMainWindow, "Dialog closed.");

    currentScene->setFocus(Qt::OtherFocusReason);
    currentScene->clearSelection();
    emit ui->view->itemClicked(nullptr);           // Hide visualization to avoid a crash

    ui->zoomPan_Action->setChecked(false);         //Disable Pan Zoom
    ui->view->zoomPanEnabled(false);

    ui->zoomToArea_Action->setChecked(false);      //Disable Zoom to Area
    ui->view->zoomToAreaEnabled(false);

    switch (currentTool)
    {
        case Tool::Arrow:
            ui->arrowPointer_ToolButton->setChecked(false);
            ui->arrow_Action->setChecked(false);
            helpLabel->setText(QString(""));

            // Crash: using CRTL+Z while using line tool.
            undoAction->setEnabled(false);
            redoAction->setEnabled(false);
            VAbstractTool::m_suppressContextMenu = true;
            return;
        case Tool::BasePoint:
        case Tool::SinglePoint:
        case Tool::DoublePoint:
        case Tool::LinePoint:
        case Tool::AbstractSpline:
        case Tool::Cut:
        case Tool::LAST_ONE_DO_NOT_USE:
        case Tool::NodePoint:
        case Tool::NodeArc:
        case Tool::NodeElArc:
        case Tool::NodeSpline:
        case Tool::NodeSplinePath:
            Q_UNREACHABLE(); //-V501
            //Nothing to do here because we can't create this tool from main window.
            break;
        case Tool::EndLine:
            ui->pointAtDistanceAngle_ToolButton->setChecked(false);
            break;
        case Tool::Line:
            ui->line_ToolButton->setChecked(false);
            break;
        case Tool::AlongLine:
            ui->alongLine_ToolButton->setChecked(false);
            break;
        case Tool::Midpoint:
            ui->midpoint_ToolButton->setChecked(false);
            break;
        case Tool::ShoulderPoint:
            ui->shoulderPoint_ToolButton->setChecked(false);
            break;
        case Tool::Normal:
            ui->normal_ToolButton->setChecked(false);
            break;
        case Tool::Bisector:
            ui->bisector_ToolButton->setChecked(false);
            break;
        case Tool::LineIntersect:
            ui->lineIntersect_ToolButton->setChecked(false);
            break;
        case Tool::Spline:
            ui->curve_ToolButton->setChecked(false);
            break;
        case Tool::CubicBezier:
            ui->curveWithCPs_ToolButton->setChecked(false);
            break;
        case Tool::Arc:
            ui->arc_ToolButton->setChecked(false);
            break;
        case Tool::ArcWithLength:
            ui->arcWithLength_ToolButton->setChecked(false);
            break;
        case Tool::SplinePath:
            ui->spline_ToolButton->setChecked(false);
            break;
        case Tool::CubicBezierPath:
            ui->splineWithCPs_ToolButton->setChecked(false);
            break;
        case Tool::PointOfContact:
            ui->pointOfContact_ToolButton->setChecked(false);
            break;
        case Tool::Piece:
            ui->addPatternPiece_ToolButton->setChecked(false);
            break;
        case Tool::InternalPath:
            ui->internalPath_ToolButton->setChecked(false);
            break;
        case Tool::Height:
            ui->height_ToolButton->setChecked(false);
            break;
        case Tool::Triangle:
            ui->triangle_ToolButton->setChecked(false);
            break;
        case Tool::PointOfIntersection:
            ui->pointIntersectXY_ToolButton->setChecked(false);
            break;
        case Tool::CutSpline:
            ui->pointAlongCurve_ToolButton->setChecked(false);
            break;
        case Tool::CutSplinePath:
            ui->pointAlongSpline_ToolButton->setChecked(false);
            break;
        case Tool::Union:
            ui->unitePieces_ToolButton->setChecked(false);
            break;
        case Tool::CutArc:
            ui->pointAlongArc_ToolButton->setChecked(false);
            break;
        case Tool::LineIntersectAxis:
            ui->lineIntersectAxis_ToolButton->setChecked(false);
            break;
        case Tool::CurveIntersectAxis:
            ui->curveIntersectAxis_ToolButton->setChecked(false);
            break;
        case Tool::ArcIntersectAxis:
            ui->arcIntersectAxis_ToolButton->setChecked(false);
            break;
        case Tool::PointOfIntersectionArcs:
            ui->pointOfIntersectionArcs_ToolButton->setChecked(false);
            break;
        case Tool::PointOfIntersectionCircles:
            ui->pointOfIntersectionCircles_ToolButton->setChecked(false);
            break;
        case Tool::PointOfIntersectionCurves:
            ui->pointOfIntersectionCurves_ToolButton->setChecked(false);
            break;
        case Tool::PointFromCircleAndTangent:
            ui->pointFromCircleAndTangent_ToolButton->setChecked(false);
            break;
        case Tool::PointFromArcAndTangent:
            ui->pointFromArcAndTangent_ToolButton->setChecked(false);
            break;
        case Tool::TrueDarts:
            ui->trueDarts_ToolButton->setChecked(false);
            break;
        case Tool::Group:
            ui->group_ToolButton->setChecked(false);
            break;
        case Tool::Rotation:
            ui->rotation_ToolButton->setChecked(false);
            break;
        case Tool::MirrorByLine:
            ui->mirrorByLine_ToolButton->setChecked(false);
            break;
        case Tool::MirrorByAxis:
            ui->mirrorByAxis_ToolButton->setChecked(false);
            break;
        case Tool::Move:
            ui->move_ToolButton->setChecked(false);
            break;
        case Tool::EllipticalArc:
            ui->ellipticalArc_ToolButton->setChecked(false);
            break;
        case Tool::AnchorPoint:
            ui->anchorPoint_ToolButton->setChecked(false);
            break;
        case Tool::InsertNodes:
            ui->insertNodes_ToolButton->setChecked(false);
            break;
    }

    // Crash: using CRTL+Z while using line tool.
    undoAction->setEnabled(qApp->getUndoStack()->canUndo());
    redoAction->setEnabled(qApp->getUndoStack()->canRedo());
}

QT_WARNING_POP

//---------------------------------------------------------------------------------------------------------------------
/**
 * @brief handleArrowTool enable arrow tool.
 */
void  MainWindow::handleArrowTool(bool checked)
{
    if (checked && currentTool != Tool::Arrow)
    {
        qCDebug(vMainWindow, "Arrow tool.");
        CancelTool();
        ui->arrowPointer_ToolButton->setChecked(true);
        ui->arrow_Action->setChecked(true);
        currentTool = Tool::Arrow;
        emit EnableItemMove(true);
        emit ItemsSelection(SelectionType::ByMouseRelease);
        VAbstractTool::m_suppressContextMenu = false;

        // Only true for rubber band selection
        emit EnableLabelSelection(true);
        emit EnablePointSelection(false);
        emit EnableLineSelection(false);
        emit EnableArcSelection(false);
        emit EnableElArcSelection(false);
        emit EnableSplineSelection(false);
        emit EnableSplinePathSelection(false);
        emit EnableNodeLabelSelection(true);
        emit EnableNodePointSelection(true);
        emit enablePieceSelection(true);// Disable when done with pattern piece visualization

        // Hovering
        emit EnableLabelHover(true);
        emit EnablePointHover(true);
        emit EnableLineHover(true);
        emit EnableArcHover(true);
        emit EnableElArcHover(true);
        emit EnableSplineHover(true);
        emit EnableSplinePathHover(true);
        emit EnableNodeLabelHover(true);
        emit EnableNodePointHover(true);
        emit enablePieceHover(true);

        ui->view->allowRubberBand(true);

        ui->view->viewport()->unsetCursor();
        helpLabel->setText("");
        ui->view->setShowToolOptions(true);
        qCDebug(vMainWindow, "Enabled arrow tool.");
    }
    else
    {
        ui->view->viewport()->setCursor(QCursor(Qt::ArrowCursor));
        ui->arrowPointer_ToolButton->setChecked(true);
        ui->arrow_Action->setChecked(true);
    }
}

//---------------------------------------------------------------------------------------------------------------------
/**
 * @brief keyPressEvent handle key press events.
 * @param event key event.
 */
void MainWindow::keyPressEvent(QKeyEvent *event)
{
    switch (event->key())
    {
        case Qt::Key_Escape:
            handleArrowTool(true);
            break;
        case Qt::Key_Return:
        case Qt::Key_Enter:
            EndVisualization();
            break;
        case Qt::Key_Space:
            if (qApp->Seamly2DSettings()->isPanActiveSpaceKey())
            {
                ui->zoomPan_Action->setChecked(true);
            }
            break;
        default:
            break;
    }
    QMainWindow::keyPressEvent (event);
}

//---------------------------------------------------------------------------------------------------------------------
/**
 * @brief keyReleaseEvent handle key press events.
 * @param event key event.
 */
void MainWindow::keyReleaseEvent(QKeyEvent *event)
{
    switch (event->key())
    {
        case Qt::Key_Space:
            if (qApp->Seamly2DSettings()->isPanActiveSpaceKey())
            {
                ui->zoomPan_Action->setChecked(false);
            }
        default:
            break;
    }
    QMainWindow::keyReleaseEvent(event);
}

//---------------------------------------------------------------------------------------------------------------------
/**
 * @brief SaveCurrentScene save scene options before set another.
 */
void MainWindow::SaveCurrentScene()
{
    if (mode == Draw::Calculation || mode == Draw::Modeling)
    {
        VMainGraphicsScene *scene = qobject_cast<VMainGraphicsScene *>(currentScene);
        SCASSERT(scene != nullptr)

        /*Save transform*/
        scene->setCurrentTransform(ui->view->transform());
        /*Save scroll bars value for previous scene.*/
        QScrollBar *horScrollBar = ui->view->horizontalScrollBar();
        scene->setHorScrollBar(horScrollBar->value());
        QScrollBar *verScrollBar = ui->view->verticalScrollBar();
        scene->setVerScrollBar(verScrollBar->value());
    }
}

//---------------------------------------------------------------------------------------------------------------------
/**
 * @brief RestoreCurrentScene restore scene options after change.
 */
void MainWindow::RestoreCurrentScene()
{
    VMainGraphicsScene *scene = qobject_cast<VMainGraphicsScene *>(currentScene);
    SCASSERT(scene != nullptr)

    /*Set transform for current scene*/
    ui->view->setTransform(scene->transform());
    zoomScaleChanged(ui->view->transform().m11());
    /*Set value for current scene scroll bar.*/
    QScrollBar *horScrollBar = ui->view->horizontalScrollBar();
    horScrollBar->setValue(scene->getHorScrollBar());
    QScrollBar *verScrollBar = ui->view->verticalScrollBar();
    verScrollBar->setValue(scene->getVerScrollBar());
}

//---------------------------------------------------------------------------------------------------------------------
/**
 * @brief showDraftMode show draw scene.
 * @param checked true - button checked.
 */
void MainWindow::showDraftMode(bool checked)
{
    if (checked)
    {
        ui->toolbox_StackedWidget->setCurrentIndex(0);
        qCDebug(vMainWindow, "Show draft scene");
        handleArrowTool(true);

        leftGoToStage->setPixmap(QPixmap("://icon/24x24/fast_forward_left_to_right_arrow.png"));
        rightGoToStage->setPixmap(QPixmap("://icon/24x24/left_to_right_arrow.png"));

        ui->showDraftMode->setChecked(true);
        ui->pieceMode_Action->setChecked(false);
        ui->layoutMode_Action->setChecked(false);

        SaveCurrentScene();

        currentScene = draftScene;
        ui->view->setScene(currentScene);
        RestoreCurrentScene();

        mode = Draw::Calculation;
        draftBlockComboBox->setCurrentIndex(currentBlockIndex); //restore current draft block
        drawMode = true;

        setToolsEnabled(true);
        setWidgetsEnabled(true);

        draftScene->enablePiecesMode(qApp->Seamly2DSettings()->getShowControlPoints());
        draftScene->setOriginsVisible(qApp->Settings()->getShowAxisOrigin());

        updateViewToolbar();

        //ui->toggleAnchorPoints_Action->setChecked(qApp->Settings()->getShowAnchorPoints());
        //draftScene->setOriginsVisible(qApp->Settings()->getShowAnchorPoints());

        ui->useToolColor_Action->setChecked(qApp->Settings()->getUseToolColor());

        ui->draft_ToolBox->setCurrentIndex(currentToolBoxIndex);

        if (qApp->patternType() == MeasurementsType::Multisize)
        {
            gradationHeightsLabel->setVisible(true);
            gradationHeights->setVisible(true);
            gradationSizesLabel->setVisible(true);
            gradationSizes->setVisible(true);
        }
        ui->groups_DockWidget->setWidget(groupsWidget);
        ui->groups_DockWidget->setWindowTitle(tr("Group Manager"));
    }
    else
    {
        ui->showDraftMode->setChecked(true);
    }
}

//---------------------------------------------------------------------------------------------------------------------
/**
 * @brief showPieceMode show Piece scene.
 * @param checked true - button checked.
 */
void MainWindow::showPieceMode(bool checked)
{
    if (checked)
    {
        ui->toolbox_StackedWidget->setCurrentIndex(1);
        handleArrowTool(true);

        if(drawMode)
        {
            currentBlockIndex = draftBlockComboBox->currentIndex();         // Save current draftf block.
            drawMode = false;
        }
        draftBlockComboBox->setCurrentIndex(draftBlockComboBox->count()-1); // Need to get data about all blocks.

        leftGoToStage->setPixmap(QPixmap("://icon/24x24/right_to_left_arrow.png"));
        rightGoToStage->setPixmap(QPixmap("://icon/24x24/left_to_right_arrow.png"));

        ui->showDraftMode->setChecked(false);
        ui->pieceMode_Action->setChecked(true);
        ui->layoutMode_Action->setChecked(false);

        if(!qApp->getOpeningPattern())
        {
            if (pattern->DataPieces()->count() == 0)
            {
                QMessageBox::information(this, tr("Piece mode"), tr("You can't use Piece mode yet. "
                                                                     "Please, create at least one pattern piece."),
                                         QMessageBox::Ok, QMessageBox::Ok);
                showDraftMode(true);
                return;
            }
        }

        patternPiecesWidget->updateList();

        qCDebug(vMainWindow, "Show piece scene");
        SaveCurrentScene();

        currentScene = pieceScene;
        emit ui->view->itemClicked(nullptr);
        ui->view->setScene(currentScene);
        RestoreCurrentScene();

        if (mode == Draw::Calculation)
        {
            currentToolBoxIndex = ui->piece_ToolBox->currentIndex();
        }
        mode = Draw::Modeling;
        setToolsEnabled(true);
        setWidgetsEnabled(true);

        pieceScene->setOriginsVisible(qApp->Settings()->getShowAxisOrigin());

        updateViewToolbar();

        ui->piece_ToolBox->setCurrentIndex(ui->piece_ToolBox->indexOf(ui->details_Page));

        if (qApp->patternType() == MeasurementsType::Multisize)
        {
            gradationHeightsLabel->setVisible(true);
            gradationHeights->setVisible(true);
            gradationSizesLabel->setVisible(true);
            gradationSizes->setVisible(true);
        }
        ui->groups_DockWidget->setWidget(patternPiecesWidget);
        ui->groups_DockWidget->setWindowTitle(tr("Pattern Pieces"));

        helpLabel->setText("");
    }
    else
    {
        ui->pieceMode_Action->setChecked(true);
    }
}

//---------------------------------------------------------------------------------------------------------------------
/**
 * @brief showLayoutMode show layout scene.
 * @param checked true - button checked.
 */
void MainWindow::showLayoutMode(bool checked)
{
    if (checked)
    {
        ui->toolbox_StackedWidget->setCurrentIndex(2);
        handleArrowTool(true);

        if(drawMode)
        {
            currentBlockIndex = draftBlockComboBox->currentIndex();//save current drfat block
            drawMode = false;
        }
        draftBlockComboBox->setCurrentIndex(draftBlockComboBox->count()-1);// Need to get data about all draft blocks

        leftGoToStage->setPixmap(QPixmap("://icon/24x24/right_to_left_arrow.png"));
        rightGoToStage->setPixmap(QPixmap("://icon/24x24/fast_forward_right_to_left_arrow.png"));

        ui->showDraftMode->setChecked(false);
        ui->pieceMode_Action->setChecked(false);
        ui->layoutMode_Action->setChecked(true);

        QHash<quint32, VPiece> pieces;
        if(!qApp->getOpeningPattern())
        {
            const QHash<quint32, VPiece> *allPieces = pattern->DataPieces();
            if (allPieces->count() == 0)
            {
                QMessageBox::information(this, tr("Layout mode"), tr("You can't use Layout mode yet. "
                                                                     "Please, create at least one pattern piece."),
                                         QMessageBox::Ok, QMessageBox::Ok);
                showDraftMode(true);
                return;
            }
            else
            {
                QHash<quint32, VPiece>::const_iterator i = allPieces->constBegin();
                while (i != allPieces->constEnd())
                {
                    if (i.value().isInLayout())
                    {
                        pieces.insert(i.key(), i.value());
                    }
                    ++i;
                }

                if (pieces.count() == 0)
                {
                    QMessageBox::information(this, tr("Layout mode"),  tr("You can't use Layout mode yet. Please, "
                                                                          "include at least one pattern piece in layout."),
                                             QMessageBox::Ok, QMessageBox::Ok);
                    mode == Draw::Calculation ? showDraftMode(true) : showPieceMode(true);
                    return;
                }
            }
        }

        draftBlockComboBox->setCurrentIndex(-1);// Hide pattern pieces

        qCDebug(vMainWindow, "Show layout scene");

        SaveCurrentScene();

        try
        {
            pieceList = preparePiecesForLayout(pieces);
        }

        catch (VException &exception)
        {
            pieceList.clear();
            QMessageBox::warning(this, tr("Layout mode"),
                                 tr("You can't use Layout mode yet.") + QLatin1String(" \n") + exception.ErrorMessage(),
                                 QMessageBox::Ok, QMessageBox::Ok);
            mode == Draw::Calculation ? showDraftMode(true) : showPieceMode(true);
            return;
        }

        currentScene = tempSceneLayout;
        emit ui->view->itemClicked(nullptr);
        ui->view->setScene(currentScene);

        if (mode == Draw::Calculation)
        {
            currentToolBoxIndex = ui->layout_ToolBox->currentIndex();
        }
        mode = Draw::Layout;
        setToolsEnabled(true);
        setWidgetsEnabled(true);
        ui->layout_ToolBox->setCurrentIndex(ui->layout_ToolBox->indexOf(ui->layout_Page));

        mouseCoordinates->updateCoordinates(QPointF());

        if (qApp->patternType() == MeasurementsType::Multisize)
        {
            gradationHeightsLabel->setVisible(false);
            gradationHeights->setVisible(false);
            gradationSizesLabel->setVisible(false);
            gradationSizes->setVisible(false);
        }

        showLayoutPages(ui->listWidget->currentRow());

        if (scenes.isEmpty())
        {
            ui->layoutSettings_ToolButton->click();
        }

        helpLabel->setText("");
    }
    else
    {
        ui->layoutMode_Action->setChecked(true);
    }
}

//---------------------------------------------------------------------------------------------------------------------
/**
 * @brief SaveAs save as pattern file.
 * @return true for successes saving.
 */
bool MainWindow::SaveAs()
{
    if (patternReadOnly)
    {
        QMessageBox messageBox(this);
        messageBox.setIcon(QMessageBox::Warning);
        messageBox.setText(tr("Can not save file."));
        messageBox.setInformativeText(tr("Pattern is read only."));
        messageBox.setDefaultButton(QMessageBox::Ok);
        messageBox.setStandardButtons(QMessageBox::Ok);
        messageBox.exec();
        return false;
    }
    QString filters(tr("Pattern files") + QLatin1String("(*.") + sm2dExt + QLatin1String(")"));
    QString filePath = qApp->getFilePath();
    QString dir;
    QString fileName;
    bool usedNotExistedDir = false;
    if (filePath.isEmpty())
    {
        dir = qApp->Seamly2DSettings()->GetPathPattern();
        fileName = tr("pattern");
    }
    else
    {
        dir = QFileInfo(filePath).path();
        fileName = QFileInfo(filePath).baseName();
    }

    auto RemoveTempDir = [usedNotExistedDir, dir]()
    {
        if (usedNotExistedDir)
        {
            QDir directory(dir);
            directory.rmpath(".");
        }
    };

    QDir directory(dir);
    if (!directory.exists())
    {
        usedNotExistedDir = directory.mkpath(".");
    }

    fileName = fileDialog(this, tr("Save as"),
                                        dir + QLatin1String("/") + fileName + QLatin1String(".") + sm2dExt,
                                        filters, nullptr, QFileDialog::DontUseNativeDialog, QFileDialog::AnyFile,
                                        QFileDialog::AcceptSave);

    if (fileName.isEmpty())
    {
        RemoveTempDir();
        return false;
    }

    QFileInfo fileInfo(fileName);
    if (fileInfo.suffix().isEmpty() && fileInfo.suffix() != sm2dExt)
    {
        fileName += QLatin1String(".") + sm2dExt;
    }

    if (fileInfo.exists() && fileName != filePath)
    {
        // Temporarily try to lock the file before saving
        // Also help to rewrite current read-only pattern
        VLockGuard<char> lock(fileName);
        if (!lock.IsLocked())
        {
            qCWarning(vMainWindow, "%s",
                       qUtf8Printable(tr("Failed to lock. File with this name is opened in another window.")));
            RemoveTempDir();
            return false;
        }
    }

    // Need for restoring previous state in case of failure
    const bool wasModified = doc->IsModified(); // Need because SetReadOnly() will change internal state
    QString error;
    const bool result = SavePattern(fileName, error);
    if (result == false)
    {
        QMessageBox messageBox(this);
        messageBox.setIcon(QMessageBox::Warning);
        messageBox.setInformativeText(tr("Could not save file"));
        messageBox.setDefaultButton(QMessageBox::Ok);
        messageBox.setDetailedText(error);
        messageBox.setStandardButtons(QMessageBox::Ok);
        messageBox.exec();

        // Restoring previous state
        doc->SetModified(wasModified);

        RemoveTempDir();
        return result;
    }

    QFile::remove(qApp->getFilePath() + autosavePrefix);
    m_curFileFormatVersion = VPatternConverter::PatternMaxVer;
    m_curFileFormatVersionStr = VPatternConverter::PatternMaxVerStr;

    if (fileName != filePath)
    {
        VlpCreateLock(lock, fileName);
	    if (!lock->IsLocked())
        {
            qCWarning(vMainWindow, "%s", qUtf8Printable(tr("Failed to lock. This file already opened in another window. "
														    "Expect collisions when running 2 copies of the program.")));
		    RemoveTempDir();
	        return false;
	    }
    }

    RemoveTempDir();
    return result;
}

//---------------------------------------------------------------------------------------------------------------------
/**
 * @brief Save save pattern file.
 * @return true for successes saving.
 */
bool MainWindow::Save()
{
    if (patternReadOnly)
    {
        QMessageBox messageBox(this);
        messageBox.setIcon(QMessageBox::Warning);
        messageBox.setText(tr("Can not save file."));
        messageBox.setInformativeText(tr("Pattern is read only."));
        messageBox.setDefaultButton(QMessageBox::Ok);
        messageBox.setStandardButtons(QMessageBox::Ok);
        messageBox.exec();
        return false;
    }
    if (qApp->getFilePath().isEmpty())
    {
        return SaveAs();
    }
    else
    {
        if (m_curFileFormatVersion < VPatternConverter::PatternMaxVer
                && not ContinueFormatRewrite(m_curFileFormatVersionStr, VPatternConverter::PatternMaxVerStr))
        {
            return false;
        }

#ifdef Q_OS_WIN32
        qt_ntfs_permission_lookup++; // turn checking on
#endif /*Q_OS_WIN32*/
        const bool isFileWritable = QFileInfo(qApp->getFilePath()).isWritable();

        if (!isFileWritable)
        {
            QMessageBox messageBox(this);
            messageBox.setIcon(QMessageBox::Question);
            messageBox.setText(tr("The document has no write permissions."));
            messageBox.setInformativeText("Do you want to change the permissions?");
            messageBox.setStandardButtons(QMessageBox::Yes | QMessageBox::Cancel);
            messageBox.setDefaultButton(QMessageBox::Yes);

            if (messageBox.exec() == QMessageBox::Yes)
            {
                bool changed = QFile::setPermissions(qApp->getFilePath(),
                                                    QFileInfo(qApp->getFilePath()).permissions() | QFileDevice::WriteUser);
#ifdef Q_OS_WIN32
                qt_ntfs_permission_lookup--; // turn it off again
#endif /*Q_OS_WIN32*/

                if (!changed)
                {
                    QMessageBox messageBox(this);
                    messageBox.setIcon(QMessageBox::Warning);
                    messageBox.setText(tr("Cannot set permissions for %1 to writable.").arg(qApp->getFilePath()));
                    messageBox.setInformativeText(tr("Could not save the file."));
                    messageBox.setDefaultButton(QMessageBox::Ok);
                    messageBox.setStandardButtons(QMessageBox::Ok);
                    messageBox.exec();
                    return false;
                }
            }
            else
            {
                return false;
            }
        }

        QString error;
        bool result = SavePattern(qApp->getFilePath(), error);
        if (result)
        {
            QFile::remove(qApp->getFilePath() + autosavePrefix);
            m_curFileFormatVersion = VPatternConverter::PatternMaxVer;
            m_curFileFormatVersionStr = VPatternConverter::PatternMaxVerStr;
        }
        else
        {
            QMessageBox messageBox(this);
            messageBox.setIcon(QMessageBox::Warning);
            messageBox.setText(tr("Could not save the file"));
            messageBox.setDefaultButton(QMessageBox::Ok);
            messageBox.setDetailedText(error);
            messageBox.setStandardButtons(QMessageBox::Ok);
            messageBox.exec();
        }
        return result;
    }
}

//---------------------------------------------------------------------------------------------------------------------
/**
 * @brief Open ask user select pattern file.
 */
void MainWindow::Open()
{
    qCDebug(vMainWindow, "Opening new file.");

    const QString filter = tr("Pattern files") + QLatin1String(" (*.") + valExt +
                           QLatin1String(" *.") + sm2dExt + QLatin1String(")");

    //Get list last open files
    const QStringList files = qApp->Seamly2DSettings()->GetRecentFileList();
    QString dir;
    if (files.isEmpty())
    {
        dir = QDir::homePath();
    }
    else
    {
        //Absolute path to last open file
        dir = QFileInfo(files.first()).absolutePath();
    }
    qCDebug(vMainWindow, "Run QFileDialog::getOpenFileName: dir = %s.", qUtf8Printable(dir));

    const QString filename = fileDialog(this, tr("Open file"), dir, filter, nullptr, QFileDialog::DontUseNativeDialog,
                                        QFileDialog::ExistingFile, QFileDialog::AcceptOpen);

    if (filename.isEmpty())
    {
        return;
    }
    LoadPattern(filename);
}

//---------------------------------------------------------------------------------------------------------------------
/**
 * @brief Clear reset to default window.
 */
void MainWindow::Clear()
{
    qCDebug(vMainWindow, "Resetting main window.");
    lock.reset();
    qCDebug(vMainWindow, "Unlocked pattern file.");
    showDraftMode(true);
    qCDebug(vMainWindow, "Returned to Draft mode.");
    setCurrentFile(QString());
    pattern->Clear();
    qCDebug(vMainWindow, "Clearing pattern.");
    if (!qApp->getFilePath().isEmpty() && not doc->MPath().isEmpty())
    {
        watcher->removePath(AbsoluteMPath(qApp->getFilePath(), doc->MPath()));
    }
    doc->clear();
    qCDebug(vMainWindow, "Clearing scenes.");
    draftScene->clear();
    pieceScene->clear();
    handleArrowTool(true);
    draftBlockComboBox->clear();
    ui->showDraftMode->setEnabled(false);
    ui->pieceMode_Action->setEnabled(false);
    ui->layoutMode_Action->setEnabled(false);
    ui->newDraft_Action->setEnabled(false);
    ui->renameDraft_Action->setEnabled(false);
    ui->save_Action->setEnabled(false);
    ui->saveAs_Action->setEnabled(false);
    ui->patternPreferences_Action->setEnabled(false);

    // disable zoom actions until a pattern is loaded
    zoomScaleSpinBox->setEnabled(false);
    ui->zoomIn_Action->setEnabled(false);
    ui->zoomOut_Action->setEnabled(false);
    ui->zoomToFit_Action->setEnabled(false);
    ui->zoomToSelected_Action->setEnabled(false);
    ui->zoom100Percent_Action->setEnabled(false);
    ui->zoomToPrevious_Action->setEnabled(false);
    ui->zoomToArea_Action->setEnabled(false);
    ui->zoomPan_Action->setEnabled(false);
    ui->zoomToPoint_Action->setEnabled(false);

    //disable group actions
    ui->groups_DockWidget->setEnabled(false);

    //disable history menu actions
    ui->history_Action->setEnabled(false);
    ui->table_Action->setEnabled(false);

    ui->lastTool_Action->setEnabled(false);
    ui->increaseSize_Action->setEnabled(false);
    ui->decreaseSize_Action->setEnabled(false);
    ui->useToolColor_Action->setEnabled(false);
    ui->showPointNames_Action->setEnabled(false);
    ui->toggleWireframe_Action->setEnabled(false);
    ui->toggleControlPoints_Action->setEnabled(false);
    ui->toggleAxisOrigin_Action->setEnabled(false);
    ui->toggleSeamAllowances_Action->setEnabled(false);
    ui->toggleGrainLines_Action->setEnabled(false);
    ui->toggleLabels_Action->setEnabled(false);
    //ui->toggleAnchorPoints_Action->setEnabled(false);


    //disable measurements menu actions
    ui->loadIndividual_Action->setEnabled(false);
    ui->loadMultisize_Action->setEnabled(false);
    ui->unloadMeasurements_Action->setEnabled(false);
    ui->editCurrent_Action->setEnabled(false);

    setToolsEnabled(false);
    qApp->setPatternUnit(Unit::Cm);
    qApp->setPatternType(MeasurementsType::Unknown);

#ifndef QT_NO_CURSOR
    QGuiApplication::restoreOverrideCursor();
#endif
    CleanLayout();
    pieceList.clear(); // don't move to CleanLayout()
    qApp->getUndoStack()->clear();
    toolProperties->clearPropertyBrowser();
    toolProperties->itemClicked(nullptr);
}

//---------------------------------------------------------------------------------------------------------------------
void MainWindow::FileClosedCorrect()
{
    WriteSettings();

    //File was closed correct.
    QStringList restoreFiles = qApp->Seamly2DSettings()->GetRestoreFileList();
    restoreFiles.removeAll(qApp->getFilePath());
    qApp->Seamly2DSettings()->SetRestoreFileList(restoreFiles);

    // Remove autosave file
    QFile autofile(qApp->getFilePath() + autosavePrefix);
    if (autofile.exists())
    {
        autofile.remove();
    }
    qCDebug(vMainWindow, "File %s closed correct.", qUtf8Printable(qApp->getFilePath()));
}

//---------------------------------------------------------------------------------------------------------------------
void MainWindow::fullParseFile()
{
    qCDebug(vMainWindow, "Full parsing file");

    toolProperties->clearPropertyBrowser();
    try
    {
        setGuiEnabled(true);
        doc->Parse(Document::FullParse);
    }

    catch (const VExceptionUndo &exception)
    {
        Q_UNUSED(exception)

        /* If user want undo last operation before undo we need finish broken redo operation. For those we post event
         * myself. Later in method customEvent call undo.*/
        QApplication::postEvent(this, new UndoEvent());
        return;
    }

    catch (const VExceptionObjectError &exception)
    {
        qCCritical(vMainWindow, "%s\n\n%s\n\n%s", qUtf8Printable(tr("Error parsing file.")), //-V807
                               qUtf8Printable(exception.ErrorMessage()), qUtf8Printable(exception.DetailedInformation()));
        setGuiEnabled(false);

        if (!VApplication::IsGUIMode())
        {
            qApp->exit(V_EX_NOINPUT);
        }
        return;
    }

    catch (const VExceptionConversionError &exception)
    {
        qCCritical(vMainWindow, "%s\n\n%s\n\n%s", qUtf8Printable(tr("Error can't convert value.")),
                               qUtf8Printable(exception.ErrorMessage()), qUtf8Printable(exception.DetailedInformation()));
        setGuiEnabled(false);

        if (!VApplication::IsGUIMode())
        {
            qApp->exit(V_EX_NOINPUT);
        }
        return;
    }

    catch (const VExceptionEmptyParameter &exception)
    {
        qCCritical(vMainWindow, "%s\n\n%s\n\n%s", qUtf8Printable(tr("Error empty parameter.")),
                               qUtf8Printable(exception.ErrorMessage()), qUtf8Printable(exception.DetailedInformation()));
        setGuiEnabled(false);

        if (!VApplication::IsGUIMode())
        {
            qApp->exit(V_EX_NOINPUT);
        }
        return;
    }

    catch (const VExceptionWrongId &exception)
    {
        qCCritical(vMainWindow, "%s\n\n%s\n\n%s", qUtf8Printable(tr("Error wrong id.")),
                               qUtf8Printable(exception.ErrorMessage()), qUtf8Printable(exception.DetailedInformation()));
        setGuiEnabled(false);

        if (!VApplication::IsGUIMode())
        {
            qApp->exit(V_EX_NOINPUT);
        }
        return;
    }

    catch (VException &exception)
    {
        qCCritical(vMainWindow, "%s\n\n%s\n\n%s", qUtf8Printable(tr("Error parsing file.")),
                               qUtf8Printable(exception.ErrorMessage()), qUtf8Printable(exception.DetailedInformation()));
        setGuiEnabled(false);

        if (!VApplication::IsGUIMode())
        {
            qApp->exit(V_EX_NOINPUT);
        }
        return;
    }
    catch (const std::bad_alloc &)
    {
        qCCritical(vMainWindow, "%s", qUtf8Printable(tr("Error parsing file (std::bad_alloc).")));

        setGuiEnabled(false);

        if (!VApplication::IsGUIMode())
        {
            qApp->exit(V_EX_NOINPUT);
        }
        return;
    }

    QString draftBlock;
    if (draftBlockComboBox->currentIndex() != -1)
    {
        draftBlock = draftBlockComboBox->itemText(draftBlockComboBox->currentIndex());
    }
    draftBlockComboBox->blockSignals(true);
    draftBlockComboBox->clear();

    QStringList draftBlockNames = doc->getPatternPieces();
    draftBlockNames.sort();
    draftBlockComboBox->addItems(draftBlockNames);

    if (!drawMode)
    {
        draftBlockComboBox->setCurrentIndex(draftBlockComboBox->count()-1);
    }
    else
    {
        const qint32 index = draftBlockComboBox->findText(draftBlock);
        if (index != -1)
        {
            draftBlockComboBox->setCurrentIndex(index);
        }
    }
    draftBlockComboBox->blockSignals(false);
    ui->patternPreferences_Action->setEnabled(true);

    changeDraftBlockGlobally(draftBlock);

    setToolsEnabled(draftBlockComboBox->count() > 0);
    patternPiecesWidget->updateList();

    VMainGraphicsView::NewSceneRect(draftScene, qApp->getSceneView());
    VMainGraphicsView::NewSceneRect(pieceScene, qApp->getSceneView());
}

//---------------------------------------------------------------------------------------------------------------------
void MainWindow::changeDraftBlockGlobally(const QString &draftBlock)
{
    const qint32 index = draftBlockComboBox->findText(draftBlock);
    try
    {
        if (index != -1)
        { // -1 for not found
            changeDraftBlock(index, false);
            draftBlockComboBox->blockSignals(true);
            draftBlockComboBox->setCurrentIndex(index);
            draftBlockComboBox->blockSignals(false);
        }
        else
        {
            changeDraftBlock(0, false);
        }
    }

    catch (VExceptionBadId &exception)
    {
        qCCritical(vMainWindow, "%s\n\n%s\n\n%s", qUtf8Printable(tr("Bad id.")),
                   qUtf8Printable(exception.ErrorMessage()), qUtf8Printable(exception.DetailedInformation()));
        setGuiEnabled(false);

        if (!VApplication::IsGUIMode())
        {
            qApp->exit(V_EX_NOINPUT);
        }
        return;
    }

    catch (const VExceptionEmptyParameter &exception)
    {
        qCCritical(vMainWindow, "%s\n\n%s\n\n%s", qUtf8Printable(tr("Error empty parameter.")),
                   qUtf8Printable(exception.ErrorMessage()), qUtf8Printable(exception.DetailedInformation()));
        setGuiEnabled(false);

        if (!VApplication::IsGUIMode())
        {
            qApp->exit(V_EX_NOINPUT);
        }
        return;
    }
}

//---------------------------------------------------------------------------------------------------------------------
void MainWindow::setGuiEnabled(bool enabled)
{
    if (guiEnabled != enabled)
    {
        if (enabled == false)
        {
            handleArrowTool(true);
            qApp->getUndoStack()->clear();
        }
        setWidgetsEnabled(enabled);

        guiEnabled = enabled;

        setToolsEnabled(enabled);
        ui->statusBar->setEnabled(enabled);
    #ifndef QT_NO_CURSOR
        QGuiApplication::setOverrideCursor(Qt::ArrowCursor);
    #endif
    }
}

//---------------------------------------------------------------------------------------------------------------------
/**
 * @brief setWidgetsEnabled enable action button.
 * @param enable enable value.
 */
void MainWindow::setWidgetsEnabled(bool enable)
{
    const bool draftStage = (mode == Draw::Calculation);
    const bool pieceStage = (mode == Draw::Modeling);
    const bool designStage = (draftStage || pieceStage);
    const bool layoutStage = (mode == Draw::Layout);

    draftBlockComboBox->setEnabled(enable && draftStage);
    ui->arrow_Action->setEnabled(enable && designStage);

    // enable file menu actions
    ui->save_Action->setEnabled(isWindowModified() && enable && not patternReadOnly);
    ui->saveAs_Action->setEnabled(enable);
    ui->patternPreferences_Action->setEnabled(enable && designStage);

   // enable edit  menu actions
    undoAction->setEnabled(enable && designStage && qApp->getUndoStack()->canUndo());
    redoAction->setEnabled(enable && designStage && qApp->getUndoStack()->canRedo());

    // enable view menu actions
    ui->showDraftMode->setEnabled(enable);
    ui->pieceMode_Action->setEnabled(enable);
    ui->layoutMode_Action->setEnabled(enable);
    zoomScaleSpinBox->setEnabled(enable);
    ui->zoomIn_Action->setEnabled(enable);
    ui->zoomOut_Action->setEnabled(enable);
    ui->zoomToFit_Action->setEnabled(enable);
    ui->zoomToSelected_Action->setEnabled(enable);
    ui->zoom100Percent_Action->setEnabled(enable);
    ui->zoomToPrevious_Action->setEnabled(enable);
    ui->zoomToArea_Action->setEnabled(enable);
    ui->zoomPan_Action->setEnabled(enable);
    ui->zoomToPoint_Action->setEnabled(enable && draftStage);
    m_zoomToPointComboBox->setEnabled(enable && draftStage);

    ui->increaseSize_Action->setEnabled(enable);
    ui->decreaseSize_Action->setEnabled(enable);
    ui->useToolColor_Action->setEnabled(enable && draftStage);
    ui->showPointNames_Action->setEnabled(enable);
    ui->toggleWireframe_Action->setEnabled(enable);
    ui->toggleControlPoints_Action->setEnabled(enable && draftStage);
    ui->toggleAxisOrigin_Action->setEnabled(enable);
    ui->toggleSeamAllowances_Action->setEnabled(enable && pieceStage);
    ui->toggleGrainLines_Action->setEnabled(enable && pieceStage);
    ui->toggleLabels_Action->setEnabled(enable && pieceStage);
    //ui->toggleAnchorPoints_Action->setEnabled(enable && draftStage);

    //enable group actions
    groupsWidget->setAddGroupEnabled(enable && draftStage);

    //enable tool menu actions
    ui->newDraft_Action->setEnabled(enable && draftStage);
    ui->renameDraft_Action->setEnabled(enable && draftStage);

    //enable measurement menu actions
    ui->loadIndividual_Action->setEnabled(enable && designStage);
    ui->loadMultisize_Action->setEnabled(enable && designStage);
    ui->unloadMeasurements_Action->setEnabled(enable && designStage);
    ui->table_Action->setEnabled(enable && draftStage);

    //enable history menu actions
    ui->history_Action->setEnabled(enable && draftStage);

    //enable utilities menu actions
    ui->calculator_Action->setEnabled(enable);
    ui->decimalChart_Action->setEnabled(enable);

    //enable help menu
    ui->shortcuts_Action->setEnabled(enable);

    //enable dock widget actions
    ui->groups_DockWidget->setEnabled(enable && designStage);
    ui->toolProperties_DockWidget->setEnabled(enable && draftStage);
    ui->layoutPages_DockWidget->setEnabled(enable && layoutStage);
    actionDockWidgetToolOptions->setEnabled(enable && designStage);
    actionDockWidgetGroups->setEnabled(enable && designStage);
    actionDockWidgetLayouts->setEnabled(enable && layoutStage);

    //Now we don't want allow user call context menu
    draftScene->setToolsDisabled(!enable, doc->getActiveDraftBlockName());
    ui->view->setEnabled(enable);
}

//---------------------------------------------------------------------------------------------------------------------
void MainWindow::UpdateHeightsList(const QStringList &list)
{
    QString value;
    if (gradationHeights->currentIndex() != -1)
    {
        value = gradationHeights->currentText();
    }

    gradationHeights->blockSignals(true);
    gradationHeights->clear();
    gradationHeights->addItems(list);
    gradationHeights->blockSignals(false);

    int index = gradationHeights->findText(value);
    if (index != -1)
    {
        gradationHeights->setCurrentIndex(index);
    }
    else
    {
        ChangedHeight(0);
    }
}

//---------------------------------------------------------------------------------------------------------------------
void MainWindow::UpdateSizesList(const QStringList &list)
{
    QString value;
    if (gradationSizes->currentIndex() != -1)
    {
        value = gradationSizes->currentText();
    }

    gradationSizes->blockSignals(true);
    gradationSizes->clear();
    gradationSizes->addItems(list);
    gradationSizes->blockSignals(false);

    int index = gradationSizes->findText(value);
    if (index != -1)
    {
        gradationSizes->setCurrentIndex(index);
    }
    else
    {
        ChangedSize(0);
    }
}

//---------------------------------------------------------------------------------------------------------------------
/**
 * @brief NewPattern create new empty pattern.
 */
void MainWindow::New()
{
    if (draftBlockComboBox->count() == 0)
    {
        // Creating a new pattern design requires creating a new draft block
        qCDebug(vMainWindow, "New Draft Block.");
        QString draftBlockName = tr("Draft block %1").arg(draftBlockComboBox->count()+1);
        qCDebug(vMainWindow, "Generated Draft Block name: %s", qUtf8Printable(draftBlockName));

        qCDebug(vMainWindow, "First Draft Block");
        DialogNewPattern newPattern(pattern, draftBlockName, this);
        if (newPattern.exec() == QDialog::Accepted)
        {
            draftBlockName = newPattern.name();
            qApp->setPatternUnit(newPattern.PatternUnit());
            qCDebug(vMainWindow, "Draft Block name: %s", qUtf8Printable(draftBlockName));
        }
        else
        {
            qCDebug(vMainWindow, "Creating new Draft Block was canceled.");
            return;
        }

        //Set scene size to size scene view
        VMainGraphicsView::NewSceneRect(draftScene, ui->view);
        VMainGraphicsView::NewSceneRect(pieceScene, ui->view);

        addDraftBlock(draftBlockName);

        mouseCoordinates = new MouseCoordinates(qApp->patternUnit());
        ui->statusBar->addPermanentWidget((mouseCoordinates));

        m_curFileFormatVersion = VPatternConverter::PatternMaxVer;
        m_curFileFormatVersionStr = VPatternConverter::PatternMaxVerStr;
    }
    else
    {
        OpenNewSeamly2D();
    }
}

//---------------------------------------------------------------------------------------------------------------------
/**
 * @brief haveChange enable action save if we have unsaved change.
 */
void MainWindow::patternChangesWereSaved(bool saved)
{
    if (guiEnabled)
    {
        const bool state = doc->IsModified() || !saved;
        setWindowModified(state);
        not patternReadOnly ? ui->save_Action->setEnabled(state): ui->save_Action->setEnabled(false);
        isLayoutStale = true;
    }
}

//---------------------------------------------------------------------------------------------------------------------
/**
 * @brief ChangedSize change new size value.
 * @param index index of the selected item.
 */
void MainWindow::ChangedSize(int index)
{
    const int size = static_cast<int>(VContainer::size());
    if (updateMeasurements(AbsoluteMPath(qApp->getFilePath(), doc->MPath()),
                           gradationSizes.data()->itemText(index).toInt(),
                           static_cast<int>(VContainer::height())))
    {
        doc->LiteParseTree(Document::LiteParse);
        emit pieceScene->DimensionsChanged();
    }
    else
    {
        qCWarning(vMainWindow, "%s", qUtf8Printable(tr("Couldn't update measurements.")));

        const qint32 index = gradationSizes->findText(QString().setNum(size));
        if (index != -1)
        {
            gradationSizes->setCurrentIndex(index);
        }
        else
        {
            qCWarning(vMainWindow, "Couldn't restore size value.");
        }
    }
}

//---------------------------------------------------------------------------------------------------------------------
/**
 * @brief ChangedGrowth change new height value.
 * @param index index of the selected item.
 */
void MainWindow::ChangedHeight(int index)
{
    const int height = static_cast<int>(VContainer::height());
    if (updateMeasurements(AbsoluteMPath(qApp->getFilePath(), doc->MPath()), static_cast<int>(VContainer::size()),
                           gradationHeights.data()->itemText(index).toInt()))
    {
        doc->LiteParseTree(Document::LiteParse);
        emit pieceScene->DimensionsChanged();
    }
    else
    {
        qCWarning(vMainWindow, "%s", qUtf8Printable(tr("Couldn't update measurements.")));

        const qint32 index = gradationHeights->findText(QString().setNum(height));
        if (index != -1)
        {
            gradationHeights->setCurrentIndex(index);
        }
        else
        {
            qCWarning(vMainWindow, "Couldn't restore height value.");
        }
    }
}

//---------------------------------------------------------------------------------------------------------------------
void MainWindow::SetDefaultHeight()
{
    const QString defHeight = QString().setNum(doc->GetDefCustomHeight());
    int index = gradationHeights->findText(defHeight);
    if (index != -1)
    {
        gradationHeights->setCurrentIndex(index);
    }
    else
    {
        index = gradationHeights->findText(QString().setNum(VContainer::height()));
        if (index != -1)
        {
            gradationHeights->setCurrentIndex(index);
        }
    }
    VContainer::setHeight(gradationHeights->currentText().toInt());
}

//---------------------------------------------------------------------------------------------------------------------
void MainWindow::SetDefaultSize()
{
    const QString defSize = QString().setNum(doc->GetDefCustomSize());
    int index = gradationSizes->findText(defSize);
    if (index != -1)
    {
        gradationSizes->setCurrentIndex(index);
    }
    else
    {
        index = gradationSizes->findText(QString().setNum(VContainer::size()));
        if (index != -1)
        {
            gradationSizes->setCurrentIndex(index);
        }
    }
    VContainer::setSize(gradationSizes->currentText().toInt());
}

//---------------------------------------------------------------------------------------------------------------------
/**
 * @brief setToolsEnabled enable button.
 * @param enable enable value.
 */
void MainWindow::setToolsEnabled(bool enable)
{
    bool draftTools = false;
    bool pieceTools = false;
    bool layoutTools = false;

    switch (mode)
    {
        case Draw::Calculation:
            draftTools = enable;
            break;
        case Draw::Modeling:
            pieceTools = enable;
            break;
        case Draw::Layout:
            layoutTools = enable;
            break;
        default:
            break;
    }

    // This check helps to find missed tools
    Q_STATIC_ASSERT_X(static_cast<int>(Tool::LAST_ONE_DO_NOT_USE) == 53, "Not all tools were handled.");

    //Toolbox Drafting Tools
    //Points
    ui->pointAtDistanceAngle_ToolButton->setEnabled(draftTools);
    ui->alongLine_ToolButton->setEnabled(draftTools);
    ui->normal_ToolButton->setEnabled(draftTools);
    ui->bisector_ToolButton->setEnabled(draftTools);
    ui->shoulderPoint_ToolButton->setEnabled(draftTools);
    ui->pointOfContact_ToolButton->setEnabled(draftTools);
    ui->triangle_ToolButton->setEnabled(draftTools);
    ui->pointIntersectXY_ToolButton->setEnabled(draftTools);
    ui->height_ToolButton->setEnabled(draftTools);
    ui->lineIntersectAxis_ToolButton->setEnabled(draftTools);
    ui->midpoint_ToolButton->setEnabled(draftTools);

    //Lines
    ui->line_ToolButton->setEnabled(draftTools);
    ui->lineIntersect_ToolButton->setEnabled(draftTools);

    //Curves
    ui->curve_ToolButton->setEnabled(draftTools);
    ui->spline_ToolButton->setEnabled(draftTools);
    ui->curveWithCPs_ToolButton->setEnabled(draftTools);
    ui->splineWithCPs_ToolButton->setEnabled(draftTools);
    ui->pointAlongCurve_ToolButton->setEnabled(draftTools);
    ui->pointAlongSpline_ToolButton->setEnabled(draftTools);
    ui->pointOfIntersectionCurves_ToolButton->setEnabled(draftTools);
    ui->curveIntersectAxis_ToolButton->setEnabled(draftTools);

    //Arcs
    ui->arc_ToolButton->setEnabled(draftTools);
    ui->pointAlongArc_ToolButton->setEnabled(draftTools);
    ui->arcIntersectAxis_ToolButton->setEnabled(draftTools);
    ui->pointOfIntersectionArcs_ToolButton->setEnabled(draftTools);
    ui->pointOfIntersectionCircles_ToolButton->setEnabled(draftTools);
    ui->pointFromCircleAndTangent_ToolButton->setEnabled(draftTools);
    ui->pointFromArcAndTangent_ToolButton->setEnabled(draftTools);
    ui->arcWithLength_ToolButton->setEnabled(draftTools);
    ui->ellipticalArc_ToolButton->setEnabled(draftTools);

    //Operations
    ui->group_ToolButton->setEnabled(draftTools);
    ui->rotation_ToolButton->setEnabled(draftTools);
    ui->mirrorByLine_ToolButton->setEnabled(draftTools);
    ui->mirrorByAxis_ToolButton->setEnabled(draftTools);
    ui->move_ToolButton->setEnabled(draftTools);
    ui->trueDarts_ToolButton->setEnabled(draftTools);
    ui->exportDraftBlocks_ToolButton->setEnabled(draftTools);

    //Piece
    ui->addPatternPiece_ToolButton->setEnabled(draftTools);
    ui->anchorPoint_ToolButton->setEnabled(draftTools  & (pattern->DataPieces()->size() > 0));
    ui->internalPath_ToolButton->setEnabled(draftTools & (pattern->DataPieces()->size() > 0));
    ui->insertNodes_ToolButton->setEnabled(draftTools   & (pattern->DataPieces()->size() > 0));

    //Details
    ui->unitePieces_ToolButton->setEnabled(pieceTools);
    ui->exportPiecesAs_ToolButton->setEnabled(pieceTools);

    //Layout
    ui->layoutSettings_ToolButton->setEnabled(layoutTools);

    //enable Toolbox Toolbar actions
    ui->arrow_Action->setEnabled(enable);
    ui->points_Action->setEnabled(draftTools);
    ui->lines_Action->setEnabled(draftTools);
    ui->arcs_Action->setEnabled(draftTools);
    ui->curves_Action->setEnabled(draftTools);
    ui->modifications_Action->setEnabled(draftTools);
    ui->pieces_Action->setEnabled(draftTools);
    ui->details_Action->setEnabled(pieceTools);
    ui->layout_Action->setEnabled(layoutTools);

    //Menu Actions
    //Points
    ui->midpoint_Action->setEnabled(draftTools);
    ui->pointAtDistanceAngle_Action->setEnabled(draftTools);
    ui->pointAlongLine_Action->setEnabled(draftTools);
    ui->pointAlongPerpendicular_Action->setEnabled(draftTools);
    ui->bisector_Action->setEnabled(draftTools);
    ui->pointOnShoulder_Action->setEnabled(draftTools);
    ui->pointOfContact_Action->setEnabled(draftTools);
    ui->triangle_Action->setEnabled(draftTools);
    ui->pointIntersectXY_Action->setEnabled(draftTools);
    ui->perpendicularPoint_Action->setEnabled(draftTools);
    ui->pointIntersectAxis_Action->setEnabled(draftTools);

    //Lines
    ui->lineTool_Action->setEnabled(draftTools);
    ui->lineIntersect_Action->setEnabled(draftTools);

    //Curves
    ui->curve_Action->setEnabled(draftTools);
    ui->spline_Action->setEnabled(draftTools);
    ui->curveWithCPs_Action->setEnabled(draftTools);
    ui->splineWithCPs_Action->setEnabled(draftTools);
    ui->pointAlongCurve_Action->setEnabled(draftTools);
    ui->pointAlongSpline_Action->setEnabled(draftTools);
    ui->curveIntersectCurve_Action->setEnabled(draftTools);
    ui->splineIntersectAxis_Action->setEnabled(draftTools);

    //Arcs
    ui->arcTool_Action->setEnabled(draftTools);
    ui->pointAlongArc_Action->setEnabled(draftTools);
    ui->arcIntersectAxis_Action->setEnabled(draftTools);
    ui->arcIntersectArc_Action->setEnabled(draftTools);
    ui->circleIntersect_Action->setEnabled(draftTools);
    ui->circleTangent_Action->setEnabled(draftTools);
    ui->arcTangent_Action->setEnabled(draftTools);;
    ui->arcWithLength_Action->setEnabled(draftTools);
    ui->ellipticalArc_Action->setEnabled(draftTools);

    //Operations
    ui->group_Action->setEnabled(draftTools);
    ui->rotation_Action->setEnabled(draftTools);
    ui->mirrorByLine_Action->setEnabled(draftTools);
    ui->mirrorByAxis_Action->setEnabled(draftTools);
    ui->move_Action->setEnabled(draftTools);
    ui->trueDarts_Action->setEnabled(draftTools);
    ui->exportDraftBlocks_Action->setEnabled(draftTools);

    //Piece
    ui->addPiece_Action->setEnabled(draftTools);
    ui->anchorPoint_Action->setEnabled(draftTools & (pattern->DataPieces()->size() > 0));
    ui->internalPath_Action->setEnabled(draftTools & (pattern->DataPieces()->size() > 0));
    ui->insertNodes_Action->setEnabled(draftTools & (pattern->DataPieces()->size() > 0));

    //Details
    ui->union_Action->setEnabled(pieceTools);
    ui->exportPieces_Action->setEnabled(pieceTools);

    //Layout
    ui->newPrintLayout_Action->setEnabled(layoutTools);
    ui->exportLayout_Action->setEnabled(layoutTools);
    ui->lastTool_Action->setEnabled(draftTools);

    ui->arrowPointer_ToolButton->setEnabled(draftTools || pieceTools);
    ui->arrowPointer_ToolButton->setChecked(draftTools || pieceTools);
    ui->arrow_Action->setChecked(draftTools || pieceTools);
}

//---------------------------------------------------------------------------------------------------------------------
void MainWindow::SetLayoutModeActions()
{
    const bool enabled = not scenes.isEmpty();

    ui->exportLayout_ToolButton->setEnabled(enabled);
    ui->exportAs_Action->setEnabled(enabled);
    ui->printPreview_Action->setEnabled(enabled);
    ui->printPreviewTiled_Action->setEnabled(enabled);
    ui->print_Action->setEnabled(enabled);
    ui->printTiled_Action->setEnabled(enabled);
}

//---------------------------------------------------------------------------------------------------------------------
/**
 * @brief MinimumScrollBar set scroll bar to minimum.
 */
void MainWindow::MinimumScrollBar()
{
    QScrollBar *horScrollBar = ui->view->horizontalScrollBar();
    horScrollBar->setValue(horScrollBar->minimum());
    QScrollBar *verScrollBar = ui->view->verticalScrollBar();
    verScrollBar->setValue(verScrollBar->minimum());
}

//---------------------------------------------------------------------------------------------------------------------
/**
 * @brief SavePattern save pattern file.
 * @param fileName pattern file name.
 * @return true if all is good.
 */
bool MainWindow::SavePattern(const QString &fileName, QString &error)
{
    qCDebug(vMainWindow, "Saving pattern file %s.", qUtf8Printable(fileName));
    QFileInfo tempInfo(fileName);

    const QString filename = AbsoluteMPath(qApp->getFilePath(), doc->MPath());
    if (!filename.isEmpty() && qApp->getFilePath() != fileName)
    {
        doc->SetMPath(RelativeMPath(fileName, filename));
    }

    const bool result = doc->SaveDocument(fileName, error);
    if (result)
    {
        if (tempInfo.suffix() != QLatin1String("autosave"))
        {
            setCurrentFile(fileName);
            helpLabel->setText(tr("File saved"));
            qCDebug(vMainWindow, "File %s saved.", qUtf8Printable(fileName));
            patternChangesWereSaved(result);
        }
    }
    else
    {
        doc->SetMPath(filename);
        emit doc->UpdatePatternLabel();
        qCWarning(vMainWindow, "Could not save file %s. %s.", qUtf8Printable(fileName), qUtf8Printable(error));
    }
    return result;
}

//---------------------------------------------------------------------------------------------------------------------
/**
 * @brief AutoSavePattern start safe saving.
 */
void MainWindow::AutoSavePattern()
{
    if (patternReadOnly)
    {
        return;
    }
    qCDebug(vMainWindow, "Autosaving pattern.");

    if (qApp->getFilePath().isEmpty() == false && this->isWindowModified() == true)
    {
        QString autofile = qApp->getFilePath() + autosavePrefix;
        QString error;
        SavePattern(autofile, error);
    }
}

//---------------------------------------------------------------------------------------------------------------------
/**
 * @brief Sets the current file for the application.
 * 
 * @param fileName The name of the file to set as the current file.
 * 
 * @details This function sets the current file for the application, updates various internal states,
 * and emits signals for further updates. It sets the current file path in the application, marks the
 * pattern as changed, emits a signal to update the pattern label, and sets the undo stack as clean.
 * If the file is not empty and the application is in GUI mode, it also updates the recent file list
 * and restore file list in application settings. Finally, it updates the main window's title.
 */
void MainWindow::setCurrentFile(const QString &fileName)
{
    qCDebug(vMainWindow, "Set current name to \"%s\"", qUtf8Printable(fileName));

    // Set the current file path in the application.
    qApp->setFilePath(fileName);

    // Mark the pattern as changed and emit a signal to update the pattern label.
    doc->SetPatternWasChanged(true);
    emit doc->UpdatePatternLabel();

    // Set the undo stack as clean.
    qApp->getUndoStack()->setClean();

    // Update recent file list and restore file list if the file is not empty and in GUI mode.
    if (!qApp->getFilePath().isEmpty() && VApplication::IsGUIMode())
    {
        qCDebug(vMainWindow, "Updating recent file list.");
        VSettings *settings = qApp->Seamly2DSettings();
        QStringList files = settings->GetRecentFileList();
        files.removeAll(fileName);
        files.prepend(fileName);
        while (files.size() > MaxRecentFiles)
        {
            files.removeLast();
        }

        settings->SetRecentFileList(files);

        // Update actions for recent files.
        UpdateRecentFileActions();

        qCDebug(vMainWindow, "Updating restore file list.");
        QStringList restoreFiles = settings->GetRestoreFileList();
        restoreFiles.removeAll(fileName);
        restoreFiles.prepend(fileName);
        settings->SetRestoreFileList(restoreFiles);
    }

    // Update the main window's title to reflect the current file.
    UpdateWindowTitle();
}
// End MainWindow::setCurrentFile()

//---------------------------------------------------------------------------------------------------------------------
/**
 * @brief Reads and applies application settings from persistent storage.
 * 
 * @details This function reads various application settings from persistent storage and applies
 * them to the main window. It restores the main window's geometry, window state, and toolbar state.
 * It also handles settings related to scene antialiasing, stack limit for undo/redo operations,
 * and the display style of tool button icons with text.
 */
void MainWindow::ReadSettings()
{
    qCDebug(vMainWindow, "Reading settings.");

    // Get a pointer to the application settings.
    const VSettings *settings = qApp->Seamly2DSettings();

    // Restore the main window's geometry from settings.
    restoreGeometry(settings->GetGeometry());

    // Restore the main window's window state from settings.
    restoreState(settings->GetWindowState());

    // Restore the toolbar state from settings, specifying the application version.
    restoreState(settings->GetToolbarsState(), APP_VERSION);

    // Handle scene antialiasing settings.
    const bool graphOutputValue = settings->GetGraphicalOutput();
    ui->view->setRenderHint(QPainter::Antialiasing, graphOutputValue);
    ui->view->setRenderHint(QPainter::SmoothPixmapTransform, graphOutputValue);

    // Set the stack limit for undo/redo operations.
    qApp->getUndoStack()->setUndoLimit(settings->GetUndoCount());

    // Apply text under tool button icon style settings.
    ToolBarStyles();

    // Update the visibility status of various dock widgets.
    isToolOptionsDockVisible = ui->toolProperties_DockWidget->isVisible();
    isGroupsDockVisible      = ui->groups_DockWidget->isVisible();
    isLayoutsDockVisible     = ui->layoutPages_DockWidget->isVisible();
    isToolboxDockVisible     = ui->toolbox_DockWidget->isVisible();
}
// End MainWindow::ReadSettings()

//---------------------------------------------------------------------------------------------------------------------
/**
 * @brief Writes the application settings to persistent storage.
 * 
 * @details This function is responsible for saving various application settings to
 * persistent storage. It saves the main window's geometry, window state, and toolbar state.
 * Additionally, it ensures that the draft mode is shown before saving the settings.
 */
void MainWindow::WriteSettings()
{
    // Show the draft mode before saving settings.
    showDraftMode(true);

    // Get a pointer to the application settings.
    VSettings *settings = qApp->Seamly2DSettings();

    // Save the main window's geometry to settings.
    settings->SetGeometry(saveGeometry());

    // Save the main window's window state to settings.
    settings->SetWindowState(saveState());

    // Save the toolbar state to settings, specifying the application version.
    settings->SetToolbarsState(saveState(APP_VERSION));
}
// End MainWindow::WriteSettings()

//---------------------------------------------------------------------------------------------------------------------
/**
 * @brief Checks if the current pattern has unsaved changes and prompts the user to save.
 * 
 * @details This function checks if the current pattern has unsaved changes and displays
 * a message box to prompt the user to save those changes. The user can choose to save,
 * not save, or cancel the operation. It also handles different button texts and window
 * properties based on the current state of the application.
 * 
 * @return true if the user chose to save or if there were no unsaved changes, false if the user canceled.
 */
bool MainWindow::MaybeSave()
{
    // Check if the window is modified (unsaved changes) and if GUI is enabled.
    if (this->isWindowModified() && guiEnabled)
    {
        // Create a message box to prompt the user.
        QScopedPointer<QMessageBox> messageBox(new QMessageBox(tr("Unsaved changes"),
                                                               tr("The pattern has been modified.\n"
                                                                  "Do you want to save your changes?"),
                                                               QMessageBox::Warning, QMessageBox::Yes, QMessageBox::No,
                                                               QMessageBox::Cancel, this, Qt::Sheet));

        // Set default and escape buttons, and customize button text.
        messageBox->setDefaultButton(QMessageBox::Yes);
        messageBox->setEscapeButton(QMessageBox::Cancel);
        messageBox->setButtonText(QMessageBox::Yes,
                                  qApp->getFilePath().isEmpty() || patternReadOnly ? tr("Save...") : tr("Save"));
        messageBox->setButtonText(QMessageBox::No, tr("Don't Save"));

        // Configure window modality and flags.
        messageBox->setWindowModality(Qt::ApplicationModal);
        messageBox->setWindowFlags(windowFlags() & ~Qt::WindowContextHelpButtonHint
                                                 & ~Qt::WindowMaximizeButtonHint
                                                 & ~Qt::WindowMinimizeButtonHint);

        // Execute the message box and capture the user's response.
        const auto ret = static_cast<QMessageBox::StandardButton>(messageBox->exec());

        // Handle the user's choice.
        switch (ret)
        {
            case QMessageBox::Yes:
                if (patternReadOnly)
                {
                    return SaveAs();
                }
                else
                {
                    return Save();
                }
            case QMessageBox::No:
                return true;
            case QMessageBox::Cancel:
                return false;
            default:
                break;
        }
    }

    // If there are no unsaved changes, return true.
    return true;
}
// End MainWindow::MaybeSave()

//---------------------------------------------------------------------------------------------------------------------
/**
 * @brief Updates the list of recent files in the File menu.
 *
 * This function retrieves the list of recent files from application settings and updates
 * the recent file actions displayed in the File menu. It ensures that the most recent files
 * are listed and sets their visibility accordingly.
 */
void MainWindow::UpdateRecentFileActions()
{
    qCDebug(vMainWindow, "Updating recent file actions.");

    // Retrieve the list of recent files from application settings.
    const QStringList files = qApp->Seamly2DSettings()->GetRecentFileList();

    // Determine the number of recent files to display, limited by MaxRecentFiles constant.
    const int numRecentFiles = qMin(files.size(), static_cast<int>(MaxRecentFiles));

    // Iterate over the recent file actions and update their text, data, and visibility.
    for (int i = 0; i < numRecentFiles; ++i)
    {
        QString text = QString("&%1. %2").arg(i + 1).arg(strippedName(files.at(i)));
        recentFileActs[i]->setText(text);
        recentFileActs[i]->setData(files.at(i));
        recentFileActs[i]->setVisible(true);
    }

    // Hide any remaining recent file actions beyond the specified limit.
    for (int j = numRecentFiles; j < MaxRecentFiles; ++j)
    {
        recentFileActs[j]->setVisible(false);
    }

    // Set the visibility of the separator action based on whether there are recent files.
    separatorAct->setVisible(numRecentFiles > 0);
}
// End MainWindow::UpdateRecentFileActions()

//---------------------------------------------------------------------------------------------------------------------
/**
 * @brief Creates menus for the main application window.
 *
 * This function initializes and configures various menus in the main application window, including
 * the file menu, edit menu, view menu, toolbars menu, and recent files submenu. It also adds Undo and
 * Redo actions to the edit menu and connects toolbar visibility actions to their respective toolbars.
 */
void MainWindow::CreateMenus()
{
    // Add last 5 most recent projects to the file menu.
    for (int i = 0; i < MaxRecentFiles; ++i)
    {
        ui->file_Menu->insertAction(ui->exit_Action, recentFileActs[i]);
    }

    separatorAct = new QAction(this);
    separatorAct->setSeparator(true);
    ui->file_Menu->insertAction(ui->exit_Action, separatorAct);

    // Update the list of recent files in the menu.
    UpdateRecentFileActions();

    // Add Undo and Redo actions to the edit menu.
    QList<QKeySequence> undoShortcuts;
    undoShortcuts.append(QKeySequence(Qt::ControlModifier + Qt::Key_Z));
    undoShortcuts.append(QKeySequence(Qt::AltModifier + Qt::Key_Backspace));

    undoAction = qApp->getUndoStack()->createUndoAction(this, tr("&Undo"));
    undoAction->setShortcuts(undoShortcuts);
    undoAction->setIcon(QIcon::fromTheme("edit-undo"));
    connect(undoAction, &QAction::triggered, toolProperties, &VToolOptionsPropertyBrowser::refreshOptions);
    ui->edit_Menu->addAction(undoAction);
    ui->edit_Toolbar->addAction(undoAction);

    QList<QKeySequence> redoShortcuts;
    redoShortcuts.append(QKeySequence(Qt::ControlModifier + Qt::Key_Y));
    redoShortcuts.append(QKeySequence(Qt::ControlModifier + Qt::ShiftModifier + Qt. Key_Z));
    redoShortcuts.append(QKeySequence(Qt::AltModifier + Qt::ShiftModifier + Qt::Key_Backspace));

    redoAction = qApp->getUndoStack()->createRedoAction(this, tr("&Redo"));
    redoAction->setShortcuts(redoShortcuts);
    redoAction->setIcon(QIcon::fromTheme("edit-redo"));
    connect(redoAction, &QAction::triggered, toolProperties, &VToolOptionsPropertyBrowser::refreshOptions);
    ui->edit_Menu->addAction(redoAction);
    ui->edit_Toolbar->addAction(redoAction);

    separatorAct = new QAction(this);
    separatorAct->setSeparator(true);
    ui->edit_Menu->addAction(separatorAct);

    // Add dock widgets.
    AddDocks();

    /**
     * This section adds toolbar visibility actions and connects them to their respective toolbars.
     * Creates a submenu in the "View" menu for managing the visibility of various toolbars.
     * It adds actions to show or hide each toolbar and connects them to their corresponding visibility slots.
     */

    separatorAct = new QAction(this);
    separatorAct->setSeparator(true);
    ui->view_Menu->addAction(separatorAct);

    // Create a submenu titled "Toolbars" in the "View" menu.
    QMenu *menu = new QMenu(tr("Toolbars"));
    ui->view_Menu->addMenu(menu);

    /**
     * This section manages the visibility of various toolbars and adds actions to toggle their visibility. 
     * Each action is connected to the corresponding toolbar's visibility slot, 
     * ensuring that the toolbars can be shown or hidden as desired by the user.
     */

    // Add an action to show/hide the "File" toolbar and connect it to its visibility slot.
    menu->addAction(ui->file_ToolBar->toggleViewAction());
    connect(ui->file_ToolBar, &QToolBar::visibilityChanged, this, [this](bool visible)
    {
        ui->file_ToolBar->setVisible(visible);
    });

    // Add similar actions and connections for other toolbars such as "Edit," "View," "Mode," etc.
    // Edit
    menu->addAction(ui->edit_Toolbar->toggleViewAction());
    connect(ui->edit_Toolbar, &QToolBar::visibilityChanged, this, [this](bool visible)
    {
        ui->edit_Toolbar->setVisible(visible);
    });
    // View
    menu->addAction(ui->view_ToolBar->toggleViewAction());
    connect(ui->view_ToolBar, &QToolBar::visibilityChanged, this, [this](bool visible)
    {
        ui->view_ToolBar->setVisible(visible);
    });
    // Mode
    menu->addAction(ui->mode_ToolBar->toggleViewAction());
    connect(ui->mode_ToolBar, &QToolBar::visibilityChanged, this, [this](bool visible)
    {
        ui->mode_ToolBar->setVisible(visible);
    });
    // Draft
    menu->addAction(ui->draft_ToolBar->toggleViewAction());
    connect(ui->draft_ToolBar, &QToolBar::visibilityChanged, this, [this](bool visible)
    {
        ui->draft_ToolBar->setVisible(visible);
    });
    // Zoom
    menu->addAction(ui->zoom_ToolBar->toggleViewAction());
    connect(ui->zoom_ToolBar, &QToolBar::visibilityChanged, this, [this](bool visible)
    {
        ui->zoom_ToolBar->setVisible(visible);
    });
    // Toolbox & Draft mode tools
    menu->addAction(ui->tools_ToolBox_ToolBar->toggleViewAction());
    menu->addAction(ui->points_ToolBar->toggleViewAction());
    menu->addAction(ui->lines_ToolBar->toggleViewAction());
    menu->addAction(ui->curves_ToolBar->toggleViewAction());
    menu->addAction(ui->arcs_ToolBar->toggleViewAction());
    menu->addAction(ui->operations_ToolBar->toggleViewAction());
    menu->addAction(ui->pieces_ToolBar->toggleViewAction());
    menu->addAction(ui->details_ToolBar->toggleViewAction());
    menu->addAction(ui->layout_ToolBar->toggleViewAction());
    // Pointname
    menu->addAction(ui->pointName_ToolBar->toggleViewAction());
}
// End MainWindow::CreateMenus()

//---------------------------------------------------------------------------------------------------------------------
/** Preprocessor directive to silence certain compiler warnings for specific blocks of code 
 * without disabling them globally for the entire project.
 */
QT_WARNING_PUSH
QT_WARNING_DISABLE_GCC("-Wswitch-default")

/**
 * @brief Restore the last used tool's state and settings.
 * 
 * This function is responsible for restoring the state and settings of the last used tool.
 * It sets the appropriate UI elements and calls the corresponding handler function for the tool.
 * 
 * @note The function uses a switch statement to handle various tools based on the last used tool.
 *       It also ensures that all tools are properly handled, and it may include some unreachable code
 *       for tools that cannot be created from the main window.
 */
void MainWindow::LastUsedTool()
{
    // This check helps to find missed tools in the switch
    Q_STATIC_ASSERT_X(static_cast<int>(Tool::LAST_ONE_DO_NOT_USE) == 53, "Not all tools were handled.");

    if (currentTool == lastUsedTool)
    {
        return;
    }

    switch (lastUsedTool)
    {
        // Handle each tool by setting UI elements and calling the appropriate handler function
        case Tool::Arrow:
            ui->arrowPointer_ToolButton->setChecked(true);
            ui->arrow_Action->setChecked(true);
            handleArrowTool(true);
            break;
        // Handle various tools (unreachable code as they can't be created from the main window)
        case Tool::BasePoint:
        case Tool::SinglePoint:
        case Tool::DoublePoint:
        case Tool::LinePoint:
        case Tool::AbstractSpline:
        case Tool::Cut:
        case Tool::LAST_ONE_DO_NOT_USE:
        case Tool::NodePoint:
        case Tool::NodeArc:
        case Tool::NodeElArc:
        case Tool::NodeSpline:
        case Tool::NodeSplinePath:
            Q_UNREACHABLE(); //-V501
            // Nothing to do here because we can't create this tool from the main window.
            break;
        // Handle other tools by setting UI elements and calling their handler functions
        // (e.g., Line, Midpoint, Spline, Arc, etc.)
        case Tool::EndLine:
            ui->pointAtDistanceAngle_ToolButton->setChecked(true);
            handlePointAtDistanceAngleTool(true);
            break;
        case Tool::Line:
            ui->line_ToolButton->setChecked(true);
            handleLineTool(true);
            break;
        // Handle more tools...
        // (you can add comments for each tool here)
    }
}
// End MainWindow::LastUsedTool()
// Preprocessor directive to stop silencing certain compiler warnings
QT_WARNING_POP

//---------------------------------------------------------------------------------------------------------------------
/**
 * @brief Add and configure dock widgets in the main window.
 * 
 * @details This function adds dock widgets to the main window, configures their visibility actions,
 * and sets up connections to track the visibility state of each dock widget.
 * Additionally, it manages the tabbing and splitting of dock widgets.
 *
 * @note The function adds, configures, and manages various dock widgets in the main window,
 *       including the tool options, groups, layouts, and toolbox dock widgets.
 *       It also sets up connections to track the visibility changes of these widgets.
 */
void MainWindow::AddDocks()
{
    // Add tool options dock widget
    actionDockWidgetToolOptions = ui->toolProperties_DockWidget->toggleViewAction();
    ui->view_Menu->addAction(actionDockWidgetToolOptions);
    connect(ui->toolProperties_DockWidget, &QDockWidget::visibilityChanged, this, [this](bool visible)
    {
        isToolOptionsDockVisible = visible;
    });

    // Add groups dock widget
    actionDockWidgetGroups = ui->groups_DockWidget->toggleViewAction();
    ui->view_Menu->addAction(actionDockWidgetGroups);
    connect(ui->groups_DockWidget, &QDockWidget::visibilityChanged, this, [this](bool visible)
    {
        isGroupsDockVisible = visible;
    });

    // Add layouts dock widget
    actionDockWidgetLayouts = ui->layoutPages_DockWidget->toggleViewAction();
    ui->view_Menu->addAction(actionDockWidgetLayouts);
    connect(ui->layoutPages_DockWidget, &QDockWidget::visibilityChanged, this, [this](bool visible)
    {
        isLayoutsDockVisible = visible;
    });

    // Add toolbox dock widget
    actionDockWidgetToolbox = ui->toolbox_DockWidget->toggleViewAction();
    ui->view_Menu->addAction(actionDockWidgetToolbox);
    connect(ui->toolbox_DockWidget, &QDockWidget::visibilityChanged, this, [this](bool visible)
    {
        isToolboxDockVisible  = visible;
    });

    // Tabify and split dock widgets for organization
    tabifyDockWidget(ui->groups_DockWidget, ui->toolProperties_DockWidget);
    splitDockWidget(ui->toolProperties_DockWidget, ui->layoutPages_DockWidget, Qt::Vertical);
}
// End MainWindow::AddDocks()

//---------------------------------------------------------------------------------------------------------------------
/**
 * @brief Initialize the dock containers and widgets.
 * 
 * @details This function initializes the docking containers and various widgets used in the main window.
 * It sets the tab positions for the dock widgets, creates and connects widgets like the property editor,
 * groups manager, and pattern pieces widget.
 *
 * @note The function configures the tab positions for the dock containers on the right and left sides of the window.
 *       It also initializes the property editor and creates widgets for managing groups and pattern pieces.
 */
void MainWindow::InitDocksContain()
{
    // Configure tab positions for dock containers
    setTabPosition(Qt::RightDockWidgetArea, QTabWidget::West);
    setTabPosition(Qt::LeftDockWidgetArea, QTabWidget::East);

    // Initialize the property editor
    initPropertyEditor();

    // Initialize the Groups manager
    qCDebug(vMainWindow, "Initialize Groups manager.");
    groupsWidget = new GroupsWidget(pattern, doc, this);
    ui->groups_DockWidget->setWidget(groupsWidget);
    connect(doc, &VAbstractPattern::updateGroups, this, &MainWindow::updateGroups);

    // Initialize the Pattern Pieces widget
    patternPiecesWidget = new PiecesWidget(pattern, doc, this);
    connect(doc, &VPattern::FullUpdateFromFile, patternPiecesWidget, &PiecesWidget::updateList);
    connect(doc, &VPattern::UpdateInLayoutList, patternPiecesWidget, &PiecesWidget::togglePiece);
    connect(doc, &VPattern::showPiece, patternPiecesWidget, &PiecesWidget::selectPiece);
    connect(patternPiecesWidget, &PiecesWidget::Highlight, pieceScene, &VMainGraphicsScene::HighlightItem);
    patternPiecesWidget->setVisible(false);

    // Set the current index of the toolbox stacked widget
    ui->toolbox_StackedWidget->setCurrentIndex(0);
}

// End MainWindow::InitDocksContain()

//---------------------------------------------------------------------------------------------------------------------
/**
 * @brief Opens a new Seamly2D document.
 * 
 * @details This function is used to open a new Seamly2D document from a file.
 *
 * @param fileName The file name of the Seamly2D document to be opened.
 * @return true if the document is opened successfully.
 * @return false if the document is not opened.
 *
 * @note If the current document is modified or there is already a loaded file,
 *       it creates a new Seamly2D instance with the provided file name and returns true.
 *       Otherwise, it returns false.
 */
bool MainWindow::OpenNewSeamly2D(const QString &fileName) const
{
    if (this->isWindowModified() || qApp->getFilePath().isEmpty() == false)
    {
        // Create a new Seamly2D instance with the provided file name
        VApplication::NewSeamly2D(fileName);
        return true;
    }
    return false;
}
// End MainWindow::OpenNewSeamly2D()

//---------------------------------------------------------------------------------------------------------------------
/**
 * @brief Connects various actions in the menus, preferences, printing, tools, and other actions to corresponding tool handlers, slots, or functions
 * 
 */
void MainWindow::CreateActions()
{
    // Set up the main user interface
    ui->setupUi(this);
    // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
    // Connect actions in the "Files" menu to corresponding slots
    connect(ui->actionNew, &QAction::triggered, this, &MainWindow::New);
    connect(ui->actionOpen, &QAction::triggered, this, &MainWindow::Open);
    connect(ui->save_Action, &QAction::triggered, this, &MainWindow::Save);
    connect(ui->saveAs_Action, &QAction::triggered, this, &MainWindow::SaveAs);
    // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
    // Connect the "Close Pattern" action to a lambda function
    // that checks if the current pattern needs saving before closing it
    connect(ui->closePattern_Action, &QAction::triggered, this, [this]()
    {
        if (MaybeSave())
        {
            FileClosedCorrect();
            Clear();
        }
    });
    // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
    // Connect various actions related to printing and exporting
    connect(ui->exportAs_Action, &QAction::triggered, this, &MainWindow::exportLayoutAs);
    connect(ui->printPreview_Action, &QAction::triggered, this, &MainWindow::PrintPreviewOrigin);
    connect(ui->printPreviewTiled_Action, &QAction::triggered, this, &MainWindow::PrintPreviewTiled);
    connect(ui->print_Action, &QAction::triggered, this, &MainWindow::PrintOrigin);
    connect(ui->printTiled_Action, &QAction::triggered, this, &MainWindow::PrintTiled);
    // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
    // Connect the "App Preferences" and "Pattern Preferences" actions to their respective slots
    connect(ui->appPreferences_Action, &QAction::triggered, this, &MainWindow::Preferences);
    connect(ui->patternPreferences_Action, &QAction::triggered, this, [this]()
    {
        // Open the pattern properties dialog and connect signals to update lists
        DialogPatternProperties proper(doc, pattern, this);
        connect(&proper, &DialogPatternProperties::UpdateGradation, this, [this]()
        {
            UpdateHeightsList(MeasurementVariable::ListHeights(doc->GetGradationHeights(), qApp->patternUnit()));
            UpdateSizesList(MeasurementVariable::ListSizes(doc->GetGradationSizes(), qApp->patternUnit()));
        });
        proper.exec();
    });
    // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
    // Disable the "Pattern Preferences" action initially
    ui->patternPreferences_Action->setEnabled(false);
    // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
    // Create actions for recent files loaded by the main window application
    for (int i = 0; i < MaxRecentFiles; ++i)
    {
        QAction *action = new QAction(this);
        action->setVisible(false);
        recentFileActs[i] = action;
    // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
        // Connect each recent file action to a slot that loads the selected pattern
        connect(recentFileActs[i], &QAction::triggered, this, [this]()
        {
            if (QAction *action = qobject_cast<QAction*>(sender()))
            {
                const QString filePath = action->data().toString();
                if (!filePath.isEmpty())
                {
                    LoadPattern(filePath);
                }
            }
        });
    }
    // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
    // Connect the "Document Info" action to show a dialog displaying document information
    connect(ui->documentInfo_Action, &QAction::triggered, this, [this]()
    {
        // Create and display the document information dialog
        ShowInfoDialog *infoDialog = new ShowInfoDialog(doc, this);
        infoDialog->setAttribute(Qt::WA_DeleteOnClose, true);
        infoDialog->adjustSize();
        infoDialog->show();
    });
    // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
    // Connect the "Exit" action to close the main window
    connect(ui->exit_Action, &QAction::triggered, this, &MainWindow::close);

    // Edit Menu
    // Connect the "Label Template Editor" action to open and execute the label template editor dialog
    connect(ui->labelTemplateEditor_Action, &QAction::triggered, this, [this]()
    {
        EditLabelTemplateDialog editor(doc);
        editor.exec();
    });
    // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
    // View Menu
    // Connect various actions to toggle different view options
    connect(ui->showDraftMode, &QAction::triggered, this, &MainWindow::showDraftMode);
    connect(ui->pieceMode_Action, &QAction::triggered, this, &MainWindow::showPieceMode);
    connect(ui->layoutMode_Action, &QAction::triggered, this, &MainWindow::showLayoutMode);
    connect(ui->toggleWireframe_Action, &QAction::triggered, this, [this](bool checked)
    {
        // Toggle wireframe view and update scenes
        qApp->Seamly2DSettings()->setWireframe(checked);
        emit ui->view->itemClicked(nullptr);
        upDateScenes();
    });
    connect(ui->toggleControlPoints_Action, &QAction::triggered, this, [this](bool checked)
    {
        // Toggle control points view and update scenes
        qApp->Seamly2DSettings()->setShowControlPoints(checked);
        emit ui->view->itemClicked(nullptr);
        draftScene->enablePiecesMode(checked);
    });
    connect(ui->toggleAxisOrigin_Action, &QAction::triggered, this, [this](bool checked)
    {
        // Toggle axis origin view for draft and piece scenes
        qApp->Seamly2DSettings()->setShowAxisOrigin(checked);
        draftScene->setOriginsVisible(checked);
        pieceScene->setOriginsVisible(checked);
    });
    connect(ui->toggleSeamAllowances_Action, &QAction::triggered, this, [this](bool checked)
    {
        // Toggle seam allowances view and update scenes
        qApp->Seamly2DSettings()->setShowSeamAllowances(checked);
        emit ui->view->itemClicked(nullptr);
        refreshSeamAllowances();
    });
    connect(ui->toggleGrainLines_Action, &QAction::triggered, this, [this](bool checked)
    {
        // Toggle grain lines view and update scenes
        qApp->Seamly2DSettings()->setShowGrainlines(checked);
        emit ui->view->itemClicked(nullptr);
        refreshGrainLines();
    });
    connect(ui->toggleLabels_Action, &QAction::triggered, this, [this](bool checked)
    {
        // Toggle labels view and update scenes
        qApp->Seamly2DSettings()->setShowLabels(checked);
        emit ui->view->itemClicked(nullptr);
        refreshLabels();
    });
    // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
    // Connect actions for changing the font size of point names
    connect(ui->increaseSize_Action, &QAction::triggered, this, [this]()
    {
        // Increase font size for point names and update scenes
        int index = qMin(fontSizeComboBox->currentIndex() + 1, fontSizeComboBox->count() - 1);
        fontSizeComboBox->setCurrentIndex(index);
        qApp->Seamly2DSettings()->setPointNameSize(fontSizeComboBox->currentText().toInt());
        upDateScenes();
    });
    connect(ui->decreaseSize_Action, &QAction::triggered, this, [this]()
    {
        // Decrease font size for point names and update scenes
        const int index = qMax(fontSizeComboBox->currentIndex() - 1, 0);
        fontSizeComboBox->setCurrentIndex(index);
        qApp->Seamly2DSettings()->setPointNameSize(fontSizeComboBox->currentText().toInt());
        upDateScenes();
    });
    connect(ui->showPointNames_Action, &QAction::triggered, this, [this](bool checked)
    {
        // Toggle visibility of point names and update scenes
        qApp->Seamly2DSettings()->setHidePointNames(checked);
        upDateScenes();
    });
    connect(ui->useToolColor_Action, &QAction::triggered, this, [this](bool checked)
    {
        // Toggle whether to use tool color and update scenes
        qApp->Seamly2DSettings()->setUseToolColor(checked);
        upDateScenes();
    });
    // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
    // Tools Menu
    // Connect the "New Draft" action to create a new draft block
    connect(ui->newDraft_Action, &QAction::triggered, this, [this]()
    {
        qCDebug(vMainWindow, "New Draft Block.");
        QString draftBlockName = tr("Draft Block %1").arg(draftBlockComboBox->count() + 1);
        qCDebug(vMainWindow, "Generated Draft Block name: %s", qUtf8Printable(draftBlockName));
        qCDebug(vMainWindow, "Draft Block count %d", draftBlockComboBox->count());
        draftBlockName = createDraftBlockName(draftBlockName);
        qCDebug(vMainWindow, "Draft Block name: %s", qUtf8Printable(draftBlockName));
        if (draftBlockName.isEmpty())
        {
            qCDebug(vMainWindow, "Draft Block name is empty.");
            return;
        }
        addDraftBlock(draftBlockName);
    });
    // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
    // Tools->Point submenu actions
    // Connect each action in the Point submenu to a corresponding tool handler

    // Connect "Midpoint" action to activate the Midpoint tool
    connect(ui->midpoint_Action, &QAction::triggered, this, [this]
    {
        ui->draft_ToolBox->setCurrentWidget(ui->points_Page);
        ui->midpoint_ToolButton->setChecked(true);
        handleMidpointTool(true);
    });

    // Connect "Point at Distance and Angle" action to activate the Point at Distance and Angle tool
    connect(ui->pointAtDistanceAngle_Action, &QAction::triggered, this, [this]
    {
        ui->draft_ToolBox->setCurrentWidget(ui->points_Page);
        ui->pointAtDistanceAngle_ToolButton->setChecked(true);
        handlePointAtDistanceAngleTool(true);
    });

    // Connect "Point Along Line" action to activate the Point Along Line tool
    connect(ui->pointAlongLine_Action, &QAction::triggered, this, [this]
    {
        ui->draft_ToolBox->setCurrentWidget(ui->points_Page);
        ui->alongLine_ToolButton->setChecked(true);
        handleAlongLineTool(true);
    });

    // Connect "Point Along Perpendicular" action to activate the Point Along Perpendicular tool
    connect(ui->pointAlongPerpendicular_Action, &QAction::triggered, this, [this]
    {
        ui->draft_ToolBox->setCurrentWidget(ui->points_Page);
        ui->normal_ToolButton->setChecked(true);
        handleNormalTool(true);
    });

    // Connect "Bisector" action to activate the Bisector tool
    connect(ui->bisector_Action, &QAction::triggered, this, [this]
    {
        ui->draft_ToolBox->setCurrentWidget(ui->points_Page);
        ui->bisector_ToolButton->setChecked(true);
        handleBisectorTool(true);
    });

    // Connect "Point on Shoulder" action to activate the Point on Shoulder tool
    connect(ui->pointOnShoulder_Action, &QAction::triggered, this, [this]
    {
        ui->draft_ToolBox->setCurrentWidget(ui->points_Page);
        ui->shoulderPoint_ToolButton->setChecked(true);
        handleShoulderPointTool(true);
    });

    // Connect "Point of Contact" action to activate the Point of Contact tool
    connect(ui->pointOfContact_Action, &QAction::triggered, this, [this]
    {
        ui->draft_ToolBox->setCurrentWidget(ui->points_Page);
        ui->pointOfContact_ToolButton->setChecked(true);
        handlePointOfContactTool(true);
    });

    // Connect "Triangle" action to activate the Triangle tool
    connect(ui->triangle_Action, &QAction::triggered, this, [this]
    {
        ui->draft_ToolBox->setCurrentWidget(ui->points_Page);
        ui->triangle_ToolButton->setChecked(true);
        handleTriangleTool(true);
    });

    // Connect "Point Intersect XY" action to activate the Point Intersect XY tool
    connect(ui->pointIntersectXY_Action, &QAction::triggered, this, [this]
    {
        ui->draft_ToolBox->setCurrentWidget(ui->points_Page);
        ui->pointIntersectXY_ToolButton->setChecked(true);
        handlePointIntersectXYTool(true);
    });

    // Connect "Perpendicular Point" action to activate the Perpendicular Point tool
    connect(ui->perpendicularPoint_Action, &QAction::triggered, this, [this]
    {
        ui->draft_ToolBox->setCurrentWidget(ui->points_Page);
        ui->height_ToolButton->setChecked(true);
        handleHeightTool(true);
    });

    // Connect "Point Intersect Axis" action to activate the Point Intersect Axis tool
    connect(ui->pointIntersectAxis_Action, &QAction::triggered, this, [this]
    {
        ui->draft_ToolBox->setCurrentWidget(ui->points_Page);
        ui->lineIntersectAxis_ToolButton->setChecked(true);
        handleLineIntersectAxisTool(true);
    });

    // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
    //Tools->Line submenu actions
    connect(ui->lineTool_Action, &QAction::triggered, this, [this]
    {

        ui->draft_ToolBox->setCurrentWidget(ui->lines_Page);
        ui->line_ToolButton->setChecked(true);
        handleLineTool(true);
    });
    connect(ui->lineIntersect_Action, &QAction::triggered, this, [this]
    {
        ui->draft_ToolBox->setCurrentWidget(ui->lines_Page);
        ui->lineIntersect_ToolButton->setChecked(true);
        handleLineIntersectTool(true);
    });

    // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
    // Tools->Curve submenu actions
    // Connect each action in the Curve submenu to a corresponding tool handler

    // Connect "Curve" action to activate the Curve tool
    connect(ui->curve_Action, &QAction::triggered, this, [this]
    {
        ui->draft_ToolBox->setCurrentWidget(ui->curves_Page);
        ui->curve_ToolButton->setChecked(true);
        handleCurveTool(true);
    });

    // Connect "Spline" action to activate the Spline tool
    connect(ui->spline_Action, &QAction::triggered, this, [this]
    {
        ui->draft_ToolBox->setCurrentWidget(ui->curves_Page);
        ui->spline_ToolButton->setChecked(true);
        handleSplineTool(true);
    });

    // Connect "Curve with Control Points" action to activate the Curve with Control Points tool
    connect(ui->curveWithCPs_Action, &QAction::triggered, this, [this]
    {
        ui->draft_ToolBox->setCurrentWidget(ui->curves_Page);
        ui->curveWithCPs_ToolButton->setChecked(true);
        handleCurveWithControlPointsTool(true);
    });

    // Connect "Spline with Control Points" action to activate the Spline with Control Points tool
    connect(ui->splineWithCPs_Action, &QAction::triggered, this, [this]
    {
        ui->draft_ToolBox->setCurrentWidget(ui->curves_Page);
        ui->splineWithCPs_ToolButton->setChecked(true);
        handleSplineWithControlPointsTool(true);
    });

    // Connect "Point Along Curve" action to activate the Point Along Curve tool
    connect(ui->pointAlongCurve_Action, &QAction::triggered, this, [this]
    {
        ui->draft_ToolBox->setCurrentWidget(ui->curves_Page);
        ui->pointAlongCurve_ToolButton->setChecked(true);
        handlePointAlongCurveTool(true);
    });

    // Connect "Point Along Spline" action to activate the Point Along Spline tool
    connect(ui->pointAlongSpline_Action, &QAction::triggered, this, [this]
    {
        ui->draft_ToolBox->setCurrentWidget(ui->curves_Page);
        ui->pointAlongSpline_ToolButton->setChecked(true);
        handlePointAlongSplineTool(true);
    });

    // Connect "Curve Intersect Curve" action to activate the Curve Intersect Curve tool
    connect(ui->curveIntersectCurve_Action, &QAction::triggered, this, [this]
    {
        ui->draft_ToolBox->setCurrentWidget(ui->curves_Page);
        ui->pointOfIntersectionCurves_ToolButton->setChecked(true);
        handleCurveIntersectCurveTool(true);
    });

    // Connect "Spline Intersect Axis" action to activate the Spline Intersect Axis tool
    connect(ui->splineIntersectAxis_Action, &QAction::triggered, this, [this]
    {
        ui->draft_ToolBox->setCurrentWidget(ui->curves_Page);
        ui->curveIntersectAxis_ToolButton->setChecked(true);
        handleCurveIntersectAxisTool(true);
    });

    // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
    // Tools->Arc submenu actions
    // Connect each action in the Arc submenu to a corresponding tool handler

    // Connect "Arc Tool" action to activate the Arc Tool
    connect(ui->arcTool_Action, &QAction::triggered, this, [this]
    {
        ui->draft_ToolBox->setCurrentWidget(ui->arcs_Page);
        ui->arc_ToolButton->setChecked(true);
        handleArcTool(true);
    });

    // Connect "Point Along Arc" action to activate the Point Along Arc tool
    connect(ui->pointAlongArc_Action, &QAction::triggered, this, [this]
    {
        ui->draft_ToolBox->setCurrentWidget(ui->arcs_Page);
        ui->pointAlongArc_ToolButton->setChecked(true);
        handlePointAlongArcTool(true);
    });

    // Connect "Arc Intersect Axis" action to activate the Arc Intersect Axis tool
    connect(ui->arcIntersectAxis_Action, &QAction::triggered, this, [this]
    {
        ui->draft_ToolBox->setCurrentWidget(ui->arcs_Page);
        ui->arcIntersectAxis_ToolButton->setChecked(true);
        handleArcIntersectAxisTool(true);
    });

    // Connect "Arc Intersect Arc" action to activate the Arc Intersect Arc tool
    connect(ui->arcIntersectArc_Action, &QAction::triggered, this, [this]
    {
        ui->draft_ToolBox->setCurrentWidget(ui->arcs_Page);
        ui->pointOfIntersectionArcs_ToolButton->setChecked(true);
        handlePointOfIntersectionArcsTool(true);
    });

    // Connect "Circle Intersect" action to activate the Circle Intersect tool
    connect(ui->circleIntersect_Action, &QAction::triggered, this, [this]
    {
        ui->draft_ToolBox->setCurrentWidget(ui->arcs_Page);
        ui->pointOfIntersectionCircles_ToolButton->setChecked(true);
        handlePointOfIntersectionCirclesTool(true);
    });

    // Connect "Circle Tangent" action to activate the Circle Tangent tool
    connect(ui->circleTangent_Action, &QAction::triggered, this, [this]
    {
        ui->draft_ToolBox->setCurrentWidget(ui->arcs_Page);
        ui->pointFromCircleAndTangent_ToolButton->setChecked(true);
        handlePointFromCircleAndTangentTool(true);
    });

    // Connect "Arc Tangent" action to activate the Arc Tangent tool
    connect(ui->arcTangent_Action, &QAction::triggered, this, [this]
    {
        ui->draft_ToolBox->setCurrentWidget(ui->arcs_Page);
        ui->pointFromArcAndTangent_ToolButton->setChecked(true);
        handlePointFromArcAndTangentTool(true);
    });

    // Connect "Arc With Length" action to activate the Arc With Length tool
    connect(ui->arcWithLength_Action, &QAction::triggered, this, [this]
    {
        ui->draft_ToolBox->setCurrentWidget(ui->arcs_Page);
        ui->arcWithLength_ToolButton->setChecked(true);
        handleArcWithLengthTool(true);
    });

    // Connect "Elliptical Arc" action to activate the Elliptical Arc tool
    connect(ui->ellipticalArc_Action, &QAction::triggered, this, [this]
    {
        ui->draft_ToolBox->setCurrentWidget(ui->arcs_Page);
        ui->ellipticalArc_ToolButton->setChecked(true);
        handleEllipticalArcTool(true);
    });

    // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
    // Tools->Operations submenu actions
    // Connect each action in the Operations submenu to a corresponding tool handler

    // Connect "Group" action to activate the Group Tool
    connect(ui->group_Action, &QAction::triggered, this, [this]
    {
        ui->draft_ToolBox->setCurrentWidget(ui->operations_Page);
        ui->group_ToolButton->setChecked(true);
        handleGroupTool(true);
    });

    // Connect "Rotation" action to activate the Rotation Tool
    connect(ui->rotation_Action, &QAction::triggered, this, [this]
    {
        ui->draft_ToolBox->setCurrentWidget(ui->operations_Page);
        ui->rotation_ToolButton->setChecked(true);
        handleRotationTool(true);
    });

    // Connect "Mirror by Line" action to activate the Mirror by Line Tool
    connect(ui->mirrorByLine_Action, &QAction::triggered, this, [this]
    {
        ui->draft_ToolBox->setCurrentWidget(ui->operations_Page);
        ui->mirrorByLine_ToolButton->setChecked(true);
        handleMirrorByLineTool(true);
    });

    // Connect "Mirror by Axis" action to activate the Mirror by Axis Tool
    connect(ui->mirrorByAxis_Action, &QAction::triggered, this, [this]
    {
        ui->draft_ToolBox->setCurrentWidget(ui->operations_Page);
        ui->mirrorByAxis_ToolButton->setChecked(true);
        handleMirrorByAxisTool(true);
    });

    // Connect "Move" action to activate the Move Tool
    connect(ui->move_Action, &QAction::triggered, this, [this]
    {
        ui->draft_ToolBox->setCurrentWidget(ui->operations_Page);
        ui->move_ToolButton->setChecked(true);
        handleMoveTool(true);
    });

    // Connect "True Darts" action to activate the True Darts Tool
    connect(ui->trueDarts_Action, &QAction::triggered, this, [this]
    {
        ui->draft_ToolBox->setCurrentWidget(ui->operations_Page);
        ui->trueDarts_ToolButton->setChecked(true);
        handleTrueDartTool(true);
    });

    // Connect "Export Draft Blocks" action to export draft blocks
    connect(ui->exportDraftBlocks_Action, &QAction::triggered, this, [this]
    {
        ui->draft_ToolBox->setCurrentWidget(ui->operations_Page);
        exportDraftBlocksAs();
    });

    // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
    // Tools->Details submenu actions
    // Connect each action in the Details submenu to a corresponding tool handler

    // Connect "Union" action to activate the Union Tool
    connect(ui->union_Action, &QAction::triggered, this, [this]
    {
        ui->piece_ToolBox->setCurrentWidget(ui->details_Page);
        ui->unitePieces_ToolButton->setChecked(true);
        handleUnionTool(true);
    });

    // Connect "Export Pieces" action to export pieces
    connect(ui->exportPieces_Action, &QAction::triggered, this, [this]
    {
        ui->piece_ToolBox->setCurrentWidget(ui->details_Page);
        exportPiecesAs();
    });

    // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
    // Tools->Piece submenu actions
    // Connect each action in the Piece submenu to a corresponding tool handler

    // Connect "Add Pattern Piece" action to activate the Add Pattern Piece Tool
    connect(ui->addPiece_Action, &QAction::triggered, this, [this]
    {
        ui->draft_ToolBox->setCurrentWidget(ui->piece_Page);
        ui->addPatternPiece_ToolButton->setChecked(true);
        handlePatternPieceTool(true);
    });

    // Connect "Anchor Point" action to activate the Anchor Point Tool
    connect(ui->anchorPoint_Action, &QAction::triggered, this, [this]
    {
        ui->draft_ToolBox->setCurrentWidget(ui->piece_Page);
        ui->anchorPoint_ToolButton->setChecked(true);
        handleAnchorPointTool(true);
    });

    // Connect "Internal Path" action to activate the Internal Path Tool
    connect(ui->internalPath_Action, &QAction::triggered, this, [this]
    {
        ui->draft_ToolBox->setCurrentWidget(ui->piece_Page);
        ui->internalPath_ToolButton->setChecked(true);
        handleInternalPathTool(true);
    });

    // Connect "Insert Nodes" action to activate the Insert Nodes Tool
    connect(ui->insertNodes_Action, &QAction::triggered, this, [this]
    {
        ui->draft_ToolBox->setCurrentWidget(ui->piece_Page);
        ui->insertNodes_ToolButton->setChecked(true);
        handleInsertNodesTool(true);
    });

  // -----------------------------------------------------------------
    // Tools->Layout submenu actions
    // Connect each action in the Layout submenu to a corresponding tool handler

    // Connect "New Print Layout" action to activate the New Layout Tool
    connect(ui->newPrintLayout_Action, &QAction::triggered, this, [this]
    {
        ui->layout_ToolBox->setCurrentWidget(ui->layout_Page);
        ui->layoutSettings_ToolButton->setChecked(true);
        handleNewLayout(true);
    });

    // Connect "Export Layout" action to export the current layout
    connect(ui->exportLayout_Action, &QAction::triggered, this, [this]
    {
        ui->layout_ToolBox->setCurrentWidget(ui->layout_Page);
        exportLayoutAs();
    });

    // Connect "Last Used Tool" action to execute the LastUsedTool slot
    connect(ui->lastTool_Action, &QAction::triggered, this, &MainWindow::LastUsedTool);

    // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
    // Measurements menu
    // Connect each action in the Measurements menu to a corresponding slot or function

    // Connect "Open SeamlyMe" action to open the SeamlyMe application
    connect(ui->openSeamlyMe_Action, &QAction::triggered, this, [this]()
    {
        const QString seamlyme = qApp->SeamlyMeFilePath();
        const QString workingDirectory = QFileInfo(seamlyme).absoluteDir().absolutePath();
        QStringList arguments;
        if (isNoScaling)
        {
            arguments.append(QLatin1String("--") + LONG_OPTION_NO_HDPI_SCALING);
        }
        QProcess::startDetached(seamlyme, arguments, workingDirectory);
    });

    // Connect "Edit Current Measurements" action to show measurements for editing
    connect(ui->editCurrent_Action, &QAction::triggered, this, &MainWindow::ShowMeasurements);

    // Connect "Unload Measurements" action to unload measurements
    connect(ui->unloadMeasurements_Action, &QAction::triggered, this, &MainWindow::UnloadMeasurements);

    // Connect "Load Individual Measurements" action to load individual measurements
    connect(ui->loadIndividual_Action, &QAction::triggered, this, &MainWindow::LoadIndividual);

    // Connect "Load Multisize Measurements" action to load multisize measurements
    connect(ui->loadMultisize_Action, &QAction::triggered, this, &MainWindow::LoadMultisize);

    // Connect "Sync Measurements" action to synchronize measurements
    connect(ui->syncMeasurements_Action, &QAction::triggered, this, &MainWindow::SyncMeasurements);

    // Connect "Table" action to open the Variable Table dialog
    connect(ui->table_Action, &QAction::triggered, this, [this](bool checked)
    {
        if (checked)
        {
            dialogTable = new DialogVariables(pattern, doc, this);
            connect(dialogTable.data(), &DialogVariables::updateProperties, toolProperties,
                    &VToolOptionsPropertyBrowser::refreshOptions);
            connect(dialogTable.data(), &DialogVariables::DialogClosed, this, [this]()
            {
                ui->table_Action->setChecked(false);
                if (dialogTable != nullptr)
                {
                    delete dialogTable;
                }
            });
            dialogTable->show();
        }
        else
        {
            ui->table_Action->setChecked(true);
            dialogTable->activateWindow();
        }
    });

    // Connect "Export Variables to CSV" action to export variables to a CSV file
    connect(ui->exportVariablesToCSV_Action, &QAction::triggered, this, &MainWindow::handleExportToCSV);

    // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
    // History menu
    // Connect "History" action to show the history dialog
    connect(ui->history_Action, &QAction::triggered, this, [this](bool checked)
    {
        if (checked)
        {
            // Create and show the history dialog
            historyDialog = new HistoryDialog(pattern, doc, this);
            connect(this, &MainWindow::RefreshHistory, historyDialog.data(), &HistoryDialog::updateHistory);
            connect(historyDialog.data(), &HistoryDialog::DialogClosed, this, [this]()
            {
                ui->history_Action->setChecked(false);
                if (historyDialog != nullptr)
                {
                    delete historyDialog;
                }
            });
            historyDialog->show();
        }
        else
        {
            // If already open, activate the existing history dialog
            ui->history_Action->setChecked(true);
            historyDialog->activateWindow();
        }
    });

    // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
    // Utilities menu
    // Connect "Calculator" action to show the calculator dialog
    connect(ui->calculator_Action, &QAction::triggered, this, [this]()
    {
        CalculatorDialog *calcDialog = new CalculatorDialog(this);
        calcDialog->setAttribute(Qt::WA_DeleteOnClose, true);
        calcDialog->setWindowTitle(tr("Calculator"));
        calcDialog->adjustSize();
        calcDialog->show();
    });

    // Connect "Decimal Chart" action to show the decimal chart dialog
    connect(ui->decimalChart_Action, &QAction::triggered, this, [this]()
    {
        DecimalChartDialog *decimalchartDialog = new DecimalChartDialog(this);
        decimalchartDialog->setAttribute(Qt::WA_DeleteOnClose, true);
        decimalchartDialog->show();
    });

    // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
    // Help menu
    // Connect various actions in the Help menu to open external resources

    // Show keyboard shortcuts dialog
    connect(ui->shortcuts_Action, &QAction::triggered, this, [this]()
    {
        ShortcutsDialog *shortcutsDialog = new ShortcutsDialog(this);
        shortcutsDialog->setAttribute(Qt::WA_DeleteOnClose, true);
        shortcutsDialog->show();
    });

    // Open the Seamly2D wiki in a web browser
    connect(ui->wiki_Action, &QAction::triggered, this, []()
    {
        qCDebug(vMainWindow, "Showing online help");
        QDesktopServices::openUrl(QUrl(QStringLiteral("https://wiki.seamly.net/wiki/Main_Page")));
    });

    // Open the Seamly2D forum in a web browser
    connect(ui->forum_Action, &QAction::triggered, this, []()
    {
        qCDebug(vMainWindow, "Opening forum");
        QDesktopServices::openUrl(QUrl(QStringLiteral("https://forum.seamly.io/")));
    });

    // Report a bug by opening a new issue on GitHub
    connect(ui->reportBug_Action, &QAction::triggered, this, []()
    {
        qCDebug(vMainWindow, "Reporting bug");
        QDesktopServices::openUrl(QUrl(QStringLiteral(
            "https://github.com/FashionFreedom/Seamly2D/issues/new?&labels=bug&template=bug_report.md&title=BUG%3A")));
    });

    // Show the "About Qt" dialog
    connect(ui->aboutQt_Action, &QAction::triggered, this, [this]()
    {
        QMessageBox::aboutQt(this, tr("About Qt"));
    });

    // Show the "About Seamly2D" dialog
    connect(ui->aboutSeamly2D_Action, &QAction::triggered, this, [this]()
    {
        DialogAboutApp *aboutDialog = new DialogAboutApp(this);
        aboutDialog->setAttribute(Qt::WA_DeleteOnClose, true);
        aboutDialog->show();
    });

    // - - - - - - - - - - - - - - - - - - - - - - - - - - - - 
    // Toolbox toolbar
    // Connect each action in the toolbox toolbar to its corresponding tool handler

    connect(ui->arrow_Action,         &QAction::triggered, this, &MainWindow::handleArrowTool);
    connect(ui->points_Action,        &QAction::triggered, this, &MainWindow::handlePointsMenu);
    connect(ui->lines_Action,         &QAction::triggered, this, &MainWindow::handleLinesMenu);
    connect(ui->arcs_Action,          &QAction::triggered, this, &MainWindow::handleArcsMenu);
    connect(ui->curves_Action,        &QAction::triggered, this, &MainWindow::handleCurvesMenu);
    connect(ui->modifications_Action, &QAction::triggered, this, &MainWindow::handleOperationsMenu);
    connect(ui->details_Action,       &QAction::triggered, this, &MainWindow::handlePatternPiecesMenu);
    connect(ui->pieces_Action,        &QAction::triggered, this, &MainWindow::handlePieceMenu);
    connect(ui->layout_Action,        &QAction::triggered, this, &MainWindow::handleLayoutMenu);

// End MainWindow::CreateActions()

//---------------------------------------------------------------------------------------------------------------------
/**
 * @brief Initializes the autosave feature with user-defined settings.
 * 
 * @details This function creates and configures a QTimer for autosaving the pattern at regular intervals. The autosave
 * timer is set to the specified interval, and the function connects the timer's timeout signal to the AutoSavePattern slot.
 * The autosave feature can be enabled or disabled based on the user's preference, and the autosave interval is also
 * customizable. If autosave is enabled, the timer is started, and information about the autosave interval is logged.
 * 
 * @note The function deletes any existing autosave timer before creating a new one.
 */
void MainWindow::InitAutoSave()
{
    // Delete existing autosave timer
    delete autoSaveTimer;
    autoSaveTimer = nullptr;

    // Create a new QTimer for autosaving
    autoSaveTimer = new QTimer(this);
    autoSaveTimer->setTimerType(Qt::VeryCoarseTimer);

    // Connect the timer's timeout signal to the AutoSavePattern slot
    connect(autoSaveTimer, &QTimer::timeout, this, &MainWindow::AutoSavePattern);
    
    // Stop the timer initially
    autoSaveTimer->stop();

    // Check if autosave is enabled in settings
    if (qApp->Seamly2DSettings()->GetAutosaveState())
    {
        // Retrieve autosave interval from settings
        const qint32 autoTime = qApp->Seamly2DSettings()->getAutosaveInterval();
        
        // Start the autosave timer with the specified interval
        autoSaveTimer->start(autoTime * 60000); // Convert minutes to milliseconds
        qCInfo(vMainWindow, "Autosaving every %d minutes.", autoTime);
    }

    // Set the autosave timer in the application
    qApp->setAutoSaveTimer(autoSaveTimer);
}
// End MainWindow::InitAutoSave()

//---------------------------------------------------------------------------------------------------------------------
/**
 * @brief Creates a new draft block name based on user input.
 * 
 * @details This function displays a dialog to the user to input a draft block name. It checks if the entered name already
 * exists in the draft block combo box. If it does, a warning message is shown, and the user is prompted to retry or cancel.
 * The function repeats this process until a unique name is entered or the user cancels the dialog. The entered name is then
 * returned.
 * 
 * @param text The initial text value for the draft block name.
 * @return QString The entered draft block name, or an empty string if the user cancels the dialog.
 * 
 * @note The function dynamically creates a QInputDialog and manages its memory. It also adjusts the window flags to remove
 * unnecessary buttons for the specific use case.
 */
QString MainWindow::createDraftBlockName(const QString &text)
{
    // Create a new input dialog
    QInputDialog *dialog = new QInputDialog(this);
    
    // Configure the input dialog
    dialog->setInputMode(QInputDialog::TextInput);
    dialog->setLabelText(tr("Name:"));
    dialog->setTextEchoMode(QLineEdit::Normal);
    dialog->setWindowTitle(tr("Draft block."));
    dialog->setWindowFlags(windowFlags() & ~Qt::WindowContextHelpButtonHint
                                         & ~Qt::WindowMaximizeButtonHint
                                         & ~Qt::WindowMinimizeButtonHint);
    dialog->resize(300, 100);
    dialog->setTextValue(text);
    
    QString draftBlockName;

    while (1)
    {
        // Execute the input dialog
        const bool result = dialog->exec();
        draftBlockName = dialog->textValue();

        // If the user cancels or enters an empty name, return an empty string
        if (result == false || draftBlockName.isEmpty())
        {
            delete dialog;
            return QString();
        }

        // Check if the entered name already exists
        if (draftBlockComboBox->findText(draftBlockName) == -1)
        {
            break; // Exit the loop if the name is unique
        }

        // Name already exists, show a warning message
        QMessageBox messageBox;
        messageBox.setWindowTitle(tr("Name Exists"));
        messageBox.setIcon(QMessageBox::Warning);
        messageBox.setStandardButtons(QMessageBox::Retry | QMessageBox::Cancel);
        messageBox.setDefaultButton(QMessageBox::Retry);
        messageBox.setText(tr("The action can't be completed because the Draft Block name already exists."));
        int boxResult = messageBox.exec();

        switch (boxResult)
        {
            case QMessageBox::Retry:
                break;    // Repeat Dialog
            case QMessageBox::Cancel:
                return QString();  // Exit Dialog
            default:
                break;   // Should never be reached
        }
    }

    // Cleanup and return the entered name
    delete dialog;
    return draftBlockName;
}
// End MainWindow::createDraftBlockName()

//---------------------------------------------------------------------------------------------------------------------
/**
 * @brief Destructor for the MainWindow class.
 * 
 * @details This destructor is responsible for cleaning up resources and 
 * memory allocated for the MainWindow object.
 * It cancels any active tool, cleans up the layout, and deletes the 
 * associated document (doc) and user interface (ui).
 * 
 * @note This destructor is automatically called when an instance 
 * of the MainWindow class goes out of scope or is explicitly
 * deleted using the 'delete' keyword.
 */
MainWindow::~MainWindow()
{
    // Cancel any active tool
    CancelTool();

    // Clean up the layout
    CleanLayout();

    // Delete the associated document
    delete doc;

    // Delete the user interface
    delete ui;
}

// End MainWindow::~MainWindow()
//---------------------------------------------------------------------------------------------------------------------
/**
 * @brief Loads a Seamly2D pattern file.
 * 
 * @details This function loads a Seamly2D pattern from the specified file. It performs various checks
 * and updates, including checking for unsaved changes, validating the file, handling measurement files,
 * locking the pattern file, and updating the UI accordingly. The function returns true if the loading
 * is successful and false otherwise.
 * 
 * @param fileName The name of the pattern file to be loaded.
 * @param customMeasureFile The custom measurement file associated with the pattern file.
 * @return true If the pattern is successfully loaded.
 * @return false If there are errors or the loading process is unsuccessful.
 */
bool MainWindow::LoadPattern(const QString &fileName, const QString& customMeasureFile)
{
    qCInfo(vMainWindow, "Loading new file %s.", qUtf8Printable(fileName));

    // Check for unsaved changes or if more than one file is loaded simultaneously
    if (OpenNewSeamly2D(fileName))
    {
        return false;
    }

    // Check for empty file name
    if (fileName.isEmpty())
    {
        qCWarning(vMainWindow, "New loaded filename is empty.");
        Clear();
        return false;
    }

    try
    {
        // Undocumented feature of Seamly2D: Setting up associations for SeamlyMe on macOS
        MeasurementDoc measurements(pattern);
        measurements.setSize(VContainer::rsize());
        measurements.setHeight(VContainer::rheight());
        measurements.setXMLContent(fileName);

        // if the measurement file is recognized as Multisize or Individual (e.g. A valid SeamlyMe measurement file)
        if (measurements.Type() == MeasurementsType::Multisize || measurements.Type() == MeasurementsType::Individual)
        {
            // get the working directory path
            const QString seamlyme = qApp->SeamlyMeFilePath();
            const QString workingDirectory = QFileInfo(seamlyme).absoluteDir().absolutePath();

            QStringList arguments = QStringList() << fileName;
            if (isNoScaling)
            {
                arguments.append(QLatin1String("--") + LONG_OPTION_NO_HDPI_SCALING);
            }

            // Start the SeamlyMe process
            QProcess::startDetached(seamlyme, arguments, workingDirectory);
            qApp->exit(V_EX_OK);
            return false; // Stop processing further
        }
    }

    catch (VException &exception)
    {
        qCCritical(vMainWindow, "%s\n\n%s\n\n%s", qUtf8Printable(tr("File exception.")),
                   qUtf8Printable(exception.ErrorMessage()), qUtf8Printable(exception.DetailedInformation()));

        Clear();
        if (!VApplication::IsGUIMode())
        {
            // exit application if Seamly2D is not running in GUI mode -- it's running in CLI mode
            qApp->exit(V_EX_NOINPUT);
        }
        return false;
    }

    qCDebug(vMainWindow, "Locking file");
    VlpCreateLock(lock, fileName);

    // Check if file is currently locked by another process or instance of Seamly
    if (lock->IsLocked())
    {
        qCInfo(vMainWindow, "Pattern file %s was locked.", qUtf8Printable(fileName));
    }
    else
    {
        if (!IgnoreLocking(lock->GetLockError(), fileName))
        {
            return false;
        }
    }

    // At this stage, the scene is empty. Fit scene size to view size
    VMainGraphicsView::NewSceneRect(draftScene, ui->view);
    VMainGraphicsView::NewSceneRect(pieceScene, ui->view);

    qApp->setOpeningPattern(); // Begin opening file
    try
    {
        // Convert the pattern file to the internal format
        VPatternConverter converter(fileName);
        m_curFileFormatVersion = converter.GetCurrentFormatVarsion();
        m_curFileFormatVersionStr = converter.GetVersionStr();
        doc->setXMLContent(converter.Convert());

        if (!customMeasureFile.isEmpty())
        {
            doc->SetMPath(RelativeMPath(fileName, customMeasureFile));
        }

        qApp->setPatternUnit(doc->MUnit());
        const QString path = AbsoluteMPath(fileName, doc->MPath());

        if (!path.isEmpty())
        {
            // Check if the measurement file exists
            const QString newPath = checkPathToMeasurements(fileName, path);

            if (newPath.isEmpty())
            {
                qApp->setOpeningPattern(); // End opening file
                Clear();
                qCCritical(vMainWindow, "%s", qUtf8Printable(tr("The measurements file '%1' could not be found.")
                                                             .arg(path)));
                if (!VApplication::IsGUIMode())
                {
                    qApp->exit(V_EX_NOINPUT);
                }
                return false;
            }

            if (!loadMeasurements(newPath))
            {
                qCCritical(vMainWindow, "%s", qUtf8Printable(tr("The measurements file '%1' could not be found.")
                                                             .arg(newPath)));
                qApp->setOpeningPattern(); // End opening file
                Clear();
                if (!VApplication::IsGUIMode())
                {
                    qApp->exit(V_EX_NOINPUT);
                }
                return false;
            }
            else
            {
                ui->unloadMeasurements_Action->setEnabled(true);
                watcher->addPath(path);
                ui->editCurrent_Action->setEnabled(true);
            }
        }

        if (qApp->patternType() == MeasurementsType::Unknown)
        {// Show toolbar only if no measurements were uploaded.
            initStatusBar();
        }
    }

    catch (VException &exception)
    {
        qCCritical(vMainWindow, "%s\n\n%s\n\n%s", qUtf8Printable(tr("File exception.")),
                   qUtf8Printable(exception.ErrorMessage()), qUtf8Printable(exception.DetailedInformation()));

        qApp->setOpeningPattern(); // End opening file
        Clear();
        if (!VApplication::IsGUIMode())
        {
            // exit application if running in CLI mode
            qApp->exit(V_EX_NOINPUT);
        }
        return false;
    }

    fullParseFile();

    if (guiEnabled)
    { // No errors occurred
        // open file with GUI
        patternReadOnly = doc->IsReadOnly();
        setWidgetsEnabled(true);
        setCurrentFile(fileName);
        helpLabel->setText(tr("File loaded"));
        qCDebug(vMainWindow, "%s", qUtf8Printable(helpLabel->text()));

        // Fit scene size to the best size for the first show
        zoomFirstShow();
        updateZoomToPointComboBox(draftPointNamesList());

        showDraftMode(true);

        qApp->setOpeningPattern(); // End opening file
        return true;
    }
    else
    {
        // open file and process CLI command 
        qApp->setOpeningPattern(); // End opening file
        return false;
    }
}
// End MainWindow::LoadPattern()

//---------------------------------------------------------------------------------------------------------------------
/**
 * @brief Retrieves the list of files that need to be restored.
 * 
 * @details This function examines the list of files provided by Seamly2DSettings
 * and filters out the files that are currently locked. The resulting list
 * contains files that need to be reopened after a crash or unexpected shutdown.
 * The function also updates the Seamly2DSettings to remove files that do not
 * exist.
 * 
 * @return QStringList A list of files that need to be restored.
 */
QStringList MainWindow::GetUnlokedRestoreFileList() const
{
    QStringList restoreFiles;

    // Take all files that need to be restored
    QStringList files = qApp->Seamly2DSettings()->GetRestoreFileList();

    if (files.size() > 0)
    {
        // Iterate through the files to find those that need reopening
        for (int i = 0; i < files.size(); ++i)
        {
            // Seeking file that really needs to be reopened
            VLockGuard<char> lock(files.at(i));
            
            if (lock.IsLocked())
            {
                restoreFiles.append(files.at(i));
            }
        }

        // Clearing list after filtering
        for (int i = 0; i < restoreFiles.size(); ++i)
        {
            files.removeAll(restoreFiles.at(i));
        }

        // Clear all files that do not exist.
        QStringList filtered;
        for (int i = 0; i < files.size(); ++i)
        {
            if (QFileInfo(files.at(i)).exists())
            {
                filtered.append(files.at(i));
            }
        }

        // Update Seamly2DSettings with the filtered list
        qApp->Seamly2DSettings()->SetRestoreFileList(filtered);
    }

    return restoreFiles;
}
// End MainWindow::GetUnlokedRestoreFileList()

//---------------------------------------------------------------------------------------------------------------------
/**
 * @brief Applies the common toolbar style to multiple toolbars in the main window.
 * 
 * @details This function sets the style properties, such as icon size and button size, for the specified toolbars.
 */
void MainWindow::ToolBarStyles()
{
    ToolBarStyle(ui->draft_ToolBar);
    ToolBarStyle(ui->mode_ToolBar);
    ToolBarStyle(ui->edit_Toolbar);
    ToolBarStyle(ui->zoom_ToolBar);
    ToolBarStyle(ui->file_ToolBar);

    // Set the font and size for the font combo box based on application settings.
    fontComboBox->setCurrentFont(qApp->Seamly2DSettings()->getPointNameFont());
    int index = fontSizeComboBox->findData(qApp->Seamly2DSettings()->getPointNameSize());
    fontSizeComboBox->setCurrentIndex(index);
}

/**
 * @brief Resets the origins and makes them visible in the drafting and piece scenes.
 * 
 * @details This function initializes the origins in both the drafting and piece scenes and makes them visible.
 */
void MainWindow::resetOrigins()
{
    // Initialize and make the origins visible in the drafting scene.
    draftScene->InitOrigins();
    draftScene->setOriginsVisible(true);

    // Initialize and make the origins visible in the piece scene.
    pieceScene->InitOrigins();
    pieceScene->setOriginsVisible(true);
}
// End MainWindow::ToolBarStyles()

//---------------------------------------------------------------------------------------------------------------------
/**
 * @brief Displays the layout pages based on the given index.
 * 
 * @param index The index of the layout page to be displayed.
 *              If the index is out of bounds, the temporary layout scene is set to the main view.
 *              Otherwise, the scene corresponding to the specified index is set to the main view.
 */
void MainWindow::showLayoutPages(int index)
{
    // Check if the index is within the valid range of layout scenes.
    if (index < 0 || index >= scenes.size())
    {
        // If the index is out of bounds, set the temporary layout scene to the main view.
        ui->view->setScene(tempSceneLayout);
    }
    else
    {
        // If the index is valid, set the scene corresponding to the specified index to the main view.
        ui->view->setScene(scenes.at(index));
    }

    // Adjust the view to fit the entire scene while maintaining the aspect ratio.
    ui->view->fitInView(ui->view->scene()->sceneRect(), Qt::KeepAspectRatio);
}
// End MainWindow::showLayoutPages()

//---------------------------------------------------------------------------------------------------------------------
/**
 * @brief Opens the preferences dialog to allow users to customize application settings.
 * 
 * @details This function ensures that the preferences dialog is not opened multiple times concurrently by using a static
 * guard pointer. It sets a wait cursor during the dialog creation to indicate ongoing processing. The preferences
 * dialog is then created, connected to various signals for updating UI elements and settings, and executed modally.
 * If the user accepts the changes in the dialog, it initializes auto-save settings.
 */
void MainWindow::Preferences()
{
    // Check if the preferences dialog is already open to prevent multiple instances.
    static QPointer<DialogPreferences> guard;

    if (guard.isNull())
    {
        // Set the cursor to a wait cursor during the dialog creation.
        QGuiApplication::setOverrideCursor(QCursor(Qt::WaitCursor));

        // Create an instance of the preferences dialog.
        DialogPreferences *preferences = new DialogPreferences(this);

        // Use QScopedPointer to ensure proper cleanup in case of exceptions.
        QScopedPointer<DialogPreferences> dialog(preferences);

        // Assign the created dialog to the guard pointer.
        guard = preferences;

        // Connect various signals to update UI elements and settings when preferences are changed.
        connect(dialog.data(), &DialogPreferences::updateProperties, this, &MainWindow::WindowsLocale); // Must be first
        connect(dialog.data(), &DialogPreferences::updateProperties, this, &MainWindow::ToolBarStyles);
        connect(dialog.data(), &DialogPreferences::updateProperties, this, &MainWindow::updateToolBarVisibility);
        connect(dialog.data(), &DialogPreferences::updateProperties, this, &MainWindow::refreshLabels);
        connect(dialog.data(), &DialogPreferences::updateProperties, this, &MainWindow::resetOrigins);
        connect(dialog.data(), &DialogPreferences::updateProperties, this, &MainWindow::upDateScenes);
        connect(dialog.data(), &DialogPreferences::updateProperties, this, &MainWindow::updateViewToolbar);
        connect(dialog.data(), &DialogPreferences::updateProperties, this, &MainWindow::resetPanShortcuts);
        connect(dialog.data(), &DialogPreferences::updateProperties, this, [this](){emit doc->FullUpdateFromFile();});
        // ... Additional connections for updating other UI elements and settings ...

        // Connect signals to reset scroll bars and animations in the main graphics view.
        connect(dialog.data(), &DialogPreferences::updateProperties, ui->view, &VMainGraphicsView::resetScrollBars);
        connect(dialog.data(), &DialogPreferences::updateProperties, ui->view, &VMainGraphicsView::resetScrollAnimations);

        // Restore the cursor to its original state.
        QGuiApplication::restoreOverrideCursor();

        // Execute the preferences dialog modally.
        if (guard->exec() == QDialog::Accepted)
        {
            // If the user accepted changes, initialize auto-save settings.
            InitAutoSave();
        }
    }
}
// End MainWindow::Preferences()

//---------------------------------------------------------------------------------------------------------------------
#if defined(Q_OS_MAC)
/** @brief Launches the SeamlyMe application to create and edit measurements.
 * 
 * @details This function is specific to macOS (Q_OS_MAC). It retrieves the path to the SeamlyMe executable, determines the
 * working directory, and launches the SeamlyMe application with optional arguments. If the `isNoScaling` flag is set,
 * it includes the appropriate command line option for disabling high DPI scaling. This function is conditioned by the
 * OS macro to ensure it is only compiled and executed on macOS platforms.
 */
void MainWindow::CreateMeasurements()
{
    // --> Launch SeamlyMe application <--

    // Get the path to the SeamlyMe executable.
    const QString seamlyme = qApp->SeamlyMeFilePath();

    // Determine the working directory for the SeamlyMe process.
    const QString workingDirectory = QFileInfo(seamlyme).absoluteDir().absolutePath();

    // Prepare the list of command line arguments.
    QStringList arguments;
    
    // Add the no scaling option if required.
    if (isNoScaling)
    {
        arguments.append(QLatin1String("--") + LONG_OPTION_NO_HDPI_SCALING);
    }

    // Start the SeamlyMe process in detached mode with the specified arguments and working directory.
    QProcess::startDetached(seamlyme, arguments, workingDirectory);
}
#endif
// End #if Q_OS_MAC MainWindow::CreateMeasurements()

//---------------------------------------------------------------------------------------------------------------------
/**
 * @brief Initiates the export of the entire layout.
 * 
 * @details This function is triggered when the user wants to export the entire layout. It sets up the UI state for export,
 * checks if the layout is stale and prompts the user to continue if necessary, opens a dialog to get export options,
 * and performs the export operation. It also handles errors and updates the UI after the export.
 */
void MainWindow::exportLayoutAs()
{
    // Select the export layout tool button.
    ui->exportLayout_ToolButton->setChecked(true);

    // Check if the layout is marked as stale.
    if (isLayoutStale)
    {
        // Prompt the user to continue if the layout is stale.
        if (ContinueIfLayoutStale() == QMessageBox::No)
        {
            ui->exportLayout_ToolButton->setChecked(false);
            return;
        }
    }

    try
    {
        // Open a dialog to get export options and the destination path.
        ExportLayoutDialog dialog(scenes.size(), Draw::Layout, FileName(), this);

        // Check if the user canceled the export operation.
        if (dialog.exec() == QDialog::Rejected)
        {
            ui->exportLayout_ToolButton->setChecked(false);
            return;
        }

        // Perform the actual export of the layout.
        ExportData(QVector<VLayoutPiece>(), dialog);
    }
    catch (const VException &exception)
    {
        // Handle export exceptions and display an error message.
        ui->exportLayout_ToolButton->setChecked(false);
        qCritical("%s\n\n%s\n\n%s", qUtf8Printable(tr("Export exception.")),
                  qUtf8Printable(exception.ErrorMessage()), qUtf8Printable(exception.DetailedInformation()));
        return;
    }

    // Reset the export layout tool button to its default state.
    ui->exportLayout_ToolButton->setChecked(false);
}
// End MainWindow::exportLayoutAs()

//---------------------------------------------------------------------------------------------------------------------
/**
 * @brief Initiates the export of pattern pieces.
 * 
 * @details This function is called when the user wants to export pattern pieces. It sets up the UI state for export,
 * filters the pieces that are part of the layout, prepares the pieces for export, prompts the user for
 * export options, and performs the export operation. It also handles errors and updates the UI after the export.
 */
void MainWindow::exportPiecesAs()
{
    // Clear any help labels.
    helpLabel->setText(QString(""));

    // Deselect other tool buttons and select the export tool button.
    ui->arrowPointer_ToolButton->setChecked(false);
    ui->arrow_Action->setChecked(false);
    ui->exportPiecesAs_ToolButton->setChecked(true);

    // Retrieve all pieces from the pattern data.
    const QHash<quint32, VPiece> *allPieces = pattern->DataPieces();

    // Filter out only the pieces that are included in the layout.
    QHash<quint32, VPiece> piecesInLayout;
    for (QHash<quint32, VPiece>::const_iterator i = allPieces->constBegin(); i != allPieces->constEnd(); ++i)
    {
        if (i.value().isInLayout())
        {
            piecesInLayout.insert(i.key(), i.value());
        }
        ++i;
    }

    // Check if there are any pieces to export.
    if (piecesInLayout.count() == 0)
    {
        QMessageBox::information(this, tr("Layout mode"), tr("You don't have any pieces to export. Please, "
                                                            "include at least one piece in layout."),
                                QMessageBox::Ok, QMessageBox::Ok);
        return;
    }

    QVector<VLayoutPiece> pieceList;

    try
    {
        // Prepare the pieces for export.
        pieceList = preparePiecesForLayout(piecesInLayout);
    }
    catch (VException &exception)
    {
        // Handle the case where preparation for export fails.
        QMessageBox::warning(this, tr("Export pieces"),
                            tr("Can't export pieces.") + QLatin1String(" \n") + exception.ErrorMessage(),
                            QMessageBox::Ok, QMessageBox::Ok);
        return;
    }

    try
    {
        // Open a dialog to get export options and the destination path.
        ExportLayoutDialog dialog(1, Draw::Modeling, FileName(), this);
        dialog.setWindowTitle("Export Pattern Pieces");

        // Check if the user canceled the export operation.
        if (dialog.exec() == QDialog::Rejected)
        {
            ui->exportPiecesAs_ToolButton->setChecked(false);
            return;
        }

        // Perform the actual export of pattern pieces.
        ExportData(pieceList, dialog);
    }
    catch (const VException &exception)
    {
        // Handle export exceptions and display an error message.
        ui->exportPiecesAs_ToolButton->setChecked(false);
        qCritical("%s\n\n%s\n\n%s", qUtf8Printable(tr("Export exception.")),
                    qUtf8Printable(exception.ErrorMessage()), qUtf8Printable(exception.DetailedInformation()));
        return;
    }

    // Reset tool buttons to default state.
    ui->arrowPointer_ToolButton->setChecked(true);
    ui->arrow_Action->setChecked(true);
    ui->exportPiecesAs_ToolButton->setChecked(false);
}
// End MainWindow::exportPiecesAs()

//---------------------------------------------------------------------------------------------------------------------
/**
 * @brief Exports draft blocks as selected file format.
 * 
 * @details This function initiates the process of exporting draft blocks. It prepares the scene,
 * prompts the user for export options, and then calls the corresponding export function
 * based on the chosen file format.
 */
void MainWindow::exportDraftBlocksAs()
{
    // Clear any help labels.
    helpLabel->setText(QString(""));

    // Deselect other tool buttons and select the export tool button.
    ui->arrowPointer_ToolButton->setChecked(false);
    ui->arrow_Action->setChecked(false);
    ui->exportDraftBlocks_ToolButton->setChecked(true);

    // Store the current view state.
    int vScrollBar = ui->view->verticalScrollBar()->value();
    int hScrollBar = ui->view->horizontalScrollBar()->value();
    QTransform viewTransform = ui->view->transform();

    // Include all items in the draft scene for export.
    ui->view->zoomToFit();
    ui->view->repaint();
    ui->view->zoom100Percent();

    // Enable all draft blocks in the scene.
    const QList<QGraphicsItem *> items = draftScene->items();
    for (auto *item : items)
    {
        item->setEnabled(true);
    }
    ui->view->repaint();

    // Hide the origins during export.
    draftScene->setOriginsVisible(false);

    // Open a file dialog to get export options and the destination path.
    ExportLayoutDialog dialog(1, Draw::Calculation, FileName(), this);
    dialog.setWindowTitle("Export Draft Blocks");

    if (dialog.exec() == QDialog::Accepted)
    {
        // Construct the full filename based on the chosen format.
        const QString filename = QString("%1/%2%3")
            .arg(dialog.path())                                             // 1
            .arg(dialog.fileName())                                         // 2
            .arg(ExportLayoutDialog::exportFormatSuffix(dialog.format()));  // 3

        QRectF rect;
        rect = draftScene->itemsBoundingRect();
        draftScene->update(rect);
        QGraphicsRectItem *paper = new QGraphicsRectItem(rect);
        QMarginsF margins = QMarginsF(0, 0, 0, 0);

        // Determine the export format and call the corresponding export function.
        switch (dialog.format())
        {
            case LayoutExportFormat::SVG:
                exportSVG(filename, paper, draftScene);
                break;
            case LayoutExportFormat::PNG:
                exportPNG(filename, draftScene);
                break;
            case LayoutExportFormat::JPG:
                exportJPG(filename, draftScene);
                break;
            case LayoutExportFormat::BMP:
                exportBMP(filename, draftScene);
                break;
            case LayoutExportFormat::TIF:
                exportTIF(filename, draftScene);
                break;
            case LayoutExportFormat::PPM:
                exportPPM(filename, draftScene);
                break;
            case LayoutExportFormat::PDF:
                exportPDF(filename, paper, draftScene, true, margins);
                break;
            case LayoutExportFormat::PDFTiled:
            case LayoutExportFormat::OBJ:
            case LayoutExportFormat::PS:
                exportPS(filename, paper, draftScene, true, margins);
                break;
            case LayoutExportFormat::EPS:
                exportEPS(filename, paper, draftScene, true, margins);
                break;
            case LayoutExportFormat::DXF_AC1006_Flat:
            case LayoutExportFormat::DXF_AC1009_Flat:
            case LayoutExportFormat::DXF_AC1012_Flat:
            case LayoutExportFormat::DXF_AC1014_Flat:
            case LayoutExportFormat::DXF_AC1015_Flat:
            case LayoutExportFormat::DXF_AC1018_Flat:
            case LayoutExportFormat::DXF_AC1021_Flat:
            case LayoutExportFormat::DXF_AC1024_Flat:
            case LayoutExportFormat::DXF_AC1027_Flat:
            case LayoutExportFormat::DXF_AC1006_AAMA:
            case LayoutExportFormat::DXF_AC1009_AAMA:
            case LayoutExportFormat::DXF_AC1012_AAMA:
            case LayoutExportFormat::DXF_AC1014_AAMA:
            case LayoutExportFormat::DXF_AC1015_AAMA:
            case LayoutExportFormat::DXF_AC1018_AAMA:
            case LayoutExportFormat::DXF_AC1021_AAMA:
            case LayoutExportFormat::DXF_AC1024_AAMA:
            case LayoutExportFormat::DXF_AC1027_AAMA:
            case LayoutExportFormat::DXF_AC1006_ASTM:
            case LayoutExportFormat::DXF_AC1009_ASTM:
            case LayoutExportFormat::DXF_AC1012_ASTM:
            case LayoutExportFormat::DXF_AC1014_ASTM:
            case LayoutExportFormat::DXF_AC1015_ASTM:
            case LayoutExportFormat::DXF_AC1018_ASTM:
            case LayoutExportFormat::DXF_AC1021_ASTM:
            case LayoutExportFormat::DXF_AC1024_ASTM:
            case LayoutExportFormat::DXF_AC1027_ASTM:
            default:
                break;
        }
    }

    // Disable draft blocks in the scene except the current active block.
    doc->changeActiveDraftBlock(doc->getActiveDraftBlockName(), Document::FullParse);

    // Restore the visibility of the origins.
    draftScene->setOriginsVisible(qApp->Settings()->getShowAxisOrigin());

    // Restore the scale, scrollbars, and the current active draft block.
    ui->view->setTransform(viewTransform);
    VMainGraphicsView::NewSceneRect(ui->view->scene(), ui->view);
    zoomScaleChanged(ui->view->transform().m11());

    ui->view->verticalScrollBar()->setValue(vScrollBar);
    ui->view->horizontalScrollBar()->setValue(hScrollBar);

    // Reset tool buttons to default state.
    ui->arrowPointer_ToolButton->setChecked(true);
    ui->arrow_Action->setChecked(true);
    ui->exportDraftBlocks_ToolButton->setChecked(false);
}
// End MainWindow::exportDraftBlocksAs()

//---------------------------------------------------------------------------------------------------------------------
/**
 * @brief Reopens files after a crash if there are autosave files available.
 * 
 * @param args List of command-line arguments.
 * 
 * @details This function is responsible for checking and reopening files that were open during a previous session
 * that ended with a crash. It looks for autosave files and prompts the user to reopen the files.
 * If the user agrees, it attempts to restore the files from their autosave versions.
 * 
 * @note: The function removes files from the command-line arguments to avoid opening them twice.
 */
void MainWindow::ReopenFilesAfterCrash(QStringList &args)
{
    // Get the list of files with available autosave versions.
    const QStringList files = GetUnlokedRestoreFileList();
    
    // Check if there are files with autosave versions.
    if (files.size() > 0)
    {
        qCDebug(vMainWindow, "Reopen files after crash.");

        QStringList restoreFiles;
        
        // Filter out files that have corresponding autosave versions.
        for (int i = 0; i < files.size(); ++i)
        {
            QFile file(files.at(i) + autosavePrefix);
            if (file.exists())
            {
                restoreFiles.append(files.at(i));
            }
        }

        // Check if there are files to restore.
        if (restoreFiles.size() > 0)
        {
            QMessageBox::StandardButton reply;
            const QString mes = tr("Seamly2D didn't shut down correctly. Do you want to reopen files (%1) you had open?")
                    .arg(restoreFiles.size());
            reply = QMessageBox::question(this, tr("Reopen files."), mes, QMessageBox::Yes | QMessageBox::No,
                                            QMessageBox::Yes);

            // If the user chooses to reopen files, proceed with the restoration.
            if (reply == QMessageBox::Yes)
            {
                qCDebug(vMainWindow, "User said Yes.");

                // Iterate through files to restore and attempt to copy the autosave version back.
                for (int i = 0; i < restoreFiles.size(); ++i)
                {
                    QString error;

                    // Safely copy the autosave version back to the original file location.
                    if (VDomDocument::SafeCopy(restoreFiles.at(i) + autosavePrefix, restoreFiles.at(i), error))
                    {
                        QFile autoFile(restoreFiles.at(i) + autosavePrefix);
                        autoFile.remove();
                        
                        // Load the pattern from the restored file.
                        LoadPattern(restoreFiles.at(i));
                        
                        // Remove the file from the command-line arguments to avoid opening it twice.
                        args.removeAll(restoreFiles.at(i));
                    }
                    else
                    {
                        qCWarning(vMainWindow, "Could not copy %s%s to %s %s",
                                qUtf8Printable(restoreFiles.at(i)), qUtf8Printable(autosavePrefix),
                                qUtf8Printable(restoreFiles.at(i)), qUtf8Printable(error));
                    }
                }
            }
        }
    }
}
// End MainWindow::ReopenFilesAfterCrash()

//---------------------------------------------------------------------------------------------------------------------
/**
 * @brief Checks and updates the path to the measurements file.
 * 
 * @param patternPath The path to the pattern file.
 * @param path The current path to the measurements file.
 * @return QString The updated path to the measurements file.
 * 
 * @details This function checks if the provided measurements file path exists. If not, it prompts the user to update the file location
 * through a file dialog. The function handles the case of both individual and multisize measurements, including conversion and
 * saving if needed. It also ensures that the pattern type is correctly set based on the measurements file.
 */
QString MainWindow::checkPathToMeasurements(const QString &patternPath, const QString &path) {
    // Check if the provided path is empty.
    if (path.isEmpty()) {
        return path;
    }

    // Check if the file at the given path exists.
    QFileInfo table(path);
    if (table.exists() == false) {
        // If running in console mode, return an empty string as console mode doesn't support fixing the path.
        if (!VApplication::IsGUIMode()) {
            return QString();
        } else {
            // If running in GUI mode, prompt the user to update the file location.
            const QString text = tr("The measurements file <br/><br/> <b>%1</b> <br/><br/> could not be found. Do you "
                                    "want to update the file location?").arg(path);
            QMessageBox::StandardButton result = QMessageBox::question(this, tr("Loading measurements file"), text,
                                                                    QMessageBox::Yes | QMessageBox::No,
                                                                    QMessageBox::Yes);
            if (result == QMessageBox::No) {
                return QString();
            } else {
                // Determine the type of measurements file (individual, multisize, or unknown).
                MeasurementsType patternType;
                if ((table.suffix() == vstExt) || (table.suffix() == smmsExt)) {
                    patternType = MeasurementsType::Multisize;
                } else if ((table.suffix() == vitExt) || (table.suffix() == smisExt)) {
                    patternType = MeasurementsType::Individual;
                } else {
                    patternType = MeasurementsType::Unknown;
                }

                QString filename;

                // Handling for Multisize measurements
                if (patternType == MeasurementsType::Multisize) {
                    const QString filter = tr("Multisize measurements") + QLatin1String(" (*.") + smmsExt +
                                            QLatin1String(" *.") + vstExt + QLatin1String(")");
                    // Use standard path to multisize measurements
                    QString dir = qApp->Seamly2DSettings()->getMultisizePath();
                    dir = VCommonSettings::prepareMultisizeTables(dir);
                    filename = fileDialog(this, tr("Open file"), dir, filter, nullptr, QFileDialog::DontUseNativeDialog,
                                            QFileDialog::ExistingFile, QFileDialog::AcceptOpen);
                }

                // Handling for Individual measurements
                else if (patternType == MeasurementsType::Individual) {
                    const QString filter = tr("Individual measurements") + QLatin1String(" (*.") + smisExt +
                                            QLatin1String(" *.") + vitExt + QLatin1String(")");
                    // Use standard path to individual measurements
                    const QString dir = qApp->Seamly2DSettings()->getIndividualSizePath();

                    bool usedNotExistedDir = false;

                    QDir directory(dir);
                    if (!directory.exists()) {
                        usedNotExistedDir = directory.mkpath(".");
                    }

                    filename = fileDialog(this, tr("Open file"), dir, filter, nullptr, QFileDialog::DontUseNativeDialog,
                                            QFileDialog::ExistingFile, QFileDialog::AcceptOpen);

                    if (usedNotExistedDir) {
                        QDir directory(dir);
                        directory.rmpath(".");
                    }
                }

                // Handling for Unknown measurements
                else {
                    const QString filter = tr("Individual measurements") + QLatin1String(" (*.") + smisExt +
                                            QLatin1String(" *.") + vitExt  + QLatin1String(");;") +
                                        tr("Multisize measurements")  + QLatin1String(" (*.") + smmsExt +
                                            QLatin1String(" *.") + vstExt  + QLatin1String(")");

                    // Use standard path to individual measurements
                    const QString dir = qApp->Seamly2DSettings()->getIndividualSizePath();
                    VCommonSettings::prepareMultisizeTables(VCommonSettings::getDefaultMultisizePath());

                    bool usedNotExistedDir = false;
                    QDir directory(path);
                    if (!directory.exists()) {
                        usedNotExistedDir = directory.mkpath(".");
                    }

                    filename = fileDialog(this, tr("Open file"), dir, filter, nullptr, QFileDialog::DontUseNativeDialog,
                                            QFileDialog::ExistingFile, QFileDialog::AcceptOpen);

                    if (usedNotExistedDir) {
                        QDir directory(dir);
                        directory.rmpath(".");
                    }
                }

                // Check if the filename is empty.
                if (filename.isEmpty()) {
                    return filename;
                } else {
                    QScopedPointer<MeasurementDoc> measurements(new MeasurementDoc(pattern));
                    measurements->setSize(VContainer::rsize());
                    measurements->setHeight(VContainer::rheight());
                    measurements->setXMLContent(filename);

                    patternType = measurements->Type();

                    // error checking - unknown format
                    if (patternType == MeasurementsType::Unknown) {
                        VException exception(tr("Measurement file has an unknown format."));
                        throw exception;
                    }

                    // update to new measurement file extensions (.smms, .smis)
                    if (patternType == MeasurementsType::Multisize) {
                        // Multisize measurements - Replace .vst extension with .smms extension
                        MultiSizeConverter converter(filename);
                        QString filename = converter.Convert();
                        if (filename.contains(".vst")) {
                            filename.replace(QString(".vst"), QString(".smms"));
                            QString error;
                            const bool result = measurements->SaveDocument(filename, error);
                            if (result) {
                                UpdateWindowTitle();
                            }
                        }
                        measurements->setXMLContent(filename); // Read again after conversion
                    } else {
                        // Individual measurements - Replace .vit extension with .smis extension
                        IndividualSizeConverter converter(filename);
                        QString filename = converter.Convert();
                        if (filename.contains(".vit")) {
                            filename.replace(QString(".vit"), QString(".smis"));
                            QString error;
                            const bool result = measurements->SaveDocument(filename, error);
                            if (result) {
                                UpdateWindowTitle();
                            }
                        }
                        measurements->setXMLContent(filename); // Read again after conversion
                    }

                    // error checking for invalid measurements
                    if (!measurements->eachKnownNameIsValid()) {
                        VException exception(tr("Measurement file contains invalid known measurement(s)."));
                        throw exception;
                    }

                    checkRequiredMeasurements(measurements.data());

                    qApp->setPatternType(patternType);

                    doc->SetMPath(RelativeMPath(patternPath, filename));
                    patternChangesWereSaved(false);
                    return filename;
                }
            }
        }
    }
    return path;
}
// End MainWindow::checkPathToMeasurements()

//---------------------------------------------------------------------------------------------------------------------
/**
 * @brief Changes the active draft block and updates the associated UI elements.
 * 
 * @param index The index of the draft block to be activated.
 * @param zoomBestFit A boolean indicating whether to zoom to the best fit after changing the draft block.
 * 
 * @details This function is responsible for changing the active draft block based on the provided index. It updates the current data,
 * emits a signal to refresh the history, and handles UI elements such as tool properties and groups. If the draw mode is active,
 * it ensures that the arrow tool is selected, and if specified, zooms to the best fit after the change.
 */
void MainWindow::changeDraftBlock(int index, bool zoomBestFit) {
    // Check if a valid index is provided.
    if (index != -1) {
        // Change the active draft block based on the provided index.
        doc->changeActiveDraftBlock(draftBlockComboBox->itemText(index));
        // Update the current data.
        doc->setCurrentData();
        // Emit a signal to refresh the history.
        emit RefreshHistory();
        
        // Check if draw mode is active.
        if (drawMode) {
            // Ensure the arrow tool is selected.
            handleArrowTool(true);
            // If specified, zoom to the best fit.
            if (zoomBestFit) {
                zoomToSelected();
            }
        }
        
        // Hide options for the tool in the previous pattern piece.
        toolProperties->itemClicked(nullptr);
        
        // Update groups in the widget.
        groupsWidget->updateGroups();
    }
}
// End MainWindow::changeDraftBlock()

//---------------------------------------------------------------------------------------------------------------------
/**
 * @brief Ends the visualization process and shows the corresponding dialog.
 * 
 * @param click A boolean parameter indicating whether a click event triggered the end of visualization.
 * 
 * @details This function is responsible for ending the visualization process and displaying 
 * the associated dialog. It checks if the dialog tool is not null,
 * and if so, it calls the `ShowDialog` function on the dialog tool, 
 * passing the provided boolean parameter to indicate whether the end was triggered
 * by a click event.
 */
void MainWindow::EndVisualization(bool click) {
    // Check if the dialog tool is not null.
    if (!dialogTool.isNull()) {
        // Call the ShowDialog function on the dialog tool, passing the click parameter.
        dialogTool->ShowDialog(click);
    }
}
// End MainWindow::EndVisualization()

//---------------------------------------------------------------------------------------------------------------------
/**
 * @brief Initializes the zoom settings for the first view display.
 * 
 * @details This function is responsible for setting up the initial zoom settings when the main window is first shown. 
 * The steps include:
 * - Checking if there are any pattern pieces in the data.
 * - If pattern pieces exist:
 *   - Shows the piece mode and calls zoomToFit to adjust the view.
 * - Checks if the draft mode is not checked and shows the draft mode.
 * - Calls zoomToSelected to focus on the selected elements.
 * - Updates the scene rectangles for the draft and piece scenes.
 * - If pattern pieces exist (again):
 *   - Shows the piece mode and calls zoomToFit for the second time to avoid coordinate changes after scaling or moving.
 * - Checks if the draft mode is not checked (again) and shows the draft mode.
 * 
 * @note: The comment in the code suggests that calling zoomToFit twice is a workaround to prevent 
 * visual issues when scaling or moving pattern pieces.
 */
void MainWindow::zoomFirstShow() {
    // Check if there are any pattern pieces in the data.
    if (pattern->DataPieces()->size() > 0) {
        // Show the piece mode and call zoomToFit to adjust the view.
        showPieceMode(true);
        ui->view->zoomToFit();
    }

    // Check if the draft mode is not checked and show the draft mode.
    if (!ui->showDraftMode->isChecked()) {
        showDraftMode(true);
    }

    // Call zoomToSelected to focus on the selected elements.
    zoomToSelected();

    // Update the scene rectangles for the draft and piece scenes.
    VMainGraphicsView::NewSceneRect(draftScene, ui->view);
    VMainGraphicsView::NewSceneRect(pieceScene, ui->view);

    // If pattern pieces exist (again):
    if (pattern->DataPieces()->size() > 0) {
        // Show the piece mode and call zoomToFit for the second time to avoid coordinate changes after scaling or moving.
        showPieceMode(true);
        ui->view->zoomToFit();
    }

    // If the draft mode is not checked (again), show the draft mode.
    if (!ui->showDraftMode->isChecked()) {
        showDraftMode(true);
    }
}
// End MainWindow::zoomFirstShow()

//---------------------------------------------------------------------------------------------------------------------
/**
 * @brief Initiates the export process based on the provided export parameters.
 * 
 * @details This function performs the export process using the given export parameters. It handles the following steps:
 * - Checks if the pattern is not in the opening state and contains pieces to export.
 * - Prepares the list of pieces for layout.
 * - Determines whether to export only pieces or the entire layout.
 * - In case of exporting only pieces:
 *   - Creates an ExportLayoutDialog with relevant parameters.
 *   - Calls the ExportData function with the prepared piece list and the dialog.
 *   - Catches and logs any export exceptions, exiting the application with an error if necessary.
 * - In case of exporting the entire layout:
 *   - Creates default generator settings based on export parameters.
 *   - Calls LayoutSettings to configure layout settings based on the generator settings.
 *   - Creates an ExportLayoutDialog with relevant parameters.
 *   - Calls the ExportData function with the prepared piece list and the dialog.
 *   - Catches and logs any export exceptions, exiting the application with an error if necessary.
 * 
 * @param expParams The export parameters provided by the command line.
 */
void MainWindow::DoExport(const VCommandLinePtr &expParams) {
    const QHash<quint32, VPiece> *pieces = pattern->DataPieces();

    // Check if the pattern is not in the opening state and contains pieces to export.
    if (!qApp->getOpeningPattern()) {
        if (pieces->count() == 0) {
            qCCritical(vMainWindow, "%s", qUtf8Printable(tr("You can't export an empty scene.")));
            qApp->exit(V_EX_DATAERR);
            return;
        }
    }

    // Prepare the list of pieces for layout.
    pieceList = preparePiecesForLayout(*pieces);

    const bool exportOnlyPieces = expParams->exportOnlyPieces();

    // Export only pieces.
    if (exportOnlyPieces) {
        try {
            ExportLayoutDialog dialog(1, Draw::Modeling, expParams->OptBaseName(), this);
            dialog.setDestinationPath(expParams->OptDestinationPath());
            dialog.selectFormat(static_cast<LayoutExportFormat>(expParams->OptExportType()));
            dialog.setBinaryDXFFormat(expParams->IsBinaryDXF());
            dialog.setTextAsPaths(expParams->isTextAsPaths());

            // Call ExportData function with the prepared piece list and the dialog.
            ExportData(pieceList, dialog);
        } catch (const VException &exception) {
            qCCritical(vMainWindow, "%s\n\n%s", qUtf8Printable(tr("Export exception.")), qUtf8Printable(exception.ErrorMessage()));
            qApp->exit(V_EX_DATAERR);
            return;
        }
    } else {
        auto settings = expParams->DefaultGenerator();

        // Check if the LayoutSettings are configured based on the generator settings.
        if (LayoutSettings(*settings.get())) {
            try {
                ExportLayoutDialog dialog(scenes.size(), Draw::Layout, expParams->OptBaseName(), this);
                dialog.setDestinationPath(expParams->OptDestinationPath());
                dialog.selectFormat(static_cast<LayoutExportFormat>(expParams->OptExportType()));
                dialog.setBinaryDXFFormat(expParams->IsBinaryDXF());

                // Call ExportData function with the prepared piece list and the dialog.
                ExportData(pieceList, dialog);
            } catch (const VException &exception) {
                qCCritical(vMainWindow, "%s\n\n%s", qUtf8Printable(tr("Export exception.")), qUtf8Printable(exception.ErrorMessage()));
                qApp->exit(V_EX_DATAERR);
                return;
            }
        } else {
            return;
        }
    }

    // Exit the application with success status.
    qApp->exit(V_EX_OK);
}
// End MainWindow::DoExport()

//---------------------------------------------------------------------------------------------------------------------
/**
 * @brief Sets the size parameter for the pattern.
 * 
 * @details This function sets the size parameter for the pattern based on the provided text value. It performs the following steps:
 * - Checks if the application is not in GUI mode.
 * - Checks if the window is modified or a file is open.
 * - Handles setting the size for a multisize pattern.
 * - Logs an error and returns false if the provided size value is not supported for the pattern.
 * 
 * @param text The text value representing the size to be set.
 * @return true if the size is set successfully; false otherwise.
 * 
 * @note This method does nothing in GUI mode and logs a warning.
 */
bool MainWindow::setSize(const QString &text) {
    // Check if the application is not in GUI mode.
    if (!VApplication::IsGUIMode()) {
        // Check if the window is modified or a file is open.
        if (this->isWindowModified() || not qApp->getFilePath().isEmpty()) {
            // Check if the pattern type is Multisize.
            if (qApp->patternType() == MeasurementsType::Multisize) {
                // Convert the size to the pattern unit and find the corresponding index in the gradation sizes dropdown.
                const int size = static_cast<int>(UnitConvertor(text.toInt(), Unit::Cm, *pattern->GetPatternUnit()));
                const qint32 index = gradationSizes->findText(QString().setNum(size));

                // Set the current index of the gradation sizes dropdown.
                if (index != -1) {
                    gradationSizes->setCurrentIndex(index);
                } else {
                    qCCritical(vMainWindow, "%s",
                                qUtf8Printable(tr("Not supported size value '%1' for this pattern file.").arg(text)));
                    return false;
                }
            } else {
                qCCritical(vMainWindow, "%s",
                            qUtf8Printable(tr("Couldn't set size. Need a file with multisize measurements.")));
                return false;
            }
        } else {
            qCCritical(vMainWindow, "%s", qUtf8Printable(tr("Couldn't set size. File wasn't opened.")));
            return false;
        }
    } else {
        // Log a warning for GUI mode and return false.
        qCWarning(vMainWindow, "%s", qUtf8Printable(tr("The method %1 does nothing in GUI mode").arg(Q_FUNC_INFO)));
        return false;
    }
    return true;
}
// End MainWindow::setSize()

//---------------------------------------------------------------------------------------------------------------------
/**
 * @brief Sets the height parameter for the pattern.
 * 
 * @details This function sets the height parameter for the pattern based on the provided text value. It performs the following steps:
 * - Checks if the application is not in GUI mode.
 * - Checks if the window is modified or a file is open.
 * - Handles setting the height for a multisize pattern.
 * - Logs an error and returns false if the provided height value is not supported for the pattern.
 * 
 * @param text The text value representing the height to be set.
 * @return true if the height is set successfully; false otherwise.
 * 
 * @note This method does nothing in GUI mode and logs a warning.
 */
bool MainWindow::setHeight(const QString &text) {
    // Check if the application is not in GUI mode.
    if (!VApplication::IsGUIMode()) {
        // Check if the window is modified or a file is open.
        if (this->isWindowModified() || not qApp->getFilePath().isEmpty()) {
            // Check if the pattern type is Multisize.
            if (qApp->patternType() == MeasurementsType::Multisize) {
                // Convert the height to the pattern unit and find the corresponding index in the gradation heights dropdown.
                const int height = static_cast<int>(UnitConvertor(text.toInt(), Unit::Cm, *pattern->GetPatternUnit()));
                const qint32 index = gradationHeights->findText(QString().setNum(height));

                // Set the current index of the gradation heights dropdown.
                if (index != -1) {
                    gradationHeights->setCurrentIndex(index);
                } else {
                    qCCritical(vMainWindow, "%s",
                                qUtf8Printable(tr("Not supported height value '%1' for this pattern file.").arg(text)));
                    return false;
                }
            } else {
                qCCritical(vMainWindow, "%s", qUtf8Printable(tr("Couldn't set height. File wasn't opened.")));
            return false;
        }
    } else {
        // Log a warning for GUI mode and return false.
        qCWarning(vMainWindow, "%s", qUtf8Printable(tr("The method %1 does nothing in GUI mode").arg(Q_FUNC_INFO)));
        return false;
    }
    return true;
}
// End MainWindow::setHeight()

//---------------------------------------------------------------------------------------------------------------------
/**
 * @brief Processes command line arguments and performs corresponding actions.
 * 
 * @details This function processes command line arguments provided to the application and performs actions such as loading
 * patterns, setting parameters, exporting, and handling errors. It is designed to be called when the application is
 * launched in command line mode.
 * 
 * @see VCommandLinePtr, qApp->CommandLine(), ReopenFilesAfterCrash(), LoadPattern(), setSize(), setHeight(), DoExport()
 */
void MainWindow::ProcessCMD() {
    // Retrieve command line arguments.
    const VCommandLinePtr cmd = qApp->CommandLine();
    auto args = cmd->OptInputFileNames();

    // Check if no scaling is enabled.
    isNoScaling = cmd->IsNoScalingEnabled();

    // Process command line arguments in GUI mode.
    if (VApplication::IsGUIMode()) {
        ReopenFilesAfterCrash(args);
    } else {
        // In console mode, check if only one input file is provided.
        if (args.size() != 1) {
            qCritical() << tr("Please, provide one input file.");
            qApp->exit(V_EX_NOINPUT);
            return;
        }
    }

    // Iterate through input files.
    for (int i = 0, sz = args.size(); i < sz; ++i) {
        // Load pattern and associated measurement file.
        const bool loaded = LoadPattern(args.at(static_cast<int>(i)), cmd->OptMeasurePath());

        // If not loaded and not in GUI mode, return after processing one input file.
        if (!loaded && not VApplication::IsGUIMode()) {
            return;
        }

        bool hSetted = true;
        bool sSetted = true;

        // In test mode or export mode, set gradation size and height if specified in the command line.
        if (loaded && (cmd->IsTestModeEnabled() || cmd->IsExportEnabled())) {
            if (cmd->IsSetGradationSize()) {
                sSetted = setSize(cmd->OptGradationSize());
            }

            if (cmd->IsSetGradationHeight()) {
                hSetted = setHeight(cmd->OptGradationHeight());
            }
        }

        // If not in test mode, check if export is enabled.
        if (!cmd->IsTestModeEnabled()) {
            if (cmd->IsExportEnabled()) {
                // Export pattern if loaded and gradation size and height are set.
                if (loaded && hSetted && sSetted) {
                    DoExport(cmd);
                    return; // Process only one input file
                } else {
                    qApp->exit(V_EX_DATAERR);
                    return;
                }
                break;
            }
        }
    }

    // In console mode, exit the application after processing.
    if (!VApplication::IsGUIMode()) {
        qApp->exit(V_EX_OK);
    }
}
// End MainWindow::ProcessCMD()

//---------------------------------------------------------------------------------------------------------------------
/**
 * @brief Retrieves and constructs the pattern file name portion of the window title.
 * 
 * @details This function constructs the pattern file name portion of the window title. If the application's file path is empty,
 * it sets the default name to "untitled.sm2d". Otherwise, it uses the application's file path as the file name.
 * An asterisk (*) is appended to the name to indicate that the pattern file has unsaved changes.
 * 
 * @return QString The pattern file name portion of the window title.
 * 
 * @see qApp->getFilePath()
 */
QString MainWindow::GetPatternFileName() {
    QString shownName = tr("untitled.sm2d");

    // Check if the application's file path is not empty.
    if (!qApp->getFilePath().isEmpty()) {
        shownName = qApp->getFilePath();
    }

    // Append an asterisk (*) to indicate unsaved changes.
    shownName += QLatin1String("[*]");
    return shownName;
}
// End MainWindow::GetPatternFileName()

//---------------------------------------------------------------------------------------------------------------------
/**
 * @brief Retrieves and constructs the measurement file name portion of the window title.
 * 
 * @details This function retrieves the measurement file name and constructs a formatted string to represent it in the window title.
 * If the measurement file path is empty, an empty string is returned. Otherwise, the measurement file name is enclosed in square brackets.
 * If the measurement file has unsaved changes, an asterisk (*) is appended to the name to indicate its modified status.
 * 
 * @return QString The measurement file name portion of the window title.
 * 
 * @see doc, AbsoluteMPath(), strippedName(), mChanges, qApp->getFilePath(), doc->MPath()
 */
QString MainWindow::GetMeasurementFileName() {
    // Check if the measurement file path is empty.
    if (doc->MPath().isEmpty()) {
        return "";
    } else {
        QString shownName(" - [");

        // Construct the measurement file name using AbsoluteMPath and strippedName.
        shownName += strippedName(AbsoluteMPath(qApp->getFilePath(), doc->MPath()));

        // Append an asterisk (*) if the measurement file has unsaved changes.
        if (mChanges) {
            shownName += QLatin1String("*");
        }

        shownName += QLatin1String("]");
        return shownName;
    }
}
// End MainWindow::GetMeasurementFileName()

//---------------------------------------------------------------------------------------------------------------------
/**
 * @brief Updates the main window title and icon based on the current pattern and measurement file information.
 * 
 * @details This function updates the title of the main window, including the application name, pattern file name, measurement file name,
 * and an indication of read-only status. It also sets the window icon based on the file status and modification state.
 * The title includes the application name, pattern file name, and measurement file name. If the pattern is read-only or the
 * file is not writable, it appends " - read only" to the title. The window path is set to the current file path.
 * On macOS, the window icon is updated based on the file's modification state.
 * 
 * @see patternReadOnly, setWindowTitle(), setWindowFilePath(), setWindowIcon(), GetPatternFileName(), GetMeasurementFileName(), darkenPixmap()
 */
void MainWindow::UpdateWindowTitle() {
    bool isFileWritable = true;

    // Check if the current file path is not empty.
    if (!qApp->getFilePath().isEmpty()) {
#ifdef Q_OS_WIN32
        // For Windows, temporarily turn on NTFS permission checking.
        qt_ntfs_permission_lookup++; 
#endif /*Q_OS_WIN32*/

        // Check if the file is writable using QFileInfo.
        isFileWritable = QFileInfo(qApp->getFilePath()).isWritable();

#ifdef Q_OS_WIN32
        // Turn off NTFS permission checking after use.
        qt_ntfs_permission_lookup--; 
#endif /*Q_OS_WIN32*/
    }

    // Construct the window title based on pattern, measurement, and read-only status.
    if (!patternReadOnly && isFileWritable) {
        setWindowTitle(VER_INTERNALNAME_STR + QString(" - ") + GetPatternFileName() + GetMeasurementFileName());
    } else {
        setWindowTitle(VER_INTERNALNAME_STR + QString(" - ") + GetPatternFileName() +
                        GetMeasurementFileName() + QString(" - ") + tr("read only"));
    }

    // Set the window path to the current file path.
    setWindowFilePath(qApp->getFilePath());

#if defined(Q_OS_MAC)
    // On macOS, set the window icon based on the file's modification state.
    static QIcon fileIcon = QIcon(QCoreApplication::applicationDirPath() +
                                    QLatin1String("/../Resources/Seamly2D.icns"));
    QIcon icon;

    // Check if the current file path is not empty.
    if (!qApp->getFilePath().isEmpty()) {
        if (!isWindowModified()) {
            icon = fileIcon;
        } else {
            // Darken the icon for modified files.
            static QIcon darkIcon;
            if (darkIcon.isNull()) {
                darkIcon = QIcon(darkenPixmap(fileIcon.pixmap(16, 16)));
            }
            icon = darkIcon;
        }
    }
    setWindowIcon(icon);
#endif //defined(Q_OS_MAC)
}

// End MainWindow::UpdateWindowTitle()

//---------------------------------------------------------------------------------------------------------------------
/**
 * @brief Updates the scenes in the main window.
 * 
 * @details This function updates the draft and piece scenes in the main window. If the draft scene (draftScene) and the
 * piece scene (pieceScene) are not null, it calls the update() function for each scene to refresh their content.
 * 
 * @note: The update() function is typically used to redraw the scene based on any changes in the underlying data.
 * 
 * @see draftScene, pieceScene, QGraphicsScene::update()
 */
void MainWindow::upDateScenes() {
    // Update the draft scene if it exists.
    if (draftScene) {
        draftScene->update();
    }

    // Update the piece scene if it exists.
    if (pieceScene) {
        pieceScene->update();
    }
}
// End MainWindow::upDateScenes()

//---------------------------------------------------------------------------------------------------------------------
/**
 * @brief Updates the state of view-related toolbar actions.
 * 
 * @details This function updates the state of various view-related actions in the main window's toolbar based on the
 * current application settings. It sets the checked state for actions such as wireframe view, display of control
 * points, axis origin, grainlines, seam allowances, and labels, reflecting the user's preferences.
 * 
 * @note: The checked state of each action is determined by querying the corresponding setting in the application.
 * 
 * @see ui->toggleWireframe_Action, ui->toggleControlPoints_Action, ui->toggleAxisOrigin_Action,
 *      ui->toggleGrainLines_Action, ui->toggleSeamAllowances_Action, ui->toggleLabels_Action,
 *      qApp->Settings(), qApp->Settings()->isWireframe(), qApp->Settings()->getShowControlPoints(),
 *      qApp->Settings()->getShowAxisOrigin(), qApp->Settings()->showGrainlines(),
 *      qApp->Settings()->showSeamAllowances(), qApp->Settings()->showLabels()
 */
void MainWindow::updateViewToolbar() {
    // Update the checked state of each action based on the corresponding application settings.
    ui->toggleWireframe_Action->setChecked(qApp->Settings()->isWireframe());
    ui->toggleControlPoints_Action->setChecked(qApp->Settings()->getShowControlPoints());
    ui->toggleAxisOrigin_Action->setChecked(qApp->Settings()->getShowAxisOrigin());
    ui->toggleGrainLines_Action->setChecked(qApp->Settings()->showGrainlines());
    ui->toggleSeamAllowances_Action->setChecked(qApp->Settings()->showSeamAllowances());
    ui->toggleLabels_Action->setChecked(qApp->Settings()->showLabels());
}
// End MainWindow::updateViewToolbar()

//---------------------------------------------------------------------------------------------------------------------
/**
 * @brief Resets pan shortcuts for the Zoom and Pan action.
 * 
 * @details This function modifies the shortcuts associated with the Zoom and Pan action in the main window.
 * It removes the Space key shortcut if panning with the Space key is not enabled in the application settings,
 * and adds it back if it is enabled. This function is typically called to update the shortcuts based on the
 * current application settings.
 * 
 * @note: The Zoom and Pan action may have multiple shortcuts associated with it, and this function modifies
 * the list of shortcuts accordingly.
 * 
 * @see QKeySequence, ui->zoomPan_Action, qApp->Seamly2DSettings()->isPanActiveSpaceKey()
 */
void MainWindow::resetPanShortcuts() {
    // Retrieve the current shortcuts associated with the Zoom and Pan action.
    QList<QKeySequence> zoomPanShortcuts = ui->zoomPan_Action->shortcuts();

    // Remove the Space key shortcut if panning with the Space key is not enabled in the application settings.
    zoomPanShortcuts.removeAll(QKeySequence(Qt::Key_Space));

    // Add back the Space key shortcut if panning with the Space key is enabled in the application settings.
    if (!qApp->Seamly2DSettings()->isPanActiveSpaceKey()) {
        zoomPanShortcuts.append(QKeySequence(Qt::Key_Space));
    }

    // Set the modified shortcuts back to the Zoom and Pan action.
    ui->zoomPan_Action->setShortcuts(zoomPanShortcuts);
}
// End MainWindow::resetPanShortcuts()

//---------------------------------------------------------------------------------------------------------------------
/**
 * @brief Handles locking errors and prompts the user for action.
 * 
 * @param error The type of locking error.
 * @param path The path of the file being locked.
 * @return true If the user chooses to ignore the locking error.
 * @return false If the user chooses to abort the operation.
 * 
 * @details This function is responsible for handling different types of locking errors that may occur when attempting
 * to lock a file. Depending on the error type, it displays a relevant warning or question dialog to the user,
 * allowing them to decide whether to ignore the locking error and continue or to abort the operation. The function
 * returns true if the user chooses to ignore the locking error and false if they choose to abort.
 * 
 * @param error The type of locking error (e.g., QLockFile::LockFailedError, QLockFile::PermissionError).
 * @param path The path of the file being locked.
 * @return true If the user chooses to ignore the locking error.
 * @return false If the user chooses to abort the operation.
 */
bool MainWindow::IgnoreLocking(int error, const QString &path) {
    QMessageBox::StandardButton answer = QMessageBox::Abort;

    // Display different dialogs based on the locking error type.
    if (VApplication::IsGUIMode()) {
        switch(error) {
            case QLockFile::LockFailedError:
                answer = QMessageBox::warning(this, tr("Locking file"),
                                                tr("This file is already open in another window. Ignore if you want "
                                                    "to continue (not recommended, can cause data corruption)."),
                                                QMessageBox::Abort | QMessageBox::Ignore, QMessageBox::Abort);
                break;
            case QLockFile::PermissionError:
                answer = QMessageBox::question(this, tr("Locking file"),
                                                tr("The lock file could not be created due to a lack of permissions. "
                                                    "Ignore if you want to continue (not recommended, can cause "
                                                    "data corruption)."),
                                                QMessageBox::Abort | QMessageBox::Ignore, QMessageBox::Abort);
                break;
            case QLockFile::UnknownError:
                answer = QMessageBox::question(this, tr("Locking file"),
                                                tr("An unknown error occurred, for instance, a full partition prevented "
                                                    "writing out the lock file. Ignore if you want to continue (not "
                                                    "recommended, can cause data corruption)."),
                                                QMessageBox::Abort | QMessageBox::Ignore, QMessageBox::Abort);
                break;
            default:
                answer = QMessageBox::Abort;
                break;
        }
    }

    // Handle the user's response to the locking error.
    if (answer == QMessageBox::Abort) {
        qCDebug(vMainWindow, "Failed to lock %s", qUtf8Printable(path));
        qCDebug(vMainWindow, "Error type: %d");
        Clear();

        // If not in GUI mode, log error details and exit the application.
        if (!VApplication::IsGUIMode()) {
            switch(error) {
                case QLockFile::LockFailedError:
                    qCCritical(vMainWindow, "%s",
                                qUtf8Printable(tr("This file is already open in another window.")));
                    break;
                case QLockFile::PermissionError:
                    qCCritical(vMainWindow, "%s",
                                qUtf8Printable(tr("The lock file could not be created, for a lack of permissions.")));
                    break;
                case QLockFile::UnknownError:
                    qCCritical(vMainWindow, "%s",
                                qUtf8Printable(tr("An unknown error occurred, for instance, a full partition prevented "
                                                    "writing out the lock file.")));
                    break;
                default:
                    break;
            }
            qApp->exit(V_EX_NOINPUT);
        }
        return false;
    }

    return true;
}

// End MainWindow::IgnoreLocking()

//---------------------------------------------------------------------------------------------------------------------
/**
 * @brief Retrieves a sorted and deduplicated list of names of draft points in the current pattern.
 * 
 * @return QStringList A QStringList containing names of draft points in the current pattern.
 * 
 * @details This function iterates through the draft objects in the current pattern, specifically looking for
 * objects of type 'Point' (GOType::Point). It collects the names of these points into a QStringList,
 * sorts the list, removes any duplicate entries, and then returns the final sorted and deduplicated
 * list of point names.
 */
QStringList MainWindow::draftPointNamesList() {
    QStringList pointNames;

    // Iterate through draft objects in the current pattern.
    for (QHash<quint32, QSharedPointer<VGObject>>::const_iterator item = pattern->DataGObjects()->begin();
            item != pattern->DataGObjects()->end();
            ++item) {
        // Check if the object is of type 'Point' and not already in the list.
        if (item.value()->getType() == GOType::Point && !pointNames.contains(item.value()->name()))
            pointNames << item.value()->name();
    }

    // Sort the list of point names and remove any duplicate entries.
    pointNames.sort();
    pointNames.removeDuplicates();

    // Return the final sorted and deduplicated list of point names.
    return pointNames;
}
// End MainWindow::draftPointNamesList()

//---------------------------------------------------------------------------------------------------------------------
/**
 * @brief Updates the zoom-to-point combo box with a new list of point names.
 * 
 * @param namesList A QStringList containing names of points to be displayed in the combo box.
 * 
 * @details This function updates the zoom-to-point combo box by first blocking signals to prevent
 * unintended actions triggered by the UI update. It then clears the existing items in the
 * combo box and adds new items from the provided QStringList. Finally, it unblocks signals
 * to allow normal UI interaction.
 */
void MainWindow::updateZoomToPointComboBox(QStringList namesList) {
    // Prevent unintended actions triggered by UI update.
    m_zoomToPointComboBox->blockSignals(true);
    
    // Clear existing items in the combo box and add new items from the QStringList.
    m_zoomToPointComboBox->clear();
    m_zoomToPointComboBox->addItems(namesList);
    
    // Unblock signals to allow normal UI interaction.
    m_zoomToPointComboBox->blockSignals(false);
}
// End MainWindow::updateZoomToPointComboBox()

//---------------------------------------------------------------------------------------------------------------------
/**
 * @brief Configures the environment for selecting points on the canvas.
 * 
 * @details This function sets up the UI environment for selecting points by disabling the
 * selection of labels, lines, arcs, splines, etc., while enabling point selection.
 * It also adjusts the hovering behavior to highlight points and disables rubber band selection.
 */
void MainWindow::ToolSelectPoint() const {
    // Disable selection for various elements.
    emit EnableLabelSelection(false);
    emit EnablePointSelection(true);
    emit EnableLineSelection(false);
    emit EnableArcSelection(false);
    emit EnableElArcSelection(false);
    emit EnableSplineSelection(false);
    emit EnableSplinePathSelection(false);

    // Enable hovering for points and disable hovering for other elements.
    emit EnableLabelHover(false);
    emit EnablePointHover(true);
    emit EnableLineHover(false);
    emit EnableArcHover(false);
    emit EnableElArcHover(false);
    emit EnableSplineHover(false);
    emit EnableSplinePathHover(false);

    // Disable rubber band selection.
    ui->view->allowRubberBand(false);
}
// End MainWindow::ToolSelectPoint()

//---------------------------------------------------------------------------------------------------------------------
/**
 * @brief Configures the environment for selecting points on mouse release.
 * 
 * @details This function sets up the UI environment for selecting points by calling
 * ToolSelectPoint() to configure point selection settings. Additionally, it emits
 * a signal to perform item selection based on the mouse release event.
 */
void MainWindow::ToolSelectPointByRelease() const {
    // Configure the environment for point selection.
    ToolSelectPoint();
    
    // Emit signal to perform item selection on mouse release.
    emit ItemsSelection(SelectionType::ByMouseRelease);
}
// End MainWindow::ToolSelectPointByRelease()

//---------------------------------------------------------------------------------------------------------------------
/**
 * @brief Initiates the tool for selecting points on the canvas by pressing the mouse.
 * 
 * @details This function configures the environment for selecting points by calling the general
 * point selection tool and specifying the selection type as "ByMousePress" for immediate
 * selection upon mouse press.
 */
void MainWindow::ToolSelectPointByPress() const {
    // Initiate the general point selection tool.
    ToolSelectPoint();

    // Specify the selection type as "ByMousePress" for immediate selection upon mouse press.
    emit ItemsSelection(SelectionType::ByMousePress);
}
// End MainWindow::ToolSelectPointByPress()

//---------------------------------------------------------------------------------------------------------------------
/**
 * @brief Initiates the tool for selecting spline curves.
 * 
 * @details This function configures the environment for selecting spline curves by disabling
 * other individual object types, enabling hovering for relevant types, and specifying
 * the selection type as "ByMouseRelease" for a more controlled selection behavior. It also
 * sets up the view to disallow rubber band selection.
 */
void MainWindow::ToolSelectSpline() const {
    // Disable selection for individual object types.
    emit EnableLabelSelection(false);
    emit EnablePointSelection(false);
    emit EnableLineSelection(false);
    emit EnableArcSelection(false);
    emit EnableElArcSelection(false);
    emit EnableSplineSelection(false);
    emit EnableSplinePathSelection(false);

    // Enable hovering for relevant object types.
    emit EnableLabelHover(false);
    emit EnablePointHover(false);
    emit EnableLineHover(false);
    emit EnableArcHover(false);
    emit EnableElArcHover(false);
    emit EnableSplineHover(true);
    emit EnableSplinePathHover(false);

    // Specify the selection type as "ByMouseRelease" for controlled selection behavior.
    emit ItemsSelection(SelectionType::ByMouseRelease);

    // Disallow rubber band selection in the view.
    ui->view->allowRubberBand(false);
}
// End MainWindow::ToolSelectSpline()

//---------------------------------------------------------------------------------------------------------------------
/**
 * @brief Initiates the tool for selecting spline paths.
 * 
 * @details This function configures the environment for selecting spline paths by disabling
 * other individual object types, enabling hovering for relevant types, and specifying
 * the selection type as "ByMouseRelease" for a more controlled selection behavior. It also
 * sets up the view to disallow rubber band selection.
 */
void MainWindow::ToolSelectSplinePath() const {
    // Disable selection for individual object types.
    emit EnableLabelSelection(false);
    emit EnablePointSelection(false);
    emit EnableLineSelection(false);
    emit EnableArcSelection(false);
    emit EnableElArcSelection(false);
    emit EnableSplineSelection(false);
    emit EnableSplinePathSelection(false);

    // Enable hovering for relevant object types.
    emit EnableLabelHover(false);
    emit EnablePointHover(false);
    emit EnableLineHover(false);
    emit EnableArcHover(false);
    emit EnableElArcHover(false);
    emit EnableSplineHover(false);
    emit EnableSplinePathHover(true);

    // Specify the selection type as "ByMouseRelease" for controlled selection behavior.
    emit ItemsSelection(SelectionType::ByMouseRelease);

    // Disallow rubber band selection in the view.
    ui->view->allowRubberBand(false);
}
// End MainWindow::ToolSelectSplinePath()

//---------------------------------------------------------------------------------------------------------------------
/**
 * @brief Initiates the tool for selecting arcs.
 * 
 * @details This function configures the environment for selecting arcs by disabling
 * other individual object types, enabling hovering for relevant types, and specifying
 * the selection type as "ByMouseRelease" for a more controlled selection behavior. It also
 * sets up the view to disallow rubber band selection.
 */
void MainWindow::ToolSelectArc() const {
    // Disable selection for individual object types.
    emit EnableLabelSelection(false);
    emit EnablePointSelection(false);
    emit EnableLineSelection(false);
    emit EnableArcSelection(false);
    emit EnableElArcSelection(false);
    emit EnableSplineSelection(false);
    emit EnableSplinePathSelection(false);

    // Enable hovering for relevant object types.
    emit EnableLabelHover(false);
    emit EnablePointHover(false);
    emit EnableLineHover(false);
    emit EnableArcHover(true);
    emit EnableElArcHover(false);
    emit EnableSplineHover(false);
    emit EnableSplinePathHover(false);

    // Specify the selection type as "ByMouseRelease" for controlled selection behavior.
    emit ItemsSelection(SelectionType::ByMouseRelease);

    // Disallow rubber band selection in the view.
    ui->view->allowRubberBand(false);
}
// End MainWindow::ToolSelectArc()

//---------------------------------------------------------------------------------------------------------------------
/**
 * @brief Initiates the tool for selecting a point on an arc.
 * 
 * @details This function configures the environment for selecting a point on an arc by disabling
 * other individual object types, enabling hovering for relevant types, and specifying
 * the selection type as "ByMouseRelease" for a more controlled selection behavior. It also
 * sets up the view to disallow rubber band selection.
 */
void MainWindow::ToolSelectPointArc() const {
    // Disable selection for individual object types.
    emit EnableLabelSelection(false);
    emit EnablePointSelection(false);
    emit EnableLineSelection(false);
    emit EnableArcSelection(false);
    emit EnableElArcSelection(false);
    emit EnableSplineSelection(false);
    emit EnableSplinePathSelection(false);

    // Enable hovering for relevant object types.
    emit EnableLabelHover(true);
    emit EnablePointHover(true);
    emit EnableLineHover(false);
    emit EnableArcHover(true);
    emit EnableElArcHover(false);
    emit EnableSplineHover(false);
    emit EnableSplinePathHover(false);

    // Specify the selection type as "ByMouseRelease" for controlled selection behavior.
    emit ItemsSelection(SelectionType::ByMouseRelease);

    // Disallow rubber band selection in the view.
    ui->view->allowRubberBand(false);
}
// End MainWindow::ToolSelectPointArc()

//---------------------------------------------------------------------------------------------------------------------
/**
 * @brief Initiates the tool for selecting curve objects.
 * 
 * @details This function configures the environment for selecting curve objects by disabling
 * other individual object types and enabling hovering for curves. It also sets up the
 * view to disallow rubber band selection and specifies the selection type as "ByMouseRelease"
 * for a more controlled selection behavior.
 */
void MainWindow::ToolSelectCurve() const {
    // Disable selection for individual object types.
    emit EnableLabelSelection(false);
    emit EnablePointSelection(false);
    emit EnableLineSelection(false);
    emit EnableArcSelection(false);
    emit EnableElArcSelection(false);
    emit EnableSplineSelection(false);
    emit EnableSplinePathSelection(false);

    // Disable hovering for non-curve object types.
    emit EnableLabelHover(false);
    emit EnablePointHover(false);
    emit EnableLineHover(false);

    // Enable hovering for curve object types.
    emit EnableArcHover(true);
    emit EnableElArcHover(true);
    emit EnableSplineHover(true);
    emit EnableSplinePathHover(true);

    // Specify the selection type as "ByMouseRelease" for controlled selection behavior.
    emit ItemsSelection(SelectionType::ByMouseRelease);

    // Disallow rubber band selection in the view.
    ui->view->allowRubberBand(false);
}
// End void MainWindow::ToolSelectCurve()

//---------------------------------------------------------------------------------------------------------------------
/**
 * @brief Initiates the tool for selecting all draft objects.
 * 
 * @details This function configures the environment for selecting all draft objects by disabling
 * individual object types and enabling hovering for different types. It also sets up the
 * view to disallow rubber band selection and specifies the selection type as "ByMouseRelease"
 * for a more controlled selection behavior.
 */
void MainWindow::selectAllDraftObjectsTool() const {
    // Disable selection for individual object types.
    emit EnableLabelSelection(false);
    emit EnablePointSelection(false);
    emit EnableLineSelection(false);
    emit EnableArcSelection(false);
    emit EnableElArcSelection(false);
    emit EnableSplineSelection(false);
    emit EnableSplinePathSelection(false);

    // Enable hovering for various draft objects.
    emit EnableLabelHover(true);
    emit EnablePointHover(true);
    emit EnableLineHover(false);  // Not allowing line hovering by default.
    emit EnableArcHover(true);
    emit EnableElArcHover(true);
    emit EnableSplineHover(true);
    emit EnableSplinePathHover(true);

    // Specify the selection type as "ByMouseRelease" for controlled selection behavior.
    emit ItemsSelection(SelectionType::ByMouseRelease);

    // Disallow rubber band selection in the view.
    ui->view->allowRubberBand(false);
}
// End MainWindow::selectAllDraftObjectsTool()

//---------------------------------------------------------------------------------------------------------------------
/**
 * @brief Initiates the tool for selecting operation objects.
 * 
 * @details This function configures the environment for selecting various operation objects,
 * including enabling or disabling selection and hovering for different types of objects.
 * It also sets up the view to allow rubber band selection and specifies the selection
 * type as "ByMouseRelease" for a more controlled selection behavior.
 */
void MainWindow::ToolSelectOperationObjects() const {
    // Enable selection for various operation objects.
    emit EnableLabelSelection(true);
    emit EnablePointSelection(true);
    emit EnableLineSelection(false);  // Not allowing line selection by default.
    emit EnableArcSelection(true);
    emit EnableElArcSelection(true);
    emit EnableSplineSelection(true);
    emit EnableSplinePathSelection(true);

    // Enable hovering for various operation objects.
    emit EnableLabelHover(true);
    emit EnablePointHover(true);
    emit EnableLineHover(false);  // Not allowing line hovering by default.
    emit EnableArcHover(true);
    emit EnableElArcHover(true);
    emit EnableSplineHover(true);
    emit EnableSplinePathHover(true);

    // Specify the selection type as "ByMouseRelease" for controlled selection behavior.
    emit ItemsSelection(SelectionType::ByMouseRelease);

    // Allow rubber band selection in the view.
    ui->view->allowRubberBand(true);
}
// End MainWindow::ToolSelectOperationObjects()

//---------------------------------------------------------------------------------------------------------------------
/**
 * @brief Initiates the tool for selecting group objects.
 * 
 * @details This function sets up the environment for selecting group objects, including
 * configuring the rubber band selection and enabling hovering for improved user interaction.
 */
void MainWindow::ToolSelectGroupObjects() const {
    // Prepare for selecting group objects by selecting operation objects.
    ToolSelectOperationObjects();

    // Enable line selection, typically used for rubber band selection.
    emit EnableLineSelection(true);

    // Enable hovering for lines to enhance user interaction.
    emit EnableLineHover(true);
}
// End MainWindow::ToolSelectGroupObjects()

//---------------------------------------------------------------------------------------------------------------------
/**
 * @brief Initiates the piece selection tool.
 * 
 * @details This function configures the environment for selecting pattern pieces.
 */
void MainWindow::selectPieceTool() const
{
    // Disable node label and point selection during piece selection.
    emit EnableNodeLabelSelection(false);
    emit EnableNodePointSelection(false);
    
    // Enable piece selection (disable when done with pattern piece visualization).
    emit enablePieceSelection(true);

    // Enable hovering for node labels, node points, and pieces.
    emit EnableNodeLabelHover(true);
    emit EnableNodePointHover(true);
    emit enablePieceHover(true);

    // Set the selection type to be triggered by mouse release.
    emit ItemsSelection(SelectionType::ByMouseRelease);

    // Disable rubber band selection in the view.
    ui->view->allowRubberBand(false);
}
// End MainWindow::selectPieceTool()