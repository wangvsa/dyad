#ifndef DYAD_TEST_SHUFFLE_UTILS_H
#define DYAD_TEST_SHUFFLE_UTILS_H
#include <mpi.h>
#include <sys/stat.h>
#include <string>
#include <vector>
#include "worker.hpp"

/** Populates flist with names of mockup sample files to be generated. */
void populate_file_list (std::vector<std::string>& flist, const size_t count);

/** Loads the file list of flist_name on the root worker and broadcasts it to all others. */
void bcast_string_vector (const std::string& flist_name,
                          std::vector<std::string>& flist,
                          int root,
                          MPI_Comm comm);

/** Generate a file under base_dir/relpath where relpath is a relative path to base_dir.
 *  File is created with access mode md and the given size.
 *  The file can be filled with sequence of bytes in a reproducible fashion such that the reader
 *  can verify that the content is intact as it was when generated.
 *  The sequence of bytes is unique to the filename and size */
bool generate_file (const std::string& base_dir,
                    const std::string& relpath,
                    size_t size,
                    const mode_t md,
                    bool validate,
                    const std::string& rank);

/** Read a file under base_dir/relpath where relpath is a relative path to base_dir.
 *  When files are staged, base_dir changes but relpath remains the same.
 *  If the file was generated, provide expected_size for extra verification.
 *  If it is 0, the size will be obtained from the file.
 *  When validate is set, the bytes read from the file will be compared against
 *  expected values to confirm correctness of the file.
 */
bool read_file (const std::string& base_dir,
                const std::string& relpath,
                size_t expected_size,
                bool validate);

/** Worker creates files in its partition. Unless a separate data_dir is given, files will be
 * created under work_dir. */
void create_files (const Worker& worker,
                   const mode_t md = S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH,
                   std::string data_dir = "");

/** Worker reads files in its partition. */
void read_files (const Worker& worker);

/** Stages files from src_dir into the worker's work_dir using the
 *  worker's assigned file partition. If src_dir is empty or identical
 *  to work_dir, the function returns false immediately.
 *  If files were generated, their size is uniform and taken from the
 *  worker. Otherwise, the size of each file is discovered via fstat.
 *  The destination files are created with access mode md.
 */
bool stage_files (const Worker& worker,
                  const std::string& src_dir,
                  bool is_generated,
                  const mode_t md = S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH);
#endif
