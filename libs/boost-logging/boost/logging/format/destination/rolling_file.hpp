// destination_rolling_file.hpp

// Boost Logging library
//
// Author: John Torjo, www.torjo.com
//
// Copyright (C) 2007 John Torjo (see www.torjo.com for email)
//
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)
//
// See http://www.boost.org for updates, documentation, and revision history.
// See http://www.torjo.com/log2/ for more details


#ifndef JT28092007_destination_rolling_file_HPP_DEFINED
#define JT28092007_destination_rolling_file_HPP_DEFINED

#if defined(_MSC_VER) && (_MSC_VER >= 1020)
# pragma once
#endif

#if defined(_MSC_VER) 
#pragma warning ( disable : 4355)
#endif 

#include <boost/logging/detail/fwd.hpp>
#include <boost/logging/detail/manipulator.hpp>
#include <boost/logging/format/destination/convert_destination.hpp>
#include <fstream>
#include <string>
#include <sstream>
#include <boost/filesystem/path.hpp>
#include <boost/filesystem/operations.hpp>

namespace boost { namespace logging { namespace destination {


/** 
    @brief Settings you can pass to the rolling file. To see how it's used, see @ref dealing_with_flags.
*/
struct rolling_file_settings {
    typedef boost::logging::detail::flag<rolling_file_settings> flag;

    rolling_file_settings() 
        : max_size_bytes(this, 1024 * 1024)
        , file_count(this, 10)
        , initial_erase(this, false)
        , start_where_size_not_exceeded(this, true) 
    {}

    /// maximum size in bytes, by default 1Mb
    flag::t<int> max_size_bytes;
    /// how many files has a rolling file, by default, 10
    flag::t<int> file_count;
    /// if true, it initially erases all files from the rolling file (by default, false)
    flag::t<bool> initial_erase;
    /// if true, it starts with the first file that hasn't exceeded the max size;
    /// otherwise, it starts with the first file (default = true)
    flag::t<bool> start_where_size_not_exceeded;
};

namespace detail {
    template<class convert_dest > 
    struct rolling_file_info {
        
        rolling_file_info (const std::string& name_prefix, rolling_file_settings flags ) 
                // many thanks to Martin Bauer
                : m_name_prefix(name_prefix), m_flags(flags), m_cur_idx(0) {
            namespace fs = boost::filesystem;
            
            if ( m_flags.initial_erase()) {
                for ( int idx = 0; idx < m_flags.file_count(); ++idx)
                    if ( fs::exists( file_name(idx) ))
                        fs::remove( file_name(idx) );
            }

            // see what file to start from
            if ( m_flags.start_where_size_not_exceeded() ) {
                for ( m_cur_idx = 0; m_cur_idx < m_flags.file_count(); ++m_cur_idx )
                    if ( fs::exists( file_name(m_cur_idx) )) {
                        if ( fs::file_size( file_name(m_cur_idx))  < m_flags.max_size_bytes() )
                            // file hasn't reached max size
                            break;
                    }
                    else
                        // file not found, we'll create it now
                        break;

            }

            recreate_file();
        }

        std::string file_name(int idx) {
            std::ostringstream out; 
            out << m_name_prefix << "." << (idx+1);
            return out.str();
        }

        void recreate_file() {
            m_out = boost::shared_ptr< std::basic_ofstream<char_type> >(new std::basic_ofstream<char_type>( file_name(m_cur_idx).c_str(),
                std::ios_base::out | std::ios_base::app));
        }

        template<class msg_type> void write( const msg_type& msg) {
            convert_dest::write(msg, (*m_out) );
            if ( m_out->tellp() > m_flags.max_size_bytes()) {
                m_cur_idx = (m_cur_idx + 1) % m_flags.file_count();
                recreate_file();
            }            
        }

        boost::shared_ptr< std::basic_ofstream<char_type> > m_out;
        std::string m_name_prefix;
        rolling_file_settings m_flags;
        // the index of the current file
        int m_cur_idx;
    };
}

/** 
    @brief Writes to multiple files: name_prefix.1, name_prefix.2, ... name_prefix.N, and then restarts from 1.

    We first write to name_prefix.1.

    The log has a max_size. When max_size is reached, we start writing to name_prefix.2. When max_size is reached, we start writing to name_prefix.3.
    And so on, until we reach name_prefix.N (N = file_count). When that gets fool, we start over, with name_prefix.1.
*/
template<class convert_dest = do_convert_destination > struct rolling_file_t : is_generic, non_const_context<detail::rolling_file_info<convert_dest> > {
    typedef non_const_context<detail::rolling_file_info<convert_dest> > non_const_context_base;

    /** 
        Constructs a rolling file

        @param name_prefix the name to be used as prefix for the files
        
        @param flags [optional] extra settings to pass to the rolling file. See rolling_file_settings and @ref dealing_with_flags.
    */
    rolling_file_t(const std::string & name_prefix, rolling_file_settings flags = rolling_file_settings() ) : non_const_context_base(name_prefix, flags) {}

    template<class msg_type> void operator()( const msg_type & msg) const {
        non_const_context_base::context().write(msg);
    }

    bool operator==(const rolling_file_t & other) const {
        return non_const_context_base::context().m_name_prefix == other.context().m_name_prefix;
    }
};

/** @brief rolling_file_t with default values. See rolling_file_t

@copydoc rolling_file_t
*/
typedef rolling_file_t<> rolling_file;




}}}

#endif

