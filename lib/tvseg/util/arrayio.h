#ifndef TVSEG_UTIL_ARRAYIO_H
#define TVSEG_UTIL_ARRAYIO_H


#include <string>
#include <vector>
#include <sstream>


//template<class E>
//std::string toVectorString(const std::vector<E> &value)
//{
//    std::ostringstream ss;
//    ss << std::boolalpha;
//    typename std::vector<E>::const_iterator it = value.begin();
//    if (it != value.end())
//    {
//        ss << *it;
//        ++it;
//    }
//    for (; it != value.end(); ++it)
//        ss << "," << *it;
////    std::cout << "writing vector '" << ss.str() << "'" << std::endl;
//    return ss.str();
//}


//template<class E>
//std::vector<E> parseStringAsVector(const std::string &str)
//{
//    std::vector<E> result;
//    std::istringstream ss(str);
//    std::string element;
////    std::cout << "reading vector '" << str << "'" << std::endl;
//    while (std::getline(ss, element, ','))
//    {
//        std::stringstream element_ss(element);
//        E element_val;
//        element_ss >> std::boolalpha >> element_val;
//        result.push_back(element_val);
////        std::cout << element_val << std::endl;
//    }
//    return result;
//}


// declare in namespace std to make sure they are found during Koenig lookup
namespace std {


template<class E>
std::ostream& operator<< (std::ostream &stream, const std::vector<E> &value)
{
    stream << std::boolalpha;
    typename std::vector<E>::const_iterator it = value.begin();
    if (it != value.end())
    {
        stream << *it;
        ++it;
    }
    for (; it != value.end(); ++it)
        stream << "," << *it;
    return stream;
}

template<class E>
std::istream& operator>> (std::istream &stream, std::vector<E> &value)
{
    value.clear();
    std::string element;
    while (std::getline(stream, element, ','))
    {
        std::stringstream ss(element);
        E element_val;
        ss >> std::boolalpha >> element_val;
        value.push_back(element_val);
    }
    return stream;
}


}


#endif // TVSEG_UTIL_ARRAYIO_H
