#ifndef DYAD_TEST_SHUFFLE_WORKER_HPP
#define DYAD_TEST_SHUFFLE_WORKER_HPP

#include <random>
#include <string>
#include <vector>

class Worker
{
   public:
    using idx_it_t = std::vector<size_t>::const_iterator;

   protected:
    using rn_gen_t = std::mt19937;
    unsigned int m_rank;               ///< rank of this worker
    unsigned int m_seed;               ///< randome seed for shuffling
    rn_gen_t m_gen;                    ///< random number generator
    std::vector<std::string> m_flist;  ///< list of file names
    std::vector<size_t> m_fidx;        ///< shuffled indices to file list
    size_t m_begin;                    ///< starting position of m_fidx partition for this worker
    size_t m_end;                      ///< end position of m_fidx partition for this worker
    size_t m_fsize;                    ///< size of a file to generate
    std::string m_work_dir;            ///< directory where working files are
    bool m_validate;                   ///< validate if a file read matches the one written

   public:
    Worker ();
    void set_rank (const int r);
    unsigned int get_rank () const;
    void clear_seed ();
    void set_seed_by_clock ();
    void set_seed (const unsigned int s);
    void set_file_list (std::vector<std::string>&& flist);
    void set_file_list (const std::vector<std::string>& flist);
    const std::vector<std::string>& get_file_list () const;
    void set_file_size (const size_t sz);
    size_t get_file_size () const;
    void set_work_dir (const std::string& wd);
    std::string get_work_dir () const;
    void set_validate ();
    void unset_validate ();
    bool get_validate () const;

    /// Shuffle the whole file index set
    void shuffle ();
    /// Determine the partition of file index set assigned to the worker
    void split (int n_ranks);
    /// Get the partition of file index set assigned to the worker
    std::pair<size_t, size_t> get_range ();
    /// Get the whole index set
    const std::vector<size_t>& get_indices () const;
    /** Get the start and end iterators for the partition of index set assigned
     * to the worker. The indices to file list can be obtained using the
     * iterators, which are ultimately used to retrieve the names of files in
     * the partition. */
    std::pair<idx_it_t, idx_it_t> get_iterator () const;
    /// Convert the list of files in the partition into a string to print out
    std::string get_my_list_str () const;

   protected:
    void seed ();
    void set_file_list ();
};

#endif  // DYAD_TEST_SHUFFLE_WORKER_HPP
