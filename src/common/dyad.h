/*****************************************************************************\
 *  Copyright (c) 2018 Lawrence Livermore National Security, LLC.  Produced at
 *  the Lawrence Livermore National Laboratory (cf, AUTHORS, DISCLAIMER.LLNS).
 *  LLNL-CODE-658032 All rights reserved.
 *
 *  This file is part of the Flux resource manager framework.
 *  For details, see https://github.com/flux-framework.
 *
 *  This program is free software; you can redistribute it and/or modify it
 *  under the terms of the GNU General Public License as published by the Free
 *  Software Foundation; either version 2 of the license, or (at your option)
 *  any later version.
 *
 *  Flux is distributed in the hope that it will be useful, but WITHOUT
 *  ANY WARRANTY; without even the IMPLIED WARRANTY OF MERCHANTABILITY or
 *  FITNESS FOR A PARTICULAR PURPOSE.  See the terms and conditions of the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License along
 *  with this program; if not, write to the Free Software Foundation, Inc.,
 *  59 Temple Place, Suite 330, Boston, MA 02111-1307 USA.
 *  See also:  http://www.gnu.org/licenses/
\*****************************************************************************/

#ifndef DYAD_H
#define DYAD_H

/*****************************************************************************
 *                                                                           *
 *                          DYAD Macro Definitions                           *
 *                                                                           *
 *****************************************************************************/

#define DYAD_KIND_PROD_ENV "DYAD_KIND_PRODUCER"
#define DYAD_KIND_CONS_ENV "DYAD_KIND_CONSUMER"
#define DYAD_PATH_PROD_ENV "DYAD_PATH_PRODUCER"
#define DYAD_PATH_CONS_ENV "DYAD_PATH_CONSUMER"
#define DYAD_CHECK_ENV     "DYAD_SYNC_HEALTH"
#define DYAD_PATH_DELIM    "/"
#define DYAD_NUM_CPA_PTS   "DYAD_NUM_CPA_POINTS"

#endif // DYAD_H
