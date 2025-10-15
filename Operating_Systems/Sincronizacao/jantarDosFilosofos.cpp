#include <iostream>
#include <thread>
#include <vector>
#include <chrono>
#include <mutex>



using std::thread;
using std::cout;
using std::endl;
using std::vector;
using std::this_thread::sleep_for;
using std::rand;
using std::chrono::milliseconds;
using std::mutex;
using std::lock;
using std::lock_guard;
using std::adopt_lock;



/**
 * Class that implements a dinner Table with all the forks resources
 */
class Table {
  public:

    /* The shared resource that each Philosopher will concur */
    vector<mutex> forks;

    Table (): forks(5)
    {
    }
};



/**
 * Class that implements the existence of a Philosopher that is
 * to think and eat
 */
class Philosopher {

  public:
    Philosopher (const int id, Table& table)
    :philosopher_id(id),
     table(table)
    {
    }

    /*
     * Spawns the parallel execution of a Philosopher that shares the same
     * table with other Philosophers
     */
    thread* Spawn ()
    {
      return new thread([=] { exist(); });
    }

  private:
    /* The Philosopher identifier */
    int philosopher_id;

    /* A reference to the shared table instance */
    Table& table;

    /**
     * The existence of a Philosopher
     */
    void exist ()
    {
      while (true) {

        think();
        eat();
      }
    }

    /**
     * The randomized time that each Philosopher spends thinking
     */
    void think ()
    {
      cout << "Philosopher " << philosopher_id << " thinking.." << endl;
    }

    /**
     * The eating process of each Philosopher instance. First of all, a
     * Philosopher tries to lock both forks that he needs to eat. There
     * are 5 forks. If one fork is acquired by any other Philosopher, the
     * thread will wait until the resource/fork is released.
     *
     * When both forks are acquired by the current thread execution, it time
     * to the Philosopher to eat. At the end of this process both forks are
     * released.
     */
    void eat ()
    {
      int left = philosopher_id;
      int right = (philosopher_id + 1) % table.forks.size();

      cout << "Philosopher " << philosopher_id << " taking forks.." << endl;

      /* Locks boths mutexes without any deadlock */
      lock(table.forks[left], table.forks[right]);

      /* At the end of this scope mutexes will be released */
      lock_guard<mutex> left_fork(table.forks[left], adopt_lock);
      lock_guard<mutex> right_fork(table.forks[right], adopt_lock);

      cout << "Philosopher " << philosopher_id << " eating.." << endl;

      cout << "Philosopher " << philosopher_id << " releasing fork.." << endl;
    }
};



int
main (int argc, char* argv[])
{
  cout << "THE PHILOSOPHERS DINNER" << endl << endl;
  Table table;

  vector<thread *> threads(5);

  for(int i=0; i < 5; i++) {

    Philosopher* philosopher = new Philosopher(i, table);
    threads[i] = philosopher->Spawn();
  }

  for(int i=0; i < 5; i++) {

    threads[i]->join();
  }

  return EXIT_SUCCESS;
}