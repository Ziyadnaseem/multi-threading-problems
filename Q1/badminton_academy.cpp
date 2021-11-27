#include <iostream>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <math.h>
#include <unistd.h>
#include <semaphore.h>
#include <bits/stdc++.h>

using namespace std;

int players_ready_count = 0;
int referees_ready_count = 0;
int warm_up_players = 0;
int referee_start = 0;
pthread_mutex_t organizer;
pthread_mutex_t court;
pthread_mutex_t print;
pthread_mutex_t warm;
pthread_mutex_t start_game_m;
pthread_cond_t cond1 = PTHREAD_COND_INITIALIZER;
pthread_cond_t player_cond = PTHREAD_COND_INITIALIZER;
pthread_cond_t referee_cond = PTHREAD_COND_INITIALIZER;
pthread_cond_t start_game_condition = PTHREAD_COND_INITIALIZER;
int count_critical = 0;

void display(string msg)
{
    // Function to print messages in terminal
    pthread_mutex_lock(&print);
    cout << msg << endl;
    pthread_mutex_unlock(&print);
}

void enterAcademy(string id)
{
    string val = id + " has entered academy";
    display(val);
}

void playerMeetOrganizer(string id)
{
    // If more than 2 players meet oragnizer then the respective threads sleep
    // until game starts for previous group
    pthread_mutex_lock(&organizer);
    if (players_ready_count >= 2)
    {
        pthread_cond_wait(&player_cond, &organizer);
    }
    else
    {
        players_ready_count++;
    }
    string val = id + " Meeting Organizer";
    display(val);
    pthread_mutex_unlock(&organizer);
}

void refMeetOrganizer(string id)
{
    // If more than 1 referee meet oragnizer then the respective threads sleep
    // until game starts for previous group
    pthread_mutex_lock(&organizer);
    if (referees_ready_count >= 1)
    {
        pthread_cond_wait(&referee_cond, &organizer);
    }
    else
    {
        referees_ready_count++;
    }
    string val = id + " Meeting Organizer";
    display(val);
    pthread_mutex_unlock(&organizer);
}

void enterCourt(string id)
{
    // All threads entering will sleep until a total of 3 threads call this function
    // The 3rd thread wakes up other 2 threads
    pthread_mutex_lock(&court);
    if (count_critical == 2)
    {
        count_critical = 0;
        pthread_cond_broadcast(&cond1);
    }
    else
    {
        count_critical++;
        pthread_cond_wait(&cond1, &court);
    }
    string val = id + " Entering Court";
    display(val);
    pthread_mutex_unlock(&court);
}

// Player Functions
void warmUp(string id)
{
    // After warming up, thread increases the global counter and when warm_up_players == 2
    // then wake up referee thread in startGame if it is in sleep
    string val = id + " Player Warming Up";
    display(val);
    sleep(1);
    pthread_mutex_lock(&warm);
    warm_up_players++;
    if (warm_up_players == 2)
    {
        pthread_cond_signal(&start_game_condition);
    }
    pthread_mutex_unlock(&warm);
}

// Refree Functions
void adjustEquipment(string id)
{
    string val = id + " Refree Adjusting Equipments";
    display(val);
    sleep(0.5);
}

void startGame(string id)
{
    // Starts game for the repective group when both players are ready else sleeps until players get ready
    // After starting game, decreases global pointers for ready players and referees
    // and wakes up player and referee threads sleeping at respective meetOrganizer functions
    // Allows organizer to meet with new groups again after startGame
    pthread_mutex_lock(&start_game_m);
    if (warm_up_players < 2)
    {
        pthread_cond_wait(&start_game_condition, &start_game_m);
    }
    warm_up_players = 0;
    pthread_mutex_lock(&organizer);
    string val = id + " Refree Starting Game";
    display(val);
    players_ready_count -= 2;
    referees_ready_count--;
    pthread_cond_signal(&player_cond);
    pthread_cond_signal(&player_cond);
    pthread_cond_signal(&referee_cond);

    pthread_mutex_unlock(&organizer);
    pthread_mutex_unlock(&start_game_m);
}

void *player(void *args)
{
    // Player thread
    int n_id = *(int *)args;
    string p_id = "P" + to_string(n_id);
    enterAcademy(p_id);
    playerMeetOrganizer(p_id);
    enterCourt(p_id);
    warmUp(p_id);
    pthread_exit(NULL);
}

void *refree(void *args)
{
    // Refree thread
    int n_id = *(int *)args;
    string p_id = "R" + to_string(n_id);
    enterAcademy(p_id);
    refMeetOrganizer(p_id);
    enterCourt(p_id);
    adjustEquipment(p_id);
    startGame(p_id);
    string val = p_id + " Finish game for Refree";
    display(val);
    pthread_exit(NULL);
}

int main()
{
    cout << "Enter number of Groups expected to come\n";
    int n;
    cin >> n;
    srand(time(0));
    pthread_t *pid = new pthread_t[3 * n];
    int curr_people_count = 0;
    int curr_player_count = 0;
    int curr_refree_count = 0;
    int last_ref = 0;
    // loop until curr_people_count does not form n groups from input
    while (curr_people_count < 3 * n)
    {
        curr_people_count++;
        // If more or equal referee count than 2 * total players count
        if (curr_refree_count >= (int)floor(curr_player_count / 2))
        {
            // Player thread here
            curr_player_count++;
            int *curr_val = new int;
            *curr_val = curr_player_count;
            pthread_create(&pid[curr_people_count], NULL, player, curr_val);
        }
        else
        {
            // Refree thread here
            curr_refree_count++;
            int *curr_val = new int;
            *curr_val = curr_refree_count;
            pthread_create(&pid[curr_people_count], NULL, refree, curr_val);
        }
        // New person comes with random(0,999) % 3 second delay
        // till then main thread sleeps
        int rand_val = rand() % 1000;
        sleep(rand_val % 3);
    }
    // Main thread waiting for last group to finish the match
    pthread_join(pid[curr_people_count], NULL);

    return 0;
}