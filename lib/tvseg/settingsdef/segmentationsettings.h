#ifndef TVSEG_SEGMENTATIONSETTINGS_H
#define TVSEG_SEGMENTATIONSETTINGS_H


#include "tvseg/settings/definesettings.h"
#include "tvseg/settings/defineparams.h"


#define TVSEG_SEGMENTATION_PARAMS_SEQ    \
    (unsigned int, numLabels, 2)

TVSEG_SETTINGS_DEFINE_PARAMS(
    (tvseg), SegmentationParams,
    TVSEG_SEGMENTATION_PARAMS_SEQ)

TVSEG_SETTINGS_DEFINE_SETTINGS(
    (tvseg), SegmentationSettings, "segmentation", SegmentationParams,
    TVSEG_SEGMENTATION_PARAMS_SEQ)

//class SegmentationSettings : public settings::Settings
//{
//private:
//    typedef settings::UIntEntry UIntValue;

//public:

//    SegmentationSettings(settings::BackendPtr backend) :
//        settings::Settings(backend, "segmentation"),
//        numLabels_(add<unsigned int>("numLabels", 2))
//    {
//    }

//    int numLabels() const { return numLabels_->get(); }
//    void setNumLabels(int value) { numLabels_->set(value); }
//private:

//    UIntValue::Ptr   numLabels_;
//};

//typedef boost::shared_ptr<SegmentationSettings> SegmentationSettingsPtr;
//typedef boost::shared_ptr<SegmentationSettings const> SegmentationSettingsConstPtr;


#endif // TVSEG_SEGMENTATIONSETTINGS_H
