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
  std::unordered_map<DIR *, std::vector<std::string>> hard_links_;
  int links_amnt = 0;
  int printed_links = 0;
  /// Class for parcing and storing cmd line args

public:
  Directory(DIR *dir, const char *rel_name)
      : dir_(dir), path_(rel_name), rel_name_(rel_name) {}
  Directory(DIR *dir, const Directory &old_dir, const char *rel_name)
      : dir_(dir), rel_name_(rel_name), files_() {
    path_ = old_dir.path_ + '/' + rel_name_;
  }

  ~Directory() { std::cout << "\n\n"; }
  /*
  for (const auto &link : hard_links_)
  {
    rewinddir(link.first);
    dirent *curr_dir = readdir(link.first);
    if (curr_dir == nullptr)
    {
      PRINT_LINE;
      continue;
    }

    struct stat dir_info;
    if (fstatat(dirfd(link.first), curr_dir->d_name, &dir_info, 0) == -1)
    {
      perror("fstatat failed");
      exit(exit_failure);
    }

    int links_amnt = 0;
    for (const auto &name : link.second)
    {
      links_amnt++;
      std::cout << name << '\n';
    }

    if (links_amnt != dir_info.st_nlink)
      std::cout << "[...incomplete...]\n";

    std::cout << "\n\n";
  }

}*/

  /// Function for dislaying whole directory
  void traverseDir() {
    assert(dir_);

    // We need to get list of all files in dir
    rewinddir(dir_);
    while (dirent *dir = readdir(dir_)) {
      if (links_amnt == 0) {
        struct stat dir_info;
        if (fstatat(dirfd(dir_), dir->d_name, &dir_info, 0) == -1) {
          perror("fstatat failed");
          exit(EXIT_FAILURE);
        }

        links_amnt = dir_info.st_nlink;
      }

      files_.push_back(dir);
    }

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

    // direntDump(dir);

    if (dir->d_type == DT_DIR) {
      std::string rel_name = dir->d_name;
      if (rel_name == std::string(".") || rel_name == std::string(".."))
        return;

      DIR *directory = opendir(rel_name.c_str());
      if (directory == nullptr)
        return;

      Directory child_dir(directory, *this, rel_name.c_str());
      child_dir.traverseDir();

      closedir(directory);
      return;
    }

    struct stat file_info;
    if (fstatat(dirfd(dir_), dir->d_name, &file_info, 0) == -1) {
      perror("fstatat failed");
      exit(EXIT_FAILURE);
    }

    printed_links++;
    std::cout << path_ + '/' + dir->d_name << '\n';

    // hard_links_[dir_].push_back(path_ + '/' + dir->d_name);
  }
};
} // anonymous namespace

int main(const int argc, char *const argv[]) {
  DIR *directory = opendir(".");
  if (directory == nullptr) {
    perror("opendir failed");
    exit(EXIT_FAILURE);
  }

  Directory dir(directory, ".");
  dir.traverseDir();

  closedir(directory);
  return 0;
}
