#include "multi_task.h"

MultiTask::MultiTask(const char *id) : Task(id) {}

void MultiTask::add(uint8_t stage, Task *task, bool mustFinish)
{
  tasks[totalTasks] = task;
  flags[totalTasks++] = (stage << 1) | mustFinish;
}

void MultiTask::initiateAttempt()
{
  currentStage = 0;
  runTasksForCurrentStage();
}

void MultiTask::process()
{
  bool tasksFinished = true;
  bool tasksSuccessful = true;

  for (uint8_t i = 0; i < totalTasks; i++)
  {
    Task *task = tasks[i];
    uint8_t taskStage = flags[i] >> 1;
    bool taskMustFinish = flags[i] & 0x01;

    if (taskStage == currentStage)
    {
      task->tick();

      if (taskMustFinish)
      {
        tasksFinished &= !task->isRunning();
        tasksSuccessful &= task->isSuccessful();
      }
    }
  }

  if (!tasksFinished) return;
  
  if (tasksSuccessful)
  {
    currentStage++;
    runTasksForCurrentStage();
  }
  else
  {
    endAttempt(false);
  }
}

void MultiTask::endAttempt(bool success)
{
  // Stop all tasks
  for (uint8_t i = 0; i < totalTasks; i++)
  {
    tasks[i]->stop();
  }

  Task::endAttempt(success);
}

void MultiTask::runTasksForCurrentStage()
{
  bool tasksFound = false;

  for (uint8_t i = 0; i < totalTasks; i++)
  {
    Task *task = tasks[i];
    uint8_t taskStage = flags[i] >> 1;

    if (taskStage == currentStage)
    {
      tasksFound = true;
      task->run();
    }
  }

  if (!tasksFound) endAttempt(true);
}