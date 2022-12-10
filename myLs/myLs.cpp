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
  /// Class for parcing and storing cmd line args
  struct Options {
    unsigned i : 1 = 0;
    unsigned R : 1 = 0;
    unsigned a : 1 = 0;
    unsigned l : 1 = 0;
    unsigned n : 1 = 0;

    Options(const int argc, char *const argv[]) {
      assert(argv);

      int opt;
      while ((opt = getopt(argc, argv, "liRan")) != -1) {
        print_debug({ SAFE_PRINTF("opt = %d\n", opt); })

            switch (opt) {
        case 'i':
          i = 1;
          break;
        case 'R':
          R = 1;
          break;
        case 'a':
          a = 1;
          break;
        case 'l':
          l = 1;
          break;
        case 'n':
          n = 1;
          break;
          // default:
          // std::cout << "Unknown cmd line option" << char (opt);
        }
      }
    }

    inline void dump() {
      std::cout << "Options dump: ";
      std::cout << "i = " << i << " R = " << R << " a = " << a << " l = " << l
                << " n = " << n << '\n';
    }
  } opts_;

public:
  Directory(DIR *dir, const char *rel_name, const int argc, char *const argv[])
      : dir_(dir), path_(rel_name), rel_name_(rel_name), opts_(argc, argv) {}
  Directory(DIR *dir, const Directory &old_dir, const char *rel_name)
      : dir_(dir), rel_name_(rel_name), files_(), opts_(old_dir.opts_) {
    path_ = this->path_ + '/' + rel_name_;
  }

  /// Function for dislaying whole directory
  void displayListDir() {
    assert(dir_);

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
      displayDirent(file);
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

#define RIGHTS_TRIPLET(mode_val, shift, who)                                   \
  {                                                                            \
    rights[shift] = (mode_val & S_IR##who) ? 'r' : '-';                        \
    rights[shift + 1] = (mode_val & S_IW##who) ? 'w' : '-';                    \
    rights[shift + 2] = (mode_val & S_IX##who) ? 'x' : '-';                    \
  }
  /// Method for printing all rights
  void printRights(mode_t mode) {
    char rights[10] = {0};

    RIGHTS_TRIPLET(mode, 0, USR);
    RIGHTS_TRIPLET(mode, 3, GRP);
    RIGHTS_TRIPLET(mode, 6, OTH);

    printf("%s ", rights);
    return;
  }

  /// Function for dislaying single file
  void displayDirent(const dirent *dir) {
    assert(dir);

    // We should print hidden files only with -a flag
    if (!opts_.a && (dir->d_name == path_ || dir->d_name[0] == '.'))
      return;

    if (opts_.R && dir->d_type == DT_DIR) {
      std::string rel_name = dir->d_name;
      if (rel_name == std::string(".") || rel_name == std::string(".."))
        return;

      DIR *directory = opendir(rel_name.c_str());
      if (directory == nullptr)
        return;

      std::cout << path_ + '/' + rel_name << ":\n";

      Directory child_dir(directory, *this, rel_name.c_str());
      child_dir.displayListDir();

      std::cout << '\n' << path_ << ":\n";

      closedir(directory);
      return;
    }

    // Printing i-node number before name
    if (opts_.i)
      std::cout << dir->d_ino << " ";

    if (opts_.l || opts_.n) {
      struct stat file_info;
      if (fstatat(dirfd(dir_), dir->d_name, &file_info, 0) == -1) {
        perror("stat failed");
        exit(EXIT_FAILURE);
      }
      // Printing right for usr and group
      printRights(file_info.st_mode);
      // Printing amnt of hardlinks
      printf("%3ld ", file_info.st_nlink);
      // Printing names
      printUserAndGroupNames(file_info.st_uid);
      // Printing file size
      printf("%8ld ", file_info.st_size);
      // Print time
      char *time_buff = ctime(&file_info.st_mtim.tv_sec);
      time_buff[strlen(time_buff) - 1] = ' ';
      std::cout << time_buff;
    }

    std::cout << dir->d_name << ((opts_.l || opts_.n) ? '\n' : ' ');
  }

  void printUserAndGroupNames(uid_t uid) {
    passwd *pwd = getpwuid(uid);
    if (pwd == nullptr) {
      perror("getpwuid failed");
      exit(EXIT_FAILURE);
    }
    passwd *grpwd = getpwuid(pwd->pw_gid);
    if (pwd == nullptr) {
      perror("getpwuid failed");
      exit(EXIT_FAILURE);
    }

    if (!opts_.n)
      std::cout << pwd->pw_name << ' ' << grpwd->pw_name << ' ';
    else
      std::cout << pwd->pw_uid << ' ' << grpwd->pw_uid << ' ';
  }
};
} // anonymous namespace

int main(const int argc, char *const argv[]) {
  DIR *directory = opendir(".");
  if (directory == nullptr) {
    perror("opendir failed");
    exit(EXIT_FAILURE);
  }

  Directory dir(directory, ".", argc, argv);
  dir.displayListDir();

  closedir(directory);
  return 0;
}
