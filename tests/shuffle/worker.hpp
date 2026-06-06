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
    unsigned int m_seed;               ///< random seed for shuffling
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
    /** Record the MPI rank of this worker. This information can be used for diagnostics. */
    void set_rank (const int r);
    unsigned int get_rank () const;
    /** Set the seed for the random number generator that will be used for shuffling. */
    void set_seed (const unsigned int s);
    /** Choose RNG seed randomly based on current wall clock. */
    void set_seed_by_clock ();
    /** Set the list of sample file names. */
    void set_file_list (std::vector<std::string>&& flist);
    /** Set the list of sample file names. */
    void set_file_list (const std::vector<std::string>& flist);
    /** Gain read-only access to the sample file list. */
    const std::vector<std::string>& get_file_list () const;
    /** Set the size of each file to generate. Default is 0.
     *  Size 0 means that files are not generated but are provided by the user. */
    void set_file_size (const size_t sz);
    /// Get the size of each file to generate. 0 means not to generate but use those provided.
    size_t get_file_size () const;
    /** Store the path to the working directory. */
    void set_work_dir (const std::string& wd);
    std::string get_work_dir () const;
    /** Store the user's option to validate files when read.
     *  Only relevant to generated files as they contain reproducible byte sequences. */
    void set_validate ();
    /// Clear the validation option indicated.
    void unset_validate ();
    /// Get the value of the validation option.
    bool get_validate () const;

    /// Shuffle the whole file index set.
    void shuffle ();
    /// Determine the partition of the file index set assigned to this worker.
    void split (int n_ranks);
    /// Get the partition of the file index set assigned to this worker.
    std::pair<size_t, size_t> get_range ();
    /// Get the whole index set
    const std::vector<size_t>& get_indices () const;
    /** Get the start and end iterators for the partition of the index set assigned
     * to this worker. The indices to file list can be obtained using the
     * iterators, which are ultimately used to retrieve the names of the files in
     * the partition. */
    std::pair<idx_it_t, idx_it_t> get_iterator () const;
    /// Convert the list of files in the partition into a string to print out
    std::string get_my_list_str () const;

   protected:
    void seed ();
    void set_file_list ();
};

#endif  // DYAD_TEST_SHUFFLE_WORKER_HPP
