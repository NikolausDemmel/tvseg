#ifndef TVSEG_UI_TVSEGWINDOW_H
#define TVSEG_UI_TVSEGWINDOW_H


#include "tvseg/segmentation.h"
#include "tvseg/settings/backend.h"
#include "tvseg/settings/serializer.h"
#include "tvseg/settings/settings.h"
#include "tvseg/settings/entrybase.h"
#include "tvseg/settingsdef/inputsettings.h"
#include "tvseg/settingsdef/outputsettings.h"
#include "tvseg/settingsdef/segmentationsettings.h"
#include "tvseg/scribbles.h"
#include "uisettings.h"
#include "settingseditor.h"
#include "consoledockwidget.h"
#include "feedbackdockwidget.h"
#include "feedbackwidget.h"
#include "algorithmwidget.h"

#include <QMainWindow>

namespace tvseg_ui {


namespace Ui {
class TVSegWindow;
}


class TVSegWindow : public QMainWindow
{
    Q_OBJECT

    Q_PROPERTY(bool algorithmRunning READ algorithmRunning WRITE setAlgorithmRunning NOTIFY algorithmRunningChanged)
    Q_PROPERTY(bool cancelRequested READ cancelRequested WRITE setCancelRequested NOTIFY cancelRequestedChanged)

public:

    class WindowSettings : public tvseg::settings::Settings
    {
    public:
        typedef boost::shared_ptr<WindowSettings> Ptr;
        typedef boost::shared_ptr<const WindowSettings> ConstPtr;
        typedef tvseg::settings::ByteArrayEntry ByteArray;
        typedef tvseg::settings::byte_array_t byte_array_t;

        WindowSettings(tvseg::settings::BackendPtr backend) :
            tvseg::settings::Settings(backend, "main_window"),
            geometry_(addEntry("geometry", ByteArray::make())),
            windowState_(addEntry("windowState", ByteArray::make()))
        {}

        const byte_array_t & geometry() const { return geometry_->get(); }
        void setGeometry(const byte_array_t &value) { geometry_->set(value); }
        const byte_array_t & windowState() const { return windowState_->get(); }
        void setWindowState(const byte_array_t &value) { windowState_->set(value); }

    private:
        ByteArray::Ptr  geometry_;
        ByteArray::Ptr   windowState_;
    };

public:
    explicit TVSegWindow(QWidget *parent = 0, std::string settingsFilename = "tvseg.ini");
    ~TVSegWindow();

//    void setSettings(tvseg::settings::BackendPtr value, std::string filename = "tvseg.ini");

    void initFromSettings();

    /// Add a message to the console, using a queued connection.
    void addConsoleMessageAsync(uint level, const std::string &msg);

    bool algorithmRunning();
    bool cancelRequested();

signals:
    void algorithmRunningChanged(bool value);
    void cancelRequestedChanged(bool value);

public slots:
    void displayKMeans();
    void displayLabels();

    void setAlgorithmRunning(bool value);
    void setCancelRequested(bool value = true);
    void algorithmStarted();
    void algorithmStopped();

    void settingsValueChanged(QtProperty* property, QVariant value);

    void runAllAlgorithms();
    void saveResult();
    void computeMetrics();

private slots: // private helper slots
    void invokeNextAlgorithm();
    void setPreset(QString name);

private slots:
    void on_saveSettingsButton_clicked();
    void on_loadImageButton_clicked();
    void on_displayLabelsButton_clicked();
    void on_actionQuit_triggered();
    void on_computeKMeansButton_clicked();
    void on_pickImageButton_clicked();
    void on_showColorButton_clicked();
    void on_showDepthButton_clicked();
    void on_showGroundTruthButton_clicked();
    void on_mainTabWidget_currentChanged(int index);

    void on_saveScribblesButton_clicked();

    void on_saveLabelMappingButton_clicked();

protected: // inherited
    void closeEvent(QCloseEvent *event);

private: // private types
    typedef QQueue<boost::function<void ()> > algorithm_queue_t;

private: // private helpers
    void updateGUIFromSettings();
    void updateSettingsEditors();
    void windowStateToSettings();
    void windowStateFromSettings();
    FeedbackWidget* getFeedbackWidget();

    void addPresets();

    void invokeAlgorithmsSequentially(algorithm_queue_t queue);

    tvseg::Scribbles getScribblesFromDisplayWidget();
    void setScribblesToDisplayWidget(const tvseg::Scribbles &scribbles);

    void setAndSaveScribbles();
    void maybeLoadScribbles();

private: // private member accessors
    tvseg::InputSettings::Ptr inputSettings();
    tvseg::InputSettings::ConstPtr inputSettings() const;

    tvseg::OutputSettings::Ptr outputSettings();
    tvseg::OutputSettings::ConstPtr outputSettings() const;

    UiSettings::Ptr uiSettings();
    UiSettings::ConstPtr uiSettings() const;

    tvseg::SegmentationSettings::Ptr segmentationSettings();
    tvseg::SegmentationSettings::ConstPtr segmentationSettings() const;

    WindowSettings::Ptr windowSettings();
    WindowSettings::ConstPtr windowSettings() const;

    tvseg::SegmentationPtr seg();
    tvseg::SegmentationConstPtr seg() const;

private: // members
    Ui::TVSegWindow *ui_;

    std::string settingsFilename_;
    tvseg::settings::BackendPtr settingsBackend_;
    tvseg::settings::SerializerPtr settingsSerializer_;
    UiSettings::Ptr uiSettings_;
    tvseg::InputSettings::Ptr inputSettings_;
    tvseg::OutputSettings::Ptr outputSettings_;
    tvseg::SegmentationSettings::Ptr segmentationSettings_;
    WindowSettings::Ptr windowSettings_;
    tvseg::SegmentationPtr segmentation_;

    ConsoleDockWidget *consoleDock_;
    FeedbackDockWidget *feedbackDock_;
    SettingsEditor *mainSettingsEditor_;
    AlgorithmWidget *solverWidget_;

    QStateMachine *stateMachine_;

    bool algorithmRunning_;
    bool cancelRequested_;

    algorithm_queue_t algorithmQueue_;
};

} // namespace tvseg_ui

#endif // TVSEG_UI_TVSEGWINDOW_H
