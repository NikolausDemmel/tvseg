#ifndef TVSEG_UI_ALGORITHMWIDGET_H
#define TVSEG_UI_ALGORITHMWIDGET_H

#include "cvimagedisplaywidget.h"
#include "settingseditor.h"
#include "tvseg/cvalgorithminterface.h"
#include "tvseg/cvalgorithminterfacebase.h"

#include <QWidget>
#include <QPushButton>
#include <QVBoxLayout>
#include <QGroupBox>
#include <QSignalMapper>
#include <QFutureWatcher>
#include <QComboBox>

namespace tvseg_ui {

class AlgorithmWidget : public QWidget
{
    Q_OBJECT
public:
    explicit AlgorithmWidget(QWidget *parent = 0);

    void addAlgorithm(const tvseg::CVAlgorithmInterface *algorithmInterface);

    void clear();

    void updateFromSettings();

    void addPreset(QString name);

signals:
    void algorithmStarted();
    void algorithmStopped();
    void saveResult();
    void runAll();
    void computeMetrics();
    void setPreset(QString name);

public slots:
    void computeButtonClicked(int id);
    void clearButtonClicked(int id);
    void showButtonClicked(int id);
    void settingsValueChanged(QtProperty* property, QVariant value);

private slots:
    void algorithmCompleted();
    void setPresetClicked();

private:
    void clearActions();

private:
    CvImageDisplayWidget *display_;
    QVBoxLayout *sidePane_;
    QGroupBox *actionsGroup_;
    QGridLayout *groupLayout_;
    SettingsEditor *settingsEditor_;
    QSignalMapper *computeButtonMapper_;
    QSignalMapper *clearButtonMapper_;
    QSignalMapper *showButtonMapper_;
    QVector<QPushButton*> clearButtons_;
    QVector<QPushButton*> showButtons_;
    QVector<tvseg::CVAlgorithmInterface::Action> actions_;
    QVector<tvseg::CVAlgorithmInterfaceBase> algorithms_;
    QMap<QtProperty*,QPair<int,int> > propertyMap_; // pair of (algoId, firstActionId)
    QFutureWatcher<void> *watcher_;
    int runningAction_;
    QVector<QString> settingsInSetup_; // list of settings entries being currenlty set up
    QPushButton *runAllButton_;
    QPushButton *saveResultButton_;
    QPushButton *computeMetricsButton_;
    QGroupBox *commandsGroup_;
    QGroupBox *presetsGroup_;
    QComboBox *presetsComboBox_;
    QPushButton *presetsSetButton_;
};

} // namespace tvseg_ui

#endif // TVSEG_UI_ALGORITHMWIDGET_H
