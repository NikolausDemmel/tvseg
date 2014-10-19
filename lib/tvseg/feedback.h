#ifndef TVSEG_FEEDBACK_H
#define TVSEG_FEEDBACK_H


#include "tvseg/params.h"

#include <string>


namespace tvseg {

// TODO: Add logging text to feedback


class Feedback {
public:
    virtual void displayImage(const float* data, Dim3 dim, const std::string& windowName) const = 0;
    virtual void progress(int value, int max, int min) const = 0;
    virtual bool isCanceled() const = 0;

    virtual ~Feedback() {}
};

// FIXME: nvcc chokes on including boost::shared_ptr
//typedef boost::shared_ptr<Feedback> FeedbackPtr;
//typedef boost::shared_ptr<Feedback const> FeedbackConstPtr;

}


#endif // TVSEG_FEEDBACK_H
