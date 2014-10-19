#ifndef TVSEG_INPUTSETTINGS_H
#define TVSEG_INPUTSETTINGS_H

#include "tvseg/settings/defineparams.h"
#include "tvseg/settings/definesettings.h"

#include "tvseg/util/arrayio.h"

#define TVSEG_INPUT_SETTINGS_SEQ                            \
    (std::string, color, "Images/test46_color.png")         \
    (bool, loadDepth, true)                                 \
    (bool, loadGroundTruth, true)                           \
    (bool, loadScribbles, true)                             \
    (bool, loadSettings, true)                              \
    (std::string, colorMatch, "_color\\(..*)")              \
    (std::string, depthReplace, "_depth\\1")                \
    (std::string, groundTruthReplace, "_label\\1")          \
    (std::string, groundTruthLabelMappingReplace, "_label_mapping.txt") \
    (std::string, scribblesReplace, "_scribbles\\1")        \
    (std::string, settingsReplace, "_settings.ini")         \
    (float, depthScale, 10000)                              \
    (std::vector<float>, intrinsics, std::vector<float>(9)) \
    (bool, overrideGroundTruthLabelMapping, false)          \
    (bool, overrideScribbles, false)                        \
    (std::vector<uint>, groundTruthLabelMappingOverride, std::vector<uint>(0))


TVSEG_SETTINGS_DEFINE_PARAMS(
    (tvseg), InputParams,
    TVSEG_INPUT_SETTINGS_SEQ)

TVSEG_SETTINGS_DEFINE_SETTINGS(
    (tvseg), InputSettings, "input", InputParams,
    TVSEG_INPUT_SETTINGS_SEQ)



///////////////////////////////
/// Old manual settings class:
///////////////////////////////

//#include "settings/settings.h"
//#include "settings/entrybase.h"

//namespace tvseg {


//class InputSettings : public settings::Settings
//{
//private:
//    typedef settings::StringEntry StringValue;
//    typedef settings::BoolEntry BoolValue;
//    typedef settings::FloatEntry FloatValue;
//    typedef settings::FloatArrayEntry FloatArrayValue;

//public:

//    InputSettings(settings::BackendPtr backend) :
//        settings::Settings(backend, "input"),
//        color_(add<std::string>("color", "Images/test46_color.png")),
//        loadDepth_(add<bool>("loadDepth", true)),
//        loadGroundTruth_(add<bool>("loadGroundTruth", true)),
//        loadScribbles_(add<bool>("loadScribbles", true)),
//        saveScribbles_(add<bool>("saveScribbles", true)),
//        colorMatch_(add<std::string>("colorMatch", "_color")),
//        depthReplace_(add<std::string>("depthReplace", "_depth")),
//        groundTruthReplace_(add<std::string>("groundTruthReplace", "_label")),
//        scribblesReplace_(add<std::string>("scribblesReplace", "_scribbles")),
//        depthScale_(add<float>("depthScale", 10000.f)),
//        intrinsics_(add("intrinsics", std::vector<float>(9)))
//    {
//    }

//    std::string color() const { return color_->get(); }
//    void set_color(const std::string &value) { color_->set(value); }

//    bool loadDepth() const { return loadDepth_->get(); }
//    void set_loadDepth(const bool value) { loadDepth_->set(value); }
//    bool loadGroundTruth() const { return loadGroundTruth_->get(); }
//    void set_loadGroundTruth(const bool value) { loadGroundTruth_->set(value); }
//    bool loadScribbles() const { return loadScribbles_->get(); }
//    void set_loadScribbles(const bool value) { loadScribbles_->set(value); }
//    bool saveScribbles() const { return saveScribbles_->get(); }
//    void set_saveScribbles(const bool value) { saveScribbles_->set(value); }


//    std::string colorMatch() const { return colorMatch_->get(); }
//    void set_colorMatch(const std::string &value) { colorMatch_->set(value); }
//    std::string depthReplace() const { return depthReplace_->get(); }
//    void set_depthReplace(const std::string &value) { depthReplace_->set(value); }
//    std::string groundTruthReplace() const { return groundTruthReplace_->get(); }
//    void set_groundTruthReplace(const std::string &value) { groundTruthReplace_->set(value); }
//    std::string scribblesReplace() const { return scribblesReplace_->get(); }
//    void set_scribblesReplace(const std::string &value) { scribblesReplace_->set(value); }

//    float depthScale() const { return depthScale_->get(); }
//    void set_depthScaleh(const float value) { depthScale_->set(value); }

//    std::vector<float> intrinsics() const { return intrinsics_->get(); }
//    void set_intrinsics(const std::vector<float> &value) { intrinsics_->set(value); }
//private:

//    StringValue::Ptr color_;
//    BoolValue::Ptr loadDepth_;
//    BoolValue::Ptr loadGroundTruth_;
//    BoolValue::Ptr loadScribbles_;
//    BoolValue::Ptr saveScribbles_;
//    StringValue::Ptr colorMatch_;
//    StringValue::Ptr depthReplace_;
//    StringValue::Ptr groundTruthReplace_;
//    StringValue::Ptr scribblesReplace_;
//    FloatValue::Ptr depthScale_;
//    FloatArrayValue::Ptr intrinsics_;
//};

//typedef boost::shared_ptr<InputSettings> InputSettingsPtr;
//typedef boost::shared_ptr<InputSettings const> InputSettingsConstPtr;


//} // namespace tvseg

#endif // TVSEG_INPUTSETTINGS_H
