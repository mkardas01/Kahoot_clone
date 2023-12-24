cmake_minimum_required(VERSION 3.10)

project(kahoot_serwer CXX)

# Ustaw standard C++
set(CMAKE_CXX_STANDARD 14)

# Dodaj flagę -Wall
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall")

# Dodaj pliki źródłowe
set(SOURCES
    serwer.cpp
    functions/data_structurs.hpp
    functions/send_data_rank.hpp
    functions/print_games.hpp
    functions/game_create.hpp
    functions/question_to_json.hpp
    functions/send_questions_or_endofgame.hpp
    functions/start_game_send_player_to_lobby.hpp
    functions/check_if_user_in_game.hpp
    functions/check_nickname_availability.hpp
    functions/player_join_rejoin_game.hpp
    functions/owner_rejon.hpp
    functions/owner_start_lobby.hpp
    functions/send_players_answer_to_owner.hpp
    functions/menage_points.hpp
    functions/handle_connection.hpp
    functions/handle_data.hpp
    functions/check_if_next_question.hpp
)

# Dodaj wykonywalny plik
add_executable(kahoot_serwer ${SOURCES})
