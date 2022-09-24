#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <assert.h>
#include <pwd.h>
#include <grp.h>
#include <stdlib.h>

#define IS_VALID(param) {                 \
  if (!param)                             \
  {                                       \
    printf("Invalid " #param "ptr ");     \
    return -1;                            \
  }                                       \
}

void printGroups(gid_t *groups, int group_amnt)
{
  IS_VALID(groups);
  
  struct group *gr;

  printf("%d", groups[0]);

  gr = getgrgid(groups[0]);
  printf("(%s)", gr->gr_name);

  for (int i = 1; i < group_amnt; ++i)
  {
    printf(",%d", groups[i]);
  
    gr = getgrgid(groups[i]);
    printf("(%s)", gr->gr_name);
  }
}

int main(const int argc, char * const argv[])
{
  if (argc > 3)
    printf("Warning! Only 1st cmd line param will be processed\n");

  struct passwd *info;
  int groups_amnt = 11;  // in real id 9 ;)
  gid_t *groups = malloc(groups_amnt * sizeof(gid_t));
  if (!groups)
  {
    perror("Malloc");
    free(groups);
    return 0;
  }

  if (argc == 1)
  {
    info = getpwuid(getuid());
    while ((groups_amnt = getgroups(groups_amnt, groups)) == -1)
    {
      groups = realloc(groups, ++groups_amnt * sizeof(gid_t));
      if (!groups)
      {
        perror("Realloc");
        return 0;
      }
    }

    printf("uid=%d(%s) gid=%d(%s) groups=", getuid(), info->pw_name, getgid(),
            info->pw_name);
    
    printGroups(groups, groups_amnt);

    free(groups);
    return 0;
  }

  info = getpwnam(argv[1]);
  
  if (!info)
  {
    perror("User");
    return 0;
  }

  printf("uid=%d(%s) gid=%d(%s) groups=", info->pw_uid, info->pw_name, info->pw_gid,
          info->pw_name);
  
  if (getgrouplist(info->pw_name, info->pw_gid, groups, &groups_amnt) == -1)
  {
    groups = realloc(groups, groups_amnt * sizeof(gid_t));
    if (!groups)
    {
      perror("Realloc");
      free(groups);
      return 0;
    }
    getgrouplist(info->pw_name, info->pw_gid, groups, &groups_amnt);
  }
  
  printGroups(groups, groups_amnt);

  free(groups);
  return 0;
}