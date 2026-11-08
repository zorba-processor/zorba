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

// Built only with ZORBA_CONTAINER_CHECKS (the glibcxx-debug preset): an
// iterator of a PtrVector (store/naive/ptr_vector.h) that is used after a
// change of the vector's contents or storage fails. Every member function
// that notifies the vector's readers (ContentReader::contentWillChange) is
// such a change, once it can no longer fail. Writing through an iterator,
// operator[], front() or back() is not, and an iterator into a copy of the
// contents (a plain pointer, as the node iterators use after
// ContentReader::copyContent) is not checked.
//
// A failed check aborts the process. The test sets PtrVectorCheck::theThrow
// to see each failure as a PtrVectorCheckFailure, and checks the abort itself
// in a child process.

#include <cstdint>
#include <cstdlib>
#include <cstring>
#include <iostream>
#include <stdexcept>
#include <string>
#include <vector>

#include <zorba/config.h>

#if defined(__unix__) || defined(__APPLE__)
#include <sys/wait.h>
#include <unistd.h>
#include <csignal>
#define CONTAINER_CHECKS_FORK 1
#endif

#include "store/naive/ptr_vector.h"

using namespace std;
using namespace zorba;
using namespace zorba::simplestore;

namespace {

int failures;

typedef PtrVector<int> Vec;

int cells[8];

void ok( bool cond, char const *what ) {
  if ( !cond ) {
    ++failures;
    cerr << "FAILED: " << what << endl;
  }
}

// Calls f and returns the condition of the check that failed, or "".
template<class F>
string error_of( F f ) {
  try {
    f();
  }
  catch ( PtrVectorCheckFailure const &e ) {
    return e.what();
  }
  return "";
}

template<class F>
void stale( F f, char const *what ) {
  if ( error_of( f ).empty() ) {
    ++failures;
    cerr << "FAILED: " << what << ": the check did not fail" << endl;
  }
}

template<class F>
void valid( F f, char const *what ) {
  string const err( error_of( f ) );
  if ( !err.empty() ) {
    ++failures;
    cerr << "FAILED: " << what << ": unexpected " << err << endl;
  }
}

// A vector with n elements, on the heap when n > INLINE_CAPACITY.
void fill( Vec &v, int n ) {
  v.reserve( 8 );
  for ( int i = 0; i < n; ++i )
    v.push_back( cells + i );
}

void test_stale() {
  // Every change makes the iterators taken before it stale: dereference,
  // increment, comparison and difference fail.
  {
    Vec v; fill( v, 3 );
    Vec::iterator i = v.begin();
    v.push_back( cells + 3 );   // no reallocation (capacity 8)
    stale( [&]{ return *i; }, "push_back: dereference" );
    stale( [&]{ ++i; }, "push_back: increment" );
    stale( [&]{ return i == v.begin(); }, "push_back: comparison" );
    stale( [&]{ return v.end() - i; }, "push_back: difference" );
  }
  {
    Vec v;
    v.push_back( cells + 0 );
    Vec::iterator i = v.begin();
    v.push_back( cells + 1 );
    v.push_back( cells + 2 );   // inline to heap
    stale( [&]{ return *i; }, "push_back with reallocation" );
  }
  {
    Vec v; fill( v, 3 );
    Vec::iterator i = v.begin() + 2;
    v.insert( v.begin(), cells + 5 );
    stale( [&]{ return *i; }, "insert" );
  }
  {
    Vec v; fill( v, 3 );
    vector<int*> src( 2, cells + 6 );
    Vec::const_iterator i = static_cast<Vec const&>( v ).begin();
    v.insert( v.end(), src.begin(), src.end() );
    stale( [&]{ return *i; }, "range insert: const_iterator" );
  }
  {
    Vec v; fill( v, 4 );
    Vec::iterator i = v.begin();
    v.erase( v.begin() + 1 );
    stale( [&]{ return *i; }, "erase" );
  }
  {
    Vec v; fill( v, 4 );
    Vec::iterator i = v.begin();
    v.clear();
    stale( [&]{ return *i; }, "clear" );
  }
  {
    Vec v; fill( v, 4 );
    Vec::iterator i = v.begin();
    v.resize( 2 );
    stale( [&]{ return *i; }, "resize" );
  }
  {
    Vec v;
    v.push_back( cells + 0 );
    Vec::iterator i = v.begin();
    v.reserve( 16 );
    stale( [&]{ return *i; }, "reserve with reallocation" );
  }
  {
    Vec v; fill( v, 3 );
    Vec::iterator i = v.begin();
    v.shrink_to_fit();
    stale( [&]{ return *i; }, "shrink_to_fit" );
  }
  {
    Vec a, b; fill( a, 1 ); fill( b, 3 );
    Vec::iterator i = a.begin(), j = b.begin();
    a.swap( b );
    stale( [&]{ return *i; }, "swap: this" );
    stale( [&]{ return *j; }, "swap: other" );
  }
  {
    Vec v; fill( v, 3 );
    Vec::reverse_iterator r = v.rbegin();
    v.erase( v.begin() );
    stale( [&]{ return *r; }, "erase: reverse_iterator" );
  }
  {
    Vec v; fill( v, 4 );
    Vec::iterator i = v.begin() + 3;
    v.erase( v.begin(), v.begin() + 2 );
    stale( [&]{ return *i; }, "erase of a range" );
    Vec::iterator j = v.begin();
    v.push_back( cells + 5 );
    stale( [&]{ v.erase( j, v.end() ); }, "erase of a range from a stale first" );
  }
  {
    // Every operation of a stale iterator fails, a converted one included.
    Vec v; fill( v, 3 );
    Vec::iterator i = v.begin() + 1;
    Vec::const_iterator c = i;   // iterator to const_iterator
    v.push_back( cells + 4 );
    stale( [&]{ return *c; }, "const_iterator converted from a stale iterator" );
    stale( [&]{ --i; }, "stale: decrement" );
    stale( [&]{ i += 1; }, "stale: +=" );
    stale( [&]{ return i[0]; }, "stale: []" );
    stale( [&]{ return i.operator->(); }, "stale: ->" );
    stale( [&]{ return i < v.end(); }, "stale: <" );
    stale( [&]{ return v.end() - c; }, "stale: difference with a const_iterator" );
  }
}

// An iterator of another vector passed to insert or erase. Without the
// checks this corrupts memory, so it runs last.
void test_foreign() {
  Vec v, w; fill( v, 3 ); fill( w, 3 );
  stale( [&]{ return v.begin() == w.begin(); },
         "comparison of two vectors' iterators" );
  stale( [&]{ return v.end() - w.begin(); },
         "difference of two vectors' iterators" );
  stale( [&]{ v.erase( w.begin() ); }, "erase with another vector's iterator" );
  stale( [&]{ v.insert( w.begin(), cells + 7 ); },
         "insert with another vector's iterator" );
}

void test_valid() {
  {
    // A converted const_iterator keeps the vector and the generation: valid
    // until the next change.
    Vec v; fill( v, 3 );
    Vec::const_iterator c = v.begin();
    valid( [&]{ ok( *c == cells + 0 && v.end() - c == 3, "converted" ); },
           "const_iterator converted from a valid iterator" );
  }
  {
    // A change that throws before it changes anything (here reserve beyond
    // the 32-bit capacity) leaves the iterators valid.
    Vec v; fill( v, 3 );
    Vec::iterator i = v.begin();
    bool threw = false;
    try {
      v.reserve( static_cast<zorba::csize>( UINT32_MAX ) + 1 );
    }
    catch ( std::length_error const& ) {
      threw = true;
    }
    ok( threw, "reserve beyond the capacity throws" );
    valid( [&]{ ok( *i == cells + 0, "after a failed reserve" ); },
           "iterator after a reserve that threw" );
  }
  {
    // The iterators that insert and erase return, and those taken after the
    // change, are valid.
    Vec v; fill( v, 3 );
    Vec::iterator i = v.insert( v.begin() + 1, cells + 5 );
    valid( [&]{ ok( *i == cells + 5, "insert result" ); }, "insert result" );
    i = v.erase( i );
    valid( [&]{ ok( *i == cells + 1, "erase result" ); }, "erase result" );
    valid( [&]{ ok( v.end() - v.begin() == 3, "size" ); }, "fresh iterators" );
  }
  {
    // Writing through an iterator, operator[], front() or back() is not a
    // change of the contents (the loaders fill nodes that way).
    Vec v; fill( v, 3 );
    Vec::iterator i = v.begin();
    Vec::iterator e = v.end();
    *i = cells + 4;
    v[1] = cells + 5;
    v.front() = cells + 6;
    v.back() = cells + 7;
    valid( [&]{ ok( *i == cells + 6 && e - i == 3, "writes" ); },
           "writes through the vector" );
  }
  {
    // reserve() within the capacity and shrink_to_fit() on an inline vector
    // do not change the storage.
    Vec v; fill( v, 3 );
    Vec::iterator i = v.begin();
    v.reserve( 4 );
    valid( [&]{ return *i; }, "reserve within the capacity" );
    Vec w; w.push_back( cells + 0 );
    Vec::iterator j = w.begin();
    w.shrink_to_fit();
    valid( [&]{ return *j; }, "shrink_to_fit inline" );
  }
  {
    // An iterator into a copy of the contents is not checked: the node
    // iterators go on that way after ContentReader::copyContent().
    Vec v; fill( v, 3 );
    vector<int*> copy( v.begin(), v.end() );
    Vec::iterator i = copy.data();
    Vec::iterator e = copy.data() + copy.size();
    v.clear();
    valid( [&]{ ok( *i == cells + 0 && e - i == 3, "copy" ); },
           "iterator into a copy" );
    Vec::iterator n = Vec::iterator();
    valid( [&]{ ok( n == Vec::iterator(), "default" ); },
           "default-constructed iterator" );
  }
}

// Without theThrow a failed check prints its condition and aborts, so that
// a query cannot catch it (try/catch * catches ZXQP0002).
void test_abort() {
#ifdef CONTAINER_CHECKS_FORK
  int fds[2];
  if ( pipe( fds ) != 0 ) {
    ok( false, "abort: pipe" );
    return;
  }
  pid_t const pid = fork();
  if ( pid == 0 ) {
    dup2( fds[1], 2 );
    close( fds[0] );
    Vec v; fill( v, 3 );
    Vec::iterator i = v.begin();
    v.push_back( cells + 3 );
    try {
      int *volatile p = *i;
      (void)p;
    }
    catch ( ... ) {
      _exit( 3 );   // caught: the check must not throw
    }
    _exit( 0 );     // not detected
  }
  close( fds[1] );
  string err;
  char buf[512];
  ssize_t n;
  while ( ( n = read( fds[0], buf, sizeof buf ) ) > 0 )
    err.append( buf, n );
  close( fds[0] );
  int status = 0;
  waitpid( pid, &status, 0 );
  ok( WIFSIGNALED( status ) && WTERMSIG( status ) == SIGABRT,
      "abort: a stale iterator aborts the process" );
  ok( err.find( "PtrVector iterator check failed" ) != string::npos &&
      err.find( "ptr_vector.h:" ) != string::npos,
      "abort: the message names the check and its location" );
#endif
}

} // namespace

namespace zorba {
namespace UnitTests {

int test_container_checks( int, char*[] ) {
  failures = 0;
  PtrVectorCheck::theThrow = true;
  test_stale();
  test_valid();
  test_foreign();
  PtrVectorCheck::theThrow = false;
  test_abort();
  cout << ( failures ? "FAILED" : "passed" ) << endl;
  return failures ? 1 : 0;
}

} // namespace UnitTests
} // namespace zorba
/* vim:set et sw=2 ts=2: */
