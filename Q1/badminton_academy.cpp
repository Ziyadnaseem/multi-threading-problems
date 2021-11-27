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
int refrees_ready_count = 0;
int warm_up_players = 0;
int refree_start = 0;
pthread_mutex_t organizer;
pthread_mutex_t court;
pthread_mutex_t print;
pthread_mutex_t warm;
pthread_mutex_t start_game_m;
pthread_cond_t cond1 = PTHREAD_COND_INITIALIZER;
pthread_cond_t player_cond = PTHREAD_COND_INITIALIZER;
pthread_cond_t refree_cond = PTHREAD_COND_INITIALIZER;
pthread_cond_t start_game_condition = PTHREAD_COND_INITIALIZER;
int count_critical = 0;

void display(string msg)
{
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
    pthread_mutex_lock(&organizer);
    if (refrees_ready_count >= 1)
    {
        pthread_cond_wait(&refree_cond, &organizer);
    }
    else
    {
        refrees_ready_count++;
    }
    string val = id + " Meeting Organizer";
    display(val);
    pthread_mutex_unlock(&organizer);
}

void enterCourt(string id)
{
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
    refrees_ready_count--;
    pthread_cond_signal(&player_cond);
    pthread_cond_signal(&player_cond);
    pthread_cond_signal(&refree_cond);

    pthread_mutex_unlock(&organizer);
    pthread_mutex_unlock(&start_game_m);
}

void *player(void *args)
{
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
    while (curr_people_count < 3 * n)
    {
        curr_people_count++;
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
        int rand_val = rand() % 1000;
        sleep(rand_val % 3);
    }
    pthread_join(pid[curr_people_count], NULL);

    return 0;
}