#ifndef TVSEG_SCRIBBLES_H
#define TVSEG_SCRIBBLES_H

#include "tvseg/util/includeopencv.h"
#include "tvseg/params.h"

#include <boost/shared_array.hpp>


namespace tvseg {


typedef std::vector<std::vector<cv::Point> > scribble_list_type;


class Scribbles {
public:
    virtual ~Scribbles() {}

    Scribbles();
    explicit Scribbles(const scribble_list_type &list);

    const scribble_list_type& get() const;
    void set(const scribble_list_type &list);

    int getNumLabels() const;

    boost::shared_array<int> getScribblesFlat() const;
    boost::shared_array<int> getNumScribblesFlat() const;

    int getTotalNumScribbles() const;

    cv::Mat getAsImage(cv::Mat labelColors, Dim2 dim) const;
    void setFromImage(cv::Mat scribbleImage, cv::Mat labelColors, Dim2 dim);

    void saveScribbleImage(std::string filename, cv::Mat labelColors, Dim2 dim) const;
    void loadScribbleImage(std::string filename, cv::Mat labelColors, Dim2 dim);

private:
    scribble_list_type list_;
};


}

#endif // TVSEG_SCRIBBLES_H
