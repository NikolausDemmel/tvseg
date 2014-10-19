#include "tvsegwindow.h"

#include "ui_tvsegwindow.h"
#include "util/valuetransition.h"
#include "algorithmwidget.h"
#include "tvseg/util/opencvhelper.h"
#include "tvseg/segmentationimpl.h"
#include "tvseg/feedbackbase.h"
#include "tvseg/settings/serializerqt.h"
#include "tvseg/util/logging.h"
#include "tvseg/util/helpers.h"
#include "tvseg/settingsdef/parzensettings.h"
#include "tvseg/settingsdef/weightsettings.h"

#include <QMessageBox>
#include <QCloseEvent>
#include <QFileDialog>

#include <QtConcurrent>




namespace tvseg_ui {




// TODO: Sychronize when calling segmentation methods in a worker thread. This includes access to logging, as well as the settings (backend, value objects).


namespace _detail {


void displayImageHelper(FeedbackWidget* w, const float* data, tvseg::Dim3 dim, std::string windowName) {
    if (!w) {
        LWARNING << "displayImageHelper received NULL widget";
        return;
    }

    if (!data) {
        LWARNING << "displayImageHelper received NULL data";
        return;
    }

    w->displayCVImageAsync(tvseg::createMat(data, dim), windowName);
}


}


TVSegWindow::TVSegWindow(QWidget *parent, std::string settingsFilename) :
    QMainWindow(parent),
    ui_(new Ui::TVSegWindow),
    settingsFilename_(settingsFilename),
    settingsBackend_(new tvseg::settings::Backend()),
    settingsSerializer_(new tvseg::settings::SerializerQt(settingsBackend_, settingsFilename_, true)),
    uiSettings_(new UiSettings(settingsBackend_)),
    inputSettings_(new tvseg::InputSettings(settingsBackend_)),
    outputSettings_(new tvseg::OutputSettings(settingsBackend_)),
    segmentationSettings_(new tvseg::SegmentationSettings(settingsBackend_)),
    windowSettings_(new WindowSettings(settingsBackend_)),
    segmentation_(new tvseg::SegmentationImpl(settingsBackend_)),
    consoleDock_(new ConsoleDockWidget(this)),
    feedbackDock_(new FeedbackDockWidget(this)),
    mainSettingsEditor_(new SettingsEditor(this)),
    solverWidget_(new AlgorithmWidget()),
    stateMachine_(new QStateMachine(this)),
    algorithmRunning_(false),
    cancelRequested_(false)
{
    ui_->setupUi(this);
    addDockWidget(Qt::BottomDockWidgetArea, consoleDock_);

    easyloggingpp::Loggers::getLogger("trivial")->registerDispatchCallback("gui_console",
        boost::bind(&TVSegWindow::addConsoleMessageAsync, this, _1, _2));

    addDockWidget(Qt::RightDockWidgetArea, feedbackDock_);
    ui_->menuWindow->addAction(consoleDock_->toggleViewAction());
    ui_->menuWindow->addAction(feedbackDock_->toggleViewAction());
    QMenu *fileMenu = menuBar()->addMenu("&File");
    QAction *quitAction = new QAction("&Quit", this); // on OS X this will be merged into the application menu
    fileMenu->addAction(quitAction);
    connect(quitAction, SIGNAL(triggered()), this, SLOT(close()));

    // image display
    ui_->inputDisplay->setLabels(seg()->labels());

    // feedback
    tvseg::FeedbackBasePtr fb(new tvseg::FeedbackBase());
    fb->setDisplayImageFn(boost::bind(&_detail::displayImageHelper, getFeedbackWidget(), _1, _2, _3));
    fb->setProgressFn(boost::bind(&FeedbackWidget::updateProgressAsync, getFeedbackWidget(), _1, _2, _3));
    fb->setCancelFlag(&cancelRequested_);
    seg()->setFeedback(fb);

    connect(getFeedbackWidget(), SIGNAL(cancelRequested()), this, SLOT(setCancelRequested()));
    connect(this, SIGNAL(algorithmRunningChanged(bool)), getFeedbackWidget(), SLOT(setRunning(bool)));

    // state machine
    QState *sIdle = new QState();
    QState *sRunning = new QState();
    sRunning->assignProperty(ui_->centralWidget, "enabled", false);
    sRunning->assignProperty(this, "cancelRequested", false);

    sIdle->addTransition(util::ValueTransition<bool>::make(this, SIGNAL(algorithmRunningChanged(bool)), true, sRunning));
    sRunning->addTransition(util::ValueTransition<bool>::make(this, SIGNAL(algorithmRunningChanged(bool)), false, sIdle));

    stateMachine_->setGlobalRestorePolicy(QStateMachine::RestoreProperties);
    stateMachine_->addState(sIdle);
    stateMachine_->addState(sRunning);
    stateMachine_->setInitialState(algorithmRunning() ? sRunning : sIdle);
    stateMachine_->start();

    // algorithm widgets
    ui_->mainTabWidget->addTab(solverWidget_, "Solver");
    solverWidget_->addAlgorithm(seg()->weightInterface());
    solverWidget_->addAlgorithm(seg()->datatermInterface());
    solverWidget_->addAlgorithm(seg()->solverInterface());
    solverWidget_->addAlgorithm(seg()->visualizerInterface());
    connect(solverWidget_, SIGNAL(algorithmStarted()), this, SLOT(algorithmStarted()));
    connect(solverWidget_, SIGNAL(algorithmStopped()), this, SLOT(algorithmStopped()));
    connect(solverWidget_, SIGNAL(runAll()), this, SLOT(runAllAlgorithms()));
    connect(solverWidget_, SIGNAL(saveResult()), this, SLOT(saveResult()));
    connect(solverWidget_, SIGNAL(computeMetrics()), this, SLOT(computeMetrics()));
    connect(solverWidget_, SIGNAL(setPreset(QString)), this, SLOT(setPreset(QString)));
    addPresets();

    if (uiSettings()->autoRun()) {
        ui_->mainTabWidget->setCurrentWidget(solverWidget_);
    }

    // settings editor
    ui_->inputSidePane->addWidget(mainSettingsEditor_, 1);
    connect(mainSettingsEditor_, SIGNAL(valueChanged(QtProperty*,QVariant)), this, SLOT(settingsValueChanged(QtProperty*,QVariant)));
    mainSettingsEditor_->addEntries(*uiSettings_);
    mainSettingsEditor_->addEntries(*segmentationSettings_);
    mainSettingsEditor_->addEntries(*inputSettings_);
    mainSettingsEditor_->addEntries(*outputSettings_);

    // update GUI from settings
    updateGUIFromSettings();
    windowStateFromSettings();
}

TVSegWindow::~TVSegWindow()
{
    easyloggingpp::Loggers::getLogger("trivial")->unregisterDispatchCallback("gui_console");
    delete ui_;
}

//void TVSegWindow::setSettings(tvseg::settings::BackendPtr value, std::string filename)
//{
//    settingsBackend_ = value;
//    settingsSerializer_.reset(new tvseg::settings::SerializerQt(settingsBackend_, filename));
//    //TODO: replace backend
//    // settings_->replaceBackend();
//    settings_.reset(new tvseg::InputSettings(value));

//    seg()->setSettingsBackend(value);

//    updateGUIFromSettings();
//}

tvseg::InputSettings::Ptr TVSegWindow::inputSettings()
{
    return inputSettings_;
}

tvseg::InputSettings::ConstPtr TVSegWindow::inputSettings() const
{
    return inputSettings_;
}

tvseg::OutputSettings::Ptr TVSegWindow::outputSettings()
{
    return outputSettings_;
}

tvseg::OutputSettings::ConstPtr TVSegWindow::outputSettings() const
{
    return outputSettings_;
}

UiSettings::Ptr TVSegWindow::uiSettings()
{
    return uiSettings_;
}

UiSettings::ConstPtr TVSegWindow::uiSettings() const
{
    return uiSettings_;
}

tvseg::SegmentationSettings::Ptr TVSegWindow::segmentationSettings()
{
    return segmentationSettings_;
}

tvseg::SegmentationSettings::ConstPtr TVSegWindow::segmentationSettings() const
{
    return segmentationSettings_;
}

TVSegWindow::WindowSettings::Ptr TVSegWindow::windowSettings()
{
    return windowSettings_;
}

TVSegWindow::WindowSettings::ConstPtr TVSegWindow::windowSettings() const
{
    return windowSettings_;
}

tvseg::SegmentationPtr TVSegWindow::seg()
{
    return segmentation_;
}

tvseg::SegmentationConstPtr TVSegWindow::seg() const
{
    return segmentation_;
}

void TVSegWindow::closeEvent(QCloseEvent *event)
{
    // TODO: capture close applciation event
    // TODO: check if algorithm is running, send cancel, allow force quit.
    // TODO: menu with saveSettings

    QMessageBox::StandardButton resBtn = QMessageBox::question( this, windowTitle(),
                                                                tr("Do you really want to close?"),
                                                                QMessageBox::No | QMessageBox::Yes,
                                                                QMessageBox::Yes);
    if (resBtn != QMessageBox::Yes) {
        event->ignore();
    } else {
//        if (algorithmRunning()) {

//        }
        windowStateToSettings();
        on_saveSettingsButton_clicked();
        QMainWindow::closeEvent(event);
    }
}

void TVSegWindow::initFromSettings()
{
    on_loadImageButton_clicked();
    on_displayLabelsButton_clicked();
    if (uiSettings()->autoRun()) {
        runAllAlgorithms();
    }
}

void TVSegWindow::addConsoleMessageAsync(uint level, const std::string &msg)
{
    using easyloggingpp::Level;
    QColor color;
    switch (level) {
    case Level::Info:
        color = QColor(11, 36, 251); // blue
        break;
    case Level::Warning:
        color = QColor(253, 164, 40); // orange
        break;
    case Level::Debug:
        color = QColor(15, 127, 18); // green
        break;
    case Level::Error:
    case Level::Fatal:
        color = QColor(252, 13, 27); // red
        break;
    default:
        color = Qt::black;
        break;
    }

    QMetaObject::invokeMethod(consoleDock_, "addMessage", Qt::QueuedConnection, Q_ARG(QString, QString::fromStdString(msg)), Q_ARG(QColor, color));
}

bool TVSegWindow::algorithmRunning()
{
    return algorithmRunning_;
}

bool TVSegWindow::cancelRequested()
{
    return cancelRequested_;
}

void TVSegWindow::on_saveSettingsButton_clicked()
{
    setAndSaveScribbles();
    if (settingsSerializer_) {
        settingsSerializer_->save();
    } else {
        LERROR << "No SettingsSerializer instantiated.";
    }
}

void TVSegWindow::on_loadImageButton_clicked()
{
    seg()->loadInput();

    updateGUIFromSettings();
    updateSettingsEditors();

    if (seg()->inputImageColorAvailable()) {
        ui_->inputDisplay->setImage(seg()->inputImageColor());
    }

    setScribblesToDisplayWidget(seg()->scribbles());
}

void TVSegWindow::updateGUIFromSettings()
{
    ui_->inputDisplay->setBrushSize(uiSettings()->scribbleBrushSize());
    ui_->inputDisplay->setBrushDensity(uiSettings()->scribbleBrushDensity());
}

void TVSegWindow::updateSettingsEditors()
{
    mainSettingsEditor_->updateFromSettings();
    solverWidget_->updateFromSettings();
}

void TVSegWindow::windowStateToSettings()
{
    QByteArray bytes = saveGeometry();
    WindowSettings::byte_array_t value(bytes.data(), bytes.data() + bytes.length());
    windowSettings()->setGeometry(value);
    bytes = saveState();
    value.assign(bytes.data(), bytes.data() + bytes.length());
    windowSettings()->setWindowState(value);
}

void TVSegWindow::windowStateFromSettings()
{
    restoreGeometry(QByteArray::fromRawData(windowSettings()->geometry().data(), windowSettings()->geometry().size()));
    restoreState(QByteArray::fromRawData(windowSettings()->windowState().data(), windowSettings()->windowState().size()));
}

void TVSegWindow::displayKMeans()
{
    if (seg()->kMeansAvailable()) {
        ui_->labelsDisplay->setImage(seg()->kMeans());
    } else {
        LWARNINGF("Cannot display %d means.", segmentationSettings()->numLabels());
    }
}

void TVSegWindow::displayLabels()
{
    if (seg()->labelsAvailable()) {
        ui_->labelsDisplay->setImage(seg()->labels());
    } else {
        LWARNINGF("Cannot display %d labels.", segmentationSettings()->numLabels());
    }
}

void TVSegWindow::setAlgorithmRunning(bool value)
{
    if (algorithmRunning_ != value) {
        algorithmRunning_ = value;
        emit algorithmRunningChanged(value);
    }
}

void TVSegWindow::setCancelRequested(bool value)
{
    if (cancelRequested_ != value) {
        cancelRequested_ = value;
        emit cancelRequestedChanged(value);
    }
}

void TVSegWindow::algorithmStarted()
{
    setAlgorithmRunning(true);
}

void TVSegWindow::algorithmStopped()
{
    setAlgorithmRunning(false);
    // invoke next algorithm from event queue, not directly
    QMetaObject::invokeMethod(this, "invokeNextAlgorithm", Qt::QueuedConnection);
}

void TVSegWindow::settingsValueChanged(QtProperty *property, QVariant /*value*/)
{
    if (property->propertyName() == "numLabels") {
        displayLabels();
        ui_->inputDisplay->setLabels(seg()->labels());
    }
    if (property->propertyName() == "scribbleBrushSize" ||
        property->propertyName() == "scribbleBrushDensity")
    {
        updateGUIFromSettings();
    }

//    LDEBUG << "tvsegwindow: Settings value changed " << property->propertyName() << ": '" << value.toString() << "'";
}

void TVSegWindow::runAllAlgorithms()
{
    algorithm_queue_t functions;
    functions.append(boost::bind(&AlgorithmWidget::computeButtonClicked, solverWidget_, 0));
    functions.append(boost::bind(&AlgorithmWidget::computeButtonClicked, solverWidget_, 1));
    functions.append(boost::bind(&AlgorithmWidget::computeButtonClicked, solverWidget_, 2));
    functions.append(boost::bind(&AlgorithmWidget::computeButtonClicked, solverWidget_, 3));
    invokeAlgorithmsSequentially(functions);
}

void TVSegWindow::saveResult()
{
    seg()->saveResult();
}

void TVSegWindow::computeMetrics()
{
    std::vector<float> metrics = seg()->computeMetrics();
    if (metrics.size() > 0) {
        std::stringstream ss;
        for (uint i = 0; i < metrics.size(); ++i) {
            ss << metrics[i] << " ";
        }
        LINFO << "Metrics: " << ss.str();
    }
}

FeedbackWidget *TVSegWindow::getFeedbackWidget()
{
    return feedbackDock_->getFeedbackWidget();
}

void TVSegWindow::invokeAlgorithmsSequentially(TVSegWindow::algorithm_queue_t queue)
{
    algorithmQueue_ = queue;
    invokeNextAlgorithm();
}

tvseg::Scribbles TVSegWindow::getScribblesFromDisplayWidget()
{
    QVector<QVector<QPoint> > scribbles = ui_->inputDisplay->getScribbles();
    tvseg::scribble_list_type list;
    list.resize(scribbles.size());
    for (int i = 0; i < scribbles.size(); ++i) {
        for (int j = 0; j < scribbles[i].size(); ++j) {
            list[i].push_back(cv::Point(scribbles[i][j].x(), scribbles[i][j].y()));
        }
    }
    return tvseg::Scribbles(list);
}

void TVSegWindow::setScribblesToDisplayWidget(const tvseg::Scribbles &input)
{
    const tvseg::scribble_list_type &scribbles = input.get();
    QVector<QVector<QPoint> > tmp;
    tmp.resize(scribbles.size());
    for (uint i = 0; i < scribbles.size(); ++i) {
        for (uint j = 0; j < scribbles[i].size(); ++j) {
            tmp[i].push_back(QPoint(scribbles[i][j].x, scribbles[i][j].y));
        }
    }
    ui_->inputDisplay->setScribbles(tmp);
}

void TVSegWindow::setAndSaveScribbles()
{
    tvseg::Scribbles scribbles = getScribblesFromDisplayWidget();
    seg()->setScribbles(scribbles);
    // always save to local scribble file to make scribble editing persistent,
    // and only save to instance specific file upon manual request by user
    if (uiSettings()->saveScribbles()) {
        seg()->saveScribbleImage("scribbles.png", scribbles);
    }
}

void TVSegWindow::invokeNextAlgorithm()
{
    if (!algorithmQueue_.empty()) {
        boost::function<void()> algo = algorithmQueue_.front();
        algorithmQueue_.pop_front();
        algo();
    }
}

namespace { // some helpers for the settings presets

void set2d(tvseg::ParzenSettings &s_p, tvseg::WeightSettings &s_w) {
    s_w.set_useDepth(false);
    s_w.set_useColor(true);
    s_p.set_colorVariance(0.05);
    s_p.set_depthVariance(0.20);
    s_p.set_fixScribblePixels(true);
    s_p.set_normalizeSpaceScale(true);
    s_p.set_scribbleDistanceDepth(false);
    s_p.set_scribbleDistanceFactor(1000);
    s_p.set_scribbleDistancePerspective(false);
    s_p.set_useColorKernel(true);
    s_p.set_useDepthKernel(false);
    s_p.set_useDistanceKernel(true);
    s_p.set_useSpatiallyActiveScribbles(false);
}

void set3d(tvseg::ParzenSettings &s_p, tvseg::WeightSettings &s_w) {
    set2d(s_p, s_w);
    s_p.set_scribbleDistanceDepth(true);
    s_p.set_scribbleDistancePerspective(false);
    s_p.set_useDepthKernel(true);
    s_p.set_useSpatiallyActiveScribbles(true);
}

void set3dNoColor(tvseg::ParzenSettings &s_p, tvseg::WeightSettings &s_w) {
    set3d(s_p, s_w);
    s_w.set_useDepth(true);
    s_w.set_useColor(false);
    s_p.set_useColorKernel(false);
}

void set3dDepthWeight(tvseg::ParzenSettings &s_p, tvseg::WeightSettings &s_w) {
    set3d(s_p, s_w);
    s_w.set_useDepth(true);
    s_w.set_useColor(true);
}

void set3dNoDistance(tvseg::ParzenSettings &s_p, tvseg::WeightSettings &s_w) {
    set3d(s_p, s_w);
    s_p.set_useDistanceKernel(false);
}

}

void TVSegWindow::addPresets()
{
    solverWidget_->addPreset("2d");
    solverWidget_->addPreset("3d");
    solverWidget_->addPreset("3d no color");
    solverWidget_->addPreset("3d depth weight");
    solverWidget_->addPreset("3d no distance");
}

void TVSegWindow::setPreset(QString name)
{
    tvseg::ParzenSettings s_p(settingsBackend_);
    tvseg::WeightSettings s_w(settingsBackend_);
    if (name == "2d") {
        set2d(s_p, s_w);
    } else if (name == "3d") {
        set3d(s_p, s_w);
    } else if (name == "3d no color") {
        set3dNoColor(s_p, s_w);
    } else if (name == "3d depth weight") {
        set3dDepthWeight(s_p, s_w);
    } else if (name == "3d no distance") {
        set3dNoDistance(s_p, s_w);
    }  else {
        LERROR << "unknown settings preset '" << name << "'";
        return;
    }
    LINFO << "updated settings with preset '" << name << "'";
    solverWidget_->updateFromSettings();
}

void TVSegWindow::on_actionQuit_triggered()
{
    qApp->quit();
}

void TVSegWindow::on_computeKMeansButton_clicked()
{
    seg()->computeKMeans();
    displayKMeans();
}

void TVSegWindow::on_displayLabelsButton_clicked()
{
    displayLabels();
}

void TVSegWindow::on_pickImageButton_clicked()
{
    QFileInfo currentFileName(QString::fromStdString(inputSettings()->color()));
    QDir currentDir = currentFileName.dir();
    if (!currentDir.exists()) {
        currentDir = QDir::current();
    }
    QString fileName = QFileDialog::getOpenFileName(this,
        tr("Open Color Image"), currentDir.path(), tr("Image Files (*.png *.jpg *.gif *.bmp)"));
    fileName = QDir::current().relativeFilePath(fileName);
    inputSettings()->set_color(fileName.toStdString());
    mainSettingsEditor_->updateFromSettings();
    on_loadImageButton_clicked();
}

void TVSegWindow::on_showColorButton_clicked()
{
    if (seg()->inputImageColorAvailable()) {
        ui_->inputDisplay->setImage(seg()->inputImageColor(), true);
    }
}

void TVSegWindow::on_showDepthButton_clicked()
{
    if (seg()->inputImageDepthAvailable()) {
        ui_->inputDisplay->setImage(seg()->inputImageDepth(), true);
    }
}

void TVSegWindow::on_showGroundTruthButton_clicked()
{
    if (seg()->inputImageGroundTruthAvailable()) {
        ui_->inputDisplay->setImage(seg()->inputImageGroundTruth(), true);
    }
}

void TVSegWindow::on_mainTabWidget_currentChanged(int index)
{
    if (ui_->mainTabWidget->widget(index) == solverWidget_) {
        setAndSaveScribbles();
    }
}


} // namespace tvseg_ui

void tvseg_ui::TVSegWindow::on_saveScribblesButton_clicked()
{
    seg()->setScribbles(getScribblesFromDisplayWidget());
    seg()->saveScribbles();
}

void tvseg_ui::TVSegWindow::on_saveLabelMappingButton_clicked()
{
    seg()->saveInputGroundTruthLabelMapping();
}
