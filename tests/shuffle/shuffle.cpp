#include <libgen.h>  // basename dirname
#include <mpi.h>
#include <sys/types.h>

#include <cerrno>
#include <cstdlib>
#include <cstring>
#include <iostream>
#include <limits>
#include <vector>

#include <dyad/utils/utils.h>
#include "cli_args.hpp"
#include "utils.hpp"
#include "worker.hpp"

using std::cerr;
using std::cout;
using std::endl;

int main (int argc, char** argv)
{
    int rank;
    int n_ranks;
    int rc = EXIT_SUCCESS;

    ProgramOptions opts;

    rc = parse_args (argc, argv, opts);
    if (rc != EXIT_SUCCESS) {
        return rc;
    }

    MPI_Init (&argc, &argv);
    MPI_Comm_rank (MPI_COMM_WORLD, &rank);
    MPI_Comm_size (MPI_COMM_WORLD, &n_ranks);

    std::vector<std::string> flist;
    if (opts.use_count) {
        populate_file_list (flist, static_cast<size_t> (opts.count));
    } else {
        bcast_string_vector (opts.list_file, flist, 0, MPI_COMM_WORLD);
    }

    if (opts.is_local) {  // if directory is local, everyone should create
        mode_t m = (S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH | S_ISGID);
        int c = mkdir_as_needed (opts.work_dir.c_str (), m);
        if (c < 0) {
            cout << "Rank " << rank << " could not create directory: '" << opts.work_dir << "'"
                 << endl;
            MPI_Abort (MPI_COMM_WORLD, errno);
            return EXIT_FAILURE;
        }
    } else if (rank == 0) {
        mode_t m = (S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH | S_ISGID);
        int c = mkdir_as_needed (opts.work_dir.c_str (), m);
        if (c < 0) {
            cout << "Could not create directory: '" << opts.work_dir << "'" << endl;
            MPI_Abort (MPI_COMM_WORLD, errno);
            return EXIT_FAILURE;
        }
    }
    MPI_Barrier (MPI_COMM_WORLD);

    Worker worker;
    if (opts.seed_set) {
        worker.set_seed (opts.seed);
    } else {
        worker.set_seed_by_clock ();
    }
    worker.set_rank (rank);
    worker.set_file_list (flist);
    worker.set_file_size (opts.fsize);
    worker.set_work_dir (opts.work_dir);
    worker.split (n_ranks);

    /*
    const auto rg = worker.get_range();

    std::stringstream ss;

    ss << "Rank " << rank << " [" << rg.first << " - " << rg.second << "]";
    cout << ss.str() << std::endl;
    cout << worker.get_my_list_str() << endl << std::flush;
    */

    if (rank == 0) {
        cout << "Preparing local files under the working data directory" << endl;
    }
    create_files (worker, 0644);

    MPI_Barrier (MPI_COMM_WORLD);

    for (unsigned i = 0u; i < opts.n_epochs; ++i) {
        if (rank == 0) {
            cout << "------ Shuffling ... -------" << endl << std::flush;
        }
        MPI_Barrier (MPI_COMM_WORLD);

        worker.shuffle ();
        // cout << i << ' ' << worker.get_my_list_str() << endl << std::flush;
        read_files (worker);
        MPI_Barrier (MPI_COMM_WORLD);
    }

    MPI_Finalize ();

    return rc;
}
