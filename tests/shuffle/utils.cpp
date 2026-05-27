#include "utils.hpp"
#include <fcntl.h>  // open
#include <mpi.h>
#include <unistd.h>
#include <cstdint>
#include <cstdlib>
#include <fstream>
#include <functional>
#include <iostream>
#include <limits>
#include <list>
#include <string>
#include <vector>
#include "worker.hpp"

using std::cerr;
using std::cout;
using std::endl;

// TODO: need to make this file I/O chunk size runtime configurable
constexpr size_t CHUNK = 65536ul;

size_t fnv1a (const std::string& s)
{
    size_t hash = 14695981039346656037ULL;
    for (unsigned char c : s) {
        hash ^= c;
        hash *= 1099511628211ULL;
    }
    return hash;
}

void populate_file_list (std::vector<std::string>& flist, const size_t count)
{
    flist.resize (count);
    for (size_t i = 0ul; i < count; ++i) {
        flist[i] = "file_" + std::to_string (i) + ".dat";
    }
}

int read_file_list (const std::string& flist_name, std::vector<std::string>& flist)
{
    std::string item_name;
    std::string line;
    std::list<std::string> fnames;

    std::ifstream flist_file;
    flist_file.open (flist_name);
    if (!flist_file) {
        return EXIT_FAILURE;
    }

    while (std::getline (flist_file, line)) {
        fnames.emplace_back (line);
        // std::cout << "file: " << line << std::endl;
    }
    flist.assign (std::make_move_iterator (fnames.begin ()),
                  std::make_move_iterator (fnames.end ()));

    return EXIT_SUCCESS;
}

void bcast_string_vector (const std::string& flist_name,
                          std::vector<std::string>& flist,
                          int root,
                          MPI_Comm comm)
{
    int count = 0;
    int rc = EXIT_SUCCESS;
    int rank;

    MPI_Comm_rank (comm, &rank);

    // Step 1: Load the file list
    if (rank == root) {
        rc = read_file_list (flist_name, flist);
        if (rc != EXIT_SUCCESS) {
            cerr << "Failed to read '" << flist_name << "'" << endl;
            MPI_Abort (comm, EXIT_FAILURE);
        }
        count = static_cast<int> (flist.size ());
    }

    // Step 2: Broadcast count
    rc = MPI_Bcast (&count, 1, MPI_INT, root, comm);
    if (rc != MPI_SUCCESS) {
        MPI_Abort (comm, rc);
    }

    // Step 3: Build and broadcast lengths
    std::vector<int> lengths (count);
    if (rank == root) {
        for (int i = 0; i < count; ++i) {
            lengths[i] = static_cast<int> (flist[i].size ());
        }
    }
    rc = MPI_Bcast (lengths.data (), count, MPI_INT, root, comm);
    if (rc != MPI_SUCCESS) {
        MPI_Abort (comm, rc);
    }

    // Step 4: Build and broadcast flat buffer
    int total_chars = 0;
    for (int l : lengths) {
        if (l > (std::numeric_limits<int>::max () - total_chars)) {
            cerr << "Total length of file names is too big for int type argumgnet of MPI_Bcast"
                 << endl;
            MPI_Abort (comm, EXIT_FAILURE);
        }
        total_chars += l;
    }

    std::string flat;
    if (rank == root) {
        flat.reserve (static_cast<size_t> (total_chars));
        for (const auto& s : flist)
            flat += s;
    } else {
        flat.resize (static_cast<size_t> (total_chars));
    }
    rc = MPI_Bcast (flat.data (), total_chars, MPI_CHAR, root, comm);
    if (rc != MPI_SUCCESS) {
        MPI_Abort (comm, rc);
    }

    // Step 5: Non-root ranks reconstruct
    if (rank != root) {
        flist.resize (count);
        size_t offset = 0ul;
        for (int i = 0; i < count; ++i) {
            flist[i] = flat.substr (offset, static_cast<size_t> (lengths[i]));
            offset += static_cast<size_t> (lengths[i]);
        }
    }
}

bool generate_file (const std::string& base_dir,
                    const std::string& relpath,
                    size_t size,
                    const mode_t md,
                    bool validate,
                    const std::string& rank)
{
    auto path = base_dir + '/' + relpath;
    int fd = open (path.c_str (), O_CREAT | O_WRONLY, md);
    if (fd < 0) {
        std::cerr << "error: could not open '" << path << "' for writing\n";
        return false;
    }

    // Seed the pattern with a hash of the filename so each file has unique content
    const size_t name_hash = fnv1a (relpath);

    // Write in chunks to avoid large memory allocation
    std::vector<uint8_t> buf (std::min (CHUNK, size));
    if (!validate) {
        const size_t bytes_to_copy = std::min (rank.length (), buf.size ());
        std::copy_n (rank.begin (), bytes_to_copy, buf.begin ());
        std::fill (buf.begin () + bytes_to_copy, buf.end (), ' ');
    }

    size_t written = 0ul;
    while (written < size) {
        size_t chunk_size = std::min (CHUNK, size - written);
        if (validate) {
            for (size_t i = 0ul; i < chunk_size; ++i) {
                // Each byte is a function of filename hash + absolute offset
                // buf[i] = static_cast<uint8_t> ((name_hash ^ (written + i)) & 0xFF);
                buf[i] = static_cast<uint8_t> ((name_hash >> (((written + i) & 7) << 3)) & 0xFF);
            }
        }
        if (chunk_size
            != static_cast<size_t> (
                write (fd, reinterpret_cast<const char*> (buf.data ()), chunk_size))) {
            std::cerr << "error: write failed on '" << relpath << "'\n";
            close (fd);
            return false;
        }
        written += chunk_size;
    }
    close (fd);

    return true;
}

bool read_file (const std::string& base_dir,
                const std::string& relpath,
                size_t expected_size,
                bool validate)
{
    auto path = base_dir + '/' + relpath;
    int fd = open (path.c_str (), O_RDONLY);
    if (fd < 0) {
        std::cerr << "error: could not open '" << path << "' for reading\n";
        return false;
    }

    const size_t name_hash = fnv1a (relpath);

    std::vector<uint8_t> buf (std::min (CHUNK, expected_size));

    size_t verified = 0ul;
    while (verified < expected_size) {
        size_t chunk_size = std::min (CHUNK, expected_size - verified);
        if (chunk_size
            != static_cast<size_t> (read (fd, reinterpret_cast<char*> (buf.data ()), chunk_size))) {
            std::cerr << "error: unexpected EOF or read error on '" << relpath << "'\n";
            close (fd);
            return false;
        }
        if (validate) {
            for (size_t i = 0ul; i < chunk_size; ++i) {
                // uint8_t expected = static_cast<uint8_t> ((name_hash ^ (verified + i)) & 0xFF);
                uint8_t expected =
                    static_cast<uint8_t> ((name_hash >> (((verified + i) & 7) << 3)) & 0xFF);
                if (buf[i] != expected) {
                    std::cerr << "error: mismatch at offset " << (verified + i) << " in '"
                              << relpath << "'"
                              << " (expected 0x" << std::hex << (int)expected << ", got 0x"
                              << (int)buf[i] << std::dec << ")\n";
                    close (fd);
                    return false;
                }
            }
        }
        verified += chunk_size;
    }
    close (fd);

    return true;
}

void create_files (const Worker& worker, const mode_t md)
{
    auto range = worker.get_iterator ();
    auto it = range.first;
    auto it_end = range.second;
    // const auto rank = worker.get_rank ();
    const auto& flist = worker.get_file_list ();
    const auto fsize = worker.get_file_size ();
    const std::string work_dir = worker.get_work_dir ();
    const bool validate = worker.get_validate ();
    const std::string rank = std::to_string (worker.get_rank ());

    for (; it != it_end; ++it) {
        auto& fn = flist[*it];
        if (fn.empty ()) {
            continue;
        }
        if (!generate_file (work_dir, fn, fsize, md, validate, rank)) {
            break;
        }
    }
}

void read_files (const Worker& worker)
{
    auto range = worker.get_iterator ();
    auto it = range.first;
    auto it_end = range.second;
    // const auto rank = worker.get_rank ();
    const auto& flist = worker.get_file_list ();
    const auto fsize = worker.get_file_size ();
    const std::string work_dir = worker.get_work_dir ();
    const bool validate = worker.get_validate ();

    for (; it != it_end; ++it) {
        auto& fn = flist[*it];
        if (fn.empty ()) {
            continue;
        }
        if (!read_file (work_dir, fn, fsize, validate)) {
            break;
        }
    }
}
