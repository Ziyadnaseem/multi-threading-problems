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
pthread_mutex_t print_mutex;
typedef struct Booth Booth;
typedef struct Evm Evm;
typedef struct Voter Voter;
vector<pthread_t> thread_info;
const int thread_limit = 100;
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
    pthread_mutex_lock(&print_mutex);
    cout << s << endl;
    pthread_mutex_unlock(&print_mutex);
}
int voter_id_generator()
{
    return current_voter_id++;
}
string entity_quote(string entity, int id)
{
    string x = "< " + entity + " : " + to_string(id) + " >";
    return x;
}
string evm_id(int id)
{
    return entity_quote("Evm", id);
}
string voter_id(int id)
{
    return entity_quote("Voter", id);
}
string booth_id(int id)
{
    return entity_quote("Booth", id);
}
void highlight_red_ln(string message)
{
    cout << "\033[31m";
    print_ln(message);
    cout << "\033[0m";
}
void highlight_green_ln(string message)
{
    cout << "\033[32m";
    print_ln(message);
    cout << "\033[0m";
}
void highlight_blue_ln(string message)
{
    cout << "\033[34m";
    print_ln(message);
    cout << "\033[0m";
}
void highlight_cyan_ln(string message)
{
    cout << "\033[36m";
    print_ln(message);
    cout << "\033[0m";
}
void highlight_yellow_ln(string message)
{
    cout << "\033[33m";
    print_ln(message);
    cout << "\033[0m";
}
void highlight_purple_ln(string message)
{
    cout << "\033[35m";
    print_ln(message);
    cout << "\033[0m";
}
/***************************************************************************************************************************/
/*                                                 ENTITY STRUCTURES & INITIALIZERS                                        */
/***************************************************************************************************************************/

/**
 * @brief EVM entity
 * 
 */
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
/**
 * @brief Voter Entity
 * 
 */
struct Voter
{
    pthread_t thread_id;
    int id;
    bool vote_status;
    Booth *back_ref;
    Evm *evm_ref;
};
/**
 * @brief Booth Entity
 * 
 */
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
/**
 * @brief Initializes the Voter
 * 
 * @param booth 
 * @return Voter* 
 */
Voter *voter_init(Booth *booth)
{
    Voter *new_voter = new Voter;
    new_voter->id = voter_id_generator();
    new_voter->vote_status = false;
    new_voter->back_ref = booth;
    return new_voter;
}
/**
 * @brief Initializes the EVM
 * 
 * @param id 
 * @param booth 
 * @return Evm* 
 */
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
 * @brief Iniialize the booth with all the voters and Evm's
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

/**
 * @brief EVM calls ths function to get its free slots allocated by voters, Whichever Evm calls first gets allocated first
 * 
 * @param booth 
 * @param count 
 * @param evm 
 */
void polling_ready_evm(Booth *booth, int count, Evm *evm)
{
    int evmid = evm->id;
    highlight_green_ln(evm_id(evmid) + " called polling_ready_evm() for slot allocation");
    pthread_mutex_lock(&(booth->evm_mutex));
    while (true)
    {
        highlight_green_ln(evm_id(evmid) + " will try to fill slots now");
        pthread_mutex_lock(&(booth->registration_mutex));
        highlight_green_ln(evm_id(evmid) + " Checking if any voters arrived [ Registered voters: " + to_string(booth->registrations) + " ] [ Voters not registered : " + to_string(booth->unregistered_voters) + " ]");
        if (booth->registrations == 0 && booth->unregistered_voters == 0)
        {
            highlight_green_ln(evm_id(evmid) + " All voters already assigned slots. Returning...");
            pthread_mutex_unlock(&(booth->registration_mutex));
            pthread_mutex_unlock(&(booth->evm_mutex));
            break;
        }
        else if (booth->registrations >= count || (booth->unregistered_voters == 0 && booth->registrations < count))
        {
            if (count > booth->registrations && booth->unregistered_voters == 0)
            {
                count = booth->registrations;
                evm->slots = booth->registrations;
                evm->tokens = booth->registrations;
            }
            evm->status = 1;
            booth->registrations -= count;
            booth->selected_evm = evm;
            highlight_green_ln(evm_id(evmid) + " Sufficient voters arrived... waking all voters and releasing tokens: " + to_string(evm->tokens));
            pthread_mutex_lock(&(evm->evm_mutex));
            pthread_cond_broadcast(&(booth->voter_to_evm));                         // tell voter_wait_for_evm to add voters to slots
            pthread_cond_wait(&(evm->evm_condition), &(booth->registration_mutex)); //wait for voter threads to reach slot
            highlight_green_ln(evm_id(evmid) + " All slots have been allocated. Returning...");
            booth->selected_evm = NULL;
            pthread_mutex_unlock(&(booth->registration_mutex));
            pthread_mutex_unlock(&(booth->evm_mutex));
            pthread_mutex_unlock(&(evm->evm_mutex));
            break;
        }
        else
        {
            highlight_green_ln(evm_id(evmid) + " Sleeping and waiting for sufficient voters to arrive");
            pthread_cond_wait(&(booth->evm_to_voter), &(booth->registration_mutex));
            pthread_mutex_unlock(&(booth->registration_mutex));
            highlight_green_ln(evm_id(evmid) + " Woken from sleep to check voters at booth");
        }
    }
}
/**
 * @brief Voter calls this function to get allocated a slot in an EVM to vote
 * 
 * @param booth 
 */
void voter_wait_for_evm(Booth *booth, Voter *voter)
{
    int voterid = voter->id;
    pthread_mutex_lock(&(booth->registration_mutex));
    highlight_yellow_ln(voter_id(voterid) + " arrived at voter_wait_for_evm() for slot allocation");
    booth->unregistered_voters--;
    booth->registrations++;
    while (true)
    {
        if (booth->selected_evm)
        {
            if (booth->selected_evm->tokens != 0)
            {
                booth->selected_evm->tokens -= 1;
                voter->evm_ref = booth->selected_evm;
                highlight_yellow_ln(voter_id(voterid) + " Token Acquired at " + evm_id(booth->selected_evm->id) + " moving towards evm...");
                if (booth->selected_evm->tokens == 0)
                    pthread_cond_signal(&(booth->selected_evm->evm_condition));
                highlight_purple_ln("Tokens left: " + to_string(booth->selected_evm->tokens) + " at " + evm_id(booth->selected_evm->id));
                pthread_mutex_unlock(&(booth->registration_mutex));
                break;
            }
            else
            {
                highlight_yellow_ln(voter_id(voterid) + " could not acquire token");
            }
        }
        // wake a single evm and ask it to see if sufficient no. of voters have arrived
        highlight_yellow_ln(voter_id(voterid) + " waking an evm to check voter arrival");
        pthread_cond_signal(&booth->evm_to_voter);
        //wait for evm to say that it is accepting slots
        highlight_yellow_ln(voter_id(voterid) + " Sleeping and waiting for evm to call");
        pthread_cond_wait(&(booth->voter_to_evm), &(booth->registration_mutex));
        highlight_yellow_ln(voter_id(voterid) + " Woken up by Evm and trying to acquire token");
        pthread_mutex_unlock(&(booth->registration_mutex));
        pthread_mutex_lock(&(booth->registration_mutex));
    }
}
/**
 * @brief Voter calls this function to cast its vote
 * 
 * @param booth 
 * @param voter 
 */
void voter_in_slot(Booth *booth, Voter *voter) // this is where voting will happen
{
    Evm *evm = voter->evm_ref;
    highlight_yellow_ln(voter_id(voter->id) + " has reached " + evm_id(evm->id) + " slot");
    pthread_mutex_lock(&(evm->evm_mutex));
    evm->vote_tokens++;
    pthread_cond_signal(&evm->evm_condition);
    pthread_cond_wait(&(evm->voter_condition), &(evm->evm_mutex));
    highlight_yellow_ln(voter_id(voter->id) + " Awoken by " + evm_id(evm->id) + " to cast vote");
    evm->vote_tokens--;
    pthread_mutex_lock(&(booth->votes_mutex));
    booth->casted_votes++;
    voter->vote_status = true;
    pthread_mutex_unlock(&(booth->votes_mutex));
    pthread_cond_signal(&evm->evm_condition);
    highlight_yellow_ln(voter_id(voter->id) + " has casted its vote at " + evm_id(evm->id));
    pthread_mutex_unlock(&(evm->evm_mutex));
}
/***************************************************************************************************************************/
/*                                                                THREADS                                                  */
/***************************************************************************************************************************/

/**
 * @brief Voter thread to call the various functions parallely
 * 
 * @param arg 
 * @return void* 
 */
void *voter_thread(void *arg)
{
    Voter *voter = (Voter *)arg;
    Booth *booth = voter->back_ref;
    highlight_yellow_ln(voter_id(voter->id) + " thread created");
    voter_wait_for_evm(booth, voter);
    voter_in_slot(booth, voter);
    highlight_yellow_ln(voter_id(voter->id) + " thread destroyed");
}
/**
 * @brief Evm thread to call the various functions parallely
 * 
 * @param arg 
 * @return void* 
 */
void *evm_thread(void *arg)
{
    Evm *evm = (Evm *)arg;
    Booth *booth = evm->back_ref;
    highlight_green_ln(evm_id(evm->id) + " is starting up...");
    while (true)
    {
        evm->status = 0;
        evm->slots = (rand() % min(range, booth->max_slots)) + 1; //number of slots for current cycle
        evm->tokens = evm->slots;
        evm->vote_tokens = 0;
        highlight_green_ln(evm_id(evm->id) + " Slots free in this cycle : [ " + to_string(evm->slots) + " ]");
        polling_ready_evm(booth, evm->slots, evm);
        pthread_mutex_lock(&(booth->votes_mutex));
        if (booth->total_voters == booth->casted_votes || !evm->status)
        {
            highlight_green_ln(evm_id(evm->id) + " is shutting down...");
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
                highlight_green_ln(evm_id(evm->id) + " Waking all voters to cast their votes");
                pthread_mutex_unlock(&(evm->evm_mutex));
                break;
            }
            highlight_green_ln(evm_id(evm->id) + " Sleeping and waiting for all voters to arrive at slots");
            pthread_cond_wait(&(evm->evm_condition), &(evm->evm_mutex));
            highlight_green_ln(evm_id(evm->id) + " woken from sleep to check if all voters arrived at slots");
            pthread_mutex_unlock(&(evm->evm_mutex));
        }

        while (true)
        {
            pthread_mutex_lock(&(evm->evm_mutex));
            highlight_green_ln(evm_id(evm->id) + " votes casted: " + to_string(evm->vote_tokens));
            if (evm->vote_tokens == 0)
            {
                pthread_cond_broadcast(&(evm->voter_condition));
                pthread_mutex_unlock(&(evm->evm_mutex));
                break;
            }
            pthread_cond_wait(&(evm->evm_condition), &(evm->evm_mutex));
            pthread_mutex_unlock(&(evm->evm_mutex));
        }
        highlight_green_ln(evm_id(evm->id) + " has completed voting cycle");
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
    highlight_cyan_ln("Created new booth thread :" + booth_id(booth->id));
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
    highlight_cyan_ln(booth_id(booth->id) + " has completed voting process");
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
        print_ln("Enter maximum number of slots per EVM ?");
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