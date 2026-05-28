#include <cassert>
#define assertm(exp, msg) assert (((void)msg, exp))

#include <algorithm>
#include <chrono>
#include <sstream>

#include "worker.hpp"

Worker::Worker ()
    : m_rank (0u),
      m_seed (0u),
      m_begin (0ul),
      m_end (0ul),
      m_fsize (0ul),
      m_work_dir ("."),
      m_validate (false)
{
}

void Worker::set_rank (const int r)
{
    m_rank = static_cast<unsigned int> (r);
}

unsigned int Worker::get_rank () const
{
    return m_rank;
}

void Worker::set_seed (const unsigned int s)
{
    m_seed = s;
    seed ();
}

void Worker::set_seed_by_clock ()
{
    m_seed = std::chrono::system_clock::now ().time_since_epoch ().count ();
    seed ();
}

void Worker::seed ()
{
    m_gen.seed (m_seed);
}

void Worker::set_file_list (std::vector<std::string>&& flist)
{
    m_flist = flist;
    set_file_list ();
}

void Worker::set_file_list (const std::vector<std::string>& flist)
{
    m_flist = flist;
    set_file_list ();
}

void Worker::set_file_list ()
{
    m_fidx.resize (m_flist.size ());
    std::iota (m_fidx.begin (), m_fidx.end (), 0ul);
}

const std::vector<std::string>& Worker::get_file_list () const
{
    return m_flist;
}

void Worker::set_file_size (const size_t sz)
{
    m_fsize = sz;
}

/** Returns the size of a file if generated. Otherwise, return 0.
 *  In that case, acurrate file size should be obtained via fstat. */
size_t Worker::get_file_size () const
{
    return m_fsize;
}

void Worker::set_work_dir (const std::string& wd)
{
    m_work_dir = (wd.empty () ? "." : wd);
}

std::string Worker::get_work_dir () const
{
    return m_work_dir;
}

void Worker::set_validate ()
{
    m_validate = true;
}

void Worker::unset_validate ()
{
    m_validate = false;
}

bool Worker::get_validate () const
{
    return m_validate;
}

void Worker::shuffle ()
{
    std::shuffle (m_fidx.begin (), m_fidx.end (), m_gen);
}

void Worker::split (int n_ranks)
{
    assertm ((n_ranks > 0), "Invalid number of ranks.");
    size_t total = m_fidx.size ();
    auto chunk = total / static_cast<size_t> (n_ranks);
    auto n_extra = total - chunk * static_cast<size_t> (n_ranks);

    if (m_rank < n_extra) {
        m_begin = (chunk + 1) * m_rank;
        m_end = m_begin + chunk + 1;
    } else {
        m_begin = chunk * m_rank + n_extra;
        m_end = m_begin + chunk;
    }
}

std::pair<size_t, size_t> Worker::get_range ()
{
    return std::make_pair (m_begin, m_end);
}

const std::vector<size_t>& Worker::get_indices () const
{
    return m_fidx;
}

std::pair<Worker::idx_it_t, Worker::idx_it_t> Worker::get_iterator () const
{
    return std::make_pair (m_fidx.cbegin () + m_begin, m_fidx.cbegin () + m_end);
}

std::string Worker::get_my_list_str () const
{
    auto range = get_iterator ();
    auto it = range.first;
    auto it_end = range.second;
    std::stringstream ss;
    ss << "Rank " << m_rank;
    std::string str = ss.str ();
    for (; it != it_end; ++it) {
        str += ' ' + m_flist[*it];
    }
    return str;
}
