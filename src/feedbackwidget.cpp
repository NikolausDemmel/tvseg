#include "feedbackwidget.h"
#include "ui_feedbackwidget.h"

#include "util/valuetransition.h"
#include "cvimagedisplaywidget.h"

#include "tvseg/util/logging.h"


Q_DECLARE_METATYPE(QSharedPointer<cv::Mat>)
Q_DECLARE_METATYPE(QSharedPointer<QString>)


namespace {

template <typename Type> class MetaTypeRegistration
{
public:
    inline MetaTypeRegistration()
    {
        qRegisterMetaType<Type>();
    }
};

// do registration during static initialization
static MetaTypeRegistration<QSharedPointer<cv::Mat> > registration1;
static MetaTypeRegistration<QSharedPointer<QString> > registration2;

} // namespace


namespace tvseg_ui {


FeedbackWidget::FeedbackWidget(QWidget *parent) :
    QWidget(parent),
    ui_(new Ui::FeedbackWidget),
    stateMachine_(new QStateMachine(this)),
    running_(false)
{
    ui_->setupUi(this);

    connect(ui_->tabWidget, SIGNAL(currentChanged(int)), this, SLOT(currentChanged(int)));

    // Create State Machine to govern interface
    QState *sIdle = new QState();
    sIdle->assignProperty(ui_->progressBox, "title", QString("Progress (idle)"));
    sIdle->assignProperty(ui_->progressBox, "enabled", false);
    connect(sIdle, SIGNAL(entered()), ui_->progressBar, SLOT(reset()));

    QState *sRunning = new QState();
    sRunning->assignProperty(ui_->progressBox, "title", QString("Progress (running)"));

    QState *sCancelling = new QState();
    sCancelling->assignProperty(ui_->progressBox, "title", QString("Progress (cancelling)"));
    //sCancelling->assignProperty(ui_->cancelButton, "text", QString("cancelling"));
    sCancelling->assignProperty(ui_->cancelButton, "enabled", false);

    sIdle->addTransition(util::ValueTransition<bool>::make(this, SIGNAL(runningChanged(bool)), true, sRunning));
    sRunning->addTransition(util::ValueTransition<bool>::make(this, SIGNAL(runningChanged(bool)), false, sIdle));
    sRunning->addTransition(this, SIGNAL(cancelRequested()), sCancelling);
    sCancelling->addTransition(util::ValueTransition<bool>::make(this, SIGNAL(runningChanged(bool)), false, sIdle));

    stateMachine_->setGlobalRestorePolicy(QStateMachine::RestoreProperties);
    stateMachine_->addState(sIdle);
    stateMachine_->addState(sRunning);
    stateMachine_->addState(sCancelling);
    stateMachine_->setInitialState(running() ? sRunning : sIdle);
    stateMachine_->start();
}


FeedbackWidget::~FeedbackWidget()
{
    delete ui_;
}


void FeedbackWidget::displayCVImage(const cv::Mat &image, QString windowName)
{
    QWidget * const tab = findTab(windowName);

    if (tab != NULL) {
        CvImageDisplayWidget* w = qobject_cast<CvImageDisplayWidget*>(tab);
        w->setImage(image);
        const int index = ui_->tabWidget->indexOf(tab);
        if (ui_->tabWidget->currentIndex() != index) {
            ui_->tabWidget->tabBar()->setTabTextColor(index, Qt::red);
        }
    } else {
        CvImageDisplayWidget * const w = new CvImageDisplayWidget(ui_->tabWidget);
        ui_->tabWidget->setCurrentIndex(ui_->tabWidget->addTab(w, windowName));
        w->setImage(image);
    }
}


void FeedbackWidget::displayCVImage(QSharedPointer<cv::Mat> image, QSharedPointer<QString> windowName)
{
    displayCVImage(*image, *windowName);
}


void FeedbackWidget::displayCVImageAsync(const cv::Mat &image, const std::string &windowName)
{
    // clone data into heap objects controlled by shared pointer (which is thread safe)
    QSharedPointer<cv::Mat> ip(new cv::Mat(image.clone()));
    QSharedPointer<QString> sp(new QString(QString::fromStdString(windowName)));

    QMetaObject::invokeMethod(this, "displayCVImage", Qt::QueuedConnection, Q_ARG(QSharedPointer<cv::Mat>, ip), Q_ARG(QSharedPointer<QString>, sp));
}


void FeedbackWidget::updateProgress(int value, int max, int min)
{
    ui_->progressBar->setRange(min, max);
    ui_->progressBar->setValue(value);
}


void FeedbackWidget::algorithmStarted()
{
    setRunning(true);
}


void FeedbackWidget::algorithmStopped()
{
    setRunning(false);
}

void FeedbackWidget::setRunning(bool value)
{
    if (running_ != value) {
        running_ = value;
        emit runningChanged(value);
    }
}


void FeedbackWidget::updateProgressAsync(int value, int max, int min)
{
    QMetaObject::invokeMethod(this, "updateProgress", Qt::QueuedConnection, Q_ARG(int, value), Q_ARG(int, max), Q_ARG(int, min));
}

bool FeedbackWidget::running()
{
    return running_;
}


QWidget *FeedbackWidget::findTab(QString tabText)
{
    for (int i = 0; i < ui_->tabWidget->count(); ++i) {
        if (ui_->tabWidget->tabText(i) == tabText) {
            return ui_->tabWidget->widget(i);
        }
    }

    return NULL;
}


void FeedbackWidget::on_cancelButton_clicked()
{
    emit cancelRequested();
}


void FeedbackWidget::on_tabWidget_tabCloseRequested(int index)
{
    delete ui_->tabWidget->widget(index);
}


void FeedbackWidget::currentChanged(int index)
{
    ui_->tabWidget->tabBar()->setTabTextColor(index, Qt::black);
}


} // namespace tvseg_ui

