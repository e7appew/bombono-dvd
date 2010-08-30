#ifndef __MLIB_RANGE_MY_ADAPTORS_H__
#define __MLIB_RANGE_MY_ADAPTORS_H__

#include <boost/range/iterator_range.hpp>
#include <boost/iterator/transform_iterator.hpp>
#include <boost/iterator/filter_iterator.hpp>

namespace RangeAdaptor {

// 
// transform
// 

// :TODO: рефакторинг range_transform & range_filter
template <class Range, class UnaryFunc> 
struct range_transform
{
    typedef typename boost::range_iterator<Range>::type iterator;
    typedef boost::transform_iterator<UnaryFunc, iterator> op_iterator;
    typedef boost::iterator_range<op_iterator> type;
};

template <class Range, class UnaryFunc>
typename range_transform<Range, UnaryFunc>::type
_transform(Range& r, const UnaryFunc& f)
{
    typedef typename boost::range_iterator<Range>::type iterator;
    iterator beg = boost::begin(r), end = boost::end(r);

    typedef boost::transform_iterator<UnaryFunc, iterator> op_iterator;
    return boost::iterator_range<op_iterator>(op_iterator(beg, f), op_iterator(end, f));
}

template <class Range, class UnaryFunc>
typename range_transform<Range, UnaryFunc>::type
transform(Range& r, const UnaryFunc& f)
{
    return _transform(r, f);
}

template <class Range, class UnaryFunc>
typename range_transform<const Range, UnaryFunc>::type
transform(const Range& r, const UnaryFunc& f)
{
    return _transform(r, f);
}

//
// filter
// 

template <class Range, class Predicate>
struct range_filter
{
    typedef typename boost::range_iterator<Range>::type iterator;
    typedef boost::filter_iterator<Predicate, iterator> op_iterator;
    typedef boost::iterator_range<op_iterator> type;
};

template <class Range, class Predicate>
typename range_filter<Range, Predicate>::type 
_filter(Range& r, const Predicate& f)
{
    typedef typename boost::range_iterator<Range>::type iterator;
    iterator beg = boost::begin(r), end = boost::end(r);

    typedef boost::filter_iterator<Predicate, iterator> op_iterator;
    return boost::iterator_range<op_iterator>(op_iterator(f, beg, end), op_iterator(f, end, end));
}

template <class Range, class Predicate>
typename range_filter<Range, Predicate>::type 
filter(Range& r, const Predicate& f)
{
    return _filter(r, f);
}

template <class Range, class Predicate>
typename range_filter<const Range, Predicate>::type 
filter(const Range& r, const Predicate& f)
{
    return _filter(r, f);
}

} // namespace RangeAdaptor

#endif // #ifndef __MLIB_RANGE_MY_ADAPTORS_H__

