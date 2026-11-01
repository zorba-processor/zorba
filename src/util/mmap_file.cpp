/*
 * Copyright 2006-2016 zorba.io
 * 
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 * 
 * http://www.apache.org/licenses/LICENSE-2.0
 * 
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include <fcntl.h>                     /* for open(2) */
#include <sys/mman.h>                  /* for mmap(2) */
#include <sys/stat.h>                  /* for stat(2) */
#include <unistd.h>                    /* for close(2) */

#include <zorba/internal/cxx_util.h>
#include <zorba/util/error_util.h>

#include "diagnostics/xquery_diagnostics.h"

#include "mmap_file.h"
#include "string_util.h"

using namespace std;

namespace zorba {

void mmap_file::close() {
  if ( addr_ && ::munmap( static_cast<char*>( addr_ ), size_ ) == -1 )
    throw ZORBA_IO_EXCEPTION( "munmap()", path_ );
  if ( fd_ != -1 )
    ::close( fd_ );
  init();
}

void mmap_file::init() {
  fd_ = -1;
  size_ = 0;
  addr_ = nullptr;
}

void mmap_file::open( char const *path, ios::openmode mode ) {
  close();
  path_ = path;


  // O_RDONLY, O_WRONLY and O_RDWR are values, not bits (O_RDONLY is 0), so
  // they cannot be OR'd together. And mmap(2) needs a descriptor open for
  // reading even for a write-only mapping, so any write access means O_RDWR.
  int const flags = (mode & ios::out) ? O_RDWR : O_RDONLY;
  int prot = PROT_NONE;
  if ( mode & ios::in )
    prot |= PROT_READ;
  if ( mode & ios::out )
    prot |= PROT_WRITE;

  if ( (fd_ = ::open( path, flags )) == -1 )
    throw ZORBA_IO_EXCEPTION( "open()", path_ );

  // Get the size from the opened descriptor, not the path, so it describes
  // the file actually being mapped.
  struct stat stat_buf;
  if ( ::fstat( fd_, &stat_buf ) == -1 )
    throw ZORBA_IO_EXCEPTION( "fstat()", path_ );
  size_ = stat_buf.st_size;

  addr_ = ::mmap( nullptr, size_, prot, MAP_SHARED, fd_, 0 );
  if ( addr_ == MAP_FAILED ) {
    addr_ = nullptr;
    throw ZORBA_IO_EXCEPTION( "mmap()", path_ );
  }

}

} // namespace zorba
/* vim:set et sw=2 ts=2: */
