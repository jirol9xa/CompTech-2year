#include <algorithm>
#include <cassert>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <ctime>
#include <dirent.h>
#include <getopt.h>
#include <grp.h>
#include <iostream>
#include <pwd.h>
#include <string>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>
#include <unordered_map>
#include <vector>

#define DEBUG_MODE 0
#include "../debugLib/deb_macro.h"

namespace {
/// Class implementation of directory
class Directory {
private:
  DIR *dir_;
  std::string path_, rel_name_;
  std::vector<dirent *> files_;
  std::unordered_map<ino64_t, std::vector<std::string>> hard_links_;
  /// Class for parcing and storing cmd line args

public:
  Directory(DIR *dir, const char *rel_name)
      : dir_(dir), path_(rel_name), rel_name_(rel_name) {}
  Directory(DIR *dir, const Directory &old_dir, const char *rel_name)
      : dir_(dir), rel_name_(rel_name), files_() {
    path_ = old_dir.path_ + '/' + rel_name_;

    print_debug({ SAFE_PRINTF("New path = %s\n", path_.c_str()); });
  }

  ~Directory() {

    bool is_print_smth = false;
    for (const auto &link : hard_links_) {
      if (link.second.size() < 2)
        continue;

      for (const auto &hard_links : link.second) {
        is_print_smth = true;
        std::cout << hard_links << '\n';
      }
    }

    if (is_print_smth)
      std::cout << "\n\n";
  }

  /// Function for dislaying whole directory
  void traverseDir() {
    assert(dir_);

    PRINT_LINE;

    // We need to get list of all files in dir
    rewinddir(dir_);
    while (dirent *dir = readdir(dir_))
      files_.push_back(dir);

    if (files_.size() == 0)
      return;

    // Now we need to sort it by names
    std::sort(files_.begin(), files_.end(), [](dirent *a, dirent *b) {
      return std::string(std::move(a->d_name)) <
             std::string(std::move(b->d_name));
    });

    for (const auto &file : files_)
      visitDirent(file);
  }

private:
  inline void direntDump(const dirent *dir) {
    assert(dir);

    std::cout << "Dirent dump:\n";
    std::cout << "Name: " << dir->d_name
              << " Type of file: " << unsigned(dir->d_type)
              << " D_off: " << dir->d_off << " Inode nimber : " << dir->d_ino
              << " Length: " << dir->d_reclen << '\n';
  }

  /// Function for dislaying single file
  void visitDirent(const dirent *dir) {
    assert(dir);

    PRINT_LINE;
    print_debug({ SAFE_PRINTF("dir name = %s\n", dir->d_name); });

    if (dir->d_type == DT_DIR) {
      PRINT_LINE;

      std::string rel_name = dir->d_name;
      if (rel_name == std::string(".") || rel_name == std::string(".."))
        return;

      DIR *directory = opendir((path_ + '/' + rel_name).c_str());
      if (directory == nullptr)
        return;

      Directory child_dir(directory, *this, rel_name.c_str());
      child_dir.traverseDir();

      closedir(directory);
      return;
    }

    /*print_debug({
      SAFE_PRINTF("I-node number = %ld, path = %s\n", dir->d_ino,
                  (path_ + '/' + dir->d_name).c_str());
    });*/

    hard_links_[dir->d_ino].push_back(path_ + '/' + dir->d_name);
  }
};
} // anonymous namespace

int main(const int argc, char *const argv[]) {
  if (argc < 2) {
    std::cout << "Write in correct format <path>\n";
    exit(EXIT_FAILURE);
  }

  DIR *directory = opendir(argv[1]);
  if (directory == nullptr) {
    perror("opendir failed");
    exit(EXIT_FAILURE);
  }

  PRINT_LINE;

  Directory dir(directory, argv[1]);
  dir.traverseDir();

  closedir(directory);
  return 0;
}
