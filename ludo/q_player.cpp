#include "q_player.hpp"
#include <random>

q_player::q_player(int _iterations):
    pos_start_of_turn(16),
    pos_end_of_turn(16),
    dice_roll(0)
{
    srand (time(NULL));
    iterations = _iterations;
    EXPLORE_RATE_DECAY = (EXPLORE_RATE / iterations) / 3;
}

void q_player::save_qtable(std::vector<std::vector<double>> &q_table, std::string filename)
{
    std::stringstream ss;
    std::ofstream output_file(filename.c_str());
    boost::archive::text_oarchive oa(output_file);
    oa << q_table;
    // output_file << ss.str();
    // output_file.close();
}

std::vector<std::vector<double>> q_player::load_qtable(std::string filename)
{
    std::vector<std::vector<double>> q_table;
    std::ifstream input_file(filename.c_str());
    boost::archive::text_iarchive ia(input_file);
    ia >> q_table;
    input_file.close();
    return q_table;
}

std::vector<int> q_player::input_to_state()
{
    std::vector<int> states(4);
    for (int i = 0; i < 4; i++) {
        int state = 0;

        // If token is able to move
        if(pos_start_of_turn[i] != 0 && pos_start_of_turn[i] < 99) {
            state = pos_start_of_turn[i] + dice_roll + 1;
            // If token moves past goal position
            if (state > 56)
                state = 55 - (state - 55 - 2);

        // Token is in home and a 6 is rolled
        } else if (pos_start_of_turn[i] == -1 && dice_roll == 6) {
            state = 1;
        }

        // Check if there are other tokens on the new state
        if (state != 0) {
            int tokens_on_state = 0;
            for (int j = 0; j < 4; j++)
                if (pos_start_of_turn[j] == state && i != j)
                    tokens_on_state++;

            state += 58 * tokens_on_state;
        }
        states[i] = state;
    }
    return(states);
}

int q_player::get_action(std::vector<std::vector<double>> q_table,
    std::vector<int> states)
{
    int best_action = 0;
    if (EXPLORE_RATE == 0 || (double)(rand() % 1000) / 1000.0 > EXPLORE_RATE) {
        // double max_q = std::numeric_limits<double>::max();
        double max_q = 0;
        for (int i = 0; i < 4; i++) {
            // std::cout << q_table[i][states[i]] << "\t";
            if (pos_start_of_turn[i] > 55 || (pos_start_of_turn[i] == -1 && dice_roll != 6))
                continue;

            if (q_table[i][states[i]] > max_q) {
                max_q = q_table[i][states[i]];
                best_action = i;
            }
        }
        // std::cout << "Greedy";
        // std::cout << std::endl;
    // Random action
    } else {
        // std::cout << "Random";
        bool token_out_of_home = false;
        for (int i = 0; i < 4; i++) {
            if (pos_start_of_turn[i] != -1 && pos_start_of_turn[i] != 99) {
                token_out_of_home = true;
                break;
            }
        }
        while (true) {
            best_action = rand() % 4;
            if (pos_start_of_turn[best_action] < 56) {
                if (pos_start_of_turn[best_action] != -1 && token_out_of_home) {
                    break;
                } else if (!token_out_of_home) {
                    break;
                }
            }
        }
    }
    // Make sure that best_action is not moving a token in goal
    while(pos_start_of_turn[best_action] == 99) {
        best_action++;
        best_action = best_action % 4;
    }

    return best_action;
}

void q_player::get_reward(std::vector<std::vector<double>> &q_table,
    std::vector<int> pos_previous_turn, int action, int state)
{
    double reward = 0;
    bool game_won = true;
    double total_diff = 0;
    static int games_played = 0;
    static int previous_state = 0;
    static int previous_action = 0;
    static int dice = 0;
    // std::cout << "\nDice: " << dice << ":\t";
    for (int i = 0; i < 4; i++) {
        int diff = pos_end_of_turn[i] - pos_previous_turn[i];
        total_diff += diff;
        // std::cout << "  " << diff << ": ";

        // Moving a token out of home
        if (pos_previous_turn[i] == -1 && pos_end_of_turn[i] == 0) {
            reward += 0.25;
            // std::cout << "H,";
            continue;
        }

        // Winning the game
        if(pos_end_of_turn[i] != 99)
            game_won = false;

        // Getting a token knocked home
        if (pos_previous_turn[i] != -1 && pos_end_of_turn[i] == -1) {
            reward -= 0.25;
            // std::cout << "D,";
            continue;
        }

        // Moving a token into goal
        if (pos_end_of_turn[i] == 99 && diff != 0) {
            reward += 0.25;
            // std::cout << "G,";
        }

        // Check if position was at goal or home
        if (pos_end_of_turn[i] == -1 || pos_end_of_turn[i] == 99)
            continue;

        // Moving token closer to goal (scaled between 0.05 and 0.15)
        if (diff > 0 && diff < 14) {
            reward += (((0.15 - 0.05) * (diff - 1)) / (13 - 1)) + 0.05;
            // std::cout << "M,";
        }

        // Moving token onto globe with no enemies
        if ((pos_end_of_turn[i] - 8) % 13 == 0 && pos_end_of_turn[i] < 52 &&
            pos_previous_turn[i] != pos_end_of_turn[i]) {
            reward += 0.2;
            // std::cout << "P,";
        }

        // Forming a blockade
        for (int j = 0; j < 4; j++) {
            if (pos_end_of_turn[i] == pos_previous_turn[j] && i != j && action == i &&
                pos_start_of_turn[j] != -1 && pos_start_of_turn[j] != 99) {
                reward += 0.05;
                // std::cout << "b,";
                break;
            }
        }
        // Knocking oponent home
        for (int j = 4; j < 16; j++) {
            if (pos_end_of_turn[i] == pos_previous_turn[j] &&
                pos_start_of_turn[j] != -1 && pos_start_of_turn[j] != 99) {
                reward += 0.15;
                // std::cout << "K,";
                break;
            }
        }
    }

    if (game_won) {
        if (EXPLORE_RATE > 0)
            EXPLORE_RATE -= EXPLORE_RATE_DECAY;
        games_played++;
        reward = 1;
    } else {
        // Losing the game
        for (int i = 4; i < 16; i++) {
            if(pos_end_of_turn[i] == 99 && pos_start_of_turn[i] == -1) {
                reward = -1;
                if (EXPLORE_RATE > 0)
                    EXPLORE_RATE -= EXPLORE_RATE_DECAY;
                games_played++;
                // game_won = false;
                break;
            }
        }
    }

    if (EXPLORE_RATE < 0)
        EXPLORE_RATE = 0;

    // std::cout << "\treward = " << reward;

    if (reward != 0) {
    // if (total_diff != 0) {
        // Update q_table
        // q_table[action][state] = q_table[action][state] + LEARNING_RATE *
        //     (reward + DISCOUNT_FACTOR * q_table[previous_action][previous_state]
        //      - q_table[action][state]);
        q_table[previous_action][previous_state] += LEARNING_RATE *
            (reward + DISCOUNT_FACTOR * q_table[action][state]
             - q_table[previous_action][previous_state]);
    }
    // std::cout << "State: " << state << "\tPrevious State: " << previous_state << std::endl;
    // std::cout << "Action: " << action << "\tPrevious Action: " << previous_action << std::endl;
    previous_state = state;
    previous_action = action;
    dice = dice_roll;
    static bool game_saved = false;
    if (games_played == iterations - 1 && !game_saved) {
        std::string filename = "../q_table" + std::to_string(iterations);
        save_qtable(q_table, filename);
        // game_saved = true;
    }
}

int q_player::make_decision()
{
    static int state = 0;
    static int action = 0;
    static bool first_turn = true;
    static std::vector<int> pos_previous_turn(16, 0);
    static std::vector<std::vector<double>> q_table(4, std::vector<double>(232, 0));

    // Initialize q_table with random values between 0 and 1
    if (first_turn) {
        q_table = load_qtable("../q_table100000");
        // for(int i = 0; i < 4; i++) {
        //     for(int j = 0; j < 232; j++) {
        //         q_table[i][j] = (double)(rand() % 1000) / 1000.0;
        //     }
        // }
        first_turn = false;
    } else {
        // get_reward(q_table, pos_previous_turn, action, state);
    }

    std::vector<int> states = input_to_state();
    int decision = get_action(q_table, states);

    action = decision;
    state = states[decision];
    // std::cout << action << "\t" << state << std::endl;
    // if (!first_turn) {
    //     get_reward(q_table, pos_previous_turn, action, state);
    // }
    pos_previous_turn = pos_start_of_turn;

    return decision;
}

void q_player::start_turn(positions_and_dice relative)
{
    dice_roll = relative.dice;
    pos_start_of_turn = relative.pos;
    int decision = make_decision();
    emit select_piece(decision);
}

void q_player::post_game_analysis(std::vector<int> relative_pos)
{
    pos_end_of_turn = relative_pos;
    bool game_complete = true;
    for(int i = 0; i < 4; i++) {
        if(pos_end_of_turn[i] < 99) {
            game_complete = false;
            break;
        }
    }
    emit turn_complete(game_complete);
}
