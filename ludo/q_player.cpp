#include "q_player.hpp"
#include <random>

q_player::q_player(int _iterations):
    pos_start_of_turn(16),
    pos_end_of_turn(16),
    dice_roll(0)
{
    srand (time(NULL));
    iterations = _iterations;
    this->EXPLORE_RATE_DECAY = (EXPLORE_RATE / iterations) / 3;
}

void q_player::save_qtable(std::vector<std::vector<double>> &q_table, std::string filename)
{
    std::stringstream ss;
    std::ofstream output_file(filename.c_str());
    boost::archive::text_oarchive oa(output_file);
    oa << q_table;
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
        int state = pos_start_of_turn[i] + 1;

        // If token moves past goal position
        if (state > 57)
            state = 58;

        state += 58 * i;
        states[i] = state;
    }
    return(states);
}

std::vector<int> q_player::get_actions()
{
    std::vector<int> actions;
    for (int i = 0; i < 4; i++) {
        int action = 0;
        int start_pos = pos_start_of_turn[i];
        int new_pos = start_pos + dice_roll;
        // Move out of home
        if (start_pos == -1 && dice_roll == 6) {
            action = 1;
        // Check if token is stuck in home or is in goal
        } else if (start_pos != -1 && start_pos != 99) {
            // Move into goal
            if (new_pos == 56 || new_pos == 50)
                action = 7;

            // Form blockade
            for (int j = 0; j < 4; j++) {
                if (new_pos == pos_start_of_turn[j] && i != j && pos_start_of_turn[j] != 99) {
                    action = 5;
                    break;
                }
            }

            // Move onto same field as an enemy
            bool kill = false;
            bool suicide = false;

            for (int j = 4; j < 16; j++) {
                if (new_pos == pos_start_of_turn[j] && start_pos != -1 && start_pos != 99) {
                    kill = true;
                    // Check if enemy token is on globe (Suicide)
                    if ((pos_start_of_turn[j] - 8) % 13 == 0 || (pos_start_of_turn[j] - 8) % 18 == 0) {
                        suicide = true;
                        break;
                    }
                    // Check if enemy has formed a blockade (Suicide)
                    for (int k = 0; k < 4; k++) {
                        int index = int(j/4) * k;
                        if (new_pos == pos_start_of_turn[index] && index != j) {
                            suicide = true;
                            break;
                        }
                    }
                }
            }

            // Protect token (goal stretch or globe)
            if ((new_pos > 50 && start_pos < 50) ||
                ((new_pos - 8) % 13 == 0 && new_pos < 52 && suicide == false)) {
                action = 6;
            }

            if (suicide) { // Suicide
                action = 4;
            } else if (kill && !suicide) { // Kill
                action = 3;
            }
            // Move token
            if (action == 0 && new_pos > start_pos)
                action = 2;
        }
        actions.push_back(action);
    }
    return actions;
}

int q_player::select_action(std::vector<std::vector<double>> q_table,
    std::vector<int> states, std::vector<int> possible_actions)
{
    int best_action = 0;
    if (EXPLORE_RATE == 0 || (double)(rand() % 1000) / 1000.0 > EXPLORE_RATE) {
        double max_q = -10000;
        for (int i = 0; i < 4; i++) {
            if (pos_start_of_turn[i] > 55 || (pos_start_of_turn[i] == -1 && dice_roll != 6))
                continue;

            if (q_table[possible_actions[i]][states[i]] > max_q && possible_actions[i] != 0) {
                max_q = q_table[possible_actions[i]][states[i]];
                best_action = i;
            }
        }
    // Random action
    } else {
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
    int action, int state, int decision)
{
    double total_diff = 0;
    static int previous_state = 0;
    static int previous_action = 0;
    static int games_played = 0;

    double reward = 0;

    // Move out of home
    if (previous_action == 1)
        reward += 0.25;

    // Move piece closest to home
    if (previous_action != 0 && previous_action != 1 && previous_action != 4) {
        bool closest = true;
        for (int i = 0; i < 4; i++) {
            if (pos_end_of_turn[decision] < pos_end_of_turn[i] &&
                decision != i && pos_end_of_turn[i] != 99) {
                    closest = false;
            }
        }
        if (closest && pos_end_of_turn[decision] < 51)
            reward += 0.1;
    }
    // Kill
    if (previous_action == 3)
        reward += 0.15;

    // Form Blockade
    if (previous_action == 5)
        reward += 0.05;

    // Protect token
    if (previous_action == 6)
        reward += 0.2;

    // Move into goal
    if (previous_action == 7)
        reward += 0.25;

    // Suicide
    if (previous_action == 4)
        reward -= 0.8;

    bool game_won = true;
    for (int i = 0; i < 4; i++) {
        // Winning the game
        if(pos_end_of_turn[i] != 99)
            game_won = false;

        // Getting a token knocked home
        if (pos_start_of_turn[i] == -1 && pos_end_of_turn[i] != -1)
            reward -= 0.25;
    }

    if (game_won) {
        if (EXPLORE_RATE > 0)
            EXPLORE_RATE -= EXPLORE_RATE_DECAY;
        games_played++;
        reward += 1;
    } else {
        // Losing the game
        for (int i = 4; i < 16; i++) {
            if(pos_end_of_turn[i] == 99 && pos_start_of_turn[i] == -1) {
                if (EXPLORE_RATE > 0)
                    EXPLORE_RATE -= EXPLORE_RATE_DECAY;
                games_played++;
                reward -= 1;
                break;
            }
        }
    }

    if (EXPLORE_RATE < 0)
        EXPLORE_RATE = 0;


    // Update q-table
    if (reward != 0) {
        q_table[previous_action][previous_state] += LEARNING_RATE *
            (reward + DISCOUNT_FACTOR * q_table[action][state]
             - q_table[previous_action][previous_state]);
    }
    previous_state = state;
    previous_action = action;
    static bool game_saved = false;
    if (games_played == iterations - 1 && !game_saved) {
        std::string filename = "../simple_q_table" + std::to_string(iterations);
        save_qtable(q_table, filename);
        game_saved = true;
    }
}

int q_player::make_decision()
{
    static bool first_turn = true;
    static std::vector<std::vector<double>> q_table(8, std::vector<double>(232, 0));

    if (first_turn) {
        for (int i = 0; i < 4; i++)
            pos_end_of_turn[i] = -1;

        if (!this->training) {
            q_table = load_qtable("../simple_q_table25");
        }
        first_turn = false;
    }

    std::vector<int> states = input_to_state();
    std::vector<int> possible_actions = get_actions();
    int decision = select_action(q_table, states, possible_actions);

    if (this->training) {
        get_reward(q_table, possible_actions[decision], states[decision], decision);
    }

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
