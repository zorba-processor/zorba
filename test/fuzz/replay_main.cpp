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

// Standalone replay driver for the fuzz targets: links against a target's
// LLVMFuzzerInitialize/LLVMFuzzerTestOneInput instead of libFuzzer, so the
// regression inputs run with any compiler (GCC included) and under whatever
// sanitizers the build uses.
//
//   <target>_replay <file-or-directory>...
//
// Directories are read when the driver runs (not when CMake configures), so
// a file dropped into test/fuzz/regressions/<target>/ is replayed without
// touching any CMake file. A crash (abort, sanitizer report) fails the run;
// the input being replayed is printed first so the log names it.

#include <algorithm>
#include <cstdint>
#include <cstdio>
#include <filesystem>
#include <fstream>
#include <iterator>
#include <string>
#include <system_error>
#include <vector>

extern "C" int LLVMFuzzerInitialize(int* argc, char*** argv);
extern "C" int LLVMFuzzerTestOneInput(const uint8_t* data, size_t size);

namespace fs = std::filesystem;

int main(int argc, char** argv)
{
  if (argc < 2)
  {
    std::fprintf(stderr, "usage: %s <file-or-directory>...\n", argv[0]);
    return 2;
  }

  std::vector<fs::path> inputs;
  for (int i = 1; i < argc; ++i)
  {
    fs::path arg(argv[i]);
    std::error_code ec;
    if (fs::is_directory(arg, ec))
    {
      std::vector<fs::path> files;
      for (fs::directory_entry const& e : fs::directory_iterator(arg))
        if (e.is_regular_file())
          files.push_back(e.path());
      std::sort(files.begin(), files.end());
      inputs.insert(inputs.end(), files.begin(), files.end());
    }
    else if (fs::is_regular_file(arg, ec))
    {
      inputs.push_back(arg);
    }
    else
    {
      std::fprintf(stderr, "%s: no such file or directory\n", argv[i]);
      return 2;
    }
  }

  LLVMFuzzerInitialize(&argc, &argv);

  for (fs::path const& p : inputs)
  {
    std::ifstream in(p, std::ios::binary);
    if (!in)
    {
      std::fprintf(stderr, "%s: cannot read\n", p.string().c_str());
      return 2;
    }
    std::vector<char> buf((std::istreambuf_iterator<char>(in)),
                          std::istreambuf_iterator<char>());
    std::printf("replay %s (%zu bytes)\n", p.string().c_str(), buf.size());
    std::fflush(stdout);
    static const uint8_t empty = 0;  // libFuzzer never passes a null pointer
    LLVMFuzzerTestOneInput(
        buf.empty() ? &empty : reinterpret_cast<const uint8_t*>(buf.data()),
        buf.size());
  }

  std::printf("replayed %zu input(s)\n", inputs.size());
  return 0;
}
