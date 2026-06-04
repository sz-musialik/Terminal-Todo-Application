#include "includes/sqlite3.h"
#include <iso646.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

enum ERR { OK, ERROR };
sqlite3 *setupDB();

enum ERR addTask(sqlite3 *db);
enum ERR completeTask(sqlite3 *db);
enum ERR removeTask(sqlite3 *db);
enum ERR updateTask(sqlite3 *db);
enum ERR listTask(sqlite3 *db);

int tasksAmount(sqlite3 *db);
int tasksCompleted(sqlite3 *db);

void menu(sqlite3 *db);

int main(int argc, char **argv) {
  sqlite3 *db = setupDB();

  if (!db) {
    return 1;
  }

  menu(db);

  sqlite3_close(db);

  return 0;
}

sqlite3 *setupDB() {
  sqlite3 *db;
  int ret_code = sqlite3_open("todo.db", &db);

  if (ret_code != SQLITE_OK) {
    fprintf(stderr, "Cannot open the database %s\n", sqlite3_errmsg(db));

    return NULL;
  }

  return db;
}

void menu(sqlite3 *db) {
  printf("+--------------------------------------+\n");
  printf("|                                      |\n");
  printf("|       CONSOLE TODO APPLICATION       |\n");
  printf("|                                      |\n");
  printf("+--------------------------------------+\n\n");

  char input;
  int tasks_count;
  int completed_count;

  while (1) {
    if ((tasks_count = tasksAmount(db)) == -1)
      return;

    if ((completed_count = tasksCompleted(db)) == -1)
      return;

    printf("You have completed %d tasks\n", completed_count);
    printf("You currently have %d tasks\n\n", tasks_count);

    printf("What would you like to do\n");
    printf("[A]dd, [L]ist, [C]omplete, [U]pdate, [R]emove, [Q]uit\n");
    printf("> ");

    input = getchar();

    while (getchar() != '\n')
      ;
    printf("\n");

    enum ERR err;

    switch (input) {
    case 'A':
    case 'a':
      if ((err = addTask(db)) == ERROR)
        return;
      break;

    case 'L':
    case 'l':
      if ((err = listTask(db)) == ERROR)
        return;
      break;

    case 'C':
    case 'c':
      if ((err = completeTask(db)) == ERROR)
        return;
      break;

    case 'U':
    case 'u':
      if ((err = updateTask(db)) == ERROR)
        return;
      break;

    case 'R':
    case 'r':
      if ((err = removeTask(db)) == ERROR)
        return;
      break;

    case 'Q':
    case 'q':
      printf("Goodbye! Quiting...\n");
      return;
    }
  }
}

enum ERR addTask(sqlite3 *db) {
  char task_name[128];
  char sql[256];

  printf("\tEnter name of the task you want to ADD: ");
  fgets(task_name, sizeof(task_name), stdin);
  task_name[strcspn(task_name, "\n")] = '\0';
  sprintf(sql, "INSERT INTO Todos(Id, Title, Completed) VALUES(NULL, '%s', 0);",
          task_name);

  char *errMsg = 0;
  int ret_code = sqlite3_exec(db, sql, 0, 0, &errMsg);

  if (ret_code != SQLITE_OK) {
    fprintf(stderr, "Could not write to database: %s\n", errMsg);
    sqlite3_free(errMsg);

    return ERROR;
  }

  return OK;
}

enum ERR completeTask(sqlite3 *db) {
  char task_id[4];
  char sql[256];

  printf("\tEnter id of the task you want to mark as COMPLETED: ");
  fgets(task_id, sizeof(task_id), stdin);

  int id = atoi(task_id);

  sprintf(sql, "UPDATE Todos SET Completed=1 WHERE id==%d;", id);

  char *errMsg = 0;
  int ret_code = sqlite3_exec(db, sql, 0, 0, &errMsg);

  if (ret_code != SQLITE_OK) {
    fprintf(stderr, "Could not mark task as completed: %s\n", errMsg);
    sqlite3_free(errMsg);

    return ERROR;
  }

  return OK;
}

enum ERR removeTask(sqlite3 *db) {
  char task_id[4];
  char sql[256];

  printf("\tEnter id of the task you want to REMOVE: ");
  fgets(task_id, sizeof(task_id), stdin);

  int id = atoi(task_id);

  sprintf(sql, "DELETE FROM Todos WHERE id==%d;", id);

  char *errMsg = 0;
  int ret_code = sqlite3_exec(db, sql, 0, 0, &errMsg);

  if (ret_code != SQLITE_OK) {
    fprintf(stderr, "Could not remove the task: %s\n", errMsg);
    sqlite3_free(errMsg);

    return ERROR;
  }

  return OK;
}

enum ERR updateTask(sqlite3 *db) {
  char task_id[4];
  char task_name[128];
  char sql[256];

  printf("\tEnter id of the task you want to UPDATE: ");
  fgets(task_id, sizeof(task_id), stdin);

  int id = atoi(task_id);

  printf("\n\tEnter new name of the task: ");
  fgets(task_name, sizeof(task_name), stdin);
  task_name[strcspn(task_name, "\n")] = '\0';

  sprintf(sql, "UPDATE Todos SET Title='%s' WHERE id==%d;", task_name, id);

  char *errMsg = 0;
  int ret_code = sqlite3_exec(db, sql, 0, 0, &errMsg);

  if (ret_code != SQLITE_OK) {
    fprintf(stderr, "Could not update the task: %s\n", errMsg);
    sqlite3_free(errMsg);

    return ERROR;
  }

  return OK;
}

enum ERR listTask(sqlite3 *db) {
  sqlite3_stmt *stmt;
  const char *sql = "SELECT * FROM Todos;";

  int ret_code = sqlite3_prepare_v2(db, sql, -1, &stmt, NULL);

  if (ret_code != SQLITE_OK) {
    fprintf(stderr, "Could not list tasks: %s\n", sqlite3_errmsg(db));

    return ERROR;
  }

  printf("%-5s%-25s%s\n", "Id", "Task", "Completed");
  printf("----------------------------------------\n");

  while ((ret_code = sqlite3_step(stmt)) == SQLITE_ROW) {
    int id = sqlite3_column_int(stmt, 0);
    char *task = (char *)sqlite3_column_text(stmt, 1);
    int completed = sqlite3_column_int(stmt, 2);

    char *done = "[x]";

    if (!completed) {
      done = "[ ]";
    }

    printf("%-5d%-25s%s\n", id, task, done);
    printf("----------------------------------------\n");
  }

  return OK;
}

int tasksAmount(sqlite3 *db) {
  int count = 0;

  sqlite3_stmt *stmt;
  const char *sql = "SELECT COUNT(*) FROM Todos;";

  int ret_code = sqlite3_prepare_v2(db, sql, -1, &stmt, NULL);

  if (ret_code != SQLITE_OK) {
    fprintf(stderr, "Could not get count of tasks: %s\n", sqlite3_errmsg(db));

    return -1;
  }

  while ((ret_code = sqlite3_step(stmt)) != SQLITE_DONE) {
    count = sqlite3_column_int(stmt, 0);
  }

  return count;
}

int tasksCompleted(sqlite3 *db) {
  int count = 0;

  sqlite3_stmt *stmt;
  const char *sql = "SELECT COUNT(*) FROM Todos WHERE Completed=1;";

  int ret_code = sqlite3_prepare_v2(db, sql, -1, &stmt, NULL);

  if (ret_code != SQLITE_OK) {
    fprintf(stderr, "Could not get count of completed tasks: %s\n",
            sqlite3_errmsg(db));

    return -1;
  }

  while ((ret_code = sqlite3_step(stmt)) != SQLITE_DONE) {
    count = sqlite3_column_int(stmt, 0);
  }

  return count;
}
