#ifndef DYAD_TEST_SHUFFLE_UTILS_H
#define DYAD_TEST_SHUFFLE_UTILS_H
#include <mpi.h>
#include <sys/stat.h>
#include <string>
#include <vector>
#include "worker.hpp"

void populate_file_list (std::vector<std::string>& flist, const size_t count);
void bcast_string_vector (const std::string& flist_name,
                          std::vector<std::string>& flist,
                          int root,
                          MPI_Comm comm);
bool generate_file (const std::string& base_dir,
                    const std::string& relpath,
                    size_t size,
                    const mode_t md,
                    bool validate,
                    const std::string& rank);
bool read_file (const std::string& base_dir,
                const std::string& relpath,
                size_t expected_size,
                bool validate);
void create_files (const Worker& worker, const mode_t md = S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH);
void read_files (const Worker& worker);
#endif
