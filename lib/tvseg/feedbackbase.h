#ifndef TVSEG_FEEDBACKBASE_H
#define TVSEG_FEEDBACKBASE_H


#include "tvseg/feedback.h"

#include "tvseg/util/includeopencv.h"

#include <boost/function.hpp>


namespace tvseg {


struct FeedbackBase: public Feedback {
    // Feedback interface
public:
    inline void displayImage(const float *data, Dim3 dim, const std::string &windowName) const {
        if (displayImageFn_) displayImageFn_(data, dim, windowName);
    }

    inline void progress(int value, int max, int min) const {
        if (progressFn_) progressFn_(value, max, min);
    }

    inline bool isCanceled() const {
        return cancelFlag_ ? *cancelFlag_ : false;
    }

public:
    typedef boost::function<void (const float *, Dim3, const std::string&)> display_image_fn_t;
    typedef boost::function<void (int, int, int)> progress_fn_t;

public:
    /// The caller retains the ownership and has to ensure that the flag boolean lives as long as the FeedbackBase object
    inline explicit FeedbackBase(display_image_fn_t df = NULL, progress_fn_t pf = NULL, volatile bool* cf = NULL) :
        displayImageFn_(df),
        progressFn_(pf),
        cancelFlag_(cf)
    {}

    inline void setDisplayImageFn(display_image_fn_t displayImageFn) {
        displayImageFn_ = displayImageFn;
    }

    inline void setProgressFn(progress_fn_t progressFn) {
        progressFn_ = progressFn;
    }

    /// The caller retains the ownership and has to ensure that the flag boolean lives as long as the FeedbackBase object
    inline void setCancelFlag(bool* cancelFlag) {
        cancelFlag_ = cancelFlag;
    }

private:
    display_image_fn_t displayImageFn_;
    progress_fn_t progressFn_;
    volatile bool* cancelFlag_; ///< NOTE: Beware that a volatile flag is probably not the most robust approach. Remember memory boundaries.
};


typedef boost::shared_ptr<FeedbackBase> FeedbackBasePtr;
typedef boost::shared_ptr<FeedbackBase const> FeedbackBaseConstPtr;


}


#endif // TVSEG_FEEDBACKBASE_H
