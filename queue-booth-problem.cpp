/**
 * @file queue-booth-problem.cpp
 * @author Soumodipta Bose
 * @brief Multi Threading problem solving for the EVM problem
 * @version 0.1
 * @date 2021-11-18
 * 
 * @copyright Copyright (c) 2021
 * 
 */
#include <bits/stdc++.h>
#include <pthread.h>
using namespace std;
typedef struct Booth Booth;
typedef struct Evm Evm;
typedef struct Voter Voter;
vector<pthread_t> thread_info;
const int thread_limit = 100;
const int voter_status_default = -1;
const int voter_status_outside_booth = 0;
const int voter_status_registered = 1;
const int voter_status_at_slot = 2;
const int voter_status_voted = 3;
int range = 10;
int current_voter_id = 1;
/***************************************************************************************************************************/
/*                                                           UTILITY FUNCTIONS                                             */
/***************************************************************************************************************************/
void line()
{
    printf("======================================================================================================\n");
}
void print(string s)
{
    cout << s;
}
void print_ln(string s)
{
    cout << s << endl;
}
int voter_id_generator()
{
    return current_voter_id++;
}
/***************************************************************************************************************************/
/*                                                 ENTITY STRUCTURES & INITIALIZERS                                        */
/***************************************************************************************************************************/
struct Evm
{
    pthread_t thread_id;
    int id;
    /**
     * @brief Total slots that are used to parallely vote
     * 
     */
    int slots;
    /**
     * @brief True - allocated False - not allocated
     * 
     */
    int status;
    int tokens;
    int vote_tokens;
    Booth *back_ref;
    pthread_mutex_t evm_mutex;
    pthread_cond_t voter_condition;
    pthread_cond_t evm_condition;
};
struct Voter
{
    pthread_t thread_id;
    int id;
    int vote_status;
    Booth *back_ref;
    Evm *evm_ref;
};
struct Booth
{
    pthread_t thread_id;
    int id;
    int evms_number;
    int max_slots;
    int total_voters;
    int unregistered_voters;
    int casted_votes;
    int registrations;
    Evm *selected_evm;
    vector<Evm *> evms;
    vector<Voter *> voters;
    /**
     * @brief Protect the CS: casted_votes
     * 
     */
    pthread_mutex_t registration_mutex;
    pthread_mutex_t votes_mutex;
    pthread_mutex_t evm_mutex;
    pthread_cond_t voter_to_evm;
    pthread_cond_t evm_to_voter;
};
Voter *voter_init(Booth *booth)
{
    Voter *new_voter = new Voter;
    new_voter->id = voter_id_generator();
    new_voter->vote_status = false;
    new_voter->back_ref = booth;
    return new_voter;
}
Evm *evm_init(int id, Booth *booth)
{
    Evm *evm = new Evm;
    evm->id = id;
    evm->status = 0;
    evm->back_ref = booth;
    evm->vote_tokens = 0;
    pthread_mutex_init(&(evm->evm_mutex), NULL);
    pthread_cond_init(&(evm->voter_condition), NULL);
    pthread_cond_init(&(evm->evm_condition), NULL);
    return evm;
}
/**
 * @brief 
 * 
 * @param id 
 * @param voters 
 * @param number_of_evm 
 * @param slots 
 * @return Booth* 
 */
Booth *booth_init(int id, int voters, int number_of_evm, int slots)
{
    Booth *booth = new Booth;
    booth->id = id;
    booth->evms_number = number_of_evm;
    booth->unregistered_voters = voters;
    booth->total_voters = voters;
    booth->max_slots = slots;
    booth->casted_votes = 0;
    pthread_mutex_init(&(booth->evm_mutex), NULL);
    pthread_mutex_init(&(booth->votes_mutex), NULL);
    pthread_mutex_init(&(booth->registration_mutex), NULL);
    pthread_cond_init(&(booth->voter_to_evm), NULL);
    pthread_cond_init(&(booth->evm_to_voter), NULL);
    for (int i = 0; i < number_of_evm; i++)
    {
        booth->evms.push_back(evm_init(i, booth));
    }
    for (int i = 0; i < voters; i++)
    {
        booth->voters.push_back(voter_init(booth));
    }
    return booth;
}
/***************************************************************************************************************************/
/*                                                           HELPER FUNCTIONS                                              */
/***************************************************************************************************************************/

void polling_ready_evm(Booth *booth, int count, Evm *evm)
{
    pthread_mutex_lock(&(booth->evm_mutex));
    print_ln("Invoking polling_ready_evm " + to_string(evm->id));
    while (true)
    {
        pthread_mutex_lock(&(booth->registration_mutex));
        print_ln("Checking if any voters arrived EVM " + to_string(evm->id) + " registered: " + to_string(booth->registrations) + " voters not registered : " + to_string(booth->unregistered_voters));
        if (booth->registrations == 0 && booth->unregistered_voters == 0)
        {
            pthread_mutex_unlock(&(booth->registration_mutex));
            pthread_mutex_unlock(&(booth->evm_mutex));
            break;
        }
        else if (booth->registrations >= count || (booth->unregistered_voters == 0 && booth->registrations < count))
        {
            print_ln("Assigning voters using tokens EVM " + to_string(evm->id));
            if (count > booth->registrations && booth->unregistered_voters == 0)
            {
                count = booth->registrations;
                evm->slots = booth->registrations;
                evm->tokens = booth->registrations;
            }
            evm->status = 1;
            booth->registrations -= count;
            booth->selected_evm = evm;
            pthread_mutex_lock(&(evm->evm_mutex));
            pthread_cond_broadcast(&(booth->voter_to_evm));                         // tell voter_wait_for_evm to add voters to slots
            pthread_cond_wait(&(evm->evm_condition), &(booth->registration_mutex)); //wait for voter threads to reach slot
            booth->selected_evm = NULL;
            pthread_mutex_unlock(&(booth->registration_mutex));
            print_ln("Slots full all voters assigned EVM " + to_string(evm->id));
            pthread_mutex_unlock(&(booth->evm_mutex));
            pthread_mutex_unlock(&(evm->evm_mutex));
            break;
        }
        else
        {
            print_ln("Sleeping EVM  waiting for voters EVM " + to_string(evm->id));
            pthread_cond_wait(&(booth->evm_to_voter), &(booth->registration_mutex));
            pthread_mutex_unlock(&(booth->registration_mutex));
            print_ln("Woken from sleep EVM " + to_string(evm->id));
        }
    }
}
/**
 * @brief Invoked by voter get a slot assigned else wait
 * 
 * @param booth 
 */
void voter_wait_for_evm(Booth *booth, Voter *voter)
{
    pthread_mutex_lock(&(booth->registration_mutex));
    print_ln("Registering Voter: " + to_string(voter->id));
    voter->vote_status = voter_status_outside_booth;
    booth->unregistered_voters--;
    booth->registrations++;
    while (true)
    {
        if (booth->selected_evm)
        {
            print_ln("Trying to acquire token Voter : " + to_string(voter->id));
            if (booth->selected_evm->tokens != 0)
            {
                booth->selected_evm->tokens -= 1;
                voter->evm_ref = booth->selected_evm;
                print_ln("Token Acquired Voter :" + to_string(voter->id) + " in EVM : " + to_string(booth->selected_evm->id));
                if (booth->selected_evm->tokens == 0)
                    pthread_cond_signal(&(booth->selected_evm->evm_condition));
                print_ln("Tokens left :" + to_string(booth->selected_evm->tokens));
                pthread_mutex_unlock(&(booth->registration_mutex));
                break;
            }
            else
            {
                print_ln("Could not acquire token Voter " + to_string(voter->id));
            }
        }
        // wake a single evm and ask it to see if sufficient no. of voters have arrived
        print_ln("Calling a random EVM from Voter : " + to_string(voter->id));
        pthread_cond_signal(&booth->evm_to_voter);
        //wait for evm to say that it is accepting slots
        print_ln("Sleeping Voter : " + to_string(voter->id));
        pthread_cond_wait(&(booth->voter_to_evm), &(booth->registration_mutex));
        print_ln("Awoken Voter : " + to_string(voter->id));
        pthread_mutex_unlock(&(booth->registration_mutex));
        pthread_mutex_lock(&(booth->registration_mutex));
    }
}
void voter_in_slot(Booth *booth, Voter *voter) // this is where voting will happen
{
    Evm *evm = voter->evm_ref;
    print_ln("Reached EVM: " + to_string(evm->id) + "  Voter : " + to_string(voter->id));
    pthread_mutex_lock(&(evm->evm_mutex));
    evm->vote_tokens++;
    pthread_cond_signal(&evm->evm_condition);
    pthread_cond_wait(&(evm->voter_condition), &(evm->evm_mutex));
    print_ln("Awoken by EVM: " + to_string(evm->id) + "  Voter : " + to_string(voter->id));
    evm->vote_tokens--;
    pthread_mutex_lock(&(booth->votes_mutex));
    booth->casted_votes++;
    pthread_mutex_unlock(&(booth->votes_mutex));
    pthread_cond_signal(&evm->evm_condition);
    print_ln("Voter: " + to_string(voter->id) + " has cast vote in EVM: " + to_string(evm->id));
    pthread_mutex_unlock(&(evm->evm_mutex));
}
/***************************************************************************************************************************/
/*                                                                THREADS                                                  */
/***************************************************************************************************************************/

/**
 * @brief Use Helper functions to get assigned a slot to cast vote
 * 
 * @param arg 
 * @return void* 
 */
void *voter_thread(void *arg)
{
    Voter *voter = (Voter *)arg;
    Booth *booth = voter->back_ref;
    Evm *evm = voter->evm_ref;
    voter_wait_for_evm(booth, voter);
    voter_in_slot(booth, voter);
}
/**
 * @brief Use Helper Functions to allocate voters to slot
 * 
 * @param arg 
 * @return void* 
 */
void *evm_thread(void *arg)
{
    Evm *evm = (Evm *)arg;
    Booth *booth = evm->back_ref;
    print_ln("EVM thread started " + to_string(evm->id));
    while (true)
    {
        evm->status = 0;
        evm->slots = (rand() % min(range, booth->max_slots)) + 1; //number of slots for current cycle
        evm->tokens = evm->slots;
        evm->vote_tokens = 0;
        print_ln("EVM: " + to_string(evm->id) + " slots : " + to_string(evm->slots));
        polling_ready_evm(booth, evm->slots, evm);
        pthread_mutex_lock(&(booth->votes_mutex));
        if (booth->total_voters == booth->casted_votes || !evm->status)
        {
            print_ln("EVM: " + to_string(evm->id) + " is shutting down");
            pthread_mutex_unlock(&(booth->votes_mutex));
            break;
        }
        pthread_mutex_unlock(&(booth->votes_mutex));
        //Voters enter slots and perform voting
        while (true)
        {
            pthread_mutex_lock(&(evm->evm_mutex));
            if (evm->vote_tokens == evm->slots)
            {
                pthread_cond_broadcast(&(evm->voter_condition));
                print_ln("Waking all voters to cast their votes EVM: " + to_string(evm->id));
                pthread_mutex_unlock(&(evm->evm_mutex));
                break;
            }
            print_ln("Sleeping and waiting for voters to get into slots EVM: " + to_string(evm->id));
            pthread_cond_wait(&(evm->evm_condition), &(evm->evm_mutex));
            print_ln("Woken by voter to check if all voters arrived EVM: " + to_string(evm->id));
            pthread_mutex_unlock(&(evm->evm_mutex));
        }

        while (true)
        {
            pthread_mutex_lock(&(evm->evm_mutex));
            if (evm->vote_tokens == 0)
            {
                pthread_cond_broadcast(&(evm->voter_condition));
                pthread_mutex_unlock(&(evm->evm_mutex));
                break;
            }
            pthread_cond_wait(&(evm->evm_condition), &(evm->evm_mutex));
            pthread_mutex_unlock(&(evm->evm_mutex));
        }
        print_ln("Completed cycle EVM: " + to_string(evm->id));
        line();
    }
}

/**
 * @brief Create Evm threads and Voter threads for each Booth and wait for them to finish
 * 
 * @param arg 
 * @return void* 
 */
void *booth_thread(void *arg)
{
    Booth *booth = (Booth *)arg;
    for (int i = 0; i < booth->evms_number; i++)
    {
        pthread_create(&(booth->evms[i]->thread_id), NULL, evm_thread, booth->evms[i]);
    }
    for (int i = 0; i < booth->total_voters; i++)
    {
        pthread_create(&(booth->voters[i]->thread_id), NULL, voter_thread, booth->voters[i]);
    }
    for (int i = 0; i < booth->evms_number; i++)
    {
        pthread_join(booth->evms[i]->thread_id, 0);
    }
    for (int i = 0; i < booth->evms_number; i++)
    {
        pthread_join(booth->evms[i]->thread_id, 0);
    }
    print_ln("Booth " + to_string(booth->id) + " has completed voting");
}
void driver()
{
    // First input all the necessary information
    int booths;
    print_ln("Booting System...");
    line();
    print_ln("Enter number of Booths ?");
    cin >> booths;
    vector<int> evms(booths);
    vector<int> slots(booths);
    vector<int> voters(booths);
    for (int i = 0; i < booths; i++)
    {
        print_ln("Booth:" + to_string(i + 1));
        print_ln("Enter number of EVMS ?");
        cin >> evms[i];
        print_ln("Enter number of slots per EVM ?");
        cin >> slots[i];
        print_ln("Enter voters queued to this booth ?");
        cin >> voters[i];
        line();
    }
    //create booth threads
    vector<Booth *> booth_list;
    for (int i = 0; i < booths; i++)
    {
        booth_list.push_back(booth_init(i, voters[i], evms[i], slots[i]));
    }
    for (int i = 0; i < booths; i++)
    {
        print_ln("Starting Booth thread:" + to_string(i));
        pthread_create(&(booth_list[i]->thread_id), NULL, booth_thread, booth_list[i]);
    }
    for (int i = 0; i < booths; i++)
    {
        pthread_join(booth_list[i]->thread_id, NULL);
    }
    line();
    print_ln("Voting has concluded");
}
int main()
{
    srand((unsigned)time(NULL));
    driver();
    return 0;
}