// Boost.Range library
//
//  Copyright Neil Groves 2009. Use, modification and
//  distribution is subject to the Boost Software License, Version
//  1.0. (See accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
//
//
// For more information, see http://www.boost.org/libs/range/
//
#include <mlib/tests/_pc_.h>

#include <mlib/range/adaptor/reversed.hpp>
#include <mlib/range/algorithm_ext.hpp>

#include <boost/assign.hpp>

#include <algorithm>
#include <list>
#include <set>
#include <vector>

namespace boost
{
    template< class Container >
    void reversed_test_impl( Container& c )
    {
        using namespace boost::adaptors;

        std::vector< int > test_result1;
        boost::push_back(test_result1, c | reversed);

        std::vector< int > test_result2;
        boost::push_back(test_result2, adaptors::reverse(c));

        std::vector< int > reference( c.rbegin(), c.rend() );

        BOOST_CHECK_EQUAL_COLLECTIONS( reference.begin(), reference.end(),
                                       test_result1.begin(), test_result1.end() );

        BOOST_CHECK_EQUAL_COLLECTIONS( reference.begin(), reference.end(),
                                       test_result2.begin(), test_result2.end() );
    }

    template< class Container >
    void reversed_test_impl()
    {
        using namespace boost::assign;

        Container c;

        // Test empty
        reversed_test_impl(c);

        // Test one
        c += 1;
        reversed_test_impl(c);

        // Test many
        c += 1,1,1,2,2,2,2,2,3,3,3,3,3,3,4,5,6,7,8,9;
        reversed_test_impl(c);
    }

    void reversed_test()
    {
        reversed_test_impl< std::vector< int > >();
        reversed_test_impl< std::list< int > >();
        reversed_test_impl< std::set< int > >();
        reversed_test_impl< std::multiset< int > >();
    }
}

BOOST_AUTO_TEST_CASE( test_range_reversed_test )
{
    boost::reversed_test();
}

