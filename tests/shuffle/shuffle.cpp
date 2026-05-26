#include <fcntl.h>   // open
#include <libgen.h>  // basename dirname
#include <mpi.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

#include <cerrno>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <fstream>
#include <iostream>
#include <list>
#include <sstream>
#include <vector>
#include <limits>

#include <dyad/utils/read_all.h>
#include <dyad/utils/utils.h>
#include "worker.hpp"
#include "cli_args.hpp"

using std::cout;
using std::cerr;
using std::endl;

int read_list (const std::string& flist_name, std::vector<std::string>& flist)
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
        rc = read_list (flist_name, flist);
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
        if (l > (std::numeric_limits<int>::max() - total_chars)) {
            cerr << "Total length of file names is too big for int type argumgnet of MPI_Bcast" << endl;
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

void create_local_files (const std::string& managed_dir,
                         const Worker& worker,
                         const mode_t md = S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH)
{
    auto range = worker.get_iterator ();
    auto it = range.first;
    auto it_end = range.second;
    const auto rank = worker.get_rank ();
    const auto& flist = worker.get_file_list ();

    std::stringstream ss;
    ss << "Owner rank " << rank << endl;
    std::string header = ss.str ();

    for (; it != it_end; ++it) {
        auto& fn = flist[*it];
        if (fn.empty ()) {
            continue;
        }
        auto path = managed_dir + '/' + fn;
        int fd = open (path.c_str (), O_CREAT | O_WRONLY, md);

        std::stringstream ss2;
        for (unsigned i = 0u; i <= rank; ++i) {
            ss2 << i << endl;
        }
        std::string str = header + fn + '\n' + ss2.str ();

        write (fd, str.c_str (), str.size ());
        close (fd);
    }
}

void read_files (const std::string& managed_dir, const Worker& worker, const bool validate = false)
{
    auto range = worker.get_iterator ();
    auto it = range.first;
    auto it_end = range.second;
    const auto rank = worker.get_rank ();
    const auto& flist = worker.get_file_list ();

    std::stringstream ss;
    ss << rank;

    for (; it != it_end; ++it) {
        auto& fn = flist[*it];
        if (fn.empty ()) {
            continue;
        }
        auto path = managed_dir + '/' + fn;
        int fd = open (path.c_str (), O_RDONLY);
        char* buf;
        auto sz = read_all (fd, reinterpret_cast<void**> (&buf));
        close (fd);

        if (validate) {
            auto fn_copy = managed_dir + "/copy." + ss.str () + '.' + fn;
            std::ofstream ofs;
            ofs.open (fn_copy);
            ofs.write (buf, sz);
            ofs.close ();
        }
    }
}

int main (int argc, char** argv)
{
    int rank;
    int n_ranks;
    int rc = EXIT_SUCCESS;

    ProgramOptions opts;

    rc = parse_args(argc, argv, opts);
    if (rc != EXIT_SUCCESS) {
        return rc;
    }

    MPI_Init (&argc, &argv);
    MPI_Comm_rank (MPI_COMM_WORLD, &rank);
    MPI_Comm_size (MPI_COMM_WORLD, &n_ranks);

    std::vector<std::string> flist;
    bcast_string_vector (opts.list_file, flist, 0, MPI_COMM_WORLD);

    if (opts.is_local) {  // if directory is local, everyone should create
        mode_t m = (S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH | S_ISGID);
        int c = mkdir_as_needed (opts.managed_dir.c_str (), m);
        if (c < 0) {
            cout << "Rank " << rank << " could not create directory: '" << opts.managed_dir << "'"
                 << endl;
            MPI_Abort (MPI_COMM_WORLD, errno);
            return EXIT_FAILURE;
        }
    } else if (rank == 0) {
        mode_t m = (S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH | S_ISGID);
        int c = mkdir_as_needed (opts.managed_dir.c_str (), m);
        if (c < 0) {
            cout << "Could not create directory: '" << opts.managed_dir << "'" << endl;
            MPI_Abort (MPI_COMM_WORLD, errno);
            return EXIT_FAILURE;
        }
    }
    MPI_Barrier (MPI_COMM_WORLD);

    Worker worker;
    worker.set_seed (opts.seed);
    worker.set_rank (rank);
    worker.set_file_list (flist);
    worker.split (n_ranks);

    /*
    const auto rg = worker.get_range();

    std::stringstream ss;

    ss << "Rank " << rank << " [" << rg.first << " - " << rg.second << "]";
    cout << ss.str() << std::endl;
    cout << worker.get_my_list_str() << endl << std::flush;
    */

    if (rank == 0) {
        cout << "Preparing local files under the managed directory" << endl;
    }
    create_local_files (opts.managed_dir, worker, 0644);

    MPI_Barrier (MPI_COMM_WORLD);

    for (unsigned i = 0u; i < opts.n_epochs; ++i) {
        if (rank == 0) {
            cout << "------ Shuffling ... -------" << endl << std::flush;
        }
        MPI_Barrier (MPI_COMM_WORLD);

        worker.shuffle ();
        // cout << i << ' ' << worker.get_my_list_str() << endl << std::flush;
        read_files (opts.managed_dir, worker);
        MPI_Barrier (MPI_COMM_WORLD);
    }

    MPI_Finalize ();

    return rc;
}
